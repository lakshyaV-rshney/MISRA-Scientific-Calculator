#include "command.h"

#include <math.h>

static double command_sin(double x, bool *error)
{
    double result = 0.0;

    if (isfinite(x) == 0) {
        *error = true;
    } else {
        result = sin(x);
        if (isfinite(result) == 0) {
            *error = true;
        }
    }

    return result;
}

static double command_cos(double x, bool *error)
{
    double result = 0.0;

    if (isfinite(x) == 0) {
        *error = true;
    } else {
        result = cos(x);
        if (isfinite(result) == 0) {
            *error = true;
        }
    }

    return result;
}

static double command_tan(double x, bool *error)
{
    double result = 0.0;

    if (isfinite(x) == 0) {
        *error = true;
    } else {
        result = tan(x);
        if (isfinite(result) == 0) {
            *error = true;
        }
    }

    return result;
}

static double command_sqrt(double x, bool *error)
{
    double result = 0.0;

    if ((x < 0.0) || (isfinite(x) == 0)) {
        *error = true;
    } else {
        result = sqrt(x);
        if (isfinite(result) == 0) {
            *error = true;
        }
    }

    return result;
}

static double command_log10(double x, bool *error)
{
    double result = 0.0;

    if ((x <= 0.0) || (isfinite(x) == 0)) {
        *error = true;
    } else {
        result = log10(x);
        if (isfinite(result) == 0) {
            *error = true;
        }
    }

    return result;
}

static double command_ln(double x, bool *error)
{
    double result = 0.0;

    if ((x <= 0.0) || (isfinite(x) == 0)) {
        *error = true;
    } else {
        result = log(x);
        if (isfinite(result) == 0) {
            *error = true;
        }
    }

    return result;
}

static double command_exp(double x, bool *error)
{
    double result = 0.0;

    if (isfinite(x) == 0) {
        *error = true;
    } else {
        result = exp(x);
        if (isfinite(result) == 0) {
            *error = true;
        }
    }

    return result;
}

static double command_abs(double x, bool *error)
{
    double result = 0.0;

    if (isfinite(x) == 0) {
        *error = true;
    } else {
        result = fabs(x);
    }

    return result;
}

static double command_asin(double x, bool *error)
{
    double result = 0.0;

    if ((isfinite(x) == 0) || (x < -1.0) || (x > 1.0)) {
        *error = true;
    } else {
        result = asin(x);
        if (isfinite(result) == 0) {
            *error = true;
        }
    }

    return result;
}

static double command_acos(double x, bool *error)
{
    double result = 0.0;

    if ((isfinite(x) == 0) || (x < -1.0) || (x > 1.0)) {
        *error = true;
    } else {
        result = acos(x);
        if (isfinite(result) == 0) {
            *error = true;
        }
    }

    return result;
}

static double command_atan(double x, bool *error)
{
    double result = 0.0;

    if (isfinite(x) == 0) {
        *error = true;
    } else {
        result = atan(x);
        if (isfinite(result) == 0) {
            *error = true;
        }
    }

    return result;
}

static double command_sinh(double x, bool *error)
{
    double result = 0.0;

    if (isfinite(x) == 0) {
        *error = true;
    } else {
        result = sinh(x);
        if (isfinite(result) == 0) {
            *error = true;
        }
    }

    return result;
}

static double command_cosh(double x, bool *error)
{
    double result = 0.0;

    if (isfinite(x) == 0) {
        *error = true;
    } else {
        result = cosh(x);
        if (isfinite(result) == 0) {
            *error = true;
        }
    }

    return result;
}

static double command_tanh(double x, bool *error)
{
    double result = 0.0;

    if (isfinite(x) == 0) {
        *error = true;
    } else {
        result = tanh(x);
        if (isfinite(result) == 0) {
            *error = true;
        }
    }

    return result;
}

static double command_floor(double x, bool *error)
{
    double result = 0.0;

    if (isfinite(x) == 0) {
        *error = true;
    } else {
        result = floor(x);
    }

    return result;
}

static double command_ceil(double x, bool *error)
{
    double result = 0.0;

    if (isfinite(x) == 0) {
        *error = true;
    } else {
        result = ceil(x);
    }

    return result;
}

static double command_round(double x, bool *error)
{
    double result = 0.0;

    if (isfinite(x) == 0) {
        *error = true;
    } else {
        result = round(x);
    }

    return result;
}

static double command_cbrt(double x, bool *error)
{
    double result = 0.0;

    if (isfinite(x) == 0) {
        *error = true;
    } else {
        result = cbrt(x);
        if (isfinite(result) == 0) {
            *error = true;
        }
    }

    return result;
}

static double command_deg(double x, bool *error)
{
    double result = 0.0;

    if (isfinite(x) == 0) {
        *error = true;
    } else {
        result = x * (180.0 / acos(-1.0));
        if (isfinite(result) == 0) {
            *error = true;
        }
    }

    return result;
}

static double command_rad(double x, bool *error)
{
    double result = 0.0;

    if (isfinite(x) == 0) {
        *error = true;
    } else {
        result = x * (acos(-1.0) / 180.0);
        if (isfinite(result) == 0) {
            *error = true;
        }
    }

    return result;
}

static const Command COMMANDS[] = {
    {1,  "cos",   "Cosine (radians)", command_cos},
    {2,  "tan",   "Tangent (radians)", command_tan},
    {3,  "sqrt",  "Square root (x >= 0)", command_sqrt},
    {4,  "sin",   "Sine (radians)", command_sin},
    {5,  "log",   "Base-10 logarithm (x > 0)", command_log10},
    {6,  "ln",    "Natural logarithm (x > 0)", command_ln},
    {7,  "exp",   "e raised to x", command_exp},
    {8,  "abs",   "Absolute value |x|", command_abs},
    {9,  "asin",  "Arcsine (-1 <= x <= 1)", command_asin},
    {10, "acos",  "Arccosine (-1 <= x <= 1)", command_acos},
    {11, "atan",  "Arctangent", command_atan},
    {12, "sinh",  "Hyperbolic sine", command_sinh},
    {13, "cosh",  "Hyperbolic cosine", command_cosh},
    {14, "tanh",  "Hyperbolic tangent", command_tanh},
    {15, "floor", "Floor: greatest integer <= x", command_floor},
    {16, "ceil",  "Ceil: least integer >= x", command_ceil},
    {17, "round", "Round to nearest integer", command_round},
    {18, "cbrt",  "Cube root", command_cbrt},
    {19, "deg",   "Convert radians to degrees", command_deg},
    {20, "rad",   "Convert degrees to radians", command_rad}
};

static const size_t COMMAND_COUNT = sizeof(COMMANDS) / sizeof(COMMANDS[0]);

const Command *command_find(int32_t code)
{
    const Command *found = NULL;
    size_t i = 0U;

    while (i < COMMAND_COUNT) {
        if (COMMANDS[i].code == code) {
            found = &COMMANDS[i];
            break;
        }
        i++;
    }

    return found;
}

size_t command_get_count(void)
{
    return COMMAND_COUNT;
}

const Command *command_get_by_index(size_t index)
{
    const Command *cmd = NULL;

    if (index < COMMAND_COUNT) {
        cmd = &COMMANDS[index];
    }

    return cmd;
}
