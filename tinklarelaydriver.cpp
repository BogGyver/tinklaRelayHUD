// Includes
#include <QObject>
#include "tinklarelaydriver.h"
extern "C" {
#include "libusb-extra.h"
}

// Definitions
const unsigned int TR_TIMEOUT = 500;  // Transfer timeout in milliseconds (increased to 500ms since version 2.0.2)

// Specific to getDescGeneric() and writeDescGeneric() (added in version 2.1.0)
const quint16 DESC_TBLSIZE = 0x0040;           // Descriptor table size, including preamble [64]
const size_t DESC_MAXIDX = DESC_TBLSIZE - 2;   // Maximum usable index [62]
const size_t DESC_IDXINCR = DESC_TBLSIZE - 1;  // Index increment or step between table preambles [63]

uint8_t tinklaRelayData[] = {0,0,0,0,0,0,0,0,0,0};

// Private generic procedure used to get any descriptor (added as a refactor in version 2.1.0)
QString TinklaRelayDriver::getDescGeneric(quint8 command, int &errcnt, QString &errstr)
{
    unsigned char controlBufferIn[DESC_TBLSIZE];
    controlTransfer(GET, command, 0x0000, 0x0000, controlBufferIn, DESC_TBLSIZE, errcnt, errstr);
    QString descriptor;
    size_t length = controlBufferIn[0];
    size_t end = length > DESC_MAXIDX ? DESC_MAXIDX : length;
    for (size_t i = 2; i < end; i += 2) {  // Process first 30 characters (bytes 2-61 of the array)
        if (controlBufferIn[i] != 0 || controlBufferIn[i + 1] != 0) {  // Filter out null characters
            descriptor += QChar(controlBufferIn[i + 1] << 8 | controlBufferIn[i]);  // UTF-16LE conversion as per the USB 2.0 specification
        }
    }
    if ((command == GET_MANUFACTURING_STRING_1 || command == GET_PRODUCT_STRING_1) && length > DESC_MAXIDX) {
        quint16 midchar = controlBufferIn[DESC_MAXIDX];  // Char in the middle (parted between two tables)
        controlTransfer(GET, command + 2, 0x0000, 0x0000, controlBufferIn, DESC_TBLSIZE, errcnt, errstr);
        midchar = static_cast<quint16>(controlBufferIn[0] << 8 | midchar);  // Reconstruct the char in the middle
        if (midchar != 0x0000) {  // Filter out the reconstructed char if the same is null
            descriptor += QChar(midchar);
        }
        end = length - DESC_IDXINCR;
        for (size_t i = 1; i < end; i += 2) {  // Process remaining characters, up to 31 (bytes 1-62 of the array)
            if (controlBufferIn[i] != 0 || controlBufferIn[i + 1] != 0) {  // Again, filter out null characters
                descriptor += QChar(controlBufferIn[i + 1] << 8 | controlBufferIn[i]);  // UTF-16LE conversion as per the USB 2.0 specification
            }
        }
    }
    return descriptor;
}

TinklaRelayDriver::TinklaRelayDriver() :
    context_(nullptr),
    handle_(nullptr),
    disconnected_(false),
    kernelWasAttached_(false)
{
    disconnected_ = true;
}

TinklaRelayDriver::~TinklaRelayDriver()
{
    close();  // The destructor is used to close the device, and this is essential so the device can be freed when the parent object is destroyed
}

// Diagnostic function used to verify if the device has been disconnected
bool TinklaRelayDriver::disconnected() const
{
    return disconnected_;  // Returns true if the device has been disconnected, or false otherwise
}

// Checks if the device is open
bool TinklaRelayDriver::isOpen() const
{
    return handle_ != nullptr;  // Returns true if the device is open, or false otherwise
}

// Safe bulk transfer
void TinklaRelayDriver::bulkTransfer(quint8 endpointAddr, unsigned char *data, int length, int *transferred, int &errcnt, QString &errstr)
{
    if (!isOpen()) {
        ++errcnt;
        errstr += QObject::tr("In bulkTransfer(): device is not open.\n");  // Program logic error
    } else {
        int result = libusb_bulk_transfer(handle_, endpointAddr, data, length, transferred, TR_TIMEOUT);
        if (result != 0 || (transferred != nullptr && *transferred != length)) {  // Since version 2.0.2, the number of transferred bytes is also verified, as long as a valid (non-null) pointer is passed via "transferred"
            ++errcnt;
            if (endpointAddr < 0x80) {
                errstr += QObject::tr("Failed bulk OUT transfer to endpoint %1 (address 0x%2).\n").arg(0x0f & endpointAddr).arg(endpointAddr, 2, 16, QChar('0'));
            } else {
                errstr += QObject::tr("Failed bulk IN transfer from endpoint %1 (address 0x%2).\n").arg(0x0f & endpointAddr).arg(endpointAddr, 2, 16, QChar('0'));
            }
            if (result == LIBUSB_ERROR_NO_DEVICE || result == LIBUSB_ERROR_IO) {  // Note that libusb_bulk_transfer() may return "LIBUSB_ERROR_IO" [-1] on device disconnect (version 2.0.2)
                disconnected_ = true;  // This reports that the device has been disconnected
            }
        }
    }
}

