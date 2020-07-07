/*
 * Copyright (C) 2014 BlueKitchen GmbH
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holders nor the names of
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 * 4. Any redistribution, use, or modification is done solely for
 *    personal benefit and not for any commercial purpose or for
 *    monetary gain.
 *
 * THIS SOFTWARE IS PROVIDED BY BLUEKITCHEN GMBH AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL MATTHIAS
 * RINGWALD OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * Please inquire about commercial licensing options at
 * contact@bluekitchen-gmbh.com
 *
 */

#define BTSTACK_FILE__ "gatt_streamer.c"

 // *****************************************************************************
 /* EXAMPLE_START(le_streamer): LE Streamer - Stream data over GATT.
  *
  * @text All newer operating systems provide GATT Client functionality.
  * This example shows how to get a maximal throughput via BLE:
  * - send whenever possible,
  * - use the max ATT MTU.
  *
  * @text In theory, we should also update the connection parameters, but we already get
  * a connection interval of 30 ms and there's no public way to use a shorter
  * interval with iOS (if we're not implementing an HID device).
  *
  * @text Note: To start the streaming, run the example.
  * On remote device use some GATT Explorer, e.g. LightBlue, BLExplr to enable notifications.
  */
  // *****************************************************************************

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "btstack.h"

const uint8_t profile_data[] =
{
    // ATT DB Version
    1,

    // 0x0001 PRIMARY_SERVICE-GAP_SERVICE
    0x0a, 0x00, 0x02, 0x00, 0x01, 0x00, 0x00, 0x28, 0x00, 0x18,
    // 0x0002 CHARACTERISTIC-GAP_DEVICE_NAME-READ
    0x0d, 0x00, 0x02, 0x00, 0x02, 0x00, 0x03, 0x28, 0x02, 0x03, 0x00, 0x00, 0x2a,
    // 0x0003 VALUE-GAP_DEVICE_NAME-READ-'LE Streamer'
    // READ_ANYBODY
    0x13, 0x00, 0x02, 0x00, 0x03, 0x00, 0x00, 0x2a, 0x4c, 0x45, 0x20, 0x53, 0x74, 0x72, 0x65, 0x61, 0x6d, 0x65, 0x72,

    // 0x0004 PRIMARY_SERVICE-GATT_SERVICE
    0x0a, 0x00, 0x02, 0x00, 0x04, 0x00, 0x00, 0x28, 0x01, 0x18,
    // 0x0005 CHARACTERISTIC-GATT_SERVICE_CHANGED-READ
    0x0d, 0x00, 0x02, 0x00, 0x05, 0x00, 0x03, 0x28, 0x02, 0x06, 0x00, 0x05, 0x2a,
    // 0x0006 VALUE-GATT_SERVICE_CHANGED-READ-''
    // READ_ANYBODY
    0x08, 0x00, 0x02, 0x00, 0x06, 0x00, 0x05, 0x2a,
    // Test Service

    // 0x0007 PRIMARY_SERVICE-0000FF10-0000-1000-8000-00805F9B34FB
    0x18, 0x00, 0x02, 0x00, 0x07, 0x00, 0x00, 0x28, 0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0x10, 0xff, 0x00, 0x00,
    // Test Characteristic A, write_without_response as well as notify
    // 0x0008 CHARACTERISTIC-0000FF11-0000-1000-8000-00805F9B34FB-WRITE_WITHOUT_RESPONSE | NOTIFY | DYNAMIC
    0x1b, 0x00, 0x02, 0x00, 0x08, 0x00, 0x03, 0x28, 0x14, 0x09, 0x00, 0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0x11, 0xff, 0x00, 0x00,
    // 0x0009 VALUE-0000FF11-0000-1000-8000-00805F9B34FB-WRITE_WITHOUT_RESPONSE | NOTIFY | DYNAMIC-''
    // WRITE_ANYBODY
    0x16, 0x00, 0x04, 0x03, 0x09, 0x00, 0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0x11, 0xff, 0x00, 0x00,
    // 0x000a CLIENT_CHARACTERISTIC_CONFIGURATION
    // READ_ANYBODY, WRITE_ANYBODY
    0x0a, 0x00, 0x0e, 0x01, 0x0a, 0x00, 0x02, 0x29, 0x00, 0x00,
    // Test Characteristic B, write_without_response as well as notify
    // 0x000b CHARACTERISTIC-0000FF12-0000-1000-8000-00805F9B34FB-WRITE_WITHOUT_RESPONSE | NOTIFY | DYNAMIC
    0x1b, 0x00, 0x02, 0x00, 0x0b, 0x00, 0x03, 0x28, 0x14, 0x0c, 0x00, 0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0x12, 0xff, 0x00, 0x00,
    // 0x000c VALUE-0000FF12-0000-1000-8000-00805F9B34FB-WRITE_WITHOUT_RESPONSE | NOTIFY | DYNAMIC-''
    // WRITE_ANYBODY
    0x16, 0x00, 0x04, 0x03, 0x0c, 0x00, 0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0x12, 0xff, 0x00, 0x00,
    // 0x000d CLIENT_CHARACTERISTIC_CONFIGURATION
    // READ_ANYBODY, WRITE_ANYBODY
    0x0a, 0x00, 0x0e, 0x01, 0x0d, 0x00, 0x02, 0x29, 0x00, 0x00,

    // END
    0x00, 0x00,
}; // total size 126 bytes 


