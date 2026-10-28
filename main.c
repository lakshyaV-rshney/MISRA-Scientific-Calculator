#include "calculator.h"
#include "command.h"

#include <stdio.h>
#include <string.h>

/*
 * CLI driver for testing and desktop use.
 * (Note: stdio is kept in main.c so that the core calculator engine
 *  remains standalone and embeddable on microcontrollers).
 */

#define INPUT_BUFFER_SIZE 1024U

static void print_result(const CalcResult *result)
{
    if (result->status == CALC_OK) {
        (void)printf("= %.15g\n", result->value);
    } else {
        (void)printf("Error [%s] at position %zu: %s\n",
                     calculator_status_string(result->status),
                     result->position,
                     result->message);
    }
    (void)fflush(stdout);
}

static void print_command_list(void)
{
    (void)puts("\nCustom command list");
    (void)puts("-------------------");

    size_t count = command_get_count();
    for (size_t i = 0U; i < count; ++i) {
        const Command *cmd = command_get_by_index(i);
        if (cmd != NULL) {
            (void)printf("#%-2d %-5s  %s\n",
                         cmd->code,
                         cmd->name,
                         cmd->description);
        }
    }

    (void)puts("\nConstants: pi, e, ans (previous result)");
    (void)puts("Operators: +  -  *  /  ^");
    (void)puts("Grouping: ( expression )");
    (void)puts("Example: 10*#1(2*pi)");
}

int main(void)
{
    char input[INPUT_BUFFER_SIZE];

    (void)puts("Scientific Calculator (Embedded C Engine)");
    (void)puts("=========================================");
    (void)puts("Enter an expression, or type 'help' for commands.");
    (void)puts("Type 'exit' or 'quit' to close the calculator.");

    for (;;) {
        (void)printf("\n> ");
        (void)fflush(stdout);

        if (fgets(input, (int)sizeof(input), stdin) == NULL) {
            (void)putchar('\n');
            break;
        }

        input[strcspn(input, "\r\n")] = '\0';

        if ((strcmp(input, "exit") == 0) ||
            (strcmp(input, "quit") == 0)) {
            break;
        }

        if (strcmp(input, "help") == 0) {
            print_command_list();
            continue;
        }

        CalcResult result = calculator_evaluate(input);
        print_result(&result);
    }

    (void)puts("Calculator closed.");
    return 0;
}
