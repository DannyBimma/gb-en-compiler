# C to British English Compiler - Build Plan

## Project Overview

### Objective
Create a compiler that translates C89/99 source code into clear, readable British English prose that accurately describes what the program does. The output should be comprehensible to someone with no programming knowledge whilst maintaining the semantic accuracy of the original code.

### Key Requirements
- Accept C89/99 source code as input
- Process code through a proper compiler pipeline (lexical analysis, parsing, semantic analysis)
- Generate British English output in `.txt` format
- Output should be grammatically correct and naturally readable
- Non-programmers should understand the program's intent and logic flow

## Architecture Overview

### High-Level Design

```
┌─────────────┐     ┌─────────────┐     ┌─────────────┐     ┌─────────────┐
│   Source    │────▶│   Lexical   │────▶│   Parser    │────▶│  Semantic   │
│   Code      │     │   Analyzer  │     │   (AST)     │     │  Analyzer   │
│   (.c)      │     │   (Lexer)   │     │             │     │             │
└─────────────┘     └─────────────┘     └─────────────┘     └─────────────┘
                                                                     │
                                                                     ▼
┌─────────────┐     ┌─────────────┐     ┌─────────────┐     ┌─────────────┐
│   English   │◀────│   English   │◀────│  Template   │◀────│  Annotated  │
│   Output    │     │  Formatter  │     │   Engine    │     │     AST     │
│   (.txt)    │     │             │     │             │     │             │
└─────────────┘     └─────────────┘     └─────────────┘     └─────────────┘
```

### Component Breakdown

1. **Lexical Analyzer (Lexer)**: Tokenizes C source code
2. **Parser**: Builds Abstract Syntax Tree (AST) from tokens
3. **Semantic Analyzer**: Validates semantics and annotates AST with type information
4. **Template Engine**: Traverses AST and maps constructs to English templates
5. **English Formatter**: Produces well-formatted, grammatically correct prose

## Compiler Pipeline Stages

### Stage 1: Lexical Analysis (Tokenization)

**Purpose**: Break source code into tokens

**Implementation Details**:
- Scan input character by character
- Identify and classify tokens:
  - Keywords: `int`, `if`, `while`, `return`, etc.
  - Identifiers: variable/function names
  - Operators: `+`, `-`, `*`, `/`, `==`, `!=`, etc.
  - Literals: numbers, strings, characters
  - Punctuation: `{`, `}`, `;`, `,`, etc.
  - Comments: single-line and multi-line

**Data Structure**:
```c
typedef enum {
    TOKEN_KEYWORD,
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_STRING,
    TOKEN_OPERATOR,
    TOKEN_PUNCTUATION,
    TOKEN_EOF
} TokenType;

typedef struct {
    TokenType type;
    char *lexeme;
    int line;
    int column;
} Token;
```

### Stage 2: Syntax Analysis (Parsing)

**Purpose**: Build Abstract Syntax Tree (AST) from tokens

**Implementation Details**:
- Recursive descent parser or LR parser
- Handle C89/99 grammar rules
- Build tree representation of program structure
- Report syntax errors with line/column information

**AST Node Types**:
```c
typedef enum {
    NODE_PROGRAM,
    NODE_FUNCTION,
    NODE_DECLARATION,
    NODE_STATEMENT,
    NODE_EXPRESSION,
    NODE_BINARY_OP,
    NODE_UNARY_OP,
    NODE_LITERAL,
    NODE_IDENTIFIER,
    NODE_IF,
    NODE_WHILE,
    NODE_FOR,
    NODE_RETURN,
    NODE_BLOCK
} NodeType;

typedef struct ASTNode {
    NodeType type;
    union {
        struct {
            char *return_type;
            char *name;
            struct ASTNode **parameters;
            struct ASTNode *body;
        } function;

        struct {
            char *data_type;
            char *name;
            struct ASTNode *initializer;
        } declaration;

        struct {
            struct ASTNode *condition;
            struct ASTNode *then_branch;
            struct ASTNode *else_branch;
        } if_stmt;

        struct {
            struct ASTNode *condition;
            struct ASTNode *body;
        } while_stmt;

        struct {
            struct ASTNode *init;
            struct ASTNode *condition;
            struct ASTNode *increment;
            struct ASTNode *body;
        } for_stmt;

        struct {
            char *operator;
            struct ASTNode *left;
            struct ASTNode *right;
        } binary_op;

        struct {
            char *value;
            char *data_type;
        } literal;
    } data;

    int line;
    int column;
} ASTNode;
```

