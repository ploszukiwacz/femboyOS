#include "ping.h"
#include "../libs/net/icmp.h"
#include "../libs/print.h"
#include "../libs/timer.h"

static uint32_t parse_ip(const char* str) {
    uint32_t ip = 0;
    uint32_t octet = 0;

    while (*str) {
        if (*str == '.') {
            ip = (ip << 8) | octet;
            octet = 0;
        }
        else if (*str >= '0' && *str <= '9') {
            octet = octet * 10 + (*str - '0');
        }
        str++;
    }

    ip = (ip << 8) | octet;
    return ip;
}

void CMD_ping(const char* args) {
    // Skip command name
    while (*args && *args != ' ') args++;
    while (*args == ' ') args++;

    if (!*args) {
        print_str("Usage: ping <ip-address>\n");
        return;
    }

    uint32_t dest_ip = parse_ip(args);
    print_str("Pinging ");
    print_ip(dest_ip);
    print_str("...\n");

    // Send 4 pings
    for (int i = 0; i < 4; i++) {
        if (icmp_send_echo_request(dest_ip, i)) {
            print_str("Ping sent.\n");
        } else {
            print_str("Failed to send ping.\n");
        }
        sleep(1000);  // Wait 1 second between pings
    }
}
