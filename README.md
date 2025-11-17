# C to British English Compiler (c2en)

A compiler that translates C89/99 source code into clear, readable British English prose. Perfect for understanding code logic, documentation, or teaching programming concepts to non-programmers.

## Overview

The C to British English Compiler (`c2en`) takes standard C programs and produces natural language descriptions that accurately explain what the code does. The output is written in proper British English with correct grammar, making programming logic accessible to anyone, regardless of technical background.

### Features

- **Full C89/99 Support**: Handles variables, functions, control flow, arrays, and more
- **Natural Language Output**: Produces grammatically correct, readable British English
- **Cross-Platform**: Builds on Linux, macOS, and Windows
- **Comprehensive Analysis**: Includes lexical, syntax, and semantic analysis
- **Educational Tool**: Ideal for teaching, documentation, and code understanding
- **Proper British Spelling**: Uses "programme," "whilst," "initialised," etc.

## Example

**Input C Code** (`hello.c`):
```c
#include <stdio.h>

int main() {
    int x = 42;
    printf("Hello, World!\n");
    return 0;
}
```

**Output British English** (`hello.txt`):
```
Programme Description
=====================

This programme consists of one function.

Function: main
--------------
This function accepts no parameters and returns a value of type int.

This is the main entry point of the programme.

The function performs the following steps:

  1. Declare a variable named 'x' of type int, initialised to the value 42.

  2. Display the message "Hello, World!\n".

  3. Return the value 0.
```

## Quick Start

### Linux / macOS

```bash
# Clone the repository
git clone https://github.com/DannyBimma/gb-en-compiler.git
cd gb-en-compiler

# Build the compiler
make

# Or use the build script
./build.sh

# Run an example
./c2en examples/hello.c

# View the output
cat examples/hello.txt
```

### Windows

```cmd
REM Clone the repository
git clone https://github.com/DannyBimma/gb-en-compiler.git
cd gb-en-compiler

REM Build with the build script (requires CMake)
build.bat

REM Or build with CMake manually
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
cd ..

REM Run an example
c2en.exe examples\hello.c

REM View the output
type examples\hello.txt
```

## Installation

### Prerequisites

- **C Compiler**: GCC, Clang, or MSVC
- **Make** (Linux/macOS) or **CMake** (cross-platform)
- **Git** (for cloning the repository)

### Building from Source

#### Option 1: Using Make (Linux/macOS)

```bash
make clean
make
```

#### Option 2: Using CMake (All Platforms)

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

#### Option 3: Build Scripts

**Linux/macOS:**
```bash
chmod +x build.sh
./build.sh
```

**Windows:**
```cmd
build.bat
```

### System-Wide Installation (Linux/macOS)

```bash
sudo make install
```

This installs `c2en` to `/usr/local/bin`, making it available system-wide.

To uninstall:
```bash
sudo make uninstall
```

## Usage

### Basic Usage

```bash
c2en <input.c>
```

This compiles `input.c` and generates `input.txt` with the English translation.

### Specify Output File

```bash
c2en input.c -o output.txt
```

### Command-Line Options

- `-o <file>` - Specify output file (default: input filename with `.txt` extension)
- `-v` - Verbose mode (show compilation stages)
- `--show-tokens` - Display tokenization result for debugging
- `--show-ast` - Display abstract syntax tree for debugging
- `--help` - Display help message
- `--version` - Display compiler version

### Examples

```bash
# Compile with verbose output
./c2en examples/factorial.c -v

# Show compilation stages
./c2en examples/calculator.c --show-tokens --show-ast

# Specify custom output file
./c2en my_program.c -o explanation.txt
```

## Supported C Language Features

### Data Types
- `int`, `char`, `float`, `double`, `void`
- Arrays (single-dimensional)

### Control Flow
- `if`, `else if`, `else` statements
- `while` loops
- `for` loops
- `do-while` loops (partial)
- `break` and `continue`

### Operations
- Arithmetic: `+`, `-`, `*`, `/`, `%`
- Comparison: `==`, `!=`, `<`, `>`, `<=`, `>=`
- Logical: `&&`, `||`, `!`
- Assignment: `=`
- Increment/Decrement: `++`, `--`

### Functions
- Function declarations and definitions
- Parameters and return values
- Function calls
- Recursion

### Other Features
- Variables and constants
- Expressions (with proper precedence)
- Comments (single-line and multi-line)
- Preprocessor directives (ignored during translation)

## Project Structure

