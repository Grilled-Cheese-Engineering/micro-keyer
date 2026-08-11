/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, i Comprehensive conditions...
 */

#ifndef _TUSB_CONFIG_H_
#define _TUSB_CONFIG_H_

#ifdef __cplusplus
extern "C" {
    #endif

    //--------------------------------------------------------------------+
    // Board Specific Configuration
    //--------------------------------------------------------------------+
    #ifndef BOARD_TUD_RHPORT
    #define BOARD_TUD_RHPORT      0
    #endif

    #ifndef BOARD_TUD_MAX_SPEED
    #define BOARD_TUD_MAX_SPEED   OPT_MODE_DEFAULT_SPEED
    #endif

    //--------------------------------------------------------------------+
    // COMMON CONFIGURATION
    //--------------------------------------------------------------------+
    #ifndef CFG_TUSB_MCU
    #error CFG_TUSB_MCU must be defined
    #endif

    #if CFG_TUSB_MCU == OPT_MCU_LPC18XX || CFG_TUSB_MCU == OPT_MCU_LPC43XX || CFG_TUSB_MCU == OPT_MCU_MIMXRT10XX || \
    CFG_TUSB_MCU == OPT_MCU_NUC505 || CFG_TUSB_MCU == OPT_MCU_CXD56
    #define CFG_TUSB_RHPORT0_MODE     (OPT_MODE_DEVICE | OPT_MODE_HIGH_SPEED)
    #else
    #define CFG_TUSB_RHPORT0_MODE     OPT_MODE_DEVICE
    #endif

    #ifndef CFG_TUSB_OS
    #define CFG_TUSB_OS                 OPT_OS_PICO
    #endif

    #ifndef CFG_TUSB_DEBUG
    #define CFG_TUSB_DEBUG        0
    #endif

    #define CFG_TUD_ENABLED       1
    #define CFG_TUD_MAX_SPEED     BOARD_TUD_MAX_SPEED

    #ifndef CFG_TUSB_MEM_SECTION
    #define CFG_TUSB_MEM_SECTION
    #endif

    #ifndef CFG_TUSB_MEM_ALIGN
    #define CFG_TUSB_MEM_ALIGN          __attribute__ ((aligned(4)))
    #endif

     //--------------------------------------------------------------------+
     // DEVICE CONFIGURATION
     //--------------------------------------------------------------------+
    #ifndef CFG_TUD_ENDPOINT0_SIZE
    #define CFG_TUD_ENDPOINT0_SIZE    64
    #endif

    //------------- CLASS -------------//
    #define CFG_TUD_HID               1
    #define CFG_TUD_CDC               0
    #define CFG_TUD_MSC               0
    #define CFG_TUD_MIDI              1
    #define CFG_TUD_VENDOR            0

    // HID buffer size Should be sufficient to hold ID (if any) + Data
    #define CFG_TUD_HID_EP_BUFSIZE    16
    #define CFG_TUD_MIDI_RX_BUFSIZE   (TUD_OPT_HIGH_SPEED ? 512 : 64)
    #define CFG_TUD_MIDI_TX_BUFSIZE   (TUD_OPT_HIGH_SPEED ? 512 : 64)

    #ifdef __cplusplus
}
#endif

#endif /* _TUSB_CONFIG_H_ */