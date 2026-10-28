#ifndef COMMAND_H
#define COMMAND_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef double (*CommandFunction)(double argument, bool *error);

typedef struct {
    int32_t code;
    const char *name;
    const char *description;
    CommandFunction function;
} Command;

const Command *command_find(int32_t code);
size_t command_get_count(void);
const Command *command_get_by_index(size_t index);

#endif /* COMMAND_H */