### Stage 3: Semantic Analysis

**Purpose**: Validate program semantics and collect type information

**Implementation Details**:
- Build symbol table for variables and functions
- Perform type checking
- Check for undeclared variables
- Validate function calls (argument count and types)
- Annotate AST nodes with type information
- Report semantic errors

**Symbol Table Structure**:
```c
typedef struct Symbol {
    char *name;
    char *type;
    char *scope;
    int line_declared;
    struct Symbol *next;
} Symbol;

typedef struct SymbolTable {
    Symbol *head;
    struct SymbolTable *parent;  // For nested scopes
} SymbolTable;
```

### Stage 4: English Template Generation

**Purpose**: Convert AST nodes to English phrases

**Implementation Details**:
- Traverse AST in depth-first order
- Map each node type to English template
- Handle context-sensitive translations
- Maintain narrative flow and coherence

**Translation Strategy**:

#### Control Flow
- **Program**: "This programme consists of [N] functions..."
- **Function**: "The function named [name] accepts [parameters] and returns [type]..."
- **If Statement**: "If the condition [condition] is true, then [then-block]. Otherwise, [else-block]."
- **While Loop**: "Whilst the condition [condition] remains true, repeatedly [body]."
- **For Loop**: "Beginning with [init], and continuing whilst [condition] holds, repeatedly [body], and after each iteration [increment]."

#### Declarations
- **Variable Declaration**: "Declare a variable named [name] of type [type], initialised to [value]."
- **Array Declaration**: "Declare an array named [name] containing [size] elements of type [type]."

#### Expressions
- **Assignment**: "Set the variable [name] to the value [expression]."
- **Arithmetic**:
  - `a + b` → "the sum of [a] and [b]"
  - `a - b` → "the difference between [a] and [b]"
  - `a * b` → "the product of [a] and [b]"
  - `a / b` → "[a] divided by [b]"
  - `a % b` → "the remainder when [a] is divided by [b]"

#### Comparisons
- `a == b` → "[a] is equal to [b]"
- `a != b` → "[a] is not equal to [b]"
- `a < b` → "[a] is less than [b]"
- `a > b` → "[a] is greater than [b]"
- `a <= b` → "[a] is less than or equal to [b]"
- `a >= b` → "[a] is greater than or equal to [b]"

#### Logical Operators
- `a && b` → "both [a] and [b]"
- `a || b` → "either [a] or [b]"
- `!a` → "not [a]"

#### Function Calls
- `printf("Hello")` → "Display the message 'Hello' to the user."
- `scanf("%d", &x)` → "Read an integer from the user and store it in variable [x]."
- `strlen(str)` → "Determine the length of the text stored in [str]."

### Stage 5: English Formatting and Output

**Purpose**: Format English output for readability

**Implementation Details**:
- Add proper indentation for nested structures
- Use paragraph breaks for logical sections
- Number steps in sequential operations
- Add section headers for functions
- Ensure proper British spelling (colour, initialise, whilst, programme)
- Generate final `.txt` file

**Formatting Rules**:
- Main program description at the top
- Each function gets its own section with heading
- Nested structures indented with 2 spaces
- Use numbered lists for sequences of operations
- Use bullet points for collections or options

## Translation Examples

### Example 1: Simple Calculator

**C Code**:
```c
#include <stdio.h>

int add(int a, int b) {
    return a + b;
}

int main() {
    int x = 5;
    int y = 10;
    int result = add(x, y);
    printf("Result: %d\n", result);
    return 0;
}
```

