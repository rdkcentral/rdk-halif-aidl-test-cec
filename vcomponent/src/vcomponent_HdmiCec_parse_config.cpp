/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2024 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/** @brief
* vcomponent_HdmiCec_parse_config.cpp
*
* Implementation of vcomponent HdmiCec driver
*/

#include "vcomponent_HdmiCec_parse_config.h"

cecConfiguration_t gCecConfiguration;

static std::string prefix;
static std::string key;

static void* vcomponentkvp_create_instance(char *fileName)
{
    ut_kvp_instance_t* instance;
    ut_kvp_status_t result;

    instance = ut_kvp_createInstance();

    if(!instance)
    {
        UT_LOG_ERROR("[VCOMPONENT_HDMICEC]Failed to create ut kvp instance");
        return nullptr;
    }

    result = ut_kvp_open(instance, fileName);

    if(result != UT_KVP_STATUS_SUCCESS)
    {
        UT_LOG_ERROR("[VCOMPONENT_HDMICEC]Failed to open test configuration file");
        ut_kvp_destroyInstance(instance);
        return nullptr;
    }

    return (ut_kvp_instance_t*)instance;
}

static void vcomponentkvp_destroy_instance(void *instance)
{
    ut_kvp_destroyInstance((ut_kvp_instance_t*)instance);
}

void vcomponent_HdmiCec_prase_config(char *cofigurationFile)
{
    char resultKVP[UT_KVP_MAX_ELEMENT_SIZE] = {0};

    void *testconfig = vcomponentkvp_create_instance(cofigurationFile);
    ut_kvp_instance_t* kvpTestInstance = (ut_kvp_instance_t*)testconfig;

    gCecConfiguration.devices_connected.clear();
    gCecConfiguration.cec_opcodes.clear();

    // Parse all devices
    uint32_t deviceCount = ut_kvp_getListCount(kvpTestInstance, HDCM_DEVICE);

    for (uint32_t i = 0; i < deviceCount; ++i)
    {
        deviceInfo_t dev;

        prefix = std::string(HDCM_DEVICE) + "." + std::to_string(i) + ".";

        ut_kvp_getStringField(kvpTestInstance, (prefix + "name").c_str(), resultKVP, sizeof(resultKVP));
        dev.name = resultKVP;

        ut_kvp_getStringField(kvpTestInstance, (prefix + "language").c_str(), resultKVP, sizeof(resultKVP));
        dev.language.push_back(resultKVP[0]);
        dev.language.push_back(resultKVP[1]);
        dev.language.push_back(resultKVP[2]);

        dev.cec_version = ut_kvp_getUInt8Field(kvpTestInstance, (prefix + "cec_version").c_str());
        dev.logical_address = ut_kvp_getUInt8Field(kvpTestInstance, (prefix + "logical_address").c_str());
        dev.physical_address[0] = ut_kvp_getUInt8Field(kvpTestInstance, (prefix + "physical_address.0").c_str());
        dev.physical_address[1] = ut_kvp_getUInt8Field(kvpTestInstance, (prefix + "physical_address.1").c_str());

        dev.vendor_id = ut_kvp_getUInt32Field(kvpTestInstance, (prefix + "vendor_id").c_str());
        dev.device_type = ut_kvp_getUInt8Field(kvpTestInstance, (prefix + "device_type").c_str());
        dev.power_status = ut_kvp_getUInt8Field(kvpTestInstance, (prefix + "power_status").c_str());
        dev.fault = ut_kvp_getUInt8Field(kvpTestInstance, (prefix + "fault").c_str());

        if(dev.fault == 0x1)
            gCecConfiguration.par_fault_devices.push_back(dev.logical_address);
        else if(dev.fault == 0x2)
            gCecConfiguration.fault_devices.push_back(dev.logical_address);
        else
            gCecConfiguration.working_devices.push_back(dev.logical_address);

        gCecConfiguration.connected_devices.push_back(dev.logical_address);

        gCecConfiguration.devices_connected.push_back(dev);
    }

    // Parse all CEC opcodes and responses
    uint32_t msgCount = ut_kvp_getListCount(kvpTestInstance, HDCM_CEC_MESSAGES);
    for (uint32_t i = 0; i < msgCount; ++i)
    {
        cecResponse_t resp;

        prefix = std::string(HDCM_CEC_MESSAGES) + "." + std::to_string(i) + ".";

        // --- Request ---
        key = prefix + "request.opcode";
        resp.command.opcode = ut_kvp_getUInt8Field(kvpTestInstance, key.c_str());

        key = prefix + "request.type";
        ut_kvp_getStringField(kvpTestInstance, key.c_str(), resultKVP, sizeof(resultKVP));
        if (strcmp(resultKVP, "broadcast") == 0 || strcmp(resultKVP, "Broadcast") == 0)
            resp.command.type = CEC_TYPE_BROADCAST;
        else
            resp.command.type = CEC_TYPE_DIRECT;

        key = prefix + "request.payload";
        int payloadSize = ut_kvp_getListCount(kvpTestInstance, key.c_str());

        resp.command.payload.clear();
        for(uint8_t j = 0; j < payloadSize; j++)
        {
            //TODO: ignore income payload
        }

        // --- Response ---
        key = prefix + "response.opcode";

        if(ut_kvp_fieldPresent(kvpTestInstance, key.c_str()))
        {
            resp.response.opcode = ut_kvp_getUInt8Field(kvpTestInstance, key.c_str());

            key = prefix + "response.type";

            ut_kvp_getStringField(kvpTestInstance, key.c_str(), resultKVP, sizeof(resultKVP));
            if (strcmp(resultKVP, "broadcast") == 0 || strcmp(resultKVP, "Broadcast") == 0)
                resp.response.type = CEC_TYPE_BROADCAST;
            else
                resp.response.type = CEC_TYPE_DIRECT;

            key = prefix + "response.payload";
            payloadSize = ut_kvp_getListCount(kvpTestInstance, key.c_str());
            resp.response.payload.clear();

            for(uint8_t j = 0; j < payloadSize; j++)
            {
                string payloadKey = key + "." + to_string(j);
                ut_kvp_getStringField(kvpTestInstance, payloadKey.c_str(), resultKVP, sizeof(resultKVP));
                resp.response.payload.push_back(resultKVP);
            }
        }
        else
        {
            resp.response.opcode = 0xFF;
        }

        gCecConfiguration.cec_opcodes.push_back(resp);
    }

    vcomponentkvp_destroy_instance(kvpTestInstance);
}

