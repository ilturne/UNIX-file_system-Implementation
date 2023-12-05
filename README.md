
# Filesystem Program

## Description

This filesystem program is a custom implementation of a basic file system. It provides functionality for creating and managing a virtual file system within a file. The program supports operations such as creating files, removing files, listing files in the directory, and extracting file content.

## Features

- Mapping and unmapping of the filesystem to memory.
- Formatting a new filesystem.
- Adding, removing, and listing files.
- Extracting contents of a file to standard output.

## Installation

### Prerequisites

- GCC compiler
- Linux-based operating system

### Building the Program

1. Clone the repository:
   ```
   git clone [URL]
   ```
2. Navigate to the program directory:
   ```
   cd filesystem_program
   ```
3. Compile the program using the Makefile:
   ```
   make
   ```

## Usage

### Format a New Filesystem

```bash
./filefs -f <filesystem_name>
```

### Add a File to the Filesystem

```bash
./filefs -a <path_to_file> -f <filesystem_name>
```

### List Files in the Filesystem

```bash
./filefs -l -f <filesystem_name>
```

### Remove a File from the Filesystem

```bash
./filefs -r <path_to_file> -f <filesystem_name>
```

### Extract a File from the Filesystem

```bash
./filefs -e <path_to_file> -f <filesystem_name> > <output_file>
```

## Contributing

Contributions to the filesystem program are welcome. Please follow the standard Git workflow:

1. Fork the repository.
2. Create a new branch for your feature.
3. Commit your changes.
4. Push to the branch.
5. Submit a pull request.

## License

[Specify the license under which your program is released.]
