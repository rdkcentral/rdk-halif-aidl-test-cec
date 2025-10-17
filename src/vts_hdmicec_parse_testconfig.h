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
 * @addtogroup HDMI_CEC Hdmi Cec Module
 * @{
 */

/**
 * @addtogroup HDMI_CEC_HALTEST Hdmi Cec HAL Tests
 * @{
 */

/**
 * @defgroup HDMI_CEC_HALTEST_PARSE_TESTCONFIG_HEADER Hdmi Cec HAL Tests parse test configuration file
 * @{
 * @parblock
 *
 * ### Parse test configuration functions for Hdmi Cec :
 *
 * Parse configuration functions required for the module across all vendors.
 *
 * **Pre-Conditions:**  None @n
 * **Dependencies:** None @n
 *
 * @endparblock
 *
 */

/**
 * @file vts_hdmicec_parse_testconfig.h
 *
 */

#ifndef __VTS_HDMICEC_PARSE_TESTCONFIG_H__
#define __VTS_HDMICEC_PARSE_TESTCONFIG_H__

#include <ut.h>
#include <ut_log.h>
#include "ut_kvp_profile.h"

using namespace std;

typedef struct vts_hdmicec_metrics
{
    int32_t directedMessagesSent;
    int32_t directedMessagesSentAndAcked;
    int32_t broadcastMessagesSent;
    int32_t broadcastMessagesSentAndAcked;
    int32_t arbitrationFailures;
}vts_hdmicec_metrics_t;

typedef struct hdmicecSendMsgConfiguration
{
    std::vector<uint8_t> message;
    std::vector<uint8_t> responseMessage;
    vts_hdmicec_metrics_t metrics;
} hdmicecSendMsgConfiguration_t;

typedef struct hdmicecReceiveMsgConfiguration
{
    vector<int32_t> dutLogicalAddress;
    std::vector<uint8_t> message;
    uint32_t waitResponseTimeoutMsec;
} hdmicecReceiveMsgConfiguration_t;

#define VTS_HDMICEC_TEST_CONFIG_PATH "./test_configuration/vts_l3_HdmiCec.yaml"

UT_status_t vts_hdmicec_parse_sendMessage_config(char *cofigurationFile, const char *module, const char *testGroup,
                                                 const char *testName, std::vector<hdmicecSendMsgConfiguration_t>& testConfigurationList,
                                                 std::vector<int32_t>& sourceLogicalAddresses, int32_t& delayBetweenMsgsMsec);

UT_status_t vts_hdmicec_parse_receiveMessage_config(char *cofigurationFile, const char *module, const char *testGroup,
                                                    const char *testName, vector<hdmicecReceiveMsgConfiguration_t> &testConfigurationList);

#endif //__VTS_HDMICEC_PARSE_TESTCONFIG_H__

/** @} */ // End of HDMI_CEC_HALTEST_PARSE_TESTCONFIG_HEADER
/** @} */ // End of HDMI_CEC_HALTEST
/** @} */ // End of HDMI_CEC
/** @} */ // End of HPK