#include "xw_core.h"
#include <string.h>

static XWPadFrame neutral_frame(uint8_t connected) {
    XWPadFrame f;
    memset(&f, 0, sizeof(f));
    f.connected = connected;
    f.left_x = f.left_y = f.right_x = f.right_y = 128;
    return f;
}

int xw_is_supported_receiver(uint16_t vid, uint16_t pid) {
    if (vid != XW_MICROSOFT_VID) return 0;
    return pid == XW_RECEIVER_PID_OFFICIAL ||
           pid == XW_RECEIVER_PID_XBOX ||
           pid == XW_RECEIVER_PID_UNOFFICIAL;
}

void xw_init(XWReceiver *r) {
    unsigned i;
    if (!r) return;
    memset(r, 0, sizeof(*r));
    for (i = 0; i < XW_VIRTUAL_COUNT; ++i) r->virtual_to_radio[i] = -1;
    for (i = 0; i < XW_RADIO_COUNT; ++i) r->radios[i].last = neutral_frame(0);
}

static int16_t le16s(const uint8_t *p) {
    return (int16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8));
}

/* nearest-integer mapping maps the Xbox centre position 0 -> 128 */
static uint8_t axis_to_8(int16_t value, int reverse) {
    int32_t v = reverse ? -(int32_t)value : (int32_t)value;
    if (v > 32767) v = 32767;
    uint32_t u = (uint32_t)(v + 32768);
    return (uint8_t)((u * 255u + 32767u) / 65535u);
}

static XWPadFrame decode_x360(const uint8_t *b) {
    XWPadFrame f = neutral_frame(1);
    const uint8_t d = b[2], buttons = b[3];
    if (d & 0x01) f.buttons |= XW_BTN_UP;
    if (d & 0x02) f.buttons |= XW_BTN_DOWN;
    if (d & 0x04) f.buttons |= XW_BTN_LEFT;
    if (d & 0x08) f.buttons |= XW_BTN_RIGHT;
    if (d & 0x10) f.buttons |= XW_BTN_OPTIONS;
    if (d & 0x20) f.buttons |= XW_BTN_SHARE;
    if (d & 0x40) f.buttons |= XW_BTN_L3;
    if (d & 0x80) f.buttons |= XW_BTN_R3;
    if (buttons & 0x01) f.buttons |= XW_BTN_L1;
    if (buttons & 0x02) f.buttons |= XW_BTN_R1;
    if (buttons & 0x04) f.buttons |= XW_BTN_PS;
    if (buttons & 0x10) f.buttons |= XW_BTN_CROSS;
    if (buttons & 0x20) f.buttons |= XW_BTN_CIRCLE;
    if (buttons & 0x40) f.buttons |= XW_BTN_SQUARE;
    if (buttons & 0x80) f.buttons |= XW_BTN_TRIANGLE;
    f.l2 = b[4]; f.r2 = b[5];
    f.left_x  = axis_to_8(le16s(b + 6), 0);
    f.left_y  = axis_to_8(le16s(b + 8), 1);
    f.right_x = axis_to_8(le16s(b + 10), 0);
    f.right_y = axis_to_8(le16s(b + 12), 1);
    return f;
}

XWPacketResult xw_receive(XWReceiver *r, unsigned slot,
                           const uint8_t *packet, size_t len,
                           uint64_t time_ms) {
    XWRadio *radio;
    const uint8_t *body;
    if (!r || !packet || slot >= XW_RADIO_COUNT || len < 2)
        return XW_PACKET_INVALID;
    radio = &r->radios[slot];

    /* xpad: byte 0 bit 3 signals a radio/headset presence update.
       byte 1 bit 7 indicates a controller is present (not just a headset). */
    if ((packet[0] & 0x08u) != 0) {
        int connected = (packet[1] & 0x80u) != 0;
        if (!connected) {
            radio->connected = 0;
            radio->has_input = 0;
            radio->last = neutral_frame(0);
            xw_reconcile(r);
            return XW_PACKET_DISCONNECTED;
        }
        radio->connected = 1;
        radio->has_input = 0;
        radio->last = neutral_frame(1);
        xw_reconcile(r);
        return XW_PACKET_CONNECTED;
    }

    /* Wireless receiver's four-byte wrapper must be removed before parsing
       the standard Xbox 360 20-byte controller report. */
    if (packet[1] != 0x01u) return XW_PACKET_IGNORED;
    if (len < 18) return XW_PACKET_INVALID; /* 4 wrapper + 14 used fields */
    body = packet + 4;
    if (body[0] != 0x00u || (body[1] != 0x13u && body[1] != 0x14u))
        return XW_PACKET_IGNORED;

    /* If loaded after pairing, receiving valid data is enough to mark present. */
    radio->connected = 1;
    radio->last = decode_x360(body);
    radio->has_input = 1;
    radio->input_time_ms = time_ms;
    xw_reconcile(r);
    return XW_PACKET_INPUT;
}

