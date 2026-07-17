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

static UT_status_t vts_hdmicec_parse_testconfig_instance(void *instance, const std::string &cecMsgKey,
                                                         hdmicecSendMsgConfiguration_t &testConfiguration)
{
    int32_t messageSize = 0;
    ut_kvp_instance_t* kvpTestInstance = (ut_kvp_instance_t*)instance;

    // Parse Test Parameters
    const std::string parametersKey = cecMsgKey + ".Parameters";

    messageSize = ut_kvp_getListCount(kvpTestInstance, (parametersKey + ".message").c_str());
    testConfiguration.repeat = ut_kvp_getUInt32Field(kvpTestInstance, (parametersKey + ".repeat").c_str());
    if(messageSize > 0)
    {
        for (int32_t index = 0; index < messageSize; index++)
        {
            testConfiguration.message.push_back(ut_kvp_getUInt8Field(kvpTestInstance, (parametersKey + ".message." + std::to_string(index)).c_str()));
        }
    }
    else
    {
        UT_LOG_ERROR("Failed to parse message from test configuration");
        return UT_STATUS_FAILURE;
    }

    //Parse expected results
    const std::string expectedKey = cecMsgKey + ".Expected_Results";

    //Parse expected metrics
    const std::string metricsKey = expectedKey + ".metrics";
    testConfiguration.metrics.directedMessagesSent = ut_kvp_getUInt32Field(kvpTestInstance, (metricsKey + ".directedMessagesSent").c_str());
    testConfiguration.metrics.directedMessagesSentAndAcked = ut_kvp_getUInt32Field(kvpTestInstance, (metricsKey + ".directedMessagesSentAndAcked").c_str());
    testConfiguration.metrics.broadcastMessagesSent = ut_kvp_getUInt32Field(kvpTestInstance, (metricsKey + ".broadcastMessagesSent").c_str());
    testConfiguration.metrics.broadcastMessagesSentAndAcked = ut_kvp_getUInt32Field(kvpTestInstance, (metricsKey + ".broadcastMessagesSentAndAcked").c_str());
    testConfiguration.metrics.arbitrationFailures = ut_kvp_getUInt32Field(kvpTestInstance, (metricsKey + ".arbitrationFailures").c_str());

    return UT_STATUS_OK;
}

UT_status_t vts_hdmicec_parse_sendMessage_config(char *configurationFile, const char *module, const char *testGroup,
                                                 const char *testName, vector<hdmicecSendMsgConfiguration_t>& testConfigurationList,
                                                 std::vector<int32_t>& sourceLogicalAddresses, int32_t& delayBetweenMsgsMsec)
{
    char resultKVP[UT_KVP_MAX_ELEMENT_SIZE] = {0};
    hdmicecSendMsgConfiguration_t testConfiguration;

    void *testconfig = vcomponentkvp_create_instance(configurationFile);
    ut_kvp_instance_t* kvpTestInstance = (ut_kvp_instance_t*)testconfig;

    const std::string testList = string(module) + "." + string(testGroup) + "." + string(testName);
    const std::string cecMsgKey = testList + ".cec_messages";

    uint32_t logicalAddressLen = ut_kvp_getListCount(kvpTestInstance, (testList + ".source_logical_address").c_str());
    if (logicalAddressLen > 0)
    {
        for (uint32_t index = 0; index < logicalAddressLen; index++)
        {
            sourceLogicalAddresses.push_back(ut_kvp_getUInt32Field(kvpTestInstance, (testList + ".source_logical_address." + std::to_string(index)).c_str()));
        }
    }
    else
    {
        UT_LOG_ERROR("Failed to parse initiator from test configuration");
        return UT_STATUS_FAILURE;
    }

    delayBetweenMsgsMsec = ut_kvp_getUInt32Field(kvpTestInstance, (testList + ".delay_between_messages").c_str());

    int32_t messageCount = ut_kvp_getListCount(kvpTestInstance, cecMsgKey.c_str());

    for(int32_t index = 0; index < messageCount; index++)
    {
        std::string cecIndex = cecMsgKey + "." + std::to_string(index);
        testConfiguration = hdmicecSendMsgConfiguration_t{};
        UT_status_t status = vts_hdmicec_parse_testconfig_instance(kvpTestInstance, cecIndex, testConfiguration);
        if(status != UT_STATUS_OK)
        {
            UT_LOG_ERROR("Failed to parse test configuration for %s", cecIndex.c_str());
            vcomponentkvp_destroy_instance(kvpTestInstance);
            return status;
        }
        testConfigurationList.push_back(testConfiguration);
    }
    vcomponentkvp_destroy_instance(kvpTestInstance);
    return UT_STATUS_OK;
}

UT_status_t vts_hdmicec_parse_receiveMessage_config(char *configurationFile, const char *module, const char *testGroup,
                                                    const char *testName, vector<hdmicecReceiveMsgConfiguration_t> &testConfigurationList,
                                                    std::vector<int32_t>& sourceLogicalAddresses)
{
    void *testconfig = vcomponentkvp_create_instance(configurationFile);
    ut_kvp_instance_t* kvpTestInstance = (ut_kvp_instance_t*)testconfig;
    hdmicecReceiveMsgConfiguration_t testConfiguration;

    const std::string testList = string(module) + "." + string(testGroup) + "." + string(testName);

    std::string logicalAddressKey = testList + ".source_logical_address";
    int32_t logicalAddressLen = ut_kvp_getListCount(kvpTestInstance, logicalAddressKey.c_str());
    for (int32_t index = 0; index < logicalAddressLen; index++)
    {
        sourceLogicalAddresses.push_back(ut_kvp_getUInt32Field(kvpTestInstance, (logicalAddressKey + "." + std::to_string(index)).c_str()));
    }

    const std::string cecMessage = testList + ".cec_messages";
    int32_t responseCount = ut_kvp_getListCount(kvpTestInstance, cecMessage.c_str());

    for(int32_t index = 0; index < responseCount; index++)
    {
        testConfiguration = hdmicecReceiveMsgConfiguration_t{};

        std::string waitKey = cecMessage + "." + std::to_string(index) + ".Parameters.wait_for_response_timeout";
        std::string messageKey = cecMessage + "." + std::to_string(index) + ".Parameters.message";
        std::string repeatKey = cecMessage + "." + std::to_string(index) + ".Parameters.repeat";

        testConfiguration.waitResponseTimeoutMsec = ut_kvp_getUInt32Field(kvpTestInstance, waitKey.c_str());

        int32_t messageLen = ut_kvp_getListCount(kvpTestInstance, messageKey.c_str());
        for (int32_t index = 0; index < messageLen; index++)
        {
            testConfiguration.message.push_back(ut_kvp_getUInt8Field(kvpTestInstance, (messageKey + "." + std::to_string(index)).c_str()));
        }

        testConfiguration.repeat = ut_kvp_getUInt32Field(kvpTestInstance, repeatKey.c_str());

        testConfigurationList.push_back(testConfiguration);
    }

    vcomponentkvp_destroy_instance(kvpTestInstance);
    return UT_STATUS_OK;
}

/** @} */ // End of HDMI_CEC_HALTEST_PARSE_TESTCONFIG
/** @} */ // End of HDMI_CEC_HALTEST
/** @} */ // End of HDMI_CEC
/** @} */ // End of HPK