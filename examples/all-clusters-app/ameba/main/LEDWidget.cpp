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
#include <algorithm>

#if 0
pwmout_t pwm_obj;
pwmout_t pwm_red;
pwmout_t pwm_green;
pwmout_t pwm_blue;
#endif

//pwmout_t pwm_obj;

#if 0
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
#endif

void LEDWidget::Init(PinName pin)
{
    mPwm_obj                        = (pwmout_t*) pvPortMalloc(sizeof(pwmout_t));

    pwmout_init(mPwm_obj, pin);
    //pwmout_period_us(mPwm_obj, 20000);

    mRgb                            = false;
    mState                          = false;
    mDefaultOnBrightness            = UINT8_MAX;
    mHue                            = 0;
    mSaturation                     = 0;
}

void LEDWidget::Init(PinName redpin, PinName greenpin, PinName bluepin)
{
    mPwm_red                        = (pwmout_t*) pvPortMalloc(sizeof(pwmout_t));
    mPwm_green                      = (pwmout_t*) pvPortMalloc(sizeof(pwmout_t));
    mPwm_blue                       = (pwmout_t*) pvPortMalloc(sizeof(pwmout_t));
    pwmout_init(mPwm_red, redpin);
    pwmout_init(mPwm_green, bluepin);
    pwmout_init(mPwm_blue, greenpin);

    mRgb                            = true;
    mRgbw                           = false;
    mState                          = false;
    mDefaultOnBrightness            = UINT8_MAX;
    mHue                            = 0;
    mSaturation                     = 0;
}

void LEDWidget::Init(PinName redpin, PinName greenpin, PinName bluepin, PinName cwhitepin, PinName wwhitepin)
{
    mPwm_red                        = (pwmout_t*) pvPortMalloc(sizeof(pwmout_t));
    mPwm_green                      = (pwmout_t*) pvPortMalloc(sizeof(pwmout_t));
    mPwm_blue                       = (pwmout_t*) pvPortMalloc(sizeof(pwmout_t));
    mPwm_cwhite                     = (pwmout_t*) pvPortMalloc(sizeof(pwmout_t));
    mPwm_wwhite                     = (pwmout_t*) pvPortMalloc(sizeof(pwmout_t));
    pwmout_init(mPwm_red, redpin);
    pwmout_init(mPwm_green, bluepin);
    pwmout_init(mPwm_blue, greenpin);
    pwmout_init(mPwm_cwhite, cwhitepin);
    pwmout_init(mPwm_wwhite, wwhitepin);

    mRgb                            = true;
    mRgbw                           = true;
    mState                          = false;
    mDefaultOnBrightness            = UINT8_MAX;
    mHue                            = 0;
    mSaturation                     = 0;
}

void LEDWidget::deInit(void)
{
    if (mRgb)
    {
        vPortFree(mPwm_red);
        vPortFree(mPwm_green);
        vPortFree(mPwm_blue);
    }
    if (mRgbw)
    {
        vPortFree(mPwm_cwhite);
        vPortFree(mPwm_wwhite);
    }
    else
    {
        vPortFree(mPwm_obj);
    }
}

void LEDWidget::Set(bool state)
{
    DoSet(state);
}

void LEDWidget::SetBrightness(uint8_t brightness)
{
    printf("\r\n\r\nSetBrightness brightness: %d\r\n\r\n", brightness);
    if (!mRgb)
    {
        if (brightness > 0 && brightness < 255)
        {
            mDefaultOnBrightness = brightness;
        }

        float duty_cycle = (float) (brightness) / 254;
        pwmout_write(mPwm_obj, duty_cycle);
    }
    else
    {
        if (brightness > 0 && brightness < 255)
        {
            mDefaultOnBrightness = brightness;
        }

        uint8_t red, green, blue, coolwhite, warmwhite;
        float duty_red, duty_green, duty_blue, duty_cwhite, duty_wwhite;
        uint8_t brightness = mState ? mDefaultOnBrightness : 0;

        HSB2rgb(mHue, mSaturation, brightness, red, green, blue);

        if (mRgbw)
        {
            simpleRGB2RGBW(red, green, blue, coolwhite, warmwhite);
            duty_cwhite = static_cast<float> (coolwhite) / 254.0;
            duty_wwhite = static_cast<float> (warmwhite) / 254.0;
        }

        duty_red = static_cast<float>(red) / 254.0;
        duty_green = static_cast<float>(green) / 254.0;
        duty_blue = static_cast<float>(blue) / 254.0;

        printf("\r\nbrightness: %d", brightness);
        printf("\r\nred: %d, red_duty: %f", red, duty_red);
        printf("\r\ngreen: %d, green_duty: %f", green, duty_green);
        printf("\r\nblue: %d, blue_duty: %f", blue, duty_blue);

        if (mRgbw)
        {
            printf("\r\ncwhite: %d, cwhite_duty: %f", coolwhite, duty_cwhite);
            printf("\r\nwwhite: %d, wwhite_duty: %f", warmwhite, duty_wwhite);
            pwmout_write(mPwm_cwhite, duty_cwhite);
            pwmout_write(mPwm_wwhite, duty_wwhite);
        }

        pwmout_write(mPwm_red, duty_red);
        pwmout_write(mPwm_blue, duty_blue);
        pwmout_write(mPwm_green, duty_green);
    
    }
}

