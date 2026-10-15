#ifndef XW_CORE_H
#define XW_CORE_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define XW_RADIO_COUNT 4
#define XW_VIRTUAL_COUNT 3
#define XW_VIRTUAL_HANDLE_BASE 1001
#define XW_NEUTRAL_TIMEOUT_MS 2500u

/* Identifiers found in xpad/Linux. These do not cover every aftermarket receiver. */
#define XW_MICROSOFT_VID 0x045e
#define XW_RECEIVER_PID_OFFICIAL 0x0719
#define XW_RECEIVER_PID_XBOX 0x0291
#define XW_RECEIVER_PID_UNOFFICIAL 0x02a9

/* A separate USB interface + interrupt IN endpoint for each radio connection. */
typedef struct {
    uint8_t interface_number;
    uint8_t endpoint_in;
    uint8_t endpoint_out;
    uint8_t found;
} XWUsbChannel;

/* Buttons translated to generic DS4 semantics. Not an OrbisPadData ABI struct. */
enum {
    XW_BTN_CROSS   = (1u << 0), XW_BTN_CIRCLE   = (1u << 1),
    XW_BTN_SQUARE  = (1u << 2), XW_BTN_TRIANGLE = (1u << 3),
    XW_BTN_L1      = (1u << 4), XW_BTN_R1       = (1u << 5),
    XW_BTN_L3      = (1u << 6), XW_BTN_R3       = (1u << 7),
    XW_BTN_OPTIONS = (1u << 8), XW_BTN_SHARE    = (1u << 9),
    XW_BTN_PS      = (1u << 10),
    XW_BTN_UP      = (1u << 11), XW_BTN_DOWN    = (1u << 12),
    XW_BTN_LEFT    = (1u << 13), XW_BTN_RIGHT   = (1u << 14)
};

typedef struct {
    uint8_t connected;
    uint32_t buttons;
    uint8_t l2, r2;
    uint8_t left_x, left_y, right_x, right_y;
} XWPadFrame;

typedef struct {
    uint8_t connected;
    uint8_t has_input;
    uint64_t input_time_ms;
    XWPadFrame last;
} XWRadio;

typedef struct {
    XWRadio radios[XW_RADIO_COUNT];
    int8_t virtual_to_radio[XW_VIRTUAL_COUNT];
} XWReceiver;

typedef enum {
    XW_PACKET_INVALID = -1,
    XW_PACKET_IGNORED = 0,
    XW_PACKET_CONNECTED = 1,
    XW_PACKET_DISCONNECTED = 2,
    XW_PACKET_INPUT = 3
} XWPacketResult;

int xw_is_supported_receiver(uint16_t vid, uint16_t pid);
void xw_init(XWReceiver *receiver);
XWPacketResult xw_receive(XWReceiver *receiver, unsigned radio_slot,
                           const uint8_t *packet, size_t packet_size,
                           uint64_t time_ms);
void xw_reconcile(XWReceiver *receiver);
int xw_radio_for_virtual(const XWReceiver *receiver, unsigned virtual_slot);
int xw_virtual_handle(unsigned virtual_slot);
int xw_virtual_slot_from_handle(int handle);
XWPadFrame xw_frame_for_virtual(const XWReceiver *receiver, unsigned virtual_slot,
                                uint64_t now_ms);

/* Parse standard USB configuration descriptor bytes; no libusb dependency.
   Caller supplies the entire wTotalLength buffer. Returns channels found (0..4),
   or -1 on malformed descriptors. Does not claim USB interfaces. */
int xw_parse_configuration(const uint8_t *config, size_t length,
                            XWUsbChannel channels[XW_RADIO_COUNT]);

#ifdef __cplusplus
}
#endif
#endif
