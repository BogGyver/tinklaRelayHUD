#ifndef TINKLARELAYDRIVER_H
#define TINKLARELAYDRIVER_H

// Includes
#include <QString>
#include <QStringList>
#include <QVector>
#include <libusb-1.0/libusb.h>

//First Byte After DATA
#define REL_GEAR_IN_NEUTRAL 1
#define REL_OPTION1_ON 2
#define REL_OPTION2_ON 4
#define REL_OPTION3_ON 8
#define REL_OPTION4_ON 16
#define REL_CAR_ON 32
#define REL_GEAR_IN_REVERSE 64
#define REL_GEAR_IN_FORWARD 128
//Second Byte After DATA
#define REL_BRAKE_HOLD 1
#define REL_LEFT_TURN_SIGNAL 2
#define REL_RIGHT_TURN_SIGNAL 4
#define REL_BRAKE_PRESSED 8
#define REL_HIGHBEAMS_ON 16
#define REL_LIGHT_ON 32
#define REL_BELOW_20MPH 64
#define REL_USE_IMPERIAL_FOR_SPEED 128
//Third Byte After DATA
#define REL_TPMS_ALERT_ON 1
#define REL_LEFT_STEERING_ANGLE_ABOVE_45DEG 2
#define REL_RIGHT_STEERING_ANGLE_ABOVE_45DEG 4
#define REL_AP_ON 8
#define REL_CAR_CHARGING 16
#define REL_LEFT_SIDE_BSM 32
#define REL_RIGHT_SIDE_BSM 64
#define REL_TACC_ONLY_ACTIVE 128
//Eighth Byte adter DATA
#define REL_AP_AVAILABLE 128

//CONTROL READ VALUES
#define GET_TINKLA_RELAY_DATA 0xFE
#define GET_TINKLA_RELAY_DATA_SIZE 0x0a

class TinklaRelayDriver
{
private:
    libusb_context *context_;
    libusb_device_handle *handle_;
    bool disconnected_, kernelWasAttached_;

    QString getDescGeneric(quint8 command, int &errcnt, QString &errstr);
    void writeDescGeneric(const QString &descriptor, quint8 command, int &errcnt, QString &errstr);

public:
    // Class definitions
    static const quint16 VID = 0xbbaa;     // Default USB vendor ID
    static const quint16 PID = 0xddcc;     // Default USB product ID
    static const int SUCCESS = 0;          // Returned by open() if successful
    static const int ERROR_INIT = 1;       // Returned by open() in case of a libusb initialization failure
    static const int ERROR_NOT_FOUND = 2;  // Returned by open() if the device was not found
    static const int ERROR_BUSY = 3;       // Returned by open() if the device is already in use

    // Descriptor specific definitions
    static const size_t DESCMXL_MANUFACTURER = 62;  // Maximum length of manufacturer descriptor
    static const size_t DESCMXL_PRODUCT = 62;       // Maximum length of product descriptor
    static const size_t DESCMXL_SERIAL = 30;        // Maximum length of serial descriptor

    // The following values are applicable to controlTransfer()
    static const quint8 GET = 0x80;                                 // Device-to-Host vendor request
    static const quint8 SET = 0x20;                                 // Host-to-Device vendor request
    struct USBConfig {
        quint16 vid;     // Vendor ID (little-endian)
        quint16 pid;     // Product ID (little-endian)
        quint8 majrel;   // Major release version
        quint8 minrel;   // Minor release version
        quint8 maxpow;   // Maximum consumption current (raw value in 2mA units)
        quint8 powmode;  // Power mode
        quint8 trfprio;  // Transfer priority

        bool operator ==(const USBConfig &other) const;
        bool operator !=(const USBConfig &other) const;
    };

