/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 * @file DeviceCallbacks.cpp
 *
 * Implements all the callbacks to the application from the CHIP Stack
 *
 **/
#include "DeviceCallbacks.h"

#include "CHIPDeviceManager.h"
#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/cluster-id.h>
#include <app/CommandHandler.h>
#include <app/server/Dnssd.h>
#include <app/util/af.h>
#include <app/util/basic-types.h>
#include <app/util/util.h>
#include <app/util/attribute-storage-null-handling.h>
#include <lib/dnssd/Advertiser.h>
#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>
#include <support/logging/Constants.h>

#include "Globals.h"
#include "LEDWidget.h"

using namespace ::chip;
using namespace ::chip::app;
using namespace ::chip::app::Clusters;
using namespace ::chip::Inet;
using namespace ::chip::System;
using namespace ::chip::DeviceLayer;
using namespace ::chip::DeviceManager;
using namespace ::chip::Logging;

uint32_t identifyTimerCount;
constexpr uint32_t kIdentifyTimerDelayMS = 250;

void DeviceCallbacks::DeviceEventCallback(const ChipDeviceEvent * event, intptr_t arg)
{
    switch (event->Type)
    {
    case DeviceEventType::kInternetConnectivityChange:
        OnInternetConnectivityChange(event);
        break;

    case DeviceEventType::kSessionEstablished:
        OnSessionEstablished(event);
        break;
    case DeviceEventType::kInterfaceIpAddressChanged:
        if ((event->InterfaceIpAddressChanged.Type == InterfaceIpChangeType::kIpV4_Assigned) ||
            (event->InterfaceIpAddressChanged.Type == InterfaceIpChangeType::kIpV6_Assigned))
        {
            // MDNS server restart on any ip assignment: if link local ipv6 is configured, that
            // will not trigger a 'internet connectivity change' as there is no internet
            // connectivity. MDNS still wants to refresh its listening interfaces to include the
            // newly selected address.
            chip::app::DnssdServer::Instance().StartServer();
        }
        break;
    }
}

void DeviceCallbacks::PostAttributeChangeCallback(EndpointId endpointId, ClusterId clusterId, AttributeId attributeId, uint8_t mask,
                                                  uint8_t type, uint16_t size, uint8_t * value)
{
    switch (clusterId)
    {
    case ZCL_ON_OFF_CLUSTER_ID:
        OnOnOffPostAttributeChangeCallback(endpointId, attributeId, value);
        break;

    case Clusters::LevelControl::Id:
        OnLevelControlAttributeChangeCallback(endpointId, attributeId, value);
        break;

    case Clusters::ColorControl::Id:
        OnColorControlAttributeChangeCallback(endpointId, attributeId, value);
        break;

    case ZCL_IDENTIFY_CLUSTER_ID:
        OnIdentifyPostAttributeChangeCallback(endpointId, attributeId, value);
        break;

    default:
        break;
    }
}

void DeviceCallbacks::OnInternetConnectivityChange(const ChipDeviceEvent * event)
{
    if (event->InternetConnectivityChange.IPv4 == kConnectivity_Established)
    {
        ChipLogProgress(DeviceLayer, "Server ready at: %s:%d", event->InternetConnectivityChange.address, CHIP_PORT);
        chip::app::DnssdServer::Instance().StartServer();
    }
    else if (event->InternetConnectivityChange.IPv4 == kConnectivity_Lost)
    {
        ChipLogProgress(DeviceLayer, "Lost IPv4 connectivity...");
    }
    if (event->InternetConnectivityChange.IPv6 == kConnectivity_Established)
    {
        ChipLogProgress(DeviceLayer, "IPv6 Server ready...");
        chip::app::DnssdServer::Instance().StartServer();
    }
    else if (event->InternetConnectivityChange.IPv6 == kConnectivity_Lost)
    {
        ChipLogProgress(DeviceLayer, "Lost IPv6 connectivity...");
    }
}

void DeviceCallbacks::OnSessionEstablished(const ChipDeviceEvent * event)
{
    if (event->SessionEstablished.IsCommissioner)
    {
        ChipLogProgress(DeviceLayer, "Commissioner detected!");
    }
}

void DeviceCallbacks::OnOnOffPostAttributeChangeCallback(EndpointId endpointId, AttributeId attributeId, uint8_t * value)
{
    VerifyOrExit(attributeId == ZCL_ON_OFF_ATTRIBUTE_ID,
                 ChipLogError(DeviceLayer, "Unhandled Attribute ID: '0x%04x", attributeId));
    VerifyOrExit(endpointId == 1,
                 ChipLogError(DeviceLayer, "Unexpected EndPoint ID: `0x%02x'", endpointId));

    // At this point we can assume that value points to a bool value.
    mEndpointOnOffState[endpointId - 1] = *value;
    ChipLogProgress(DeviceLayer, "Calling Set with value: %d", *value);
    rgbwLED.Set(*value);

exit:
    return;
}

