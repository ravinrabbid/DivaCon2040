#ifndef _USB_USB_DRIVER_H_
#define _USB_USB_DRIVER_H_

#include <stdint.h>

#define USBD_MANUFACTURER "Project DivaCon"
#define USBD_PRODUCT "DivaCon rev1"

#define USBD_MAX_POWER_MAX (500)

#ifdef __cplusplus
extern "C" {
#endif

enum {
    USBD_STR_LANGUAGE,
    USBD_STR_MANUFACTURER,
    USBD_STR_PRODUCT,
    USBD_STR_SERIAL,
    USBD_STR_CDC,
    USBD_STR_XINPUT,
    USBD_STR_DIRECTINPUT,
    USBD_STR_SWITCH,
    USBD_STR_RPI_RESET,
};

typedef enum {
    USB_MODE_DIRECTINPUT,
    USB_MODE_SWITCH,
    USB_MODE_XINPUT,
    USB_MODE_DEBUG,
} usb_mode_t;

typedef struct {
    uint8_t *data;
    uint16_t size;
} usb_report_t;

extern char *const usbd_desc_str[];

void usb_driver_init(usb_mode_t mode);
void usb_driver_task();

void usb_driver_send_report(usb_report_t report);

#ifdef __cplusplus
}
#endif

#endif // _USB_USB_DRIVER_H_
