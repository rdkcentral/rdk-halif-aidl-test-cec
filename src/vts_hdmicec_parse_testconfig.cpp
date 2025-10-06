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
 * @defgroup HDMI_CEC_HALTEST_PARSE_TESTCONFIG Hdmi Cec HAL Tests parse test configuration file
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
 * @file vts_hdmicec_parse_testconfig.cpp
 *
 */

#include "vts_hdmicec_parse_testconfig.h"

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

UT_status_t vts_hdmicec_parse_testconfig(char *cofigurationFile, const char *module, const char *testGroup,
                                         const char *testName, hdmicecTestConfiguration_t& testConfiguration)
{
    char resultKVP[UT_KVP_MAX_ELEMENT_SIZE] = {0};
    int32_t messageSize = 0;

    void *testconfig = vcomponentkvp_create_instance(cofigurationFile);
    ut_kvp_instance_t* kvpTestInstance = (ut_kvp_instance_t*)testconfig;

    const std::string testList = string(module) + "." + string(testGroup) + "." + string(testName);

    // Parse Test Parameters
    const std::string parmetersKey = testList + ".Parameters";

    uint32_t logicalAddressLen = ut_kvp_getListCount(kvpTestInstance, (parmetersKey + ".initiator").c_str());
    if (logicalAddressLen > 0)
    {
        for (uint32_t index = 0; index < logicalAddressLen; index++)
        {
            testConfiguration.initiator.push_back(ut_kvp_getUInt32Field(kvpTestInstance, (parmetersKey + ".initiator." + std::to_string(index)).c_str()));
        }
    }
    else
    {
        UT_LOG_ERROR("Failed to parse initiator from test configuration");
        vcomponentkvp_destroy_instance(kvpTestInstance);
        return UT_STATUS_FAILURE;
    }

    testConfiguration.destination = ut_kvp_getUInt32Field(kvpTestInstance, (parmetersKey + ".destination").c_str());

    messageSize = ut_kvp_getListCount(kvpTestInstance, (parmetersKey + ".message").c_str());
    if(messageSize > 0)
    {
        for (uint32_t index = 0; index < messageSize; index++)
        {
            testConfiguration.message.push_back(ut_kvp_getUInt8Field(kvpTestInstance, (parmetersKey + ".message." + std::to_string(index)).c_str()));
        }
    }
    else
    {
        UT_LOG_ERROR("Failed to parse message from test configuration");
        vcomponentkvp_destroy_instance(kvpTestInstance);
        return UT_STATUS_FAILURE;
    }

    //Parse control parameters
    const std::string controlKey = testList + ".Controls";
    testConfiguration.waitResponseTimeoutMsec = ut_kvp_getUInt32Field(kvpTestInstance, (controlKey + ".wait_for_response_timeout").c_str());

    //Parse expected results
    const std::string expectedKey = testList + ".Expected_Results";
    testConfiguration.onMessageSentStatus = ut_kvp_getUInt32Field(kvpTestInstance, (expectedKey + ".onMessageSent").c_str());

    messageSize = ut_kvp_getListCount(kvpTestInstance, (expectedKey + ".onMessageReceived").c_str());

    if(messageSize > 0)
    {
        for (uint32_t index = 0; index < messageSize; index++)
        {
            testConfiguration.responseMessage.push_back(ut_kvp_getUInt8Field(kvpTestInstance, (expectedKey + ".onMessageReceived." + std::to_string(index)).c_str()));
        }
    }

    vcomponentkvp_destroy_instance(kvpTestInstance);

    return UT_STATUS_OK;
}

/** @} */ // End of HDMI_CEC_HALTEST_PARSE_TESTCONFIG
/** @} */ // End of HDMI_CEC_HALTEST
/** @} */ // End of HDMI_CEC
/** @} */ // End of HPK