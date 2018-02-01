#include "USBDevice.h"

#define DEFAULT_VENDOR_ID  0x1EAF
#define DEFAULT_PRODUCT_ID 0x0004

USBCompositeDevice::USBCompositeDevice(void) {
        vendorId = 0;
        productId = 0;
        numParts = 0;
        numPlugins = 0;
        setManufacturerString(NULL);
        setProductString(NULL);
        setSerialString(DEFAULT_SERIAL_STRING);
        iManufacturer[0] = 0;
        iProduct[0] = 0;
        iSerialNumber[0] = 0;
    }

void USBCompositeDevice::setVendorId(uint16 _vendorId) {
    if (_vendorId != 0)
        vendorId = _vendorId;
    else
        vendorId = DEFAULT_VENDOR_ID;
}

void USBCompositeDevice::setProductId(uint16 _productId) {
    if (_productId != 0)
        productId = _productId;
    else
        productId = DEFAULT_PRODUCT_ID;
}

void setString(uint8* out, const usb_descriptor_string* defaultDescriptor, const char* s, uint32 maxLength) {
    if (s == NULL) {
        uint8 n = defaultDescriptor->bLength;
        uint8 m = USB_DESCRIPTOR_STRING_LEN(maxLength);
        if (n > m)
            n = m;
        memcpy(out, defaultDescriptor, n);
        out[0] = n;
    }
    else {
        uint32 n = strlen(s);
        if (n > maxLength)
            n = maxLength;
        out[0] = (uint8)n;
        out[1] = USB_DESCRIPTOR_TYPE_STRING;
        for (uint32 i=0; i<n; i++) {
            out[2 + 2*i] = (uint8)s[i];
            out[2 + 1 + 2*i] = 0;
        }
    }
}

void USBCompositeDevice::setManufacturerString(const char* s) {
    setString(iManufacturer, &usb_generic_default_iManufacturer, s, USB_MAX_MANUFACTURER_LENGTH);
}

void USBCompositeDevice::setProductString(const char* s) {
    setString(iProduct, &usb_generic_default_iProduct, s, USB_MAX_PRODUCT_LENGTH);
}

void USBCompositeDevice::setSerialString(const char* s) {
    if (s == NULL)
        haveSerialNumber = false;
    else {
        haveSerialNumber = true;
        setString(iSerialNumber, NULL, s, USB_MAX_SERIAL_NUMBER_LENGTH);
    }
}

bool USBCompositeDevice::begin() {
    if (enabled)
        return true;
    usb_generic_set_info(vendorId, productId, iManufacturer, iProduct, 
        haveSerialNumber ? iSerialNumber : NULL);
    for (uint32 i = 0 ; i < numPlugins ; i++)
        if (!plugins[i]->init())
            return false;
    if (! usb_generic_set_parts(parts, numParts))
        return false;
    usb_generic_enable();
    enabled = true;  
    return true;
}

void USBCompositeDevice::end() {
    if (!enabled)
        return;
    usb_generic_disable();
    for (uint32 i = 0 ; i < numPlugins ; i++)
        plugins[i]->stop();
    enabled = false;
}

void USBCompositeDevice::clear() {
    numParts = 0;
    numPlugins = 0;
}

bool USBCompositeDevice::add(USBPlugin* pluginPtr) {
    if (numPlugins >= USB_COMPOSITE_MAX_PLUGINS)
        return false;
    plugins[numPlugins++] = pluginPtr;
    return pluginPtr->registerParts();
}

bool USBCompositeDevice::add(USBCompositePart& part) {
    if (numParts >= USB_COMPOSITE_MAX_PARTS)
        return false;
    parts[numParts++] = &part;
    return true;
}

USBCompositeDevice USBComposite;