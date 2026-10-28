#include "parser.h"
#include "command.h"

#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/*
 * Safe bounded string copy helper.
 * Avoids including <stdio.h> / snprintf in the core math engine.
 */
static void safe_strcpy(char *dest, size_t dest_size, const char *src)
{
    if ((dest != NULL) && (src != NULL) && (dest_size > 0U)) {
        size_t i = 0U;
        while ((i < (dest_size - 1U)) && (src[i] != '\0')) {
            dest[i] = src[i];
            i++;
        }
        dest[i] = '\0';
    }
}

/* Set error status and message if no prior error has been flagged */
static void parser_set_error(Parser *parser,
                             CalcStatus status,
                             const char *message)
{
    if (parser->result.status == CALC_OK) {
        parser->result.status = status;
        parser->result.position = (size_t)(parser->cursor - parser->input);
        safe_strcpy(parser->result.message, sizeof(parser->result.message), message);
    }
}

/* Skip whitespace characters */
static void skip_spaces(Parser *parser)
{
    while ((*parser->cursor == ' ') ||
           (*parser->cursor == '\t') ||
           (*parser->cursor == '\n') ||
           (*parser->cursor == '\r')) {
        parser->cursor++;
    }
}

/* Match and consume a single expected character */
static bool match(Parser *parser, char expected)
{
    bool matched = false;
    skip_spaces(parser);

    if (*parser->cursor == expected) {
        parser->cursor++;
        matched = true;
    }

    return matched;
}

static void parse_expression(Parser *parser);

/*
 * Parse floating-point numbers without using strtod() or <errno.h>.
 * Directly computes integer, fractional, and scientific exponent parts.
 * Stores result in reg[2] (temp/factor register).
 */
static void parse_number(Parser *parser)
{
    skip_spaces(parser);

    double value = 0.0;
    bool has_digits = false;

    /* 1. Integer part */
    while ((*parser->cursor >= '0') && (*parser->cursor <= '9')) {
        int32_t digit = (int32_t)(*parser->cursor - '0');
        value = (value * 10.0) + (double)digit;
        parser->cursor++;
        has_digits = true;
    }

    /* 2. Fractional part */
    if (*parser->cursor == '.') {
        parser->cursor++;
        double factor = 0.1;
        while ((*parser->cursor >= '0') && (*parser->cursor <= '9')) {
            int32_t digit = (int32_t)(*parser->cursor - '0');
            value = value + ((double)digit * factor);
            factor = factor * 0.1;
            parser->cursor++;
            has_digits = true;
        }
    }

    /* 3. Exponent part (e.g. 1e-3, 2.5E4) */
    if ((has_digits == true) && ((*parser->cursor == 'e') || (*parser->cursor == 'E'))) {
        parser->cursor++;
        bool neg_exp = false;

        if (*parser->cursor == '+') {
            parser->cursor++;
        } else if (*parser->cursor == '-') {
            neg_exp = true;
            parser->cursor++;
        } else {
            /* No explicit sign */
        }

        bool has_exp_digits = false;
        int32_t exp_val = 0;

        while ((*parser->cursor >= '0') && (*parser->cursor <= '9')) {
            int32_t digit = (int32_t)(*parser->cursor - '0');
            if (exp_val < 300) {
                exp_val = (exp_val * 10) + digit;
            }
            parser->cursor++;
            has_exp_digits = true;
        }

        if (has_exp_digits == false) {
            parser_set_error(parser, CALC_SYNTAX_ERROR, "Invalid exponent in number.");
            value = 0.0;
        } else {
            if (neg_exp == true) {
                exp_val = -exp_val;
            }
            value = value * pow(10.0, (double)exp_val);
        }
    }

    /* Validate parsed number and store in register 2 */
    if (has_digits == false) {
        parser_set_error(parser, CALC_SYNTAX_ERROR, "Expected a number.");
        parser->reg[2] = 0.0;
    } else if (isfinite(value) == 0) {
        parser_set_error(parser, CALC_OVERFLOW, "Numeric literal is outside the supported range.");
        parser->reg[2] = 0.0;
    } else {
        parser->reg[2] = value;
    }
}

/* Parse constants (pi, e) and store in reg[2] */
static void parse_identifier(Parser *parser)
{
    skip_spaces(parser);
    const char *start = parser->cursor;

    while (((*parser->cursor >= 'a') && (*parser->cursor <= 'z')) ||
           ((*parser->cursor >= 'A') && (*parser->cursor <= 'Z')) ||
           (*parser->cursor == '_')) {
        parser->cursor++;
    }

    size_t length = (size_t)(parser->cursor - start);

    if (length == 0U) {
        parser_set_error(parser, CALC_SYNTAX_ERROR, "Expected an identifier.");
        parser->reg[2] = 0.0;
    } else if ((length == 2U) && (start[0] == 'p') && (start[1] == 'i')) {
        parser->reg[2] = acos(-1.0);
    } else if ((length == 1U) && (start[0] == 'e')) {
        parser->reg[2] = exp(1.0);
    } else if ((length == 3U) &&
               ((start[0] == 'a') || (start[0] == 'A')) &&
               ((start[1] == 'n') || (start[1] == 'N')) &&
               ((start[2] == 's') || (start[2] == 'S'))) {
        parser->reg[2] = calculator_get_ans();
    } else {
        parser_set_error(parser, CALC_SYNTAX_ERROR, "Unknown identifier. Use pi, e, or ans.");
        parser->reg[2] = 0.0;
    }
}

