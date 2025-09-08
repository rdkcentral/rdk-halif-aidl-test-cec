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
* vcomponent_HdmiCec_parse_config.h
*
* Header file for the HdmiCec parse configuration
*/

#ifndef __HDMICEC_PARSE_CONFIG_H__
#define __HDMICEC_PARSE_CONFIG_H__

#include <string>
#include <vector>

#include <ut.h>
#include <ut_log.h>
#include <ut_kvp_profile.h>
#include <ut_control_plane.h>

using namespace std;

typedef enum cecType1
{
    CEC_TYPE_BROADCAST,
    CEC_TYPE_DIRECT
} cecType_t;

typedef struct deviceInfo
{
    string name;
    uint8_t cec_version;
    uint8_t logical_address;
    uint8_t physical_address[2];
    uint32_t vendor_id;
    uint8_t device_type;
    uint8_t power_status;
    uint8_t fault;
    vector<uint8_t> language;
} deviceInfo_t;

typedef struct cecOpcode
{
    uint8_t opcode;        //CEC opcode
    cecType_t type;        //Type of the opcode Broadcast/ Direct
    vector<string> payload;  //CEC Payload
} cecOpcode_t;

typedef struct cecResponse
{
    cecOpcode_t command;
    cecOpcode_t response;
} cecResponse_t;

typedef struct cecConfiguration
{
    vector<cecResponse_t> cec_opcodes;
    uint8_t cec_version;
    vector<deviceInfo_t> devices_connected;
    vector<uint8_t> par_fault_devices;
    vector<uint8_t> fault_devices;
    vector<uint8_t> working_devices;
    vector<uint8_t> connected_devices;
} cecConfiguration_t;

extern cecConfiguration_t gCecConfiguration;

#define HDCM_DEVICE       "hdmicec.device"
#define HDCM_CEC_MESSAGES "hdmicec.cec_messages"

void vcomponent_HdmiCec_prase_config(char *);
void vcomponent_HdmiCec_get_cec_command(uint8_t opcode, cecResponse_t &cmd);
bool vcomponent_HdmiCec_get_cec_response(uint8_t src, uint8_t dst,
                                         cecOpcode_t &response, std::vector<uint8_t>& message);

#endif //__HDMICEC_PARSE_CONFIG_H__