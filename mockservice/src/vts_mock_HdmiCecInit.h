/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
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
* vts_mock_HdmiCecInit.h
*
* Header file for the HdmiCec service Initialization
*/

#ifndef __HDMICECINIT_H__
#define __HDMICECINIT_H__

#include <ut.h>
#include <ut_log.h>
#include <ut_kvp_profile.h>
#include <ut_control_plane.h>

using namespace std;

#define HDMICEC_CONFIG_VERSION "hdmicec_mock.version"
#define HDMICEC_CONNECTED_DEVICES "hdmicec_mock.connected_devices"
#define HDMICEC_FAULTY_DEVICES "hdmicec_mock.fault_devices"
#define HDMICEC_PARTIAL_FAULTY_DEVICES "hdmicec_mock.partial_fault_Devices"
#define HDMICEC_CEC "hdmicec_cec_response"
#define HDMI_CEC_MAX_PAYLOAD 128

typedef enum cecType
{
    CEC_TYPE_BROADCAST,
    CEC_TYPE_DIRECT
} cecType_t;

typedef struct cecCommandMap
{
    uint8_t cecCommand;       // CEC command code
    const char* commandName;  // Human-readable command name
    int32_t dataLength;       // Number of data bytes required for the command
} cecCommandMap_t;

typedef struct cecCommands
{
    uint8_t cecCommand;    //CEC opcode
    uint32_t payloadSize;  //CEC payload size
    cecType_t type;        //Type of the opcode Broadcast/ Direct
    vector<uint8_t> payload;  //CEC Payload
} cecCommands_t;

typedef struct cecConfigCommands
{
    cecCommands_t command;
    cecCommands_t response;
} cecConfigCommands_t;

typedef struct hdmicec_mock_config
{
    int32_t version;  // 0x01 - CEC 1.2, 0x02 - CEC 1.2a, 0x03 - CEC 1.3
                      // 0x04 - CEC 1.4, 0x05 - CEC 2.0
    vector<uint8_t> connectedDevices;
    vector<uint8_t> partialFaultyDevices;
    vector<uint8_t> faultyDevices;
    vector<cecConfigCommands_t> cecCommands;
} hdmicec_mock_config_t;

extern void *gHdmiCecMockKVPInstance;
extern hdmicec_mock_config_t gHdmiCecConfig;

void vts_get_cec_command(uint8_t opcode, cecConfigCommands_t &cmd);

bool HdmiCecInit(char *configFile);

#endif //__HDMICECINIT_H__