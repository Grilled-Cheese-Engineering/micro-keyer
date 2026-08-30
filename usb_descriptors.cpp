#include "tusb.h"
#include "pico/unique_id.h"
#include <string.h>

#define USB_VID 0xCafe
#define USB_BCD 0x0200

extern "C" bool USBMode[3];

//--------------------------------------------------------------------+
// Device Descriptor
//--------------------------------------------------------------------+

#define TUD_MIDI_DESC_IAD_LEN 8



tusb_desc_device_t const desc_device = {
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = USB_BCD,
    .bDeviceClass = TUSB_CLASS_MISC,
    .bDeviceSubClass = MISC_SUBCLASS_COMMON,
    .bDeviceProtocol = MISC_PROTOCOL_IAD,
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor = USB_VID,
    .idProduct = 0x4045, // Reset ID to bypass cached host OS configurations
    .bcdDevice = 0x0100,
    .iManufacturer = 0x01,
    .iProduct = 0x02,
    .iSerialNumber = 0x03,
    .bNumConfigurations = 0x01
};

//--------------------------------------------------------------------+
// HID Report Descriptor
//--------------------------------------------------------------------+
uint8_t const desc_hid_report[] = {
    TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(1)),
};

//--------------------------------------------------------------------+
// Configuration Descriptor Map (MIDI First to satisfy TinyUSB offsets)
//--------------------------------------------------------------------+
enum {
  ITF_NUM_MIDI = 0,
  ITF_NUM_MIDI_STREAMING,
  ITF_NUM_CDC,
  ITF_NUM_CDC_DATA,
  ITF_NUM_HID,
  ITF_NUM_TOTAL
};

// Explicit hardware endpoint routing configurations
#define EP_MIDI_OUT   0x01  // Endpoint 1 (OUT)
#define EP_MIDI_IN    0x81  // Endpoint 1 (IN)
#define EP_CDC_NOTIF  0x82  // Endpoint 2 (IN)
#define EP_CDC_OUT    0x03  // Endpoint 3 (OUT)
#define EP_CDC_IN     0x83  // Endpoint 3 (IN)
#define EP_HID_IN     0x84  // Endpoint 4 (IN)


#define CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_AUDIO_DESC_IAD_LEN + TUD_MIDI_DESC_LEN + TUD_CDC_DESC_LEN + TUD_HID_DESC_LEN)
uint8_t const desc_configuration[] = {
  // 1. Core Header Configuration Block
  TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, 0x00, 100),

  // 2. Audio/MIDI Interface Association Descriptor (IAD) Block
  // FIXED: Using direct hex values for Subclass (0x03) and Protocol (0x00)
// Change the last number to 0
  8, TUSB_DESC_INTERFACE_ASSOCIATION, ITF_NUM_MIDI, 2, TUSB_CLASS_AUDIO, 0x03, 0x00, 0,

  // Change the second argument to 0
  TUD_MIDI_DESCRIPTOR(ITF_NUM_MIDI, 0, EP_MIDI_OUT, EP_MIDI_IN, 64),

  // 4. CDC Class Block (Interfaces 2 and 3)
  TUD_CDC_DESCRIPTOR(ITF_NUM_CDC, 0, EP_CDC_NOTIF, 8, EP_CDC_OUT, EP_CDC_IN, 64),

  // 5. HID Class Block (Interface 4)
  TUD_HID_DESCRIPTOR(ITF_NUM_HID, 0, HID_ITF_PROTOCOL_NONE, sizeof(desc_hid_report), EP_HID_IN, 16, 5)
};

//--------------------------------------------------------------------+
// String Descriptors
//--------------------------------------------------------------------+
enum {
  STRID_LANGID = 0,
  STRID_MANUFACTURER,
  STRID_PRODUCT,
  STRID_SERIAL,
  STRID_CDC,
  STRID_MIDI,
  STRID_HID
};

char const* string_desc_arr[] = {
    (const char[]) {
 0x09, 0x04
},
"GCE",
"Micro Keyer",
NULL,
"Pico Virtual COM Port",
"Pico MIDI Sound Port",
"Pico Keyboard Input"
};

static uint16_t _desc_str[32 + 1];

//--------------------------------------------------------------------+
// External Safe C-Linker Binding Declarations
//--------------------------------------------------------------------+
extern "C" {

  uint8_t const* tud_descriptor_device_cb(void) {
    return (uint8_t const*)&desc_device;
  }

  uint8_t const* tud_hid_descriptor_report_cb(uint8_t instance) {
    (void)instance;
    return desc_hid_report;
  }

  uint8_t const* tud_descriptor_configuration_cb(uint8_t index) {
    (void)index;
    return desc_configuration;
  }

  void tud_midi_packet_write_pool_cb(uint8_t cable_num, uint8_t const packet) {
    (void)cable_num;
    (void)packet;
  }

  uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
    (void)langid;
    size_t chr_count;

    if (index == STRID_LANGID) {
      memcpy(&_desc_str[1], string_desc_arr[0], 2);
      chr_count = 1;
    } else if (index == STRID_SERIAL) {
      char serial_str[17]; // FIXED: Native 17-byte buffer array allocation explicitly declared
      pico_get_unique_board_id_string(serial_str, sizeof(serial_str));
      chr_count = strlen(serial_str);
      for (size_t i = 0; i < chr_count; i++) {
        _desc_str[1 + i] = serial_str[i];
      }
    } else {
      if (index >= sizeof(string_desc_arr) / sizeof(string_desc_arr[0])) return NULL;

      const char* str = string_desc_arr[index];
      chr_count = strlen(str);
      size_t const max_count = sizeof(_desc_str) / sizeof(_desc_str[0]) - 1;
      if (chr_count > max_count) chr_count = max_count;

      for (size_t i = 0; i < chr_count; i++) {
        _desc_str[1 + i] = str[i];
      }
    }

    // FIXED: Explicitly assigning metadata block down into index zero of array structure
    _desc_str[0] = (uint16_t)((TUSB_DESC_STRING << 8) | (2 * chr_count + 2));
    return _desc_str;
  }

  uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen) {
    (void)instance; (void)report_id; (void)report_type; (void)buffer; (void)reqlen;
    return 0;
  }

  void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize) {
    (void)instance; (void)report_id; (void)report_type; (void)buffer; (void)bufsize;
  }

} // extern "C"