//
// list service handle ranges
//
#define ATT_SERVICE_GAP_SERVICE_START_HANDLE 0x0001
#define ATT_SERVICE_GAP_SERVICE_END_HANDLE 0x0003
#define ATT_SERVICE_GATT_SERVICE_START_HANDLE 0x0004
#define ATT_SERVICE_GATT_SERVICE_END_HANDLE 0x0006
#define ATT_SERVICE_0000FF10_0000_1000_8000_00805F9B34FB_START_HANDLE 0x0007
#define ATT_SERVICE_0000FF10_0000_1000_8000_00805F9B34FB_END_HANDLE 0x000d

//
// list mapping between characteristics and handles
//
#define ATT_CHARACTERISTIC_GAP_DEVICE_NAME_01_VALUE_HANDLE 0x0003
#define ATT_CHARACTERISTIC_GATT_SERVICE_CHANGED_01_VALUE_HANDLE 0x0006
#define ATT_CHARACTERISTIC_0000FF11_0000_1000_8000_00805F9B34FB_01_VALUE_HANDLE 0x0009
#define ATT_CHARACTERISTIC_0000FF11_0000_1000_8000_00805F9B34FB_01_CLIENT_CONFIGURATION_HANDLE 0x000a
#define ATT_CHARACTERISTIC_0000FF12_0000_1000_8000_00805F9B34FB_01_VALUE_HANDLE 0x000c
#define ATT_CHARACTERISTIC_0000FF12_0000_1000_8000_00805F9B34FB_01_CLIENT_CONFIGURATION_HANDLE 0x000d

#define REPORT_INTERVAL_MS 3000
#define MAX_NR_CONNECTIONS 3 


static void  hci_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t* packet, uint16_t size);
static void  att_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t* packet, uint16_t size);
static int   att_write_callback(hci_con_handle_t con_handle, uint16_t att_handle, uint16_t transaction_mode, uint16_t offset, uint8_t* buffer, uint16_t buffer_size);
static void  streamer(void);

const uint8_t adv_data[] = {
    // Flags general discoverable, BR/EDR not supported
    0x02, BLUETOOTH_DATA_TYPE_FLAGS, 0x06,
    // Name
    0x0c, BLUETOOTH_DATA_TYPE_COMPLETE_LOCAL_NAME, 'L', 'E', ' ', 'S', 't', 'r', 'e', 'a', 'm', 'e', 'r',
    // Incomplete List of 16-bit Service Class UUIDs -- FF10 - only valid for testing!
    0x03, BLUETOOTH_DATA_TYPE_INCOMPLETE_LIST_OF_16_BIT_SERVICE_CLASS_UUIDS, 0x10, 0xff,
};
const uint8_t adv_data_len = sizeof(adv_data);

static btstack_packet_callback_registration_t hci_event_callback_registration;

// support for multiple clients
typedef struct {
    char name;
    int le_notification_enabled;
    uint16_t value_handle;
    hci_con_handle_t connection_handle;
    int  counter;
    char test_data[200];
    int  test_data_len;
    uint32_t test_data_sent;
    uint32_t test_data_start;
} le_streamer_connection_t;
static le_streamer_connection_t le_streamer_connections[MAX_NR_CONNECTIONS];

// round robin sending
static int connection_index;

