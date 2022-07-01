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
#pragma once

#include <platform_stdlib.h>
#include <ExtendedColorLightManager.h>
#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/cluster-id.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/att-storage.h>
#include <app/util/af.h>
#include <app/util/af-enums.h>
#include <app/util/attribute-table.h>

void ExtendedColorLightManager::Init(uint8_t test_val)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;

    uint8_t onoff_init_val = 1;
    status = emberAfWriteAttribute(1, ZCL_ON_OFF_CLUSTER_ID, ZCL_ON_OFF_ATTRIBUTE_ID, CLUSTER_MASK_SERVER, (uint8_t *) &onoff_init_val, ZCL_BOOLEAN_ATTRIBUTE_TYPE);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        printf("Error: Update on/off attribute %x", status);
    }

    uint8_t level_init_val = 254; //0xFE
    status = emberAfWriteAttribute(1, ZCL_LEVEL_CONTROL_CLUSTER_ID, ZCL_CURRENT_LEVEL_ATTRIBUTE_ID, CLUSTER_MASK_SERVER, (uint8_t *) &level_init_val, ZCL_INT8U_ATTRIBUTE_TYPE);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        printf("Error: Update current level attribute %x", status);
    }

    uint16_t cct_init_val = 27;
    status = emberAfWriteAttribute(1, ZCL_COLOR_CONTROL_CLUSTER_ID, ZCL_COLOR_CONTROL_COLOR_TEMPERATURE_ATTRIBUTE_ID, CLUSTER_MASK_SERVER, (uint8_t *) &cct_init_val, ZCL_INT16U_ATTRIBUTE_TYPE);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        printf("Error: Update color temperature attribute %x", status);
    }
}

void ExtendedColorLightManager::Set(uint8_t state)
{
    DoSet(state);
}

void ExtendedColorLightManager::DoSet(uint8_t state)
{
    //Peripheral action
    printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!ExtendedColorLight action= %d\n", state);
}
