#ifndef LIBUSBEXTRA_H
#define LIBUSBEXTRA_H

// Includes
#include <libusb-1.0/libusb.h>

// Function prototypes
libusb_device_handle *libusb_open_device_with_vid_pid_serial(libusb_context *context, uint16_t vid, uint16_t pid, unsigned char *serial);

#endif