#ifdef ENABLE_GATT_OVER_CLASSIC
static uint8_t gatt_service_buffer[70];
#endif

static void init_connections(void) {
    // track connections
    int i;
    for (i = 0; i < MAX_NR_CONNECTIONS; i++) {
        le_streamer_connections[i].connection_handle = HCI_CON_HANDLE_INVALID;
        le_streamer_connections[i].name = 'A' + i;
    }
}

static le_streamer_connection_t* connection_for_conn_handle(hci_con_handle_t conn_handle) {
    int i;
    for (i = 0; i < MAX_NR_CONNECTIONS; i++) {
        if (le_streamer_connections[i].connection_handle == conn_handle) return &le_streamer_connections[i];
    }
    return NULL;
}

static void next_connection_index(void) {
    connection_index++;
    if (connection_index == MAX_NR_CONNECTIONS) {
        connection_index = 0;
    }
}

/* @section Main Application Setup
 *
 * @text Listing MainConfiguration shows main application code.
 * It initializes L2CAP, the Security Manager, and configures the ATT Server with the pre-compiled
 * ATT Database generated from $le_streamer.gatt$. Finally, it configures the advertisements
 * and boots the Bluetooth stack.
 */

 /* LISTING_START(MainConfiguration): Init L2CAP, SM, ATT Server, and enable advertisements */

static void le_streamer_setup(void) {

    // setup le device db
    le_device_db_init();

#ifdef ENABLE_GATT_OVER_CLASSIC
    // create record for GATT and register with SDP
    memset(gatt_service_buffer, 0, sizeof(gatt_service_buffer));
    gatt_create_sdp_record(gatt_service_buffer, 0x10001, ATT_SERVICE_GATT_SERVICE_START_HANDLE, ATT_SERVICE_GATT_SERVICE_END_HANDLE);
    sdp_register_service(gatt_service_buffer);
    printf("SDP service record size: %u\n", de_get_len(gatt_service_buffer));

    // configure Classic GAP
    gap_set_local_name("GATT Streamer BR/EDR 00:00:00:00:00:00");
    gap_ssp_set_io_capability(SSP_IO_CAPABILITY_DISPLAY_YES_NO);
    gap_discoverable_control(1);
#endif

    // setup ATT server
    att_server_init(profile_data, NULL, att_write_callback);

    // register for HCI events
    hci_event_callback_registration.callback = &hci_packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);

    // register for ATT events
    att_server_register_packet_handler(att_packet_handler);

    // setup advertisements
    uint16_t adv_int_min = 0x0030;
    uint16_t adv_int_max = 0x0030;
    uint8_t adv_type = 0;
    bd_addr_t null_addr;
    memset(null_addr, 0, 6);
    gap_advertisements_set_params(adv_int_min, adv_int_max, adv_type, 0, null_addr, 0x07, 0x00);
    gap_advertisements_set_data(adv_data_len, (uint8_t*)adv_data);
    gap_advertisements_enable(1);

    // init client state
    init_connections();
}
/* LISTING_END */

/*
 * @section Track throughput
 * @text We calculate the throughput by setting a start time and measuring the amount of
 * data sent. After a configurable REPORT_INTERVAL_MS, we print the throughput in kB/s
 * and reset the counter and start time.
 */

 /* LISTING_START(tracking): Tracking throughput */

static void test_reset(le_streamer_connection_t* context) {
    context->test_data_start = btstack_run_loop_get_time_ms();
    context->test_data_sent = 0;
}

static void test_track_sent(le_streamer_connection_t* context, int bytes_sent) {
    context->test_data_sent += bytes_sent;
    // evaluate
    uint32_t now = btstack_run_loop_get_time_ms();
    uint32_t time_passed = now - context->test_data_start;
    if (time_passed < REPORT_INTERVAL_MS) return;
    // print speed
    int bytes_per_second = context->test_data_sent * 1000 / time_passed;
    printf("%c: %"PRIu32" bytes sent-> %u.%03u kB/s\n", context->name, context->test_data_sent, bytes_per_second / 1000, bytes_per_second % 1000);

    // restart
    context->test_data_start = now;
    context->test_data_sent = 0;
}
/* LISTING_END(tracking): Tracking throughput */

/*
 * @section HCI Packet Handler
 *
 * @text The packet handler is used track incoming connections and to stop notifications on disconnect
 * It is also a good place to request the connection parameter update as indicated
 * in the commented code block.
 */

 /* LISTING_START(hciPacketHandler): Packet Handler */