/* Parse custom # commands like #1(x), #3(x) */
static void parse_command(Parser *parser)
{
    skip_spaces(parser);
    parser->cursor++; /* skip '#' */

    if ((*parser->cursor < '0') || (*parser->cursor > '9')) {
        parser_set_error(parser, CALC_UNKNOWN_COMMAND, "Expected a numeric command after '#'.");
        parser->reg[2] = 0.0;
    } else {
        int32_t code = 0;
        bool overflow = false;

        while ((*parser->cursor >= '0') && (*parser->cursor <= '9')) {
            int32_t digit = (int32_t)(*parser->cursor - '0');
            if (code > ((INT32_MAX - digit) / 10)) {
                overflow = true;
                break;
            }
            code = (code * 10) + digit;
            parser->cursor++;
        }

        if (overflow == true) {
            parser_set_error(parser, CALC_UNKNOWN_COMMAND, "Command number is too large.");
            parser->reg[2] = 0.0;
        } else {
            const Command *command = command_find(code);
            if (command == NULL) {
                parser_set_error(parser, CALC_UNKNOWN_COMMAND, "Unknown '#' command.");
                parser->reg[2] = 0.0;
            } else {
                skip_spaces(parser);
                if (match(parser, '(') == false) {
                    parser_set_error(parser, CALC_MISSING_ARGUMENT, "A command requires an argument in parentheses.");
                    parser->reg[2] = 0.0;
                } else {
                    /* Check recursion depth before evaluating nested expression */
                    if (parser->depth >= MAX_RECURSION_DEPTH) {
                        parser_set_error(parser, CALC_DEPTH_EXCEEDED, "Recursion depth limit exceeded.");
                        parser->reg[2] = 0.0;
                    } else {
                        parser->depth++;
                        /* Save outer registers while evaluating inner argument */
                        double saved_r0 = parser->reg[0];
                        double saved_r1 = parser->reg[1];

                        parse_expression(parser);

                        double arg_val = parser->reg[0];
                        parser->reg[0] = saved_r0;
                        parser->reg[1] = saved_r1;
                        parser->depth--;

                        if (parser->result.status == CALC_OK) {
                            if (match(parser, ')') == false) {
                                parser_set_error(parser, CALC_SYNTAX_ERROR, "Expected ')' after command argument.");
                                parser->reg[2] = 0.0;
                            } else {
                                bool func_error = false;
                                double val = command->function(arg_val, &func_error);

                                if ((func_error == true) || (isfinite(val) == 0)) {
                                    parser_set_error(parser, CALC_DOMAIN_ERROR, "Command argument is outside the function domain.");
                                    parser->reg[2] = 0.0;
                                } else {
                                    parser->reg[2] = val;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

/* Parse primary terms: numbers, constants, commands, or parenthesized expressions */
static void parse_primary(Parser *parser)
{
    skip_spaces(parser);

    if (*parser->cursor == '\0') {
        parser_set_error(parser, CALC_SYNTAX_ERROR, "Unexpected end of expression.");
        parser->reg[2] = 0.0;
    } else if (*parser->cursor == '(') {
        parser->cursor++;
        if (parser->depth >= MAX_RECURSION_DEPTH) {
            parser_set_error(parser, CALC_DEPTH_EXCEEDED, "Recursion depth limit exceeded.");
            parser->reg[2] = 0.0;
        } else {
            parser->depth++;
            /* Save active registers before sub-expression */
            double saved_r0 = parser->reg[0];
            double saved_r1 = parser->reg[1];

            parse_expression(parser);

            double sub_val = parser->reg[0];
            parser->reg[0] = saved_r0;
            parser->reg[1] = saved_r1;
            parser->reg[2] = sub_val;
            parser->depth--;

            if (parser->result.status == CALC_OK) {
                if (match(parser, ')') == false) {
                    parser_set_error(parser, CALC_SYNTAX_ERROR, "Expected ')'.");
                    parser->reg[2] = 0.0;
                }
            }
        }
    } else if (*parser->cursor == '#') {
        parse_command(parser);
    } else if (((*parser->cursor >= '0') && (*parser->cursor <= '9')) || (*parser->cursor == '.')) {
        parse_number(parser);
    } else if (((*parser->cursor >= 'a') && (*parser->cursor <= 'z')) ||
               ((*parser->cursor >= 'A') && (*parser->cursor <= 'Z')) ||
               (*parser->cursor == '_')) {
        parse_identifier(parser);
    } else {
        parser_set_error(parser, CALC_SYNTAX_ERROR, "Expected a number, constant, command, or '('.");
        parser->reg[2] = 0.0;
    }
}

/* Parse unary + and - operators */
static void parse_unary(Parser *parser)
{
    skip_spaces(parser);

    if (match(parser, '+') == true) {
        if (parser->depth >= MAX_RECURSION_DEPTH) {
            parser_set_error(parser, CALC_DEPTH_EXCEEDED, "Recursion depth limit exceeded.");
            parser->reg[2] = 0.0;
        } else {
            parser->depth++;
            parse_unary(parser);
            parser->depth--;
        }
    } else if (match(parser, '-') == true) {
        if (parser->depth >= MAX_RECURSION_DEPTH) {
            parser_set_error(parser, CALC_DEPTH_EXCEEDED, "Recursion depth limit exceeded.");
            parser->reg[2] = 0.0;
        } else {
            parser->depth++;
            parse_unary(parser);
            parser->depth--;
            parser->reg[2] = -parser->reg[2];
        }
    } else {
        parse_primary(parser);
    }
}

/* Parse exponentiation (^) - right associative */
static void parse_power(Parser *parser)
{
    parse_unary(parser);

    if (parser->result.status == CALC_OK) {
        skip_spaces(parser);
        if (*parser->cursor == '^') {
            parser->cursor++;
            if (parser->depth >= MAX_RECURSION_DEPTH) {
                parser_set_error(parser, CALC_DEPTH_EXCEEDED, "Recursion depth limit exceeded.");
            } else {
                parser->depth++;
                double base = parser->reg[2];

                parse_power(parser);
                parser->depth--;

                if (parser->result.status == CALC_OK) {
                    double res = pow(base, parser->reg[2]);
                    if (isfinite(res) == 0) {
                        parser_set_error(parser, CALC_OVERFLOW, "Exponentiation produced a non-finite result.");
                        parser->reg[2] = 0.0;
                    } else {
                        parser->reg[2] = res;
                    }
                }
            }
        }
    }
}

/*
 * Parse terms (* and /).
 * Streams operations through reg[1] (term accumulator) and reg[2] (factor).
 */
static void parse_term(Parser *parser)
{
    parse_power(parser);

    if (parser->result.status == CALC_OK) {
        parser->reg[1] = parser->reg[2];

        while (parser->result.status == CALC_OK) {
            skip_spaces(parser);
            char op = *parser->cursor;

            if ((op == '*') || (op == '/')) {
                parser->cursor++;
                parse_power(parser);

                if (parser->result.status == CALC_OK) {
                    if (op == '*') {
                        parser->reg[1] = parser->reg[1] * parser->reg[2];
                    } else {
                        if (parser->reg[2] == 0.0) {
                            parser_set_error(parser, CALC_DIVISION_BY_ZERO, "Division by zero.");
                        } else {
                            parser->reg[1] = parser->reg[1] / parser->reg[2];
                        }
                    }

                    if (isfinite(parser->reg[1]) == 0) {
                        parser_set_error(parser, CALC_OVERFLOW, "Arithmetic operation produced a non-finite result.");
                    }
                }
            } else {
                break;
            }
        }
    }
}

/*
 * Parse expressions (+ and -).
 * Streams operations through reg[0] (expression accumulator) and reg[1] (term).
 */
static void parse_expression(Parser *parser)
{
    parse_term(parser);

    if (parser->result.status == CALC_OK) {
        parser->reg[0] = parser->reg[1];

        while (parser->result.status == CALC_OK) {
            skip_spaces(parser);
            char op = *parser->cursor;

            if ((op == '+') || (op == '-')) {
                parser->cursor++;
                parse_term(parser);

                if (parser->result.status == CALC_OK) {
                    if (op == '+') {
                        parser->reg[0] = parser->reg[0] + parser->reg[1];
                    } else {
                        parser->reg[0] = parser->reg[0] - parser->reg[1];
                    }

                    if (isfinite(parser->reg[0]) == 0) {
                        parser_set_error(parser, CALC_OVERFLOW, "Arithmetic operation produced a non-finite result.");
                    }
                }
            } else {
                break;
            }
        }
    }
}

/* Public entry point for parsing an expression string */
CalcResult parser_parse(const char *expression)
{
    Parser parser;
    parser.input = expression;
    parser.cursor = expression;
    parser.result.status = CALC_OK;
    parser.result.value = 0.0;
    parser.result.position = 0U;
    parser.result.message[0] = '\0';
    parser.depth = 0U;
    parser.reg[0] = 0.0;
    parser.reg[1] = 0.0;
    parser.reg[2] = 0.0;

    if (expression == NULL) {
        parser.result.status = CALC_EMPTY_INPUT;
        safe_strcpy(parser.result.message, sizeof(parser.result.message), "Expression is empty.");
    } else {
        skip_spaces(&parser);

        if (*parser.cursor == '\0') {
            parser.result.status = CALC_EMPTY_INPUT;
            safe_strcpy(parser.result.message, sizeof(parser.result.message), "Expression is empty.");
        } else {
            parse_expression(&parser);

            if (parser.result.status == CALC_OK) {
                parser.result.value = parser.reg[0];
                skip_spaces(&parser);

                if (*parser.cursor != '\0') {
                    parser_set_error(&parser, CALC_TRAILING_INPUT, "Unexpected characters after the expression.");
                }
            }
        }
    }

    return parser.result;
}
