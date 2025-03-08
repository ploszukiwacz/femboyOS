#include "../libs/pci.h"
#include "../libs/print.h"
#include "hardtest.h"

void CMD_hardtest() {
    pci_device_t device;

    if (pci_find_device(E1000_VENDOR_ID, E1000_DEVICE_ID, &device)) {
        print_str("[+] The intel e1000 is present.\n");
    } else {
        print_str("[-] The intel e1000 is not present.\n");
    }

}
