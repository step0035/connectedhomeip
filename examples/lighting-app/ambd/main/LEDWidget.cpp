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
#if 1 // test_event_6.lts
#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/cluster-id.h>
#include <app/server/Server.h>
#include <app/util/attribute-storage.h>
#else // master 2021-12-19
#include <app/clusters/on-off-server/on-off-server.h>
#endif

gpio_t gpio_led;

void LEDWidget::Init(PinName gpioNum)
{

    mGPIONum = gpioNum;
    mState   = false;

    if (gpioNum != (PinName)NC)
    {
        // Init LED control pin
        gpio_init(&gpio_led, gpioNum);
        gpio_dir(&gpio_led, PIN_OUTPUT);    // Direction: Output
        gpio_mode(&gpio_led, PullNone);     // No pull
        gpio_write(&gpio_led, mState);
    }
}

void LEDWidget::Set(bool state)
{
    DoSet(state);
}

void LEDWidget::DoSet(bool state)
{
    bool stateChange = (mState != state);

    mState           = state;

    if (stateChange)
    {
       gpio_write(&gpio_led, state);
    }
}

extern "C" void changeValue(bool value)
{
    uint8_t newValue = value;
    // write the new on/off value
#if 1 // test_event_6.lts
    EmberAfStatus status = emberAfWriteAttribute(1/*endpoint*/, ZCL_ON_OFF_CLUSTER_ID, ZCL_ON_OFF_ATTRIBUTE_ID, CLUSTER_MASK_SERVER,
                                                 (uint8_t *) &newValue, ZCL_BOOLEAN_ATTRIBUTE_TYPE);
#else
    // master 2021-12-19
    EmberAfStatus status = OnOffServer::Instance().setOnOffValue(1, newValue, false);
#endif
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        printf("ERR: updating on/off %x", status);
    }
}
