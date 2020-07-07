#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "btstack.h"

#ifdef HAVE_BTSTACK_STDIN 
#include "btstack_stdin.h"
static bd_addr_t device_addr;
static const char* device_addr_string = "00:1B:DC:08:E2:72";
static void show_usage(void) {
    bd_addr_t      iut_address;
    gap_local_bd_addr(iut_address);
    printf("\n--- Bluetooth Test Console %s ---\n", bd_addr_to_str(iut_address));
    printf("b      - AVDTP Sink create  connection to addr %s\n", bd_addr_to_str(device_addr));
    printf("B      - AVDTP Sink disconnect\n");
    printf("c      - AVRCP Controller create connection to addr %s\n", bd_addr_to_str(device_addr));
    printf("C      - AVRCP Controller disconnect\n");
    printf("d      - AVRCP Target create connection to addr %s\n", bd_addr_to_str(device_addr));
    printf("D      - AVRCP Target disconnect\n");
    printf("---\n");
}
static void stdin_process(char cmd) {
    uint8_t status = ERROR_CODE_SUCCESS;

    switch (cmd) {
        case 'o':
            break;
        case 'c':
            break;
        default:
            show_usage();
            break;
    }
    if (status != ERROR_CODE_SUCCESS) {
        printf("Could not perform command, status 0x%2x\n", status);
    }
}
#endif
int a2dp_setup(int argc, const char* argv[]);
int ble_setup(int argc, const char* argv[]);
int btstack_main(int argc, const char * argv[]){
    UNUSED(argc);
    (void)argv;
    l2cap_init();
    sdp_init();
    sm_init();
    sm_set_io_capabilities(IO_CAPABILITY_NO_INPUT_NO_OUTPUT);
    gap_set_local_name("AIR");
    gap_discoverable_control(1);
    gap_set_class_of_device(0x200408);

    a2dp_setup(argc, argv);
    ble_setup(argc, argv);

#ifdef HAVE_BTSTACK_STDIN
    // parse human readable Bluetooth address
    sscanf_bd_addr(device_addr_string, device_addr);
    btstack_stdin_setup(stdin_process);
#endif

    // turn on!
    printf("Starting BTstack ...\n");
    hci_power_control(HCI_POWER_ON);
    return 0;
}
