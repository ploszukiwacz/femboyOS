#ifndef PANIC_H
#define PANIC_H

#include <stdnoreturn.h>
#include "../libs/print.h"

noreturn void panic(const char* message, const char* file, int line);

// Macro to make panic calls easier
#define PANIC(msg) panic(msg, __FILE__, __LINE__)

#endif
