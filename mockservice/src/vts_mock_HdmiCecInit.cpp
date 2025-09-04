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
* vts_mock_HdmiCecInit.cpp
*
* Initializes HdmiCec service
*/

#include "vts_mock_HdmiCecInit.h"

void *gHdmiCecMockKVPInstance = nullptr;
hdmicec_mock_config_t gHdmiCecConfig = {0};

cecCommandMap_t cecCommandTable[] =
{
    {0x00, "Feature Abort", 2},
    {0x04, "Image View On", 0},
    {0x05, "Tuner Step Increment", 0},
    {0x06, "Tuner Step Decrement", 0},
    {0x07, "Tuner Device Status", 8},
    {0x08, "Give Tuner Device Status", 0},
    {0x09, "Record On", 8},
    {0x0A, "Record Status", 8},
    {0x0B, "Record Off", 0},
    {0x0D, "Text View On", 0},
    {0x0F, "Record TV Screen", 0},
    {0x1A, "Give Deck Status", 0},
    {0x1B, "Deck Status", 1},
    {0x32, "Set Menu Language", 3},
    {0x33, "Clear Analog Timer", 0},
    {0x34, "Set Analog Timer", 8},
    {0x35, "Timer Status", 3},
    {0x36, "Standby", 0},
    {0x41, "Play", 0},
    {0x42, "Deck Control", 1},
    {0x43, "Timer Cleared Status", 1},
    {0x44, "User Control Pressed", 1},
    {0x45, "User Control Released", 0},
    {0x46, "Give OSD Name", 0},
    {0x47, "Set OSD Name", 14},
    {0x64, "Set OSD String", 14},
    {0x67, "Set Timer Program Title", 14},
    {0x70, "System Audio Mode Request", 2},
    {0x71, "Give Audio Status", 0},
    {0x72, "Set System Audio Mode", 1},
    {0x7A, "Report Audio Status", 1},
    {0x7D, "Give System Audio Mode Status", 0},
    {0x7E, "System Audio Mode Status", 1},
    {0x80, "Routing Change", 4},
    {0x81, "Routing Information", 2},
    {0x82, "Active Source", 2},
    {0x83, "Give Physical Address", 0},
    {0x84, "Report Physical Address", 3},
    {0x85, "Request Active Source", 0},
    {0x86, "Set Stream Path", 2},
    {0x87, "Device Vendor ID", 3},
    {0x89, "Vendor Command", 14},
    {0x8A, "Vendor Remote Button Down", 1},
    {0x8B, "Vendor Remote Button Up", 0},
    {0x8C, "Give Device Vendor ID", 0},
    {0x8D, "Menu Request", 1},
    {0x8E, "Menu Status", 1},
    {0x8F, "Give Device Power Status", 0},
    {0x90, "Report Power Status", 1},
    {0x91, "Get Menu Language", 0},
    {0x92, "Select Analog Service", 4},
    {0x93, "Select Digital Service", 4},
    {0x97, "Set Digital Timer", 6},
    {0x99, "Clear Digital Timer", 0},
    {0x9A, "Set Audio Rate", 1},
    {0x9D, "Inactive Source", 2},
    {0x9E, "CEC Version", 1},
    {0x9F, "Get CEC Version", 0},
    {0xA0, "Vendor Command With ID", 17},
    {0xA1, "Clear External Timer", 0},
    {0xA2, "Set External Timer", 9},
    {0xA7, "Request Current Latency", 2},
    {0xA8, "Report Current Latency", 5},
    {0xC0, "Initiate ARC", 0},
    {0xC1, "Report ARC Initiated", 0},
    {0xC2, "Report ARC Terminated", 0},
    {0xC3, "Request ARC Initiation", 0},
    {0xC4, "Request ARC Termination", 0},
    {0xC5, "Terminate ARC", 0},
    {0xFF, "Abort", 0}
};

static void* vts_kvp_create_instance(char *fileName)
{
    ut_kvp_instance_t* instance;
    ut_kvp_status_t result;

    instance = ut_kvp_createInstance();

    if(!instance)
    {
        UT_LOG_ERROR("[HDMICEC_MOCK]Failed to create ut kvp instance");
        return nullptr;
    }

    result = ut_kvp_open(instance, fileName);

    if(result != UT_KVP_STATUS_SUCCESS)
    {
        UT_LOG_ERROR("[HDMICEC_MOCK]Failed to open test configuration file");
        ut_kvp_destroyInstance(instance);
        return nullptr;
    }

    return (ut_kvp_instance_t*)instance;
}

static void vts_parse_api_response_config(void *pTestInstance)
{
}

static void vts_parse_get_command(ut_kvp_instance_t* kvpTestInstance, string startKey, cecCommands_t &cmd)
{
    char resultKVP[UT_KVP_MAX_ELEMENT_SIZE] = {0};
    string command = startKey + "." + "command";
    string type = startKey + "." + "type";
    string payload = startKey + "." + "payload";

    cmd.cecCommand = ut_kvp_getUInt8Field(kvpTestInstance, command.c_str());
    ut_kvp_getStringField(kvpTestInstance, type.c_str(), resultKVP, UT_KVP_MAX_ELEMENT_SIZE);

    if (string(resultKVP) == "Broadcast")
        cmd.type = CEC_TYPE_BROADCAST;
    else
        cmd.type = CEC_TYPE_DIRECT;

    cmd.payloadSize = ut_kvp_getListCount(kvpTestInstance, payload.c_str());
    for(uint8_t j = 0; j < cmd.payloadSize; j++)
    {
        string payloadKey = startKey + "." + "payload" + "." + to_string(j);
        uint8_t payloadVal = ut_kvp_getUInt8Field(kvpTestInstance, payloadKey.c_str());
        cmd.payload.push_back(payloadVal);
    }
}

