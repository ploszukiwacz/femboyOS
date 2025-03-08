#include "../libs/print.h"
#include "fortune.h"
#include "../kernel/panic.h"

void CMD_fortune() {
    const char* fortunes[] = {
        "A journey of a thousand miles begins with a single step.",
        "The best code is no code at all.",
        "Your OS will someday rule the world.",
        "Have you tried turning it off and on again?",
        "All your base are belong to us.",
        "run sudo rm -fr /* on a linux btw system",
        "Segmentation fault (core dumped)",
        "Today is a good day to code.",
        "You will soon receive a mysterious pull request.",
        "A wild bug appears! Developer uses Debug. It's super effective!",
        "There's no place like 127.0.0.1",
        "To be is to do - Socrates. To do is to be - Sartre. Do be do be do - Sinatra.",
        "The cake is a lie, but the pie is 3.14159265358979323846...",
        "Time is an illusion. Lunchtime doubly so.",
        "Beware the Jabberwock, my son! The jaws that bite, the claws that catch!",
        "You're breathtaking!",
        "hey claude code me a os.",
        "PANIC"
    };

    // Use tick count to get a pseudo-random fortune
    extern volatile uint64_t tick_count;
    uint32_t index = tick_count % (sizeof(fortunes) / sizeof(fortunes[0]));

    if (fortunes[index] == "PANIC") {
        PANIC("Idk, felt like it :3");
    }

    print_str("Your fortune: ");
    print_str(fortunes[index]);
    print_str("\n");
}