// Closes the device safely, if open
void TinklaRelayDriver::close()
{
    if (isOpen()) {  // This condition avoids a segmentation fault if the calling algorithm tries, for some reason, to close the same device twice (e.g., if the device is already closed when the destructor is called)
        libusb_release_interface(handle_, 0);  // Release the interface
        if (kernelWasAttached_) {  // If a kernel driver was attached to the interface before
            libusb_attach_kernel_driver(handle_, 0);  // Reattach the kernel driver
        }
        libusb_close(handle_);  // Close the device
        libusb_exit(context_);  // Deinitialize libusb
        handle_ = nullptr;  // Required to mark the device as closed
    }
}

// Opens the device having the given VID, PID and, optionally, the given serial number, and assigns its handle
// Since version 2.1.0, it is not required to specify a serial number
int TinklaRelayDriver::open(const QString &serial)
{
    int retval;
    if (isOpen()) {  // Just in case the calling algorithm tries to open a device that was already sucessfully open, or tries to open different devices concurrently, all while using (or referencing to) the same object
        retval = SUCCESS;
        disconnected_ = false;
    } else if (libusb_init(&context_) != 0) {  // Initialize libusb. In case of failure
        retval = ERROR_INIT;
    } else {  // If libusb is initialized
        if (serial.isNull()) {  // Note that serial, by omission, is a null QString
            handle_ = libusb_open_device_with_vid_pid(context_, VID, PID);  // If no serial number is specified, this will open the first device found with matching VID and PID
        } else {
            handle_ = libusb_open_device_with_vid_pid_serial(context_, VID, PID, reinterpret_cast<unsigned char *>(serial.toLatin1().data()));
        }
        if (handle_ == nullptr) {  // If the previous operation fails to get a device handle
            libusb_exit(context_);  // Deinitialize libusb
            retval = ERROR_NOT_FOUND;
        } else {  // If the device is successfully opened and a handle obtained
            if (libusb_kernel_driver_active(handle_, 0) == 1) {  // If a kernel driver is active on the interface
                libusb_detach_kernel_driver(handle_, 0);  // Detach the kernel driver
                kernelWasAttached_ = true;  // Flag that the kernel driver was attached
            } else {
                kernelWasAttached_ = false;  // The kernel driver was not attached
            }
            if (libusb_claim_interface(handle_, 0) != 0) {  // Claim the interface. In case of failure
                if (kernelWasAttached_) {  // If a kernel driver was attached to the interface before
                    libusb_attach_kernel_driver(handle_, 0);  // Reattach the kernel driver
                }
                libusb_close(handle_);  // Close the device
                libusb_exit(context_);  // Deinitialize libusb
                handle_ = nullptr;  // Required to mark the device as closed
                retval = ERROR_BUSY;
            } else {
                disconnected_ = false;  // Note that this flag is never assumed to be true for a device that was never opened - See constructor for details!
                retval = SUCCESS;
            }
        }
    }
    return retval;
}

// Safe control transfer
void TinklaRelayDriver::controlTransfer(quint8 bmRequestType, quint8 bRequest, quint16 wValue, quint16 wIndex, unsigned char *data, quint16 wLength, int &errcnt, QString &errstr)
{
    if (!isOpen()) {
        ++errcnt;
        errstr += QObject::tr("In controlTransfer(): device is not open.\n");  // Program logic error
    } else {
        int result = libusb_control_transfer(handle_, bmRequestType, bRequest, wValue, wIndex, data, wLength, TR_TIMEOUT);
        if (result != wLength) {
            ++errcnt;
            errstr += QObject::tr("Failed control transfer (0x%1, 0x%2).\n").arg(bmRequestType, 2, 16, QChar('0')).arg(bRequest, 2, 16, QChar('0'));
            if (result == LIBUSB_ERROR_NO_DEVICE || result == LIBUSB_ERROR_IO || result == LIBUSB_ERROR_PIPE) {  // Note that libusb_control_transfer() may return "LIBUSB_ERROR_IO" [-1] or "LIBUSB_ERROR_PIPE" [-9] on device disconnect (version 2.0.2)
                disconnected_ = true;  // This reports that the device has been disconnected
            }
        }
    }
}