static void hci_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t* packet, uint16_t size) {
    UNUSED(channel);
    UNUSED(size);

    uint16_t conn_interval;
    hci_con_handle_t con_handle;
    switch (packet_type) {
    case HCI_EVENT_PACKET:
        switch (hci_event_packet_get_type(packet)) {
        case BTSTACK_EVENT_STATE:
            // BTstack activated, get started
            if (btstack_event_state_get_state(packet) == HCI_STATE_WORKING) {
                printf("To start the streaming, please run the le_streamer_client example on other device, or use some GATT Explorer, e.g. LightBlue, BLExplr.\n");
            }
            break;
        case HCI_EVENT_DISCONNECTION_COMPLETE:
            con_handle = hci_event_disconnection_complete_get_connection_handle(packet);
            printf("- LE Connection %04x: disconnect, reason %02x\n", con_handle, hci_event_disconnection_complete_get_reason(packet));
            break;
        case HCI_EVENT_LE_META:
            switch (hci_event_le_meta_get_subevent_code(packet)) {
            case HCI_SUBEVENT_LE_CONNECTION_COMPLETE:
                // print connection parameters (without using float operations)
                con_handle = hci_subevent_le_connection_complete_get_connection_handle(packet);
                conn_interval = hci_subevent_le_connection_complete_get_conn_interval(packet);
                printf("- LE Connection %04x: connected - connection interval %u.%02u ms, latency %u\n", con_handle, conn_interval * 125 / 100,
                    25 * (conn_interval & 3), hci_subevent_le_connection_complete_get_conn_latency(packet));

                // request min con interval 15 ms for iOS 11+ 
                printf("- LE Connection %04x: request 15 ms connection interval\n", con_handle);
                gap_request_connection_parameter_update(con_handle, 12, 12, 0, 0x0048);
                break;
            case HCI_SUBEVENT_LE_CONNECTION_UPDATE_COMPLETE:
                // print connection parameters (without using float operations)
                con_handle = hci_subevent_le_connection_update_complete_get_connection_handle(packet);
                conn_interval = hci_subevent_le_connection_update_complete_get_conn_interval(packet);
                printf("- LE Connection %04x: connection update - connection interval %u.%02u ms, latency %u\n", con_handle, conn_interval * 125 / 100,
                    25 * (conn_interval & 3), hci_subevent_le_connection_update_complete_get_conn_latency(packet));
                break;
            default:
                break;
            }
            break;
        default:
            break;
        }
    }
}

/* LISTING_END */

/*
 * @section ATT Packet Handler
 *
 * @text The packet handler is used to track the ATT MTU Exchange and trigger ATT send
 */

 /* LISTING_START(attPacketHandler): Packet Handler */
