#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"
#include "lexer.h"
#include "parser.h"
#include "semantic.h"
#include "translator.h"
#include "formatter.h"

/* Command line options */
typedef struct {
    char* input_file;
    char* output_file;
    int show_tokens;
    int show_ast;
    int verbose;
    int show_help;
    int show_version;
} Options;

/* Print usage information */
static void print_usage(const char* program_name) {
    printf("C to British English Compiler (c2en) - Version %s\n\n", C2EN_VERSION_STRING);
    printf("Usage: %s <input.c> [options]\n\n", program_name);
    printf("Options:\n");
    printf("  -o <file>       Specify output file (default: input filename with .txt extension)\n");
    printf("  -v              Verbose mode (show compilation stages)\n");
    printf("  --show-tokens   Display tokenization result\n");
    printf("  --show-ast      Display abstract syntax tree\n");
    printf("  --help          Display this help message\n");
    printf("  --version       Display compiler version\n\n");
    printf("Examples:\n");
    printf("  %s hello.c                    # Compile hello.c to hello.txt\n", program_name);
    printf("  %s factorial.c -o output.txt  # Compile to specific output file\n", program_name);
    printf("  %s test.c -v                  # Compile with verbose output\n\n", program_name);
}

/* Print version information */
static void print_version(void) {
    printf("C to British English Compiler (c2en)\n");
    printf("Version: %s\n", C2EN_VERSION_STRING);
    printf("Build Date: %s\n", __DATE__);
    printf("Compiler: %s\n",
#ifdef __clang__
           "Clang"
#elif defined(__GNUC__)
           "GCC"
#elif defined(_MSC_VER)
           "MSVC"
#else
           "Unknown"
#endif
    );
    printf("C Standard: C99\n");
}

/* Parse command line arguments */
static Options parse_arguments(int argc, char** argv) {
    Options opts = {0};

    if (argc < 2) {
        opts.show_help = 1;
        return opts;
    }

    for (int i = 1; i < argc; i++) {
        if (string_equals(argv[i], "--help")) {
            opts.show_help = 1;
        } else if (string_equals(argv[i], "--version")) {
            opts.show_version = 1;
        } else if (string_equals(argv[i], "-v")) {
            opts.verbose = 1;
        } else if (string_equals(argv[i], "--show-tokens")) {
            opts.show_tokens = 1;
        } else if (string_equals(argv[i], "--show-ast")) {
            opts.show_ast = 1;
        } else if (string_equals(argv[i], "-o")) {
            if (i + 1 < argc) {
                opts.output_file = argv[++i];
            } else {
                log_message(LOG_ERROR, "Option -o requires an argument");
                opts.show_help = 1;
            }
        } else if (argv[i][0] == '-') {
            log_message(LOG_ERROR, "Unknown option: %s", argv[i]);
            opts.show_help = 1;
        } else {
            if (!opts.input_file) {
                opts.input_file = argv[i];
            } else {
                log_message(LOG_ERROR, "Multiple input files specified");
                opts.show_help = 1;
            }
        }
    }

    /* Generate default output filename if not specified */
    if (opts.input_file && !opts.output_file && !opts.show_help && !opts.show_version) {
        size_t len = strlen(opts.input_file);
        char* output = (char*)safe_malloc(len + 5);
        strcpy(output, opts.input_file);

        /* Replace .c extension with .txt */
        if (len > 2 && output[len-2] == '.' && output[len-1] == 'c') {
            output[len-2] = '\0';
        } else if (len > 2 && output[len-1] == 'c') {
            output[len-1] = '\0';
        }
        strcat(output, ".txt");
        opts.output_file = output;
    }

    return opts;
}

/* Main compilation function */
static int compile(Options* opts) {
    if (opts->verbose) {
        log_message(LOG_INFO, "Starting compilation of %s", opts->input_file);
    }

    /* Read source file */
    if (opts->verbose) {
        log_message(LOG_INFO, "Reading source file...");
    }
    char* source = read_file(opts->input_file);
    if (!source) {
        log_message(LOG_ERROR, "Failed to read input file: %s", opts->input_file);
        return 1;
    }

    /* Lexical analysis */
    if (opts->verbose) {
        log_message(LOG_INFO, "Performing lexical analysis...");
    }
    TokenList* tokens = tokenize(source, opts->input_file);

    if (opts->show_tokens) {
        printf("\n=== TOKENS ===\n");
        for (int i = 0; i < tokens->count; i++) {
            Token* token = tokens->tokens[i];
            printf("%d:%d  %-15s  '%s'\n",
                   token->line, token->column,
                   token_type_to_string(token->type),
                   token->lexeme);
        }
        printf("\n");
    }

    /* Check for lexer errors */
    if (tokens->count > 0 && tokens->tokens[tokens->count - 1]->type == TOKEN_ERROR) {
        log_message(LOG_ERROR, "Lexical analysis failed");
        token_list_destroy(tokens);
        free(source);
        return 1;
    }

    /* Syntax analysis */
    if (opts->verbose) {
        log_message(LOG_INFO, "Performing syntax analysis...");
    }
    ASTNode* ast = parse(tokens, opts->input_file);

    if (!ast) {
        log_message(LOG_ERROR, "Syntax analysis failed");
        token_list_destroy(tokens);
        free(source);
        return 1;
    }

    if (opts->show_ast) {
        printf("\n=== ABSTRACT SYNTAX TREE ===\n");
        ast_print(ast, 0);
        printf("\n");
    }

    /* Semantic analysis */
    if (opts->verbose) {
        log_message(LOG_INFO, "Performing semantic analysis...");
    }
    if (!analyze_semantics(ast, opts->input_file)) {
        log_message(LOG_ERROR, "Semantic analysis failed");
        ast_destroy(ast);
        token_list_destroy(tokens);
        free(source);
        return 1;
    }

    /* Translation to English */
    if (opts->verbose) {
        log_message(LOG_INFO, "Translating to British English...");
    }
    char* english = translate_to_english(ast);

    /* Format output */
    if (opts->verbose) {
        log_message(LOG_INFO, "Formatting output...");
    }
    char* formatted = format_english_output(english);

    /* Write output file */
    if (opts->verbose) {
        log_message(LOG_INFO, "Writing output to %s", opts->output_file);
    }
    if (!write_file(opts->output_file, formatted)) {
        log_message(LOG_ERROR, "Failed to write output file");
        free(formatted);
        free(english);
        ast_destroy(ast);
        token_list_destroy(tokens);
        free(source);
        return 1;
    }

    if (opts->verbose) {
        log_message(LOG_INFO, "Compilation completed successfully!");
    } else {
        printf("Successfully compiled %s to %s\n", opts->input_file, opts->output_file);
    }

    /* Cleanup */
    free(formatted);
    free(english);
    ast_destroy(ast);
    token_list_destroy(tokens);
    free(source);

    return 0;
}

/* Main entry point */
int main(int argc, char** argv) {
    Options opts = parse_arguments(argc, argv);

    if (opts.show_help) {
        print_usage(argv[0]);
        return 0;
    }

    if (opts.show_version) {
        print_version();
        return 0;
    }

    if (!opts.input_file) {
        log_message(LOG_ERROR, "No input file specified");
        print_usage(argv[0]);
        return 1;
    }

    return compile(&opts);
}