```
gb-en-compiler/
├── src/                    # Source code
│   ├── main.c             # Entry point and CLI
│   ├── lexer.c/h          # Lexical analyzer (tokenization)
│   ├── parser.c/h         # Syntax analyzer (AST construction)
│   ├── ast.c/h            # Abstract Syntax Tree structures
│   ├── semantic.c/h       # Semantic analyzer
│   ├── symbol_table.c/h   # Symbol table management
│   ├── translator.c/h     # C to English translation
│   ├── formatter.c/h      # Output formatting
│   └── utils.c/h          # Utility functions
├── examples/              # Example C programs
│   ├── hello.c
│   ├── factorial.c
│   ├── calculator.c
│   ├── loop.c
│   └── conditional.c
├── build/                 # Build artifacts (generated)
├── Makefile              # Build system for Linux/macOS
├── CMakeLists.txt        # Cross-platform build configuration
├── build.sh              # Build script for Unix systems
├── build.bat             # Build script for Windows
├── BUILD_PLAN.md         # Detailed development plan
├── LICENSE               # License information
└── README.md             # This file
```

## How It Works

The compiler follows a traditional compilation pipeline:

1. **Lexical Analysis**: Source code is tokenized into meaningful symbols
2. **Syntax Analysis**: Tokens are parsed into an Abstract Syntax Tree (AST)
3. **Semantic Analysis**: Type checking and symbol table construction
4. **Translation**: AST is traversed and converted to English templates
5. **Formatting**: Output is formatted with proper British English conventions

### Translation Examples

#### Variables
```c
int x = 42;
```
→ *"Declare a variable named 'x' of type int, initialised to the value 42."*

#### Expressions
```c
result = a + b * c;
```
→ *"Set 'result' to the sum of 'a' and the product of 'b' and 'c'."*

#### Control Flow
```c
if (age >= 18) {
    printf("Adult\n");
}
```
→ *"If the condition 'age' is greater than or equal to the value 18 is true, then: Display the message 'Adult\n'."*

#### Loops
```c
for (int i = 0; i < 10; i++) {
    total = total + i;
}
```
→ *"Beginning with declaring a counter variable 'i' initialised to 0, and continuing whilst 'i' is less than the value 10, repeatedly perform the following operations, incrementing 'i' by 1 after each iteration: Set 'total' to the sum of 'total' and 'i'."*

## British English Conventions

The compiler uses proper British English spelling and phrasing:

- **programme** (not "program")
- **initialised** (not "initialized")
- **whilst** (not "while" in prose)
- **colour** (not "color")
- **analyse** (not "analyze")

## Development

### Running Tests

```bash
# Build and test with example programs
make test

# Or run examples manually
./c2en examples/hello.c
./c2en examples/factorial.c
./c2en examples/calculator.c
```

### Adding New Features

The modular architecture makes it easy to extend:

1. **Lexer** (`src/lexer.c`): Add new token types
2. **Parser** (`src/parser.c`): Add new grammar rules
3. **AST** (`src/ast.c`): Add new node types
4. **Translator** (`src/translator.c`): Add translation templates

### Debugging

```bash
# Show tokens
./c2en program.c --show-tokens

# Show AST
./c2en program.c --show-ast

# Verbose output with all stages
./c2en program.c -v
```

## Platform-Specific Notes

### Linux

- Tested on Ubuntu 20.04+, Debian 10+, Fedora 34+
- Use GCC 7+ or Clang 10+
- Standard `make` build system

### macOS

- Tested on macOS 10.15+ (Catalina and later)
- Xcode Command Line Tools required
- Supports both Intel and Apple Silicon (ARM64)
- Use `brew install gcc` if needed

### Windows

- Tested on Windows 10/11
- Visual Studio 2019+ or MinGW-w64
- CMake required (download from cmake.org)
- Use `build.bat` for easy building

## Limitations

- Focuses on C89/99 subset (no C11/C17 features yet)
- Preprocessor macros are ignored (not expanded)
- Pointers have basic support
- Structs and unions have limited support
- No support for multi-file programs (yet)

## Contributing

Contributions are welcome! Areas for improvement:

- Extended C language support (C11, C17)
- Enhanced pointer descriptions
- Struct and union translations
- Multi-file program support
- Optimization suggestions in output
- Additional language targets (American English, other languages)

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Inspired by the need to make code more accessible to non-programmers
- Uses traditional compiler design principles
- Built with proper British English conventions throughout

## Contact

For questions, issues, or contributions, please visit:
- **GitHub**: https://github.com/DannyBimma/gb-en-compiler
- **Issues**: https://github.com/DannyBimma/gb-en-compiler/issues

## Version History

### Version 1.0.0 (2025)
- Initial release
- Full C89/99 subset support
- Cross-platform builds (Linux, macOS, Windows)
- Comprehensive British English translations
- Example programs and documentation

---

**Made with British English precision and accuracy.**
