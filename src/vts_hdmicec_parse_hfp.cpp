/**
 *  If not stated otherwise in this file or this component's LICENSE
 *  file the following copyright and licenses apply:
 *
 *  Copyright 2024 RDK Management
 *
 *  Licensed under the Apache License, Version 2.0 (the License);
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an AS IS BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

/**
 * @addtogroup HPK Hardware Porting Kit
 * @{
 * @par The Hardware Porting Kit
 * HPK is the next evolution of the well-defined Hardware Abstraction Layer
 * (HAL), but augmented with more comprehensive documentation and test suites
 * that OEM or SOC vendors can use to self-certify their ports before taking
 * them to RDKM for validation or to an operator for final integration and
 * deployment. The Hardware Porting Kit effectively enables an OEM and/or SOC
 * vendor to self-certify their own Video Accelerator devices, with minimal RDKM
 * assistance.
 *
 */

/**
 * @addtogroup HDMI_CEC HDMI CEC Module
 * @{
 */

/**
 * @addtogroup HDMI_CEC_HALTEST HDMI CEC HAL Tests
 * @{
 */

/**
 * @defgroup HDMI_CEC_HALTEST_PARSE_HFP HDMI CEC HAL Tests parse Hardware feature profile
 * @{
 * @parblock
 *
 * ### Parse HFP functions for HDMI CEC :
 *
 * Parse HFP functions required for the module across all vendors.
 *
 * **Pre-Conditions:**  None @n
 * **Dependencies:** None @n
 *
 * @endparblock
 *
 */

/**
 * @file vts_hdmicec_parse_hfp.cpp
 *
 */

#include "ut_control_plane.h"
#include "ut_kvp_profile.h"
#include "vts_hdmicec_parse_hfp.h"

using namespace std;

namespace
{
using com::rdk::hal::hdmicec::DeviceType;

const ut_control_keyStringMapping_t deviceTypeMap[] =
{
    {"NONE", static_cast<int32_t>(DeviceType::NONE)},
    {"TV", static_cast<int32_t>(DeviceType::TV)},
    {"RECORDING_DEVICE", static_cast<int32_t>(DeviceType::RECORDING_DEVICE)},
    {"TUNER", static_cast<int32_t>(DeviceType::TUNER)},
    {"PLAYBACK_DEVICE", static_cast<int32_t>(DeviceType::PLAYBACK_DEVICE)},
    {"AUDIO_SYSTEM", static_cast<int32_t>(DeviceType::AUDIO_SYSTEM)},
    {"CEC_SWITCH", static_cast<int32_t>(DeviceType::CEC_SWITCH)},
    {"PROCESSOR", static_cast<int32_t>(DeviceType::PROCESSOR)},
    {nullptr, 0}
};

bool parseDeviceType(const char* value, DeviceType& deviceType)
{
    constexpr int32_t invalidDeviceType = -2;
    const uint32_t mappedDeviceType = UT_Control_GetMapValue(
        deviceTypeMap, const_cast<char*>(value), invalidDeviceType);
    if (mappedDeviceType == static_cast<uint32_t>(invalidDeviceType))
    {
        return false;
    }

    deviceType = static_cast<DeviceType>(static_cast<int32_t>(mappedDeviceType));
    return true;
}
}

hdmicec_device_info gHdmiCecDeviceInfo = {0, DeviceType::NONE, DeviceType::NONE};

UT_status_t vts_hdmicec_parse_hfp()
{
    char primaryDeviceType[UT_KVP_MAX_ELEMENT_SIZE] = {0};
    char secondaryDeviceType[UT_KVP_MAX_ELEMENT_SIZE] = {0};

    gHdmiCecDeviceInfo.hal_cec_version = static_cast<int8_t>(UT_KVP_PROFILE_GET_UINT8("hdmicec.HAL_CEC_VERSION"));

    UT_KVP_PROFILE_GET_STRING("hdmicec.PRIMARY_DEVICE_TYPE", primaryDeviceType);
    UT_KVP_PROFILE_GET_STRING("hdmicec.SECONDARY_DEVICE_TYPE", secondaryDeviceType);

    if (!parseDeviceType(primaryDeviceType, gHdmiCecDeviceInfo.primary_device_type) ||
        !parseDeviceType(secondaryDeviceType, gHdmiCecDeviceInfo.secondary_device_type))
    {
        UT_LOG_ERROR("Invalid HDMI CEC device type in HFP");
        return UT_STATUS_FAILURE;
    }

    return UT_STATUS_OK;
}

/** @} */ // End of HDMI_CEC_HALTEST_PARSE_HFP
/** @} */ // End of HDMI_CEC_HALTEST
/** @} */ // End of HDMI_CEC
/** @} */ // End of HPK