void DeviceCallbacks::OnLevelControlAttributeChangeCallback(EndpointId endpointId, AttributeId attributeId, uint8_t * value)
{
    bool onOffState    = mEndpointOnOffState[endpointId - 1];
    uint8_t brightness = onOffState ? *value : 0;

    VerifyOrExit(attributeId == ZCL_CURRENT_LEVEL_ATTRIBUTE_ID,
            ChipLogError(DeviceLayer, "Unhandled Attribute ID: '0x%04x", attributeId));
    VerifyOrExit(endpointId == 1,
            ChipLogError(DeviceLayer, "Unexpected EndPoint ID: `0x%02x'", endpointId));

    // At this point we can assume that value points to a bool value.
    ChipLogProgress(DeviceLayer, "Calling SetBrightness with value: %d", brightness);
    rgbwLED.SetBrightness(brightness);

exit:
    return;
}

void DeviceCallbacks::OnColorControlAttributeChangeCallback(EndpointId endpointId, AttributeId attributeId, uint8_t * value)
{
    VerifyOrExit(
            attributeId == ZCL_COLOR_CONTROL_CURRENT_HUE_ATTRIBUTE_ID ||
            attributeId == ZCL_COLOR_CONTROL_CURRENT_SATURATION_ATTRIBUTE_ID ||
            attributeId == ZCL_COLOR_CONTROL_COLOR_TEMPERATURE_ATTRIBUTE_ID,
            ChipLogError(DeviceLayer, "Unhandled AttributeId ID: '0x%04x", attributeId)
            );
    VerifyOrExit(
            endpointId == 1,
            ChipLogError(DeviceLayer, "Unexpected EndPoint ID: `0x%02x'", endpointId)
            );

    if (attributeId  == ZCL_COLOR_CONTROL_CURRENT_HUE_ATTRIBUTE_ID || attributeId == ZCL_COLOR_CONTROL_CURRENT_SATURATION_ATTRIBUTE_ID)
    {
        if (endpointId == 1)
        {
            uint8_t hue, saturation;
            if (attributeId == ZCL_COLOR_CONTROL_CURRENT_HUE_ATTRIBUTE_ID)
            {
                hue = *value;
                emberAfReadServerAttribute(endpointId, ZCL_COLOR_CONTROL_CLUSTER_ID, ZCL_COLOR_CONTROL_CURRENT_SATURATION_ATTRIBUTE_ID,
                                           &saturation, sizeof(uint8_t));
            }
            else
            {
                saturation = *value;
                emberAfReadServerAttribute(endpointId, ZCL_COLOR_CONTROL_CLUSTER_ID, ZCL_COLOR_CONTROL_CURRENT_HUE_ATTRIBUTE_ID, &hue,
                                           sizeof(uint8_t));
            }
            ChipLogProgress(DeviceLayer, "Calling SetColor with hue:%d, saturation:%d", hue, saturation);
            rgbwLED.SetColor(hue, saturation);
        }
    }

    if (attributeId == ZCL_COLOR_CONTROL_COLOR_TEMPERATURE_ATTRIBUTE_ID)
    {
        if (endpointId == 1)
        {
            using Traits = NumericAttributeTraits<uint16_t>;
            Traits::StorageType temp;
            uint8_t * readable   = Traits::ToAttributeStoreRepresentation(temp);
            emberAfReadServerAttribute(endpointId, ZCL_COLOR_CONTROL_CLUSTER_ID, ZCL_COLOR_CONTROL_COLOR_TEMPERATURE_ATTRIBUTE_ID, readable, sizeof(temp));

            uint16_t colortemp;
            colortemp = Traits::StorageToWorking(temp);
            rgbwLED.SetColorTemp(colortemp);
        }
    }
exit:
    return;
}

void IdentifyTimerHandler(Layer * systemLayer, void * appState, CHIP_ERROR error)
{
    if (identifyTimerCount)
    {
        identifyTimerCount--;
    }
}

void DeviceCallbacks::OnIdentifyPostAttributeChangeCallback(EndpointId endpointId, AttributeId attributeId, uint8_t * value)
{
    VerifyOrExit(attributeId == ZCL_IDENTIFY_TIME_ATTRIBUTE_ID,
                 ChipLogError(DeviceLayer, "[%s] Unhandled Attribute ID: '0x%04x", attributeId));
    VerifyOrExit(endpointId == 1,
            ChipLogError(DeviceLayer, "[%s] Unexpected EndPoint ID: `0x%02x'", endpointId));

    // timerCount represents the number of callback executions before we stop the timer.
    // value is expressed in seconds and the timer is fired every 250ms, so just multiply value by 4.
    // Also, we want timerCount to be odd number, so the ligth state ends in the same state it starts.
    identifyTimerCount = (*value) * 4;
exit:
    return;
}

bool emberAfBasicClusterMfgSpecificPingCallback(chip::app::CommandHandler * commandObj)
{
    emberAfSendDefaultResponse(emberAfCurrentCommand(), EMBER_ZCL_STATUS_SUCCESS);
    return true;
}
