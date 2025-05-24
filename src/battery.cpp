#include <A_Config.h>
#include "images/images.h"

const uint8_t *getBatteryIcon() {
    int16_t batteryLevel = hal.VCC;
    const uint8_t *batteryIcon;
    if (batteryLevel < 3500) {
        batteryIcon = battery_empty_bits;
    } else if (batteryLevel < 3700) {
        batteryIcon = battery_quarter_bits;
    } else if (batteryLevel < 3900) {
        batteryIcon = battery_three_quarters_bits;
    } else if (batteryLevel < 4300 && batteryLevel > 4100) {
        batteryIcon = battery_full_bits;
    } else if (batteryLevel > 4400 && hal.isCharging == false) {
        batteryIcon = usb_power_bits;
    } else if (hal.isCharging == true) {
        batteryIcon = battery_chongdian_bits;
    }
    return batteryIcon;
}