void xw_reconcile(XWReceiver *r) {
    unsigned i, radio;
    if (!r) return;
    for (i = 0; i < XW_VIRTUAL_COUNT; ++i) {
        int slot = r->virtual_to_radio[i];
        if (slot >= 0 && (slot >= XW_RADIO_COUNT || !r->radios[slot].connected))
            r->virtual_to_radio[i] = -1;
    }
    for (radio = 0; radio < XW_RADIO_COUNT; ++radio) {
        int assigned = 0;
        if (!r->radios[radio].connected) continue;
        for (i = 0; i < XW_VIRTUAL_COUNT; ++i)
            if (r->virtual_to_radio[i] == (int)radio) assigned = 1;
        if (assigned) continue;
        for (i = 0; i < XW_VIRTUAL_COUNT; ++i) {
            if (r->virtual_to_radio[i] == -1) {
                r->virtual_to_radio[i] = (int8_t)radio;
                break;
            }
        }
    }
}

int xw_radio_for_virtual(const XWReceiver *r, unsigned i) {
    if (!r || i >= XW_VIRTUAL_COUNT) return -1;
    return r->virtual_to_radio[i];
}
int xw_virtual_handle(unsigned i) {
    return i < XW_VIRTUAL_COUNT ? XW_VIRTUAL_HANDLE_BASE + (int)i : -1;
}
int xw_virtual_slot_from_handle(int handle) {
    /* Check bounds before subtracting: PS4 error handles may be INT_MIN.
       Signed overflow here would otherwise be undefined behaviour. */
    if (handle < XW_VIRTUAL_HANDLE_BASE ||
        handle >= XW_VIRTUAL_HANDLE_BASE + XW_VIRTUAL_COUNT) return -1;
    return handle - XW_VIRTUAL_HANDLE_BASE;
}
XWPadFrame xw_frame_for_virtual(const XWReceiver *r, unsigned virtual_slot,
                                uint64_t now_ms) {
    int radio_slot = xw_radio_for_virtual(r, virtual_slot);
    const XWRadio *radio;
    if (radio_slot < 0) return neutral_frame(0);
    radio = &r->radios[radio_slot];
    if (!radio->connected) return neutral_frame(0);
    if (!radio->has_input || now_ms < radio->input_time_ms ||
        (now_ms - radio->input_time_ms) > XW_NEUTRAL_TIMEOUT_MS)
        return neutral_frame(1); /* fail-safe: no stuck held buttons */
    return radio->last;
}

/* Parse the USB config's TLV descriptors; actual interface numbers come from
   descriptor bytes rather than hard-coding them. For an official receiver,
   slots normally occur on interfaces 0,2,4,6; endpoints 81,83,85,87. */
int xw_parse_configuration(const uint8_t *buf, size_t len,
                            XWUsbChannel channels[XW_RADIO_COUNT]) {
    size_t p = 0;
    int current_slot = -1;
    int found = 0;
    unsigned i;
    if (!buf || !channels || len < 9 || buf[1] != 0x02) return -1;
    for (i = 0; i < XW_RADIO_COUNT; ++i) memset(&channels[i], 0, sizeof(channels[i]));
    /* Keep to wTotalLength of configuration, if caller supplied more bytes. */
    size_t expected = (size_t)buf[2] | ((size_t)buf[3] << 8);
    if (expected < 9 || expected > len) return -1;
    len = expected;
    while (p < len) {
        uint8_t size, kind;
        if (len - p < 2) return -1;
        size = buf[p]; kind = buf[p + 1];
        if (size < 2 || size > len - p) return -1;
        if (kind == 0x04) { /* USB interface descriptor */
            current_slot = -1;
            if (size < 9) return -1;
            if (buf[p + 3] == 0 &&  /* alternate setting 0 */
                buf[p + 5] == 0xff && buf[p + 6] == 0x5d &&
                buf[p + 7] == 0x81 && found < XW_RADIO_COUNT) {
                current_slot = found++;
                channels[current_slot].interface_number = buf[p + 2];
            }
        } else if (kind == 0x05 && current_slot >= 0) { /* endpoint */
            if (size < 7) return -1;
            uint8_t ep = buf[p + 2];
            if ((buf[p + 3] & 3u) == 3u) { /* interrupt */
                if (ep & 0x80u) channels[current_slot].endpoint_in = ep;
                else channels[current_slot].endpoint_out = ep;
            }
        }
        p += size;
    }
    /* Only use channels that have both IN/OUT endpoints. */
    for (i = 0; i < (unsigned)found; ++i)
        channels[i].found = channels[i].endpoint_in && channels[i].endpoint_out;
    return found;
}