**British English Output**:
```
Programme Description
=====================

This programme consists of two functions: 'add' and 'main'.

Function: add
-------------
This function accepts two integer parameters named 'a' and 'b', and returns an integer value.

The function's purpose is to:
  • Return the sum of 'a' and 'b'.

Function: main
--------------
This is the main entry point of the programme. It accepts no parameters and returns an integer value to indicate success or failure.

The function performs the following steps:

  1. Declare a variable named 'x' of type integer, initialised to the value 5.

  2. Declare a variable named 'y' of type integer, initialised to the value 10.

  3. Declare a variable named 'result' of type integer, and initialise it by calling the 'add' function with arguments 'x' and 'y'.

  4. Display the message "Result: " followed by the value stored in 'result' to the user.

  5. Return the value 0 to indicate successful programme execution.
```

### Example 2: Factorial Function

**C Code**:
```c
int factorial(int n) {
    if (n <= 1) {
        return 1;
    }
    return n * factorial(n - 1);
}
```

**British English Output**:
```
Function: factorial
-------------------
This function accepts one integer parameter named 'n', and returns an integer value.

The function calculates the factorial of 'n' through the following logic:

  • If the condition "n is less than or equal to 1" is true, then:
      Return the value 1.
    Otherwise:
      Return the product of 'n' and the result of recursively calling the 'factorial' function with the argument "n minus 1".

Note: This function uses recursion, meaning it calls itself with a modified argument until it reaches a terminating condition.
```

### Example 3: Array Processing

**C Code**:
```c
int sum_array(int arr[], int size) {
    int total = 0;
    for (int i = 0; i < size; i++) {
        total = total + arr[i];
    }
    return total;
}
```

**British English Output**:
```
Function: sum_array
-------------------
This function accepts two parameters:
  • 'arr': an array of integers
  • 'size': an integer representing the number of elements in the array

The function returns an integer value representing the sum of all elements.

The function performs the following steps:

  1. Declare a variable named 'total' of type integer, initialised to 0.

  2. Beginning with declaring a counter variable 'i' initialised to 0, and continuing whilst 'i' is less than 'size', repeatedly perform the following operations, incrementing 'i' by 1 after each iteration:

      a. Set the variable 'total' to the value computed by adding 'total' and the element at position 'i' in the array 'arr'.

  3. Return the value stored in 'total', which now contains the sum of all array elements.
```

### Example 4: Conditional Logic

**C Code**:
```c
void check_age(int age) {
    if (age >= 18) {
        printf("You are an adult.\n");
    } else if (age >= 13) {
        printf("You are a teenager.\n");
    } else {
        printf("You are a child.\n");
    }
}
```

**British English Output**:
```
Function: check_age
-------------------
This function accepts one integer parameter named 'age', and returns nothing (void).

The function's purpose is to categorise a person based on their age:

  • If the condition "age is greater than or equal to 18" is true, then:
      Display the message "You are an adult." to the user.

  • Otherwise, if the condition "age is greater than or equal to 13" is true, then:
      Display the message "You are a teenager." to the user.

  • Otherwise:
      Display the message "You are a child." to the user.
```

## Implementation Phases

### Phase 1: Foundation (Weeks 1-2)
**Goal**: Establish project structure and basic tokenization

**Tasks**:
- Set up project directory structure
- Create makefile for compilation
- Implement data structures (Token, ASTNode, Symbol)
- Build basic lexer for keywords and identifiers
- Write unit tests for lexer
- Handle single-line and multi-line comments

**Deliverable**: Working lexer that can tokenize simple C programs

### Phase 2: Parser Development (Weeks 3-5)
**Goal**: Build AST from tokens

**Tasks**:
- Implement recursive descent parser
- Handle declarations (variables, functions)
- Parse expressions (arithmetic, logical, relational)
- Parse control flow statements (if, while, for)
- Build AST during parsing
- Error reporting with line/column numbers
- Write parser unit tests

**Deliverable**: Parser that produces AST for C89/99 subset

### Phase 3: Semantic Analysis (Weeks 6-7)
**Goal**: Validate program semantics

**Tasks**:
- Implement symbol table
- Scope management (global, function, block)
- Type checking for expressions
- Validate function calls
- Check variable declarations and usage
- Annotate AST with type information
- Write semantic analysis tests

**Deliverable**: Semantic analyzer that catches common errors

