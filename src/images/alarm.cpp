// Auto-generated alarm.cpp
#include "alarm.h"

// // w120h120, file: images\alarm\alarm_clock.bmp
const uint8_t alarm_clock_bits[] = {
    0x00,0x00,0xfc,0x07,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xe0,0x3f,0x00,0x00,
0x00,0xc0,0xff,0x7f,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xfe,0xff,0x03,0x00,
0x00,0xf0,0xff,0xff,0x01,0x00,0x00,0x00,0x00,0x00,0x80,0xff,0xff,0x0f,0x00,
0x00,0xfc,0xff,0xff,0x03,0x00,0x00,0x00,0x00,0x00,0xe0,0xff,0xff,0x3f,0x00,
0x00,0xfe,0xff,0xff,0x0f,0x00,0x00,0x00,0x00,0x00,0xf0,0xff,0xff,0x7f,0x00,
0x00,0xff,0xff,0xff,0x1f,0x00,0x00,0x00,0x00,0x00,0xf8,0xff,0xff,0xff,0x00,
0x80,0xff,0xff,0xff,0x1f,0x00,0x00,0x00,0x00,0x00,0xf8,0xff,0xff,0xff,0x01,
0xc0,0xff,0xff,0xff,0x0f,0x00,0x00,0x00,0x00,0x00,0xf0,0xff,0xff,0xff,0x03,
0xe0,0xff,0xff,0xff,0x07,0x00,0x00,0x00,0x00,0x00,0xe0,0xff,0xff,0xff,0x07,
0xf0,0xff,0xff,0xff,0x03,0x00,0x00,0x00,0x00,0x00,0xc0,0xff,0xff,0xff,0x0f,
0xf8,0xff,0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0xff,0x1f,
0xf8,0xff,0xff,0x7f,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xfe,0xff,0xff,0x1f,
0xfc,0xff,0xff,0x3f,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xfc,0xff,0xff,0x3f,
0xfc,0xff,0xff,0x1f,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xf8,0xff,0xff,0x3f,
0xfe,0xff,0xff,0x0f,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xf0,0xff,0xff,0x7f,
0xfe,0xff,0xff,0x03,0x00,0x00,0xe0,0xff,0x07,0x00,0x00,0xc0,0xff,0xff,0x7f,
0xfe,0xff,0xff,0x01,0x00,0x00,0xff,0xff,0xff,0x00,0x00,0x80,0xff,0xff,0x7f,
0xfe,0xff,0xff,0x00,0x00,0xf0,0xff,0xff,0xff,0x0f,0x00,0x00,0xff,0xff,0x7f,
0xff,0xff,0x7f,0x00,0x00,0xfe,0xff,0xff,0xff,0x7f,0x00,0x00,0xfe,0xff,0xff,
0xff,0xff,0x3f,0x00,0x80,0xff,0xff,0xff,0xff,0xff,0x01,0x00,0xfc,0xff,0xff,
0xff,0xff,0x0f,0x00,0xe0,0xff,0xff,0xff,0xff,0xff,0x07,0x00,0xf0,0xff,0xff,
0xff,0xff,0x07,0x00,0xf8,0xff,0xff,0xff,0xff,0xff,0x1f,0x00,0xe0,0xff,0xff,
0xff,0xff,0x03,0x00,0xfe,0xff,0xff,0xff,0xff,0xff,0x7f,0x00,0xc0,0xff,0xff,
0xff,0xff,0x01,0x80,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x01,0x80,0xff,0xff,
0xff,0xff,0x00,0xc0,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x03,0x00,0xff,0xff,
0xff,0x3f,0x00,0xe0,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x07,0x00,0xfc,0xff,
0xff,0x1f,0x00,0xf8,0xff,0xff,0xff,0x00,0xff,0xff,0xff,0x1f,0x00,0xf8,0xff,
0xfe,0x0f,0x00,0xfc,0xff,0xff,0x03,0x00,0xc0,0xff,0xff,0x3f,0x00,0xf0,0x7f,
0xfe,0x07,0x00,0xfe,0xff,0x7f,0x00,0x00,0x00,0xfe,0xff,0x7f,0x00,0xe0,0x7f,
0xfe,0x03,0x00,0xff,0xff,0x0f,0x00,0x00,0x00,0xf0,0xff,0xff,0x00,0xc0,0x7f,
0xfc,0x00,0x80,0xff,0xff,0x03,0x00,0x00,0x00,0xc0,0xff,0xff,0x01,0x00,0x3f,
0x7c,0x00,0xc0,0xff,0xff,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0x03,0x00,0x3e,
0x3c,0x00,0xe0,0xff,0x3f,0x00,0x00,0x00,0x00,0x00,0xfc,0xff,0x07,0x00,0x3c,
0x18,0x00,0xf0,0xff,0x1f,0x00,0x00,0x00,0x00,0x00,0xf8,0xff,0x0f,0x00,0x18,
0x00,0x00,0xf8,0xff,0x07,0x00,0x00,0x00,0x00,0x00,0xe0,0xff,0x1f,0x00,0x00,
0x00,0x00,0xfc,0xff,0x03,0x00,0x00,0x00,0x00,0x00,0xc0,0xff,0x3f,0x00,0x00,
0x00,0x00,0xfc,0xff,0x01,0x00,0x00,0x00,0x00,0x00,0x80,0xff,0x3f,0x00,0x00,
0x00,0x00,0xfe,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0x7f,0x00,0x00,
0x00,0x00,0xff,0x7f,0x00,0x00,0x80,0xff,0x01,0x00,0x00,0xfe,0xff,0x00,0x00,
0x00,0x00,0xff,0x3f,0x00,0x00,0xc0,0xff,0x03,0x00,0x00,0xfc,0xff,0x00,0x00,
0x00,0x80,0xff,0x1f,0x00,0x00,0xc0,0xff,0x03,0x00,0x00,0xf8,0xff,0x01,0x00,
0x00,0xc0,0xff,0x0f,0x00,0x00,0xc0,0xff,0x03,0x00,0x00,0xf0,0xff,0x03,0x00,
0x00,0xc0,0xff,0x07,0x00,0x00,0xc0,0xff,0x03,0x00,0x00,0xe0,0xff,0x03,0x00,
0x00,0xe0,0xff,0x03,0x00,0x00,0xc0,0xff,0x03,0x00,0x00,0xe0,0xff,0x07,0x00,
0x00,0xe0,0xff,0x03,0x00,0x00,0xc0,0xff,0x03,0x00,0x00,0xc0,0xff,0x07,0x00,
0x00,0xf0,0xff,0x01,0x00,0x00,0xc0,0xff,0x03,0x00,0x00,0x80,0xff,0x0f,0x00,
0x00,0xf0,0xff,0x01,0x00,0x00,0xc0,0xff,0x03,0x00,0x00,0x80,0xff,0x0f,0x00,
0x00,0xf8,0xff,0x00,0x00,0x00,0xc0,0xff,0x03,0x00,0x00,0x00,0xff,0x1f,0x00,
0x00,0xf8,0x7f,0x00,0x00,0x00,0xc0,0xff,0x03,0x00,0x00,0x00,0xfe,0x1f,0x00,
0x00,0xf8,0x7f,0x00,0x00,0x00,0xc0,0xff,0x03,0x00,0x00,0x00,0xfe,0x1f,0x00,
0x00,0xfc,0x3f,0x00,0x00,0x00,0xc0,0xff,0x03,0x00,0x00,0x00,0xfc,0x3f,0x00,
0x00,0xfc,0x3f,0x00,0x00,0x00,0xc0,0xff,0x03,0x00,0x00,0x00,0xfc,0x3f,0x00,
0x00,0xfc,0x3f,0x00,0x00,0x00,0xc0,0xff,0x03,0x00,0x00,0x00,0xfc,0x3f,0x00,
0x00,0xfe,0x1f,0x00,0x00,0x00,0xc0,0xff,0x03,0x00,0x00,0x00,0xf8,0x7f,0x00,
0x00,0xfe,0x1f,0x00,0x00,0x00,0xc0,0xff,0x03,0x00,0x00,0x00,0xf8,0x7f,0x00,
0x00,0xfe,0x1f,0x00,0x00,0x00,0xc0,0xff,0x03,0x00,0x00,0x00,0xf8,0x7f,0x00,
0x00,0xfe,0x0f,0x00,0x00,0x00,0xc0,0xff,0x03,0x00,0x00,0x00,0xf0,0x7f,0x00,
0x00,0xff,0x0f,0x00,0x00,0x00,0xc0,0xff,0x03,0x00,0x00,0x00,0xf0,0xff,0x00,
0x00,0xff,0x0f,0x00,0x00,0x00,0xc0,0xff,0x03,0x00,0x00,0x00,0xf0,0xff,0x00,
0x00,0xff,0x0f,0x00,0x00,0x00,0xc0,0xff,0x03,0x00,0x00,0x00,0xf0,0xff,0x00,
0x00,0xff,0x07,0x00,0x00,0x00,0xc0,0xff,0x03,0x00,0x00,0x00,0xe0,0xff,0x00,
0x00,0xff,0x07,0x00,0x00,0x00,0xc0,0xff,0x03,0x00,0x00,0x00,0xe0,0xff,0x00,
0x00,0xff,0x07,0x00,0x00,0x00,0xc0,0xff,0x03,0x00,0x00,0x00,0xe0,0xff,0x00,
0x00,0xff,0x07,0x00,0x00,0x00,0xc0,0xff,0x03,0x00,0x00,0x00,0xe0,0xff,0x00,
0x00,0xff,0x07,0x00,0x00,0x00,0xc0,0xff,0x03,0x00,0x00,0x00,0xe0,0xff,0x00,
0x00,0xff,0x07,0x00,0x00,0x00,0xc0,0xff,0x03,0x00,0x00,0x00,0xe0,0xff,0x00,
0x00,0xff,0x07,0x00,0x00,0x00,0xc0,0xff,0x03,0x00,0x00,0x00,0xe0,0xff,0x00,
0x00,0xff,0x07,0x00,0x00,0x00,0xc0,0xff,0x03,0x00,0x00,0x00,0xe0,0xff,0x00,
0x00,0xff,0x07,0x00,0x00,0x00,0xc0,0xff,0x03,0x00,0x00,0x00,0xe0,0xff,0x00,
0x00,0xff,0x07,0x00,0x00,0x00,0xc0,0xff,0x07,0x00,0x00,0x00,0xe0,0xff,0x00,
0x00,0xff,0x07,0x00,0x00,0x00,0xc0,0xff,0x0f,0x00,0x00,0x00,0xe0,0xff,0x00,
0x00,0xff,0x07,0x00,0x00,0x00,0x80,0xff,0x1f,0x00,0x00,0x00,0xe0,0xff,0x00,
0x00,0xff,0x07,0x00,0x00,0x00,0x80,0xff,0x7f,0x00,0x00,0x00,0xe0,0xff,0x00,
0x00,0xff,0x07,0x00,0x00,0x00,0x80,0xff,0xff,0x00,0x00,0x00,0xe0,0xff,0x00,
0x00,0xff,0x07,0x00,0x00,0x00,0x00,0xff,0xff,0x01,0x00,0x00,0xe0,0xff,0x00,
0x00,0xff,0x0f,0x00,0x00,0x00,0x00,0xff,0xff,0x03,0x00,0x00,0xf0,0xff,0x00,
0x00,0xff,0x0f,0x00,0x00,0x00,0x00,0xfc,0xff,0x0f,0x00,0x00,0xf0,0xff,0x00,
0x00,0xff,0x0f,0x00,0x00,0x00,0x00,0xf8,0xff,0x1f,0x00,0x00,0xf0,0x7f,0x00,
0x00,0xfe,0x0f,0x00,0x00,0x00,0x00,0xf0,0xff,0x3f,0x00,0x00,0xf0,0x7f,0x00,
0x00,0xfe,0x1f,0x00,0x00,0x00,0x00,0xe0,0xff,0x7f,0x00,0x00,0xf8,0x7f,0x00,
0x00,0xfe,0x1f,0x00,0x00,0x00,0x00,0x80,0xff,0xff,0x00,0x00,0xf8,0x7f,0x00,
0x00,0xfe,0x1f,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0x00,0x00,0xf8,0x7f,0x00,
0x00,0xfc,0x3f,0x00,0x00,0x00,0x00,0x00,0xfe,0x7f,0x00,0x00,0xfc,0x3f,0x00,
0x00,0xfc,0x3f,0x00,0x00,0x00,0x00,0x00,0xfc,0x7f,0x00,0x00,0xfc,0x3f,0x00,
0x00,0xfc,0x3f,0x00,0x00,0x00,0x00,0x00,0xf0,0x3f,0x00,0x00,0xfc,0x3f,0x00,
0x00,0xf8,0x7f,0x00,0x00,0x00,0x00,0x00,0xe0,0x1f,0x00,0x00,0xfe,0x1f,0x00,
0x00,0xf8,0x7f,0x00,0x00,0x00,0x00,0x00,0xc0,0x0f,0x00,0x00,0xfe,0x1f,0x00,
0x00,0xf8,0xff,0x00,0x00,0x00,0x00,0x00,0x80,0x07,0x00,0x00,0xff,0x0f,0x00,
0x00,0xf0,0xff,0x01,0x00,0x00,0x00,0x00,0x00,0x02,0x00,0x80,0xff,0x0f,0x00,
0x00,0xf0,0xff,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0xff,0x0f,0x00,
0x00,0xe0,0xff,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xc0,0xff,0x07,0x00,
0x00,0xe0,0xff,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xe0,0xff,0x07,0x00,
0x00,0xc0,0xff,0x07,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xe0,0xff,0x03,0x00,
0x00,0xc0,0xff,0x0f,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xf0,0xff,0x03,0x00,
0x00,0x80,0xff,0x1f,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xf8,0xff,0x01,0x00,
0x00,0x00,0xff,0x3f,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xfc,0xff,0x00,0x00,
0x00,0x00,0xff,0x7f,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xfe,0xff,0x00,0x00,
0x00,0x00,0xfe,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0x7f,0x00,0x00,
0x00,0x00,0xfc,0xff,0x01,0x00,0x00,0x00,0x00,0x00,0x80,0xff,0x3f,0x00,0x00,
0x00,0x00,0xfe,0xff,0x03,0x00,0x00,0x00,0x00,0x00,0xc0,0xff,0x7f,0x00,0x00,
0x00,0x00,0xff,0xff,0x07,0x00,0x00,0x00,0x00,0x00,0xe0,0xff,0xff,0x00,0x00,
0x00,0x80,0xff,0xff,0x1f,0x00,0x00,0x00,0x00,0x00,0xf8,0xff,0xff,0x01,0x00,
0x00,0xc0,0xff,0xff,0x3f,0x00,0x00,0x00,0x00,0x00,0xfc,0xff,0xff,0x03,0x00,
0x00,0xe0,0xff,0xff,0xff,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0xff,0x07,0x00,
0x00,0xf0,0xff,0xff,0xff,0x03,0x00,0x00,0x00,0xc0,0xff,0xff,0xff,0x0f,0x00,
0x00,0xf8,0xff,0xff,0xff,0x0f,0x00,0x00,0x00,0xf0,0xff,0xff,0xff,0x1f,0x00,
0x00,0xfc,0xff,0xff,0xff,0x7f,0x00,0x00,0x00,0xfe,0xff,0xff,0xff,0x3f,0x00,
0x00,0xfe,0xff,0xff,0xff,0xff,0x03,0x00,0xc0,0xff,0xff,0xff,0xff,0x7f,0x00,
0x00,0xff,0xff,0xff,0xff,0xff,0xff,0x00,0xff,0xff,0xff,0xff,0xff,0xff,0x00,
0x00,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x00,
0x00,0xff,0xff,0xcf,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xf3,0xff,0xff,0x00,
0x00,0xff,0xff,0x87,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xe1,0xff,0xff,0x00,
0x00,0xff,0xff,0x03,0xfe,0xff,0xff,0xff,0xff,0xff,0x7f,0xc0,0xff,0xff,0x00,
0x00,0xfe,0xff,0x01,0xf8,0xff,0xff,0xff,0xff,0xff,0x1f,0x80,0xff,0x7f,0x00,
0x00,0xfe,0xff,0x00,0xe0,0xff,0xff,0xff,0xff,0xff,0x07,0x00,0xff,0x7f,0x00,
0x00,0xfc,0x7f,0x00,0x80,0xff,0xff,0xff,0xff,0xff,0x01,0x00,0xfe,0x3f,0x00,
0x00,0xf8,0x3f,0x00,0x00,0xfe,0xff,0xff,0xff,0x7f,0x00,0x00,0xfc,0x1f,0x00,
0x00,0xf0,0x1f,0x00,0x00,0xf0,0xff,0xff,0xff,0x0f,0x00,0x00,0xf8,0x0f,0x00,
0x00,0xe0,0x0f,0x00,0x00,0x00,0xff,0xff,0xff,0x00,0x00,0x00,0xf0,0x07,0x00,
0x00,0x80,0x03,0x00,0x00,0x00,0xe0,0xff,0x07,0x00,0x00,0x00,0xc0,0x01,0x00
};

const image_desc alarm_images[] = {
    {alarm_clock_bits, alarm_clock_width, alarm_clock_height},
};