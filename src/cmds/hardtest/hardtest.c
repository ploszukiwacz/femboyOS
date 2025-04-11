#include "../../libs/pci.h"
#include "../../libs/print.h"
#include "hardtest.h"
#include "../command_registry.h"

void CMD_hardtest() {
    pci_device_t device;

    if (pci_find_device(E1000_VENDOR_ID, E1000_DEVICE_ID, &device)) {
        print_str("[+] The intel e1000 is present.\n");
    } else {
        print_str("[-] The intel e1000 is not present.\n");
    }
}

command_t CMD_hardtest_command = {
    .name = "hardtest",
    .short_desc = "Test the presence of hardware.",
    .usage = "hardtest",
    .long_desc = "Test the presence of hardware.",
    .examples = "hardtest",
    .execute = CMD_hardtest,
};

void CMD_init_hardtest() {
    register_command(&CMD_hardtest_command);
}