### Phase 4: English Template Engine (Weeks 8-10)
**Goal**: Convert AST to English descriptions

**Tasks**:
- Design template system for each AST node type
- Implement AST traversal mechanism
- Create translation functions for:
  - Declarations
  - Expressions
  - Statements
  - Control flow
  - Function definitions
- Handle nested structures
- Context-aware translations
- Write translation tests with example C programs

**Deliverable**: Template engine that produces basic English output

### Phase 5: English Enhancement (Weeks 11-12)
**Goal**: Improve readability and naturalness

**Tasks**:
- Implement formatting and indentation
- Add section headers and numbering
- Detect and explain common patterns (loops, recursion)
- British spelling and phrasing refinements
- Add contextual explanations (e.g., "Note: This is recursive")
- Handle edge cases and complex expressions
- Extensive testing with real-world C programs

**Deliverable**: Polished output that reads naturally

### Phase 6: Extended Features (Weeks 13-14)
**Goal**: Support more C language features

**Tasks**:
- Pointer operations and descriptions
- Struct and union declarations
- Typedef support
- Preprocessor directives (basic #define, #include)
- Standard library function recognition
- Multi-file programme support
- Optimise output for very large programs

**Deliverable**: Support for substantial C89/99 subset

### Phase 7: Testing and Refinement (Weeks 15-16)
**Goal**: Ensure reliability and quality

**Tasks**:
- Comprehensive test suite
- Test with real-world C programs
- Performance optimisation
- Memory leak detection and fixing
- Documentation (user guide, API docs)
- Bug fixes and edge case handling
- Code review and refactoring

**Deliverable**: Production-ready compiler

## Testing Strategy

### Unit Tests
- Test each component in isolation
- Lexer: Test tokenization of various constructs
- Parser: Test AST generation for different syntax
- Semantic analyzer: Test error detection
- Template engine: Test individual translations

### Integration Tests
- Test complete pipeline with sample programs
- Verify output correctness
- Check error handling and reporting

### Regression Tests
- Maintain suite of test programs
- Ensure changes don't break existing functionality

### Test Cases Categories
1. **Basic Constructs**: Variables, arithmetic, functions
2. **Control Flow**: if/else, loops, switch
3. **Complex Logic**: Nested loops, recursion, multiple conditions
4. **Edge Cases**: Empty functions, single statements, unusual formatting
5. **Error Cases**: Syntax errors, semantic errors, invalid operations

### Example Test Programs
- Hello World
- Factorial (recursive and iterative)
- Fibonacci sequence
- Array sorting algorithms
- String manipulation
- Calculator programmes
- Simple games (guess the number)

## Project Structure

```
gb-en-compiler/
├── src/
│   ├── main.c              # Entry point
│   ├── lexer.c/h           # Lexical analyzer
│   ├── parser.c/h          # Syntax analyzer
│   ├── ast.c/h             # AST data structures
│   ├── semantic.c/h        # Semantic analyzer
│   ├── symbol_table.c/h    # Symbol table management
│   ├── translator.c/h      # C to English translation
│   ├── formatter.c/h       # English output formatting
│   └── utils.c/h           # Utility functions
├── tests/
│   ├── lexer_tests.c
│   ├── parser_tests.c
│   ├── semantic_tests.c
│   ├── translator_tests.c
│   └── sample_programs/    # Test C programs
├── docs/
│   ├── BUILD_PLAN.md       # This document
│   ├── USER_GUIDE.md       # How to use the compiler
│   └── API_DOCS.md         # Developer documentation
├── examples/
│   ├── hello.c
│   ├── factorial.c
│   └── *.txt               # Corresponding English outputs
├── Makefile
├── README.md
└── LICENSE
```

## Build System

### Makefile Structure

```makefile
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g
SRC_DIR = src
TEST_DIR = tests
BUILD_DIR = build

# Source files
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

# Main target
c2en: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Tests
test: $(filter-out $(BUILD_DIR)/main.o, $(OBJS))
	$(CC) $(CFLAGS) $(TEST_DIR)/*.c $^ -o $(BUILD_DIR)/test_runner
	./$(BUILD_DIR)/test_runner

clean:
	rm -rf $(BUILD_DIR) c2en

.PHONY: test clean
```

## Usage

### Command Line Interface

```bash
# Basic usage
./c2en input.c -o output.txt

# With verbose output (show compilation stages)
./c2en input.c -o output.txt -v

# Display AST (for debugging)
./c2en input.c --show-ast

# Display tokens (for debugging)
./c2en input.c --show-tokens
```

### Options
- `-o <file>`: Specify output file (default: input filename with .txt extension)
- `-v`: Verbose mode (show compilation stages)
- `--show-tokens`: Display tokenization result
- `--show-ast`: Display abstract syntax tree
- `--help`: Display usage information
- `--version`: Display compiler version

## British English Guidelines

### Spelling Conventions
- programme (not program, except in "programming")
- colour (not color)
- initialise (not initialize)
- whilst (not while, in prose)
- analyse (not analyze)
- behaviour (not behavior)

### Phrasing Conventions
- "the function accepts" (not "takes")
- "returns nothing" for void functions
- "displays to the user" for output
- "reads from the user" for input
- Use full sentences and proper punctuation
- Prefer formal tone over casual

### Numerical Expressions
- Write "the value 5" not "5"
- Write "zero" for 0 in prose
- Use "minus" for negative numbers
- Use "divided by" not "over"

## Future Enhancements

### Phase 8+: Advanced Features
1. **C++ Support**: Extend to handle classes and objects
2. **Optimisation Notes**: Identify and explain inefficient code
3. **Flowchart Generation**: Visual representation alongside text
4. **Interactive Mode**: Ask questions about unclear variable names
5. **Difficulty Levels**: Adjust detail level for different audiences
6. **PDF Output**: Formatted documents with table of contents
7. **Syntax Highlighting**: Colour-coded output for key concepts
8. **Reverse Translation**: English back to C (ambitious!)
9. **Web Interface**: Online compiler with side-by-side view
10. **Educational Mode**: Add learning prompts and explanations

## Success Criteria

The compiler will be considered successful when:

1. **Correctness**: Accurately translates C semantics to English
2. **Readability**: Non-programmers understand programme logic
3. **Completeness**: Handles substantial subset of C89/99
4. **Robustness**: Gracefully handles errors with helpful messages
5. **Performance**: Compiles programs in reasonable time
6. **Maintainability**: Clean, documented, modular code
7. **Testability**: Comprehensive test suite with high coverage

## Potential Challenges and Solutions

### Challenge 1: Complex Expressions
**Problem**: Nested expressions like `((a + b) * (c - d)) / e` are hard to describe naturally.

**Solution**:
- Use parenthetical descriptions
- Break into sub-expressions
- Example: "the result of adding 'a' and 'b', multiplied by the result of subtracting 'd' from 'c', then divided by 'e'"

### Challenge 2: Pointer Arithmetic
**Problem**: Pointers are conceptually difficult for non-programmers.

**Solution**:
- Use accessible analogies (addresses like house numbers)
- Explain indirection clearly
- Example: "*ptr" → "the value stored at the memory location referenced by 'ptr'"

### Challenge 3: Macro Expansions
**Problem**: Preprocessor macros can obscure actual code.

**Solution**:
- Explain macro definitions separately
- Show both macro and expanded form
- Note when macros are used

### Challenge 4: Maintaining Context
**Problem**: Large functions lose narrative flow.

**Solution**:
- Add summary sections
- Use transitional phrases
- Group related operations
- Add explanatory comments about overall purpose

### Challenge 5: Ambiguous Variable Names
**Problem**: Variables like `x`, `i`, `tmp` don't convey meaning.

**Solution**:
- Use original names but add context clues
- For loop counters, explain their purpose
- Suggest better names in comments

## Conclusion

This build plan provides a comprehensive roadmap for creating a C to British English compiler. By following the phased approach and maintaining focus on readability and accuracy, the project will deliver a unique tool that bridges the gap between code and natural language, making programming logic accessible to everyone.

The modular architecture ensures maintainability, whilst the extensive testing strategy guarantees reliability. The emphasis on proper British English and clear, formal prose will result in output that is both technically accurate and genuinely readable by non-programmers.