// Helper function to list devices
QStringList TinklaRelayDriver::listDevices(int &errcnt, QString &errstr)
{
    QStringList devices;
    libusb_context *context;
    if (libusb_init(&context) != 0) {  // Initialize libusb. In case of failure
        ++errcnt;
        errstr += QObject::tr("Could not initialize libusb.\n");
    } else {  // If libusb is initialized
        libusb_device **devs;
        ssize_t devlist = libusb_get_device_list(context, &devs);  // Get a device list
        if (devlist < 0) {  // If the previous operation fails to get a device list
            ++errcnt;
            errstr += QObject::tr("Failed to retrieve a list of devices.\n");
        } else {
            for (ssize_t i = 0; i < devlist; ++i) {  // Run through all listed devices
                libusb_device_descriptor desc;
                if (libusb_get_device_descriptor(devs[i], &desc) == 0 && desc.idVendor == VID && desc.idProduct == PID) {  // If the device descriptor is retrieved, and both VID and PID correspond to the respective given values
                    libusb_device_handle *handle;
                    if (libusb_open(devs[i], &handle) == 0) {  // Open the listed device. If successfull
                        unsigned char str_desc[256];
                        libusb_get_string_descriptor_ascii(handle, desc.iSerialNumber, str_desc, static_cast<int>(sizeof(str_desc)));  // Get the serial number string in ASCII format
                        devices += reinterpret_cast<char *>(str_desc);  // Append the serial number string to the list
                        libusb_close(handle);  // Close the device
                    }
                }
            }
            libusb_free_device_list(devs, 1);  // Free device list
        }
        libusb_exit(context);  // Deinitialize libusb
    }
    return devices;
}

void TinklaRelayDriver::processDataMessage() {
  rel_gear_in_neutral = ((tinklaRelayData[0] & REL_GEAR_IN_NEUTRAL) > 0);
  rel_option1_on = ((tinklaRelayData[0] & REL_OPTION1_ON) > 0);
  rel_option2_on = ((tinklaRelayData[0] & REL_OPTION2_ON) > 0);
  rel_option3_on = ((tinklaRelayData[0] & REL_OPTION3_ON) > 0);
  rel_option4_on = ((tinklaRelayData[0] & REL_OPTION4_ON) > 0);
  rel_car_on = ((tinklaRelayData[0] & REL_CAR_ON) > 0);
  rel_gear_in_reverse = ((tinklaRelayData[0] & REL_GEAR_IN_REVERSE) > 0);
  rel_gear_in_forward = ((tinklaRelayData[0] & REL_GEAR_IN_FORWARD) > 0);

  rel_brake_hold_on = ((tinklaRelayData[1] & REL_BRAKE_HOLD) > 0);
  rel_left_turn_signal = ((tinklaRelayData[1] & REL_LEFT_TURN_SIGNAL) > 0);
  rel_right_turn_signal = ((tinklaRelayData[1] & REL_RIGHT_TURN_SIGNAL) > 0);
  rel_brake_pressed = ((tinklaRelayData[1] & REL_BRAKE_PRESSED) > 0);
  rel_highbeams_on = ((tinklaRelayData[1] & REL_HIGHBEAMS_ON) > 0);
  rel_light_on = ((tinklaRelayData[1] & REL_LIGHT_ON) > 0);
  rel_below_20mph = ((tinklaRelayData[1] & REL_BELOW_20MPH) > 0);
  rel_use_imperial = ((tinklaRelayData[1] & REL_USE_IMPERIAL_FOR_SPEED) > 0);

  rel_tpms_alert_on = ((tinklaRelayData[2] & REL_TPMS_ALERT_ON) > 0);
  rel_left_steering_above_45deg = ((tinklaRelayData[2] & REL_LEFT_STEERING_ANGLE_ABOVE_45DEG) > 0);
  rel_right_steering_above_45deg = ((tinklaRelayData[2] & REL_RIGHT_STEERING_ANGLE_ABOVE_45DEG) > 0);
  rel_AP_on = ((tinklaRelayData[2] & REL_AP_ON) > 0);
  rel_car_charging = ((tinklaRelayData[2] & REL_CAR_CHARGING) > 0);
  rel_left_side_bsm = ((tinklaRelayData[2] & REL_LEFT_SIDE_BSM) > 0);
  rel_right_side_bsm = ((tinklaRelayData[2] & REL_RIGHT_SIDE_BSM) > 0);
  rel_tacc_only_active = ((tinklaRelayData[2] & REL_TACC_ONLY_ACTIVE) > 0);

  rel_brightness = tinklaRelayData[3];

  rel_speed = tinklaRelayData[4];

  rel_power_lvl = (int16_t)((tinklaRelayData[5] << 8) | tinklaRelayData[6]);

  rel_acc_speed = tinklaRelayData[7];

  rel_speed_limit = 5 * (tinklaRelayData[8] & 0x1F);
  rel_acc_status = (tinklaRelayData[8] >> 5) & 0x03;
  rel_AP_available = ((tinklaRelayData[8] & REL_AP_AVAILABLE) > 0);
}

// Returns true if a ReadWithRTR command is currently active
bool TinklaRelayDriver::getData()
{
    int errcnt = 0;
    QString errstr;
    controlTransfer(GET, GET_TINKLA_RELAY_DATA, 0x0000, 0x0000, tinklaRelayData, GET_TINKLA_RELAY_DATA_SIZE, errcnt, errstr);
    if (errcnt > 0) {
        return false;
     } else {
        return true;
     }
}
