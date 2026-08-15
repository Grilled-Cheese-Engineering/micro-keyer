#include "tusb.h"
#include "pico/unique_id.h"
#include "display.h"

#define USB_VID   0xCafe
#define USB_BCD   0x0200

//--------------------------------------------------------------------+
// Device Descriptors
//--------------------------------------------------------------------+
// CDC needs a specific device class to load properly on all OSes (IAD)
tusb_desc_device_t const desc_device_cdc = {
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = USB_BCD,
    .bDeviceClass = TUSB_CLASS_MISC,
    .bDeviceSubClass = MISC_SUBCLASS_COMMON,
    .bDeviceProtocol = MISC_PROTOCOL_IAD,
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor = USB_VID,
    .idProduct = 0x4001,
    .bcdDevice = 0x0100,
    .iManufacturer = 0x01,
    .iProduct = 0x02,
    .iSerialNumber = 0x03,
    .bNumConfigurations = 0x01
};

// HID and MIDI can use the standard 0x00 device class
tusb_desc_device_t const desc_device_other = {
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = USB_BCD,
    .bDeviceClass = 0x00,
    .bDeviceSubClass = 0x00,
    .bDeviceProtocol = 0x00,
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor = USB_VID,
    .idProduct = 0x4002,
    .bcdDevice = 0x0100,
    .iManufacturer = 0x01,
    .iProduct = 0x02,
    .iSerialNumber = 0x03,
    .bNumConfigurations = 0x01
};

uint8_t const* tud_descriptor_device_cb(void) {
  if (USBMode == MODE_CDC_SERIAL) {
    return (uint8_t const*)&desc_device_cdc;
  }
  return (uint8_t const*)&desc_device_other;
}

//--------------------------------------------------------------------+
// HID Report Descriptor
//--------------------------------------------------------------------+

enum {
  REPORT_ID_KEYBOARD = 1,
  REPORT_ID_MOUSE // Optional, but good to have if you expand later
};

uint8_t const desc_hid_report[] = {
    TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(REPORT_ID_KEYBOARD)),
};

uint8_t const* tud_hid_descriptor_report_cb(uint8_t instance) {
  (void)instance;
  return desc_hid_report;
}

//--------------------------------------------------------------------+
// Configuration Descriptors
//--------------------------------------------------------------------+

// --- 1. CDC CONFIGURATION ---
#define CONFIG_TOTAL_LEN_CDC    (TUD_CONFIG_DESC_LEN + TUD_CDC_DESC_LEN)
#define EPNUM_CDC_NOTIF   0x81
#define EPNUM_CDC_OUT     0x02
#define EPNUM_CDC_IN      0x82

uint8_t const desc_configuration_cdc[] = {
    TUD_CONFIG_DESCRIPTOR(1, 2, 0, CONFIG_TOTAL_LEN_CDC, 0x00, 100),
    TUD_CDC_DESCRIPTOR(0, 4, EPNUM_CDC_NOTIF, 8, EPNUM_CDC_OUT, EPNUM_CDC_IN, 64)
};

// --- 2. HID CONFIGURATION ---
#define CONFIG_TOTAL_LEN_HID    (TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN)
#define EPNUM_HID         0x81

uint8_t const desc_configuration_hid[] = {
    TUD_CONFIG_DESCRIPTOR(1, 1, 0, CONFIG_TOTAL_LEN_HID, 0x00, 100),
    TUD_HID_DESCRIPTOR(0, 0, HID_ITF_PROTOCOL_NONE, sizeof(desc_hid_report), EPNUM_HID, 16, 5)
};

// --- 3. MIDI CONFIGURATION ---
#define CONFIG_TOTAL_LEN_MIDI   (TUD_CONFIG_DESC_LEN + TUD_MIDI_DESC_LEN)
#define EPNUM_MIDI_OUT    0x01
#define EPNUM_MIDI_IN     0x81

uint8_t const desc_configuration_midi[] = {
    TUD_CONFIG_DESCRIPTOR(1, 2, 0, CONFIG_TOTAL_LEN_MIDI, 0x00, 100),
    TUD_MIDI_DESCRIPTOR(0, 0, EPNUM_MIDI_OUT, EPNUM_MIDI_IN, 64)
};

// --- CONFIGURATION CALLBACK ---
uint8_t const* tud_descriptor_configuration_cb(uint8_t index) {
  (void)index;
  switch (USBMode) {
  case MODE_CDC_SERIAL: return desc_configuration_cdc;
  case MODE_HID:        return desc_configuration_hid;
  case MODE_MIDI:       return desc_configuration_midi;
  }
  return NULL;
}

//--------------------------------------------------------------------+
// String Descriptors
//--------------------------------------------------------------------+
enum {
  STRID_LANGID = 0,
  STRID_MANUFACTURER,
  STRID_PRODUCT,
  STRID_SERIAL,
};

char const* string_desc_arr[] = {
    (const char[]) {
 0x09, 0x04
}, // 0: Supported language is English (0x0409)
"TinyUSB",                     // 1: Manufacturer
"Pico Keyer",                  // 2: Product
NULL,                          // 3: Serial
};

static uint16_t _desc_str[32 + 1];

uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
  (void)langid;
  size_t chr_count;

  switch (index) {
  case STRID_LANGID:
    memcpy(&_desc_str[1], string_desc_arr[0], 2);
    chr_count = 1;
    break;

  case STRID_SERIAL: {
    char serial_str[17]; // Pico IDs are 16 hex chars + null terminator
    pico_get_unique_board_id_string(serial_str, sizeof(serial_str));

    chr_count = strlen(serial_str);
    for (size_t i = 0; i < chr_count; i++) {
      _desc_str[1 + i] = serial_str[i];
    }
    break;
  }

  default:
    if (!(index < sizeof(string_desc_arr) / sizeof(string_desc_arr[0]))) return NULL;

    const char* str = string_desc_arr[index];
    chr_count = strlen(str);
    size_t const max_count = sizeof(_desc_str) / sizeof(_desc_str[0]) - 1;
    if (chr_count > max_count) chr_count = max_count;

    for (size_t i = 0; i < chr_count; i++) {
      _desc_str[1 + i] = str[i];
    }
    break;
  }

  _desc_str[0] = (uint16_t)((TUSB_DESC_STRING << 8) | (2 * chr_count + 2));
  return _desc_str;
}

//--------------------------------------------------------------------+
// HID Callbacks (Mandatory for TinyUSB to compile, even if empty)
//--------------------------------------------------------------------+
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen) {
  (void)instance; (void)report_id; (void)report_type; (void)buffer; (void)reqlen;
  return 0;
}

void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize) {
  (void)instance; (void)report_id; (void)report_type; (void)buffer; (void)bufsize;
}