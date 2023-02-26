#ifndef _TUSB_CONFIG_H_
#define _TUSB_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------------------------------------------+
// Board Specific Configuration
//--------------------------------------------------------------------+

// RHPort number used for device can be defined by board.mk, default to port 0
#ifndef BOARD_TUD_RHPORT
#define BOARD_TUD_RHPORT (0)
#endif

// RHPort max operational speed can defined by board.mk
#ifndef BOARD_TUD_MAX_SPEED
#define BOARD_TUD_MAX_SPEED OPT_MODE_DEFAULT_SPEED
#endif

#define CFG_TUSB_RHPORT0_MODE OPT_MODE_DEVICE

//--------------------------------------------------------------------
// COMMON CONFIGURATION
//--------------------------------------------------------------------

#ifndef CFG_TUSB_DEBUG
#define CFG_TUSB_DEBUG (0)
#endif

// Enable Device stack
#define CFG_TUD_ENABLED (1)

// Default is max speed that hardware controller could support with on-chip PHY
#define CFG_TUD_MAX_SPEED BOARD_TUD_MAX_SPEED

//--------------------------------------------------------------------
// DEVICE CONFIGURATION
//--------------------------------------------------------------------

#ifndef CFG_TUD_ENDPOINT0_SIZE
#define CFG_TUD_ENDPOINT0_SIZE (64)
#endif

//------------- CLASS -------------//
#define CFG_TUD_CDC (1)
#define CFG_TUD_MSC (0)
#define CFG_TUD_MIDI (1)
#define CFG_TUD_HID (1)
#define CFG_TUD_VENDOR (0)

// Device class buffer sizes
#define CFG_TUD_CDC_RX_BUFSIZE (TUD_OPT_HIGH_SPEED ? 512 : 64)
#define CFG_TUD_CDC_TX_BUFSIZE (TUD_OPT_HIGH_SPEED ? 512 : 64)
#define CFG_TUD_CDC_EP_BUFSIZE (TUD_OPT_HIGH_SPEED ? 512 : 64)

#define CFG_TUD_HID_EP_BUFSIZE (64)

#define CFG_TUD_MIDI_RX_BUFSIZE (TUD_OPT_HIGH_SPEED ? 512 : 64)
#define CFG_TUD_MIDI_TX_BUFSIZE (TUD_OPT_HIGH_SPEED ? 512 : 64)
#define CFG_TUD_MIDI_EP_BUFSIZE (TUD_OPT_HIGH_SPEED ? 512 : 64)

#ifdef __cplusplus
}
#endif

#endif // _TUSB_CONFIG_H_