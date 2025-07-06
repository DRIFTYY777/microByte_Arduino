#include "usbHal.h"
#include "USB.h"


#include "HardwareSerial.h"

static void usbEventCallback(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    if (event_base == ARDUINO_USB_EVENTS) {
        arduino_usb_event_data_t *data = (arduino_usb_event_data_t *)event_data;
        switch (event_id) {
        case ARDUINO_USB_STARTED_EVENT: Serial.println("USB PLUGGED"); break;
        case ARDUINO_USB_STOPPED_EVENT: Serial.println("USB UNPLUGGED"); break;
        case ARDUINO_USB_SUSPEND_EVENT: Serial.printf("USB SUSPENDED: remote_wakeup_en: %u\n", data->suspend.remote_wakeup_en); break;
        case ARDUINO_USB_RESUME_EVENT:  Serial.println("USB RESUMED"); break;

        default: break;
        }
    } else if (event_base == ARDUINO_USB_CDC_EVENTS) {
        arduino_usb_cdc_event_data_t *data = (arduino_usb_cdc_event_data_t *)event_data;
        switch (event_id) {
        case ARDUINO_USB_CDC_CONNECTED_EVENT:    Serial.println("CDC CONNECTED"); break;
        case ARDUINO_USB_CDC_DISCONNECTED_EVENT: Serial.println("CDC DISCONNECTED"); break;
        case ARDUINO_USB_CDC_LINE_STATE_EVENT:   Serial.printf("CDC LINE STATE: dtr: %u, rts: %u\n", data->line_state.dtr, data->line_state.rts); break;
        case ARDUINO_USB_CDC_LINE_CODING_EVENT:
            Serial.printf(
              "CDC LINE CODING: bit_rate: %lu, data_bits: %u, stop_bits: %u, parity: %u\n", data->line_coding.bit_rate, data->line_coding.data_bits,
              data->line_coding.stop_bits, data->line_coding.parity
            );
            break;
        case ARDUINO_USB_CDC_RX_EVENT:
            Serial.printf("CDC RX [%u]:", data->rx.len);
            {
            uint8_t buf[data->rx.len];
            size_t len = USBSerial.read(buf, data->rx.len);
            Serial.write(buf, len);
            }
            Serial.println();
            break;
        case ARDUINO_USB_CDC_RX_OVERFLOW_EVENT: Serial.printf("CDC RX Overflow of %d bytes", data->rx_overflow.dropped_bytes); break;

        default: break;
        }
    }
}


void USBHAL::initCDC()
{

    USB.onEvent(usbEventCallback);
    USBSerial.onEvent(usbEventCallback);

    USBSerial.begin();
}

void USBHAL::deinitCDC()
{
    // deinitalize the USB CDC
    USBSerial.end();
}

bool USBHAL::init()
{
    // this function initializes the USB subsystem
    if (!USB.begin())
    {
        Serial.println("USB initialization failed!");
        return false;
    }
    Serial.println("USB initialized successfully");

    USB.enableDFU(); // Enable Device Firmware Update (DFU) mode

    usbMode = USB_MODE_NONE;
    return true;
}

void USBHAL::deinit()
{
    switch (usbMode)
    {
    case USB_MODE_CDC:
        deinitCDC();
        Serial.println("USB CDC deinitialized");
        break;
    case USB_MODE_HID:
        usbHid.deinit(); // Deinitialize HID
        Serial.println("USB HID deinitialized");
        break;
    case USB_MODE_MSC:
        usbStorage.deinit(); // Deinitialize USB Mass Storage
        Serial.println("USB MSC deinitialized");
        break;
    case USB_MODE_UPDATE:
        //USB.endUpdate();
        Serial.println("USB Update deinitialized");
        break;
    case USB_MODE_DEBUG:
        //USB.endDebug();
        Serial.println("USB Debug deinitialized");
        break;

    case USB_MODE_NONE:
        Serial.println("USB already deinitialized");
        break;
    default:
        Serial.println("USB deinitialized");
        break;
    }
    usbMode = USB_MODE_NONE; // Reset USB mode
}

void USBHAL::modes(uint8_t mode)
{
    deinit();
    // Set the USB mode
    switch (mode)
    {
    case USB_MODE_CDC:
        //USB.setMode(USB_MODE_CDC);
        initCDC(); // Initialize USB CDC
        Serial.println("USB Mode set to CDC");
        usbMode = USB_MODE_CDC;
        break;
    case USB_MODE_HID:
        usbHid.init(); // Initialize HID with keyboard as default
        Serial.println("USB Mode set to HID");
        usbMode = USB_MODE_HID;
        break;
    case USB_MODE_MSC:
        usbStorage.init(); // Initialize USB Mass Storage
        Serial.println("USB Mode set to MSC");
        usbMode = USB_MODE_MSC;
        break;
    case USB_MODE_UPDATE:
        //USB.setMode(USB_MODE_UPDATE);
        Serial.println("USB Mode set to Update");
        usbMode = USB_MODE_UPDATE;
        break;
    case USB_MODE_DEBUG:
        //USB.setMode(USB_MODE_DEBUG);
        Serial.println("USB Mode set to Debug");
        usbMode = USB_MODE_DEBUG;
        break;
    default:
        //USB.setMode(USB_MODE_NONE);
        Serial.println("USB Mode set to None");
        usbMode = USB_MODE_NONE;
        break;
    }
}


USBHAL usbHal;