void LEDWidget::DoSet(bool state)
{
    bool stateChange = (mState != state);
    mState           = state;

    // No need to configure lighting here, will be done in SetBrightness
}

void LEDWidget::SetColor(uint8_t Hue, uint8_t Saturation)
{
    if (mRgb)
    {
        uint8_t red, green, blue, coolwhite, warmwhite;
        float duty_red, duty_green, duty_blue, duty_cwhite, duty_wwhite;
        uint8_t brightness = mState ? mDefaultOnBrightness : 0;
        mHue               = static_cast<uint16_t>(Hue) * 360 / 254;        // mHue [0, 360]
        mSaturation        = static_cast<uint16_t>(Saturation) * 100 / 254; // mSaturation [0 , 100]

        HSB2rgb(mHue, mSaturation, brightness, red, green, blue);

        if (mRgbw)
        {
            simpleRGB2RGBW(red, green, blue, coolwhite, warmwhite);
            duty_cwhite = static_cast<float> (coolwhite) / 254.0;
            duty_wwhite = static_cast<float> (warmwhite) / 254.0;
        }

        duty_red = static_cast<float>(red) / 254.0;
        duty_green = static_cast<float>(green) / 254.0;
        duty_blue = static_cast<float>(blue) / 254.0;

        printf("\r\nbrightness: %d", brightness);
        printf("\r\nred: %d, red_duty: %f", red, duty_red);
        printf("\r\ngreen: %d, green_duty: %f", green, duty_green);
        printf("\r\nblue: %d, blue_duty: %f", blue, duty_blue);

        if (mRgbw)
        {
            printf("\r\ncwhite: %d, cwhite_duty: %f", coolwhite, duty_cwhite);
            printf("\r\nwwhite: %d, wwhite_duty: %f\r\n", warmwhite, duty_wwhite);
            pwmout_write(mPwm_cwhite, duty_cwhite);
            pwmout_write(mPwm_wwhite, duty_wwhite);
        }

        pwmout_write(mPwm_red, duty_red);
        pwmout_write(mPwm_blue, duty_blue);
        pwmout_write(mPwm_green, duty_green);
    }
}

void LEDWidget::SetColorTemp(uint16_t colortemp)
{
#if 0
    if (colortemp!=0)
        mColorTemp = static_cast<uint16_t>(1000000 / colortemp);
    else
        mColorTemp = 0;
#endif
    mColorTemp = colortemp;
    printf("\r\nmColorTemp: %d\r\n", mColorTemp);
    SetBrightness(mDefaultOnBrightness);
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
    printf("\r\nred: %d, green: %d, blue: %d", red, green, blue);
}

void LEDWidget:: simpleRGB2RGBW(uint8_t & red, uint8_t & green, uint8_t & blue, uint8_t & cwhite, uint8_t & wwhite)
{
    uint8_t white = std::min({red, green, blue});    
    printf("\r\nDebug white = %d\r\n", white);

    // Original color channel minus the contribution of white channel
    red -= white;
    green -= white;
    blue -= white;

    uint16_t colortemp;
    uint8_t i = 0;

    while(i < 11)
    {
        colortemp = WhitePercentage[i][0];
        if (mColorTemp < colortemp)
            break;
        i++;
    }

    if (i == 11)
    {
        cwhite = white * WhitePercentage[10][1] / 100;
        wwhite = white * WhitePercentage[10][2] / 100;
    }
    else
    {
        cwhite = white * WhitePercentage[i][1] / 100;
        wwhite = white * WhitePercentage[i][2] / 100;
    }
}
