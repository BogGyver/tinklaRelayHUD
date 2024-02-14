// Includes
#include <string.h>
#include "libusb-extra.h"

// Opens the device with matching VID, PID and serial number
libusb_device_handle *libusb_open_device_with_vid_pid_serial(libusb_context *context, uint16_t vid, uint16_t pid, unsigned char *serial)
{
    libusb_device **devs;
    libusb_device_handle *devhandle = NULL;
    if (libusb_get_device_list(context, &devs) >= 0) {  // If the device list is retrieved
        libusb_device *dev;
        size_t devcounter = 0;
        while ((dev = devs[devcounter++]) != NULL) {  // Walk through all the devices
            struct libusb_device_descriptor desc;
            if (libusb_get_device_descriptor(dev, &desc) == 0 && desc.idVendor == vid && desc.idProduct == pid && libusb_open(dev, &devhandle) == 0) {  // If the device descriptor is retrieved, both PID and VID match, and if the device is successfully opened
                unsigned char str_desc[256];
                libusb_get_string_descriptor_ascii(devhandle, desc.iSerialNumber, str_desc, (int)sizeof(str_desc));  // Get the serial number string in ASCII format
                if (strcmp((char *)str_desc, (char *)serial) == 0) {  // If the serial number match
                    break;
                } else {
                    libusb_close(devhandle);  // Close the device, since it is not the one with the corresponding serial number
                    devhandle = NULL;  // Set device handle value to null pointer
                }
            }
        }
        libusb_free_device_list(devs, 1);  // Free device list
    }
    return devhandle;  // Return device handle (or null pointer if no matching device was found)
}
