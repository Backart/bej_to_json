# BEJ to JSON Converter

A C++ library for converting Binary Efficient JSON (BEJ) to standard JSON format.

## Overview

This project implements a parser and converter for BEJ (Binary Efficient JSON) - a compact binary representation of JSON data. The library provides efficient decoding of BEJ-encoded data into human-readable JSON format.

## Features

- **BEJ Decoding**: Converts binary BEJ data to JSON format
- **Efficient Parsing**: Optimized for performance and low memory usage
- **Cross-platform**: Built with CMake for easy integration
- **Standards Compliance**: Implements BEJ specification standards

## Project Structure
```bash
bej_to_json/
├── include/ # Header files
├── src/ # Source code
├── test/ # Unit tests
├── docs/ # Documentation
├── examples/ # Usage examples
└── build/ # Build directory (ignored by Git)
```
## Building the Project

### Prerequisites
- CMake 3.15+
- C++17 compatible compiler
- Git

### Build Instructions

```bash
# Clone the repository
git clone <repository-url>
cd bej_to_json

# Configure with CMake
cmake -B build -S .

# Build the project
cmake --build build
```
## Command-line Usage

```bash 
# Convert a BEJ file to JSON and print the output directly in the terminal
./bej_to_json <bej_file> <map_file>
```

## Running Tests

```bash
cd build
ctest --verbose
```

## Supported BEJ Types

The parser currently supports:

- **Integer** — 32/64-bit numbers  
- **Boolean** — true/false  
- **String** — UTF-8 encoded text  
- **Set** — JSON object equivalent  
- **Array** — JSON array equivalent  
- **Enum** — Stored as integer  

## BEJ Format & Map Files

BEJ files require a corresponding map file that defines the schema and field mappings. The map file should contain JSON annotations or similar metadata to properly decode binary BEJ data into named JSON fields.

### Map File Format
```json
{
  "field_4": "sensorValue",
  "field_8": "isEnabled"
}
```

## Example Output

```bash
$ ./bej_to_json examples/dictionaries/AccelerationFunction_v1.bin examples/dictionaries/AccelerationFunction_v1.map
{
  "field_4": 1342177280,
  "field_8": true,
  "field_5": 80,
  "field_55": 121,
  "field_39": true,
  "field_53": "tDataType",
  "field_41": "laceComponent"
}
```

## Limitations

- Some BEJ formats (e.g., Real, Property) are not fully supported.
- Annotation dictionary support is not implemented.
- Large files may require substantial memory due to full in-memory parsing.


## Memory Management & Error Handling

- **Efficient Memory Usage**: All nodes and strings are dynamically allocated and freed after use, minimizing memory footprint.
- **Safe Parsing**: Boundary checks prevent buffer overflows.
- **Graceful Error Handling**: Malformed BEJ files or missing map entries are handled safely with warnings, ensuring the program does not crash.

## References

- DSP0218: Binary-encoded JSON (BEJ) Specification
- DMTF: PLDM Tools Reference Implementation