    //not sure we need these
    static const quint8 GET_USB_CONFIG = 0x60;                      // Get_USB_Config command
    static const quint16 GET_USB_CONFIG_WLEN = 0x0009;              // Get_USB_Config data stage length
    static const quint8 SET_USB_CONFIG = 0x61;                      // Set_USB_Config command
    static const quint16 SET_USB_CONFIG_WLEN = 0x000a;              // Set_USB_Config data stage length
    static const quint8 GET_MANUFACTURING_STRING_1 = 0x62;          // Get_Manufacturing_String_1 command
    static const quint16 GET_MANUFACTURING_STRING_1_WLEN = 0x0040;  // Get_Manufacturing_String_1 data stage length
    static const quint8 SET_MANUFACTURING_STRING_1 = 0x63;          // Set_Manufacturing_String_1 command
    static const quint16 SET_MANUFACTURING_STRING_1_WLEN = 0x0040;  // Set_Manufacturing_String_1 data stage length
    static const quint8 GET_MANUFACTURING_STRING_2 = 0x64;          // Get_Manufacturing_String_2 command
    static const quint16 GET_MANUFACTURING_STRING_2_WLEN = 0x0040;  // Get_Manufacturing_String_2 data stage length
    static const quint8 SET_MANUFACTURING_STRING_2 = 0x65;          // Set_Manufacturing_String_2 command
    static const quint16 SET_MANUFACTURING_STRING_2_WLEN = 0x0040;  // Set_Manufacturing_String_2 data stage length
    static const quint8 GET_PRODUCT_STRING_1 = 0x66;                // Get_Product_String_1 command
    static const quint16 GET_PRODUCT_STRING_1_WLEN = 0x0040;        // Get_Product_String_1 data stage length
    static const quint8 SET_PRODUCT_STRING_1 = 0x67;                // Set_Product_String_1 command
    static const quint16 SET_PRODUCT_STRING_1_WLEN = 0x0040;        // Set_Product_String_1 data stage length
    static const quint8 GET_PRODUCT_STRING_2 = 0x68;                // Get_Product_String_2 command
    static const quint16 GET_PRODUCT_STRING_2_WLEN = 0x0040;        // Get_Product_String_2 data stage length
    static const quint8 SET_PRODUCT_STRING_2 = 0x69;                // Set_Product_String_2 command
    static const quint16 SET_PRODUCT_STRING_2_WLEN = 0x0040;        // Set_Product_String_2 data stage length
    static const quint8 GET_SERIAL_STRING = 0x6a;                   // Get_Serial_String command
    static const quint16 GET_SERIAL_STRING_WLEN = 0x0040;           // Get_Serial_String data stage length
    static const quint8 SET_SERIAL_STRING = 0x6b;                   // Set_Serial_String command
    static const quint16 SET_SERIAL_STRING_WLEN = 0x0040;           // Set_Serial_String data stage length


    TinklaRelayDriver();
    ~TinklaRelayDriver();

    bool disconnected() const;
    bool isOpen() const;
    int open(const QString &serial);

    void bulkTransfer(quint8 endpointAddr, unsigned char *data, int length, int *transferred, int &errcnt, QString &errstr);
    void close();
    void controlTransfer(quint8 bmRequestType, quint8 bRequest, quint16 wValue, quint16 wIndex, unsigned char *data, quint16 wLength, int &errcnt, QString &errstr);
    static QStringList listDevices(int &errcnt, QString &errstr);
    void processDataMessage();
    bool getData();

    //VALUES
    volatile bool rel_option1_on = false;
    volatile bool rel_option2_on = false;
    volatile bool rel_option3_on = false;
    volatile bool rel_option4_on = false;
    volatile bool rel_car_on = false;
    volatile bool rel_gear_in_reverse = false;
    volatile bool rel_gear_in_forward = false;
    volatile bool rel_gear_in_neutral = false;
    volatile bool rel_left_turn_signal = false;
    volatile bool rel_right_turn_signal = false;
    volatile bool rel_brake_pressed = false;
    volatile bool rel_highbeams_on = false;
    volatile bool rel_light_on = false;
    volatile bool rel_below_20mph = false; //BELOW 20 MPH
    volatile bool rel_left_steering_above_45deg = false; //MORE THAN 45 DEG
    volatile bool rel_right_steering_above_45deg = false; //MORE THAN 45 DEG
    volatile bool rel_AP_on = false; //start with true if only Veh can is connected
    volatile bool rel_car_charging = false;
    volatile bool rel_left_side_bsm = false;
    volatile bool rel_right_side_bsm = false;
    volatile bool rel_tacc_only_active = false;
    volatile bool rel_use_imperial = false; //imperial vs metric for speed
    volatile bool rel_brake_hold_on = false;
    volatile bool rel_tpms_alert_on = false;
    volatile uint8_t rel_brightness = 100; // start brightness value
    volatile uint8_t rel_speed = 0; //starting speed, speed is in the UoM set on car
    volatile int16_t rel_power_lvl = 0;
    volatile bool rel_AP_available = false;
    volatile uint8_t rel_acc_status = 0;
    volatile uint8_t rel_acc_speed = 0;
    volatile uint8_t rel_speed_limit = 0;
    bool tinklaRelayInitialized = false;
};

#endif // TINKLARELAYDRIVER_H