static void vts_parse_cec_response_config(void *pTestInstance, hdmicec_mock_config_t &config)
{
    ut_kvp_instance_t* kvpTestInstance = (ut_kvp_instance_t*)pTestInstance;

    config.cecCommands.clear();

    int32_t numEntries = ut_kvp_getListCount(kvpTestInstance, HDMICEC_CEC);

    for (int i = 0; i < numEntries; ++i)
    {
        cecConfigCommands_t cfgCmd = {};

        vts_parse_get_command(kvpTestInstance, string(HDMICEC_CEC) + "." + to_string(i), cfgCmd.command);

        if(ut_kvp_fieldPresent(kvpTestInstance, (string(HDMICEC_CEC) + "." + to_string(i) + ".response.command").c_str()))
        {
            vts_parse_get_command(kvpTestInstance, string(HDMICEC_CEC) + "." + to_string(i) + ".response", cfgCmd.response);
        }
        else
        {
            cfgCmd.response.cecCommand = 0xFF; // No response defined
        }
        config.cecCommands.push_back(cfgCmd);
    }
}

static void vts_parse_mock_configration(void *pTestInstance, hdmicec_mock_config_t &config)
{
    ut_kvp_status_t status;
    ut_kvp_instance_t* kvpTestInstance = (ut_kvp_instance_t*)pTestInstance;
    int32_t connected_count = 0;
    int32_t fault_count = 0;
    int32_t partial_fault_count = 0;

    config.version = ut_kvp_getUInt32Field(kvpTestInstance, HDMICEC_CONFIG_VERSION);

    connected_count = ut_kvp_getListCount(kvpTestInstance, HDMICEC_CONNECTED_DEVICES);
    for(int32_t i = 0; i < connected_count; i++)
    {
        uint8_t device_logical;
        string device_key = string(HDMICEC_CONNECTED_DEVICES) + "." + to_string(i);
        device_logical = ut_kvp_getUInt8Field(kvpTestInstance, device_key.c_str());
        config.connectedDevices.push_back(device_logical);
    }

    fault_count = ut_kvp_getListCount(kvpTestInstance, HDMICEC_FAULTY_DEVICES);
    for(int32_t i = 0; i < fault_count; i++)
    {
        uint8_t fault_logical;
        fault_logical = ut_kvp_getUInt8Field(kvpTestInstance, (string(HDMICEC_FAULTY_DEVICES) + "." + to_string(i)).c_str());
        config.faultyDevices.push_back(fault_logical);
    }

    partial_fault_count = ut_kvp_getListCount(kvpTestInstance, HDMICEC_PARTIAL_FAULTY_DEVICES);
    for(int32_t i = 0; i < partial_fault_count; i++)
    {
        uint8_t partial_fault_logical;
        partial_fault_logical = ut_kvp_getUInt8Field(kvpTestInstance, (string(HDMICEC_PARTIAL_FAULTY_DEVICES) + "." + to_string(i)).c_str());
        config.partialFaultyDevices.push_back(partial_fault_logical);
    }

    vts_parse_cec_response_config(kvpTestInstance, config);
}

void vts_get_cec_command(uint8_t opcode, cecConfigCommands_t &cmd)
{
    for (const auto& entry : gHdmiCecConfig.cecCommands)
    {
        if (entry.command.cecCommand == opcode)
        {
            cmd = entry;
            return;
        }
    }
    cmd.command.cecCommand = 0xFF; // Indicate unsupported command
}

void vts_print_cec_command(const cecCommands_t &cmd)
{
    UT_LOG_DEBUG("CEC Command: 0x%02X", cmd.cecCommand);
    UT_LOG_DEBUG("Payload Size: %u", cmd.payloadSize);
    UT_LOG_DEBUG("Type: %s", (cmd.type == CEC_TYPE_BROADCAST) ? "Broadcast" : "Direct");
    UT_LOG_DEBUG("Payload: ");
    for (const auto& byte : cmd.payload)
    {
        UT_LOG_DEBUG("0x%02X ", byte);
    }
}

bool HdmiCecInit(char *configFile)
{
    gHdmiCecMockKVPInstance = vts_kvp_create_instance(configFile);

    if(!gHdmiCecMockKVPInstance)
    {
        UT_LOG_ERROR("[HDMICEC_MOCK]Failed to create kvp instance for config file: %s", configFile);
        return false;
    }

    vts_parse_mock_configration(gHdmiCecMockKVPInstance, gHdmiCecConfig);

    /*for (const auto& entry : gHdmiCecConfig.cecCommands)
    {
        UT_LOG_DEBUG("Parsed CEC Command:");
        vts_print_cec_command(entry.command);
        if (entry.response.cecCommand != 0xFF)
        {
            UT_LOG_DEBUG("Parsed CEC Response:");
            vts_print_cec_command(entry.response);
        }
    }*/

    return true;
}