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

#include "ut_kvp_profile.h"
#include "vts_hdmicec_parse_hfp.h"

using namespace std;

hdmicec_device_info gHdmiCecDeviceInfo = {0};

UT_status_t vts_hdmicec_parse_hfp()
{
    gHdmiCecDeviceInfo.hal_cec_version = static_cast<int8_t>(UT_KVP_PROFILE_GET_UINT8("IHdmiCec.HAL_CEC_VERSION"));
    return UT_STATUS_OK;
}

/** @} */ // End of HDMI_CEC_HALTEST_PARSE_HFP
/** @} */ // End of HDMI_CEC_HALTEST
/** @} */ // End of HDMI_CEC
/** @} */ // End of HPK