static void att_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t* packet, uint16_t size) {
    UNUSED(channel);
    UNUSED(size);

    int mtu;
    le_streamer_connection_t* context;
    switch (packet_type) {
    case HCI_EVENT_PACKET:
        switch (hci_event_packet_get_type(packet)) {
        case ATT_EVENT_CONNECTED:
            // setup new 
            context = connection_for_conn_handle(HCI_CON_HANDLE_INVALID);
            if (!context) break;
            context->counter = 'A';
            context->connection_handle = att_event_connected_get_handle(packet);
            context->test_data_len = btstack_min(att_server_get_mtu(context->connection_handle) - 3, sizeof(context->test_data));
            printf("%c: ATT connected, handle %04x, test data len %u\n", context->name, context->connection_handle, context->test_data_len);
            break;
        case ATT_EVENT_MTU_EXCHANGE_COMPLETE:
            mtu = att_event_mtu_exchange_complete_get_MTU(packet) - 3;
            context = connection_for_conn_handle(att_event_mtu_exchange_complete_get_handle(packet));
            if (!context) break;
            context->test_data_len = btstack_min(mtu - 3, sizeof(context->test_data));
            printf("%c: ATT MTU = %u => use test data of len %u\n", context->name, mtu, context->test_data_len);
            break;
        case ATT_EVENT_CAN_SEND_NOW:
            streamer();
            break;
        case ATT_EVENT_DISCONNECTED:
            context = connection_for_conn_handle(att_event_disconnected_get_handle(packet));
            if (!context) break;
            // free connection
            printf("%c: ATT disconnected, handle %04x\n", context->name, context->connection_handle);
            context->le_notification_enabled = 0;
            context->connection_handle = HCI_CON_HANDLE_INVALID;
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
}
/* LISTING_END */

/*
 * @section Streamer
 *
 * @text The streamer function checks if notifications are enabled and if a notification can be sent now.
 * It creates some test data - a single letter that gets increased every time - and tracks the data sent.
 */

 /* LISTING_START(streamer): Streaming code */
static void streamer(void) {

    // find next active streaming connection
    int old_connection_index = connection_index;
    while (1) {
        // active found?
        if ((le_streamer_connections[connection_index].connection_handle != HCI_CON_HANDLE_INVALID) &&
            (le_streamer_connections[connection_index].le_notification_enabled)) break;

        // check next
        next_connection_index();

        // none found
        if (connection_index == old_connection_index) return;
    }

    le_streamer_connection_t* context = &le_streamer_connections[connection_index];

    // create test data
    context->counter++;
    if (context->counter > 'Z') context->counter = 'A';
    memset(context->test_data, context->counter, context->test_data_len);

    // send
    att_server_notify(context->connection_handle, context->value_handle, (uint8_t*)context->test_data, context->test_data_len);

    // track
    test_track_sent(context, context->test_data_len);

    // request next send event
    att_server_request_can_send_now_event(context->connection_handle);

    // check next
    next_connection_index();
}
/* LISTING_END */

/*
 * @section ATT Write
 *
 * @text The only valid ATT write in this example is to the Client Characteristic Configuration, which configures notification
 * and indication. If the ATT handle matches the client configuration handle, the new configuration value is stored.
 * If notifications get enabled, an ATT_EVENT_CAN_SEND_NOW is requested. See Listing attWrite.
 */

 /* LISTING_START(attWrite): ATT Write */
static int att_write_callback(hci_con_handle_t con_handle, uint16_t att_handle, uint16_t transaction_mode, uint16_t offset, uint8_t* buffer, uint16_t buffer_size) {
    UNUSED(offset);

    // printf("att_write_callback att_handle %04x, transaction mode %u\n", att_handle, transaction_mode);
    if (transaction_mode != ATT_TRANSACTION_MODE_NONE) return 0;
    le_streamer_connection_t* context = connection_for_conn_handle(con_handle);
    switch (att_handle) {
    case ATT_CHARACTERISTIC_0000FF11_0000_1000_8000_00805F9B34FB_01_CLIENT_CONFIGURATION_HANDLE:
    case ATT_CHARACTERISTIC_0000FF12_0000_1000_8000_00805F9B34FB_01_CLIENT_CONFIGURATION_HANDLE:
        context->le_notification_enabled = little_endian_read_16(buffer, 0) == GATT_CLIENT_CHARACTERISTICS_CONFIGURATION_NOTIFICATION;
        printf("%c: Notifications enabled %u\n", context->name, context->le_notification_enabled);
        if (context->le_notification_enabled) {
            switch (att_handle) {
            case ATT_CHARACTERISTIC_0000FF11_0000_1000_8000_00805F9B34FB_01_CLIENT_CONFIGURATION_HANDLE:
                context->value_handle = ATT_CHARACTERISTIC_0000FF11_0000_1000_8000_00805F9B34FB_01_VALUE_HANDLE;
                break;
            case ATT_CHARACTERISTIC_0000FF12_0000_1000_8000_00805F9B34FB_01_CLIENT_CONFIGURATION_HANDLE:
                context->value_handle = ATT_CHARACTERISTIC_0000FF12_0000_1000_8000_00805F9B34FB_01_VALUE_HANDLE;
                break;
            default:
                break;
            }
            att_server_request_can_send_now_event(context->connection_handle);
        }
        test_reset(context);
        break;
    case ATT_CHARACTERISTIC_0000FF11_0000_1000_8000_00805F9B34FB_01_VALUE_HANDLE:
    case ATT_CHARACTERISTIC_0000FF12_0000_1000_8000_00805F9B34FB_01_VALUE_HANDLE:
        test_track_sent(context, buffer_size);
        break;
    default:
        printf("Write to 0x%04x, len %u\n", att_handle, buffer_size);
    }
    return 0;
}
/* LISTING_END */

int ble_setup(int argc, const char* argv[])
{
    le_streamer_setup();
    return 0;
}
/* EXAMPLE_END */