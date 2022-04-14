/*
 *
 *    Copyright (c) 2018 Nest Labs, Inc.
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/**
 * @file LEDWidget.cpp
 *
 * Implements an LED Widget controller that is usually tied to a GPIO
 * It also updates the display widget if it's enabled
 */

#include "LEDWidget.h"

pwmout_t pwm_obj;
pwmout_t pwm_red;
pwmout_t pwm_green;
pwmout_t pwm_blue;

void LEDWidget::Init(PinName pin)
{
    pwmout_init(&pwm_obj, pin);

    pwmout_init(&pwm_red, PA_19);
    pwmout_init(&pwm_blue, PA_20);
    pwmout_init(&pwm_green, PA_18);
    pwmout_period_us(&pwm_obj, 20000);

    mState                          = false;
    mDefaultOnBrightness            = UINT8_MAX;
    mHue                            = 0;
    mSaturation                     = 0;
}

void LEDWidget::Set(bool state)
{
    DoSet(state);
}

void LEDWidget::SetBrightness(uint8_t brightness)
{
    float duty_cycle = (float) (brightness) / 254;
    pwmout_write(&pwm_obj, duty_cycle);

    if (brightness > 0 && brightness < 255)
    {
        mDefaultOnBrightness = brightness;
    }
}

void LEDWidget::DoSet(bool state)
{
    bool stateChange = (mState != state);
    mState           = state;

    // No need to configure lighting here, will be don in SetBrightness
}

void LEDWidget::SetColor(uint8_t Hue, uint8_t Saturation)
{
    uint8_t red, green, blue;
    float duty_red, duty_green, duty_blue;
    uint8_t brightness = mState ? mDefaultOnBrightness : 0;
    mHue               = static_cast<uint16_t>(Hue) * 360 / 254;        // mHue [0, 360]
    mSaturation        = static_cast<uint16_t>(Saturation) * 100 / 254; // mSaturation [0 , 100]

    HSB2rgb(mHue, mSaturation, brightness, red, green, blue);
    duty_red = static_cast<float> (red) / 255;
    duty_green = static_cast<float> (green) / 255;
    duty_blue = static_cast<float> (blue) / 255;
    printf("\r\nred: %d, duty_red: %f\r\n", red, duty_red);
    printf("\r\ngreen: %d, duty_blue: %f\r\n", green, duty_green);
    printf("\r\nblue: %d, duty_blue: %f\r\n", blue, duty_blue);
    pwmout_write(&pwm_red, duty_red);
    pwmout_write(&pwm_blue, duty_blue);
    pwmout_write(&pwm_green, duty_green);
}

void LEDWidget::HSB2rgb(uint16_t Hue, uint8_t Saturation, uint8_t brightness, uint8_t & red, uint8_t & green, uint8_t & blue)
{
    uint16_t i       = Hue / 60;
    uint16_t rgb_max = brightness;
    uint16_t rgb_min = rgb_max * (100 - Saturation) / 100;
    uint16_t diff    = Hue % 60;
    uint16_t rgb_adj = (rgb_max - rgb_min) * diff / 60;

    switch (i)
    {
    case 0:
        red   = rgb_max;
        green = rgb_min + rgb_adj;
        blue  = rgb_min;
        break;
    case 1:
        red   = rgb_max - rgb_adj;
        green = rgb_max;
        blue  = rgb_min;
        break;
    case 2:
        red   = rgb_min;
        green = rgb_max;
        blue  = rgb_min + rgb_adj;
        break;
    case 3:
        red   = rgb_min;
        green = rgb_max - rgb_adj;
        blue  = rgb_max;
        break;
    case 4:
        red   = rgb_min + rgb_adj;
        green = rgb_min;
        blue  = rgb_max;
        break;
    default:
        red   = rgb_max;
        green = rgb_min;
        blue  = rgb_max - rgb_adj;
        break;
    }
}
