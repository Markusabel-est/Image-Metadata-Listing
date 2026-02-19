#include <complex>


extern "C" {
    #include <libexif/exif-data.h>
    #include <libexif/exif-tag.h>
}
#include <string>
#include <iostream>
#include <iomanip>
#include <filesystem>


using namespace std;
namespace fs = std::filesystem;


struct imageMetadata {
    string captureDate;
    string cameraModel;
};


struct fileMatch {
    string filename;
    imageMetadata metadata;

    bool matchedByName{};
    bool matchedByDate{};
    bool matchedByModel{};
};

bool endsWith(const string& str, const string& suffix) {
    //Preconditon:  str and suffix must not be empty
    //Postcondtion: returns true if str ends with suffix, false otherwise

    if (str.length() >= suffix.length()) {
        return (0 == str.compare(str.length() -suffix.length(), suffix.length(), suffix));
    }
    return false;

}
string toLower(string str) {
    //Precondtion: str must not be empty
    //Postcondtion: returns a copy of str with all characters converted to lowercase
    for (char& c : str) {
        c = tolower(c);
    }
    return str;
}
bool isJpeg(string& filename) {
    //Precondtion: filename is a valid string
    //Postcondtion: returns true if filename ends with .jpg or .jpeg, false otherwise

    string lower = toLower(filename);
    return endsWith(lower, ".jpg") || endsWith(lower, ".jpeg");
}
bool isEqual(char a, char b) {
    //Precondtion: a and b are valid characters
    //Postcondtion: returns true if a and b are equal, false otherwise
    return a == b;
}

bool matchWildcard(const string& pattern, const string& str) {
    //Precondtion: pattern and str are valid strings
    //Postcondtion: returns true if str matches pattern, supporting * as a wildcard

    string strLower = toLower(str);
    string patternLower = toLower(pattern);

    if (pattern.empty()){
        return true;
    }
    if (pattern == "*"){
        return true;
    }


    size_t t = 0;
    size_t p = 0;
    size_t starIdx = string::npos;
    size_t matchIdx = 0;

    while (t < strLower.length()) {
        if (p < patternLower.length() && isEqual(patternLower[p], strLower[t]) || patternLower[p] == '*') {
            if (patternLower[p] == '*') {
                starIdx = p;
                matchIdx = t;
                p++;
            }
            p++;
            t++;
        }
        else if (starIdx != string::npos) {
            p = starIdx +1;
            ++matchIdx;
            t = matchIdx;
        }
        else {
            return false;
        }
    }

    while (p < patternLower.length() && patternLower[p] =='*') {
        p++;
    }
    return p == patternLower.length();
}
string exifTagGetValue(ExifData* exifData, ExifIfd ifd, ExifTag tag) {
    //Precondtion: exifData may be nullptr, but should be valid if not
    //Postcondtion: returns the value of the tag in the exifData, or an empty string if the tag is not found
    if (!exifData) {
        return "";
    }


    ExifEntry* entry = exif_content_get_entry(exifData->ifd[ifd], tag);
    if (!entry) {
        return "";
    }

    char buf[1024];
    if (!exif_entry_get_value(entry,buf, sizeof(buf))) {
        return "";
    }
    return string(buf);
}


string trimTimeFromDate(const string& fullDateTime) {
    //Precondtion: fullDateTime is a valid EXIF datetime string or empty
    //Postcondtion: returns the date portion (YYYY:MM:DD) of the datetime string, or the full string if the date portion is not found


    if (fullDateTime.length() >= 10) {
        return fullDateTime.substr(0, 10);
    }
    return fullDateTime;
}

bool matchMetadata(const string& filepath, imageMetadata& metadata) {
    //Precondtion: filepath is a valid path to a JPEG file
    //Postcondtion: returns true if the metadata in the file matches the criteria, false otherwise


    ExifData* exifData = exif_data_new_from_file(filepath.c_str());
    if (!exifData) {
        return false;
    }

    metadata.captureDate = trimTimeFromDate(exifTagGetValue(exifData, EXIF_IFD_0, EXIF_TAG_DATE_TIME));
    metadata.cameraModel = exifTagGetValue(exifData, EXIF_IFD_0, EXIF_TAG_MODEL);

    exif_data_unref(exifData);
    return true;
}

vector<fileMatch> findFile(const string& dir, const string& dateMatch, const string& modelMatch, const string& nameMatch ) {
    //Precondtion: dir is a valid directory path
    //Postcondtion: returns a vector of fileMatch objects that match the criteria, or an empty vector if no files are found

    vector<fileMatch> files;
    try {
        for (const auto& entry : fs::recursive_directory_iterator(dir)) {
            if (!entry.is_regular_file()) continue;
            string filename = toLower(entry.path().filename().string());

            if (!isJpeg(filename)) continue;

            if (!nameMatch.empty() && !matchWildcard(nameMatch, filename)) continue;

            imageMetadata metadata;


            if (!matchMetadata(entry.path().string(), metadata))continue;


            bool meetsCriteria = true;
            if (!dateMatch.empty()) {
                meetsCriteria = meetsCriteria && (dateMatch == metadata.captureDate);
            }

            if (!modelMatch.empty()) {
                meetsCriteria = meetsCriteria && (matchWildcard(modelMatch, metadata.cameraModel));
            }

            if (meetsCriteria) {
                fileMatch file;
                file.filename = filename;
                file.metadata = metadata;
                file.matchedByDate = !dateMatch.empty() ;
                file.matchedByModel = !modelMatch.empty() ;
                file.matchedByName =  !nameMatch.empty();
                files.push_back(file);
            }

        }
    } catch (const fs::filesystem_error& e) {
        cerr << "Error: " <<e.what() << endl;

    }
    return files;
}


void printUsage(const char* programName) {
    //Precondtion: programName is a valid string
    //Postcondtion: prints the usage message to stdout and exits the program

    cerr << "Usage: " << programName << " DIRECTORY [OPTIONS]\n"
    << "Options:\n"
    << "  -n, --name   Match filename (case-insensitive, supports * wildcard)\n"
    << "  -d, --date   Match exact capture date (YYYY:MM:DD)\n"
    << "  -c, --camera Match camera model (case-insensitive, supports * wildcard)\n"
    << "  -h, --help   Display this help message\n"
    << "\nOutput format:\n"
    << "  MATCHES       FILENAME           CAMERA_MODEL         CAPTURE_DATE\n";
}

int main(int argc, char** argv) {
    //Precondtion: argc >= 1 and argv is a valid array of C-strings
    //Postcondtion: Parses commandline arguments and executes image matching logic. Prints matched image info in a formatted table if matches are found
    //Returns 0 on success, 1 on error or if no files matched.

    string nameMatch;
    string dateMatch;
    string modelMatch;

    if (argc < 2)  {
        printUsage(argv[0]);
        return 1;
    }

    int i  = 1;
    string directory = argv[1];
    if (directory== "-h" || directory == "--help" || directory == "-n" || directory == "--name" || directory == "-d" || directory == "--date" || directory == "-c" || directory == "--camera") {
         directory = ".";
    }
    else {
        i = 2;
        if (!fs::exists(directory)) {
            cerr << "Error: Directory '" << directory << "' does not exist\n";
            return 1;
        }
        if (!fs::is_directory(directory)) {
            cerr << "Error: '" << directory << "' is not a directory\n";
            return 1;
        }
    }

   for (; i < argc; i++) {
       string arg = argv[i];
       if (arg == "-h" || arg == "--help") {
           printUsage(argv[0]);
           return 0;
       }
       if (arg == "-n" || arg == "--name") {
           if (i + 1 <argc) {
                nameMatch = argv[++i];
           }
       }
       else if (arg == "-d" || arg == "--date") {
           if (i + 1 <argc) {
               dateMatch = argv[++i];
           }
       }
       else if (arg == "-c" || arg == "--camera") {
           if (i + 1 <argc) {
               modelMatch = argv[++i];
           }
       }
       else {
           cerr << "Unknown option: " << arg << endl;
           printUsage(argv[0]);
           return 1;
       }

   }

    vector<fileMatch> result = findFile(directory, dateMatch, modelMatch, nameMatch);
    if (result.empty()) {
        cerr << "No files found" << endl;
        return 1;
    }

    for (const auto& match : result) {
        string matchedBy = "---";
        if (match.matchedByName)matchedBy[0] = 'n';
        if (match.matchedByDate)matchedBy[1] = 'd';
        if (match.matchedByModel)matchedBy[2] = 'c';


        cout<< left
        << setw(8) << matchedBy << " " << setw(25) << match.filename << " "
        << setw(23) << match.metadata.cameraModel << " "
        << setw(20)<< match.metadata.captureDate << endl;

    }


    return 0;



}
