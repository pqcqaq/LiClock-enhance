#include <A_Config.h>

#include "images/graph.h"

float rain_data_raw[SAMPLE_COUNT];
int ydata[SCREEN_WIDTH + PX_PER_SAMPLE];

// 余弦
float cosineInterpolation(float point) {
    float result = 0.0;

    // 寻找插值区间
    int leftIndex = (int)floor(point);
    int rightIndex = leftIndex + 1;

    // 计算权重
    float weight = (1 - cos((point - leftIndex) * M_PI)) / 2;

    // 执行余弦插值计算
    result = (1 - weight) * rain_data_raw[leftIndex] +
             weight * rain_data_raw[rightIndex];

    return result;
}

// 对降水数据进行插值
void processRain(float max = 300.0) {
    float factor =
        (float)(SAMPLE_COUNT / SAMPLE_STEP - 2) / (float)(SCREEN_WIDTH);
    int i;
    for (i = 0; i < SCREEN_WIDTH + PX_PER_SAMPLE; i++) {
        ydata[i] = (int)(cosineInterpolation(((float)i) * factor) *
                         GRAPH_HEIGHT / max);
    }
}

// 下面是天气图标部分
const image_desc weather_icons_day[] = {
    {sun_bits, sun_width, sun_height},
    {moon_bits, moon_width, moon_height},
    {cloud_sun_bits, cloud_sun_width, cloud_sun_height},
    {cloud_moon_bits, cloud_moon_width, cloud_moon_height},
    {cloud_bits, cloud_width, cloud_height},
    {cloud_bits, cloud_width, cloud_height},
    {fog_bits, fog_width, fog_height},
    {fog_bits, fog_width, fog_height},
    {cloud_rain_bits, cloud_rain_width, cloud_rain_height},
    {cloud_rain_bits, cloud_rain_width, cloud_rain_height},
    {cloud_showers_bits, cloud_showers_width, cloud_showers_height},
    {cloud_showers_bits, cloud_showers_width, cloud_showers_height},
    {fog_bits, fog_width, fog_height},
    {snowflake_bits, snowflake_width, snowflake_height},
    {snowflake_bits, snowflake_width, snowflake_height},
    {snowflake_bits, snowflake_width, snowflake_height},
    {snowflake_bits, snowflake_width, snowflake_height},
    {sun_dust_bits, sun_dust_width, sun_dust_height},
    {sun_dust_bits, sun_dust_width, sun_dust_height},
    {wind_bits, wind_width, wind_height},
    //{thunderstorm_bits, thunderstorm_width, thunderstorm_height},
};