void vcomponent_HdmiCec_get_cec_command(uint8_t opcode, cecResponse_t &cmd)
{
    for (const auto& entry : gCecConfiguration.cec_opcodes)
    {
        if (entry.command.opcode == opcode)
        {
            cmd = entry;
            return;
        }
    }
    cmd.command.opcode = 0xFF; // Indicate unsupported command
}

deviceInfo_t* vcomponent_Hdmicec_get_connectedDevice(uint8_t src)
{
    if(std::find(gCecConfiguration.fault_devices.begin(), gCecConfiguration.fault_devices.end(), src) != gCecConfiguration.fault_devices.end())
        return nullptr;

    for (auto& device : gCecConfiguration.devices_connected) {
        if (device.logical_address == src) {
            return &device;
        }
    }
    return nullptr;
}
bool vcomponent_HdmiCec_get_cec_response(uint8_t src, uint8_t dst,
                                         cecOpcode_t &response, std::vector<uint8_t>& message)
{
    deviceInfo_t *pConnectedDev = vcomponent_Hdmicec_get_connectedDevice(src);

    if(pConnectedDev == nullptr)
    {
        return false;
    }
    bool found = false;
    message.push_back(src | (dst << 4));
    message.push_back(response.opcode);

    for(const auto& property : response.payload)
    {
        if(property == "physical_address")
        {
            message.push_back(pConnectedDev->physical_address[0]);
            message.push_back(pConnectedDev->physical_address[1]);
        }
        else if(property == "device_type")
        {
            message.push_back(pConnectedDev->device_type);
        }
        else if(property == "power_status")
        {
            message.push_back(pConnectedDev->power_status);
        }
        else if(property == "cec_version")
        {
            message.push_back(pConnectedDev->cec_version);
        }
        else if(property == "vendor_id")
        {
            message.push_back((pConnectedDev->vendor_id >> 16)&0xFF);
            message.push_back((pConnectedDev->vendor_id >> 8)&0xFF);
            message.push_back((pConnectedDev->vendor_id)&0xFF);
        }
        else if(property == "osd_name")
        {
            std::vector<uint8_t> asciiValues(pConnectedDev->name.begin(), pConnectedDev->name.end());
            for (uint8_t val : asciiValues)
            {
                message.push_back(val);
            }
        }
        else if(property == "language")
        {
            message.push_back(pConnectedDev->language[0]);
            message.push_back(pConnectedDev->language[1]);
            message.push_back(pConnectedDev->language[2]);
        }
        else if(property == "latency")
        {
            //TODO: move to yaml if required
            message.push_back(0x00);
            message.push_back(0x00);
            message.push_back(0x65);
            message.push_back(0x00);
            message.push_back(0x33);
        }
    }
    return true;
}