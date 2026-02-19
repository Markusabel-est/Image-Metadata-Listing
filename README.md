# Image-Metadata-Listing
A POSIX-style C++ console utility to search and filter JPEG images by filename, capture date, and camera model using EXIF metadata.
--

## Features

- Recursively searches a directory and all its subdirectories for JPEG files
- Filters by filename with case-insensitive exact or wildcard (`*`) matching
- Filters by exact capture date (`YYYY:MM:DD`)
- Filters by camera model with case-insensitive exact or wildcard (`*`) matching
- Output supports piping (POSIX-compliant)
- Basic error handling for missing directories, invalid paths, and missing EXIF data

---

## Requirements

- C++17 or later
- [libexif](https://libexif.github.io/) — for reading EXIF metadata from JPEG files

### Installing libexif

**macOS (Homebrew):**
```bash
brew install libexif
```

**Ubuntu/Debian:**
```bash
sudo apt-get install libexif-dev
```

---

## Building

Compile with `g++`:

```bash
g++ -std=c++17 -o imgmeta main.cpp $(pkg-config --cflags --libs libexif)
```

---

## Usage

```
imgmeta DIRECTORY [OPTIONS]
```

### Arguments

| Argument | Description |
|---|---|
| `DIRECTORY` | Path to the directory to search (defaults to current directory `.` if omitted with flags) |

### Options

| Option | Long form | Description |
|---|---|---|
| `-n` | `--name` | Filter by filename (case-insensitive, supports `*` wildcard) |
| `-d` | `--date` | Filter by exact capture date in `YYYY:MM:DD` format |
| `-c` | `--camera` | Filter by camera model (case-insensitive, supports `*` wildcard) |
| `-h` | `--help` | Display help message |

---

## Examples

**Search current directory for all JPEGs:**
```bash
./imgmeta .
```

**Search a specific directory:**
```bash
./imgmeta /Users/markus/Photos
```

**Filter by filename (exact match):**
```bash
./imgmeta . -n photo.jpg
```

**Filter by filename (wildcard):**
```bash
./imgmeta . -n "img_*"
```

**Filter by capture date:**
```bash
./imgmeta . -d 2024:06:15
```

**Filter by camera model (wildcard):**
```bash
./imgmeta . -c "Canon*"
```

**Combine multiple filters:**
```bash
./imgmeta /Users/markus/Photos -n "img_*" -d 2024:06:15 -c "Canon*"
```

**Pipe output to another command:**
```bash
./imgmeta . -c "Sony*" | grep "2024"
```

---

## Output Format

Results are printed in a formatted table inspired by `ls -l`:

```
MATCHES   FILENAME                  CAMERA_MODEL             CAPTURE_DATE
ndc       img_0042.jpg              Canon EOS R5             2024:06:15
n--       vacation_photo.jpg        Sony ILCE-7M3            2023:08:22
```

The `MATCHES` column shows which filters were matched:
- `n` — matched by name
- `d` — matched by date
- `c` — matched by camera model
- `-` — filter was not applied

---

## Error Handling

| Error | Message |
|---|---|
| Directory does not exist | `Error: Directory '<path>' does not exist` |
| Path is not a directory | `Error: '<path>' is not a directory` |
| No matching files found | `No files found` |
| Unknown option | `Unknown option: <flag>` followed by usage message |

---

## Project Structure

```
.
├── main.cpp       # Main source file
└── README.md      # This file
```

---

## Versioning

This project follows [Semantic Versioning](https://semver.org/).

Current version: `1.0.0`

---
