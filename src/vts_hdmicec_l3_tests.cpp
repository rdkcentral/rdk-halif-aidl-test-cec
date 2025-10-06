/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2016 RDK Management
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

/**
 * @addtogroup HPK Hardware Porting Kit
 * @{
 *
 */

/**
 * @addtogroup HDMI_CEC HDMI CEC Module
 * @{
 *
 */

/**
 * @defgroup HDMI_CEC_HALTEST HDMI CEC HAL Tests
 * @{
 *
 */

/**
 * @defgroup HDMI_CEC_HALTEST_L3 HDMI CEC HAL Tests L3 File
 * @{
 * @parblock
 *
 * ### L3 Test Cases for HDMI CEC HAL :
 *
 * This module includes Level 3 functional tests (success and failure scenarios).
 * This is to ensure that the API meets the operational requirements of the module across all vendors.
 *
 * **Pre-Conditions:** @n
 * **Dependencies:** None@n
 *
 * @endparblock
 *
 */

/**
 * @file vts_hdmicec_l3_tests.c
 *
 */
#include "vts_hdmicec_parse_hfp.h"
#include "vts_hdmicec.h"
#include "vts_hdmicec_parse_testconfig.h"

extern sp<IHdmiCec> gHDMICECHandle;
static const char *gModuleName = "hdmicec";
static const char *gTestGroup = "L3";

/* Test class registration with UT_Core */
UT_ADD_TEST_TO_GROUP(VTS_L3_HDMICEC, UT_TESTS_L3)

class VTS_L3_HDMICEC : public UTCore
{
public:
    VTS_L3_HDMICEC() : UTCore() {}

    ~VTS_L3_HDMICEC() override = default;

    void SetUp() override
    {
        android::ProcessState::self()->startThreadPool();
    }

    bool validateMetrics()
    {
        std::optional<PropertyValue> directMsgSent;
        std::optional<PropertyValue> directMsgSentAck;
        std::optional<PropertyValue> broadcastMsgSent;
        std::optional<PropertyValue> broadcastMsgSentAck;
        std::optional<PropertyValue> arbitrationFailures;

        HDMICEC_GET_PROPERTY_SUCCESS(gHDMICECHandle, hdmicec::Property::METRIC_DIRECTED_MESSAGES_SENT, directMsgSent);
        HDMICEC_GET_PROPERTY_SUCCESS(gHDMICECHandle, hdmicec::Property::METRIC_DIRECTED_MESSAGES_SENT_AND_ACKED, directMsgSentAck);
        HDMICEC_GET_PROPERTY_SUCCESS(gHDMICECHandle, hdmicec::Property::METRIC_BROADCAST_MESSAGES_SENT, broadcastMsgSent);
        HDMICEC_GET_PROPERTY_SUCCESS(gHDMICECHandle, hdmicec::Property::METRIC_BROADCAST_MESSAGE_SENT_AND_ACKED, broadcastMsgSentAck);
        HDMICEC_GET_PROPERTY_SUCCESS(gHDMICECHandle, hdmicec::Property::METRIC_ARBITRATION_FAILURES, arbitrationFailures);

        //TODO: Add validation logic
        return true;
    }
    void TearDown() override
    {
    }
};

/**
 * @brief
 *
 *
 * **Test Group ID:** Basic: 03@n
 * **Test Case ID:** 001@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:**
 *
 * **Dependencies:**
 * - A valid gHDMICECHandle must be available
 *
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * Refer to UT specification documentation [hdmicec_High-Level_TestSpecification.md](../docs/pages/hdmicec_High-Level_TestSpecification.md)
 *
 */

void readAndDiscardRestOfLine(FILE *in)
{
    int c;
    while ((c = fgetc(in)) != EOF && c != '\n');
}

void readString(char *choice)
{
    scanf("%s", choice);
    readAndDiscardRestOfLine(stdin);
}

UT_ADD_TEST(VTS_L3_HDMICEC, SendMessage)
{
    const char *testName = "SendMessage";
    hdmicecTestConfiguration_t testConfiguration;
    std::vector<uint8_t> message;
    uint8_t src;
    uint8_t dst;

    sp<HdmiCecControllerListener> controllerListener = new HdmiCecControllerListener();
    sp<HdmiCecEventListener> eventListener = new HdmiCecEventListener();
    sp<IHdmiCecController> hdmiCecController = nullptr;
    hdmicec::SendMessageStatus msgStatus;
    UT_status_t status = UT_STATUS_OK;
    vector<uint8_t> getMessage;
    hdmicec::SendMessageStatus getStatus;

    status = vts_hdmicec_parse_testconfig(VTS_HDMICEC_TEST_CONFIG_PATH, gModuleName, gTestGroup, testName, testConfiguration);

    UT_ASSERT_EQUAL_FATAL(status, UT_STATUS_OK);

    src = testConfiguration.initiator[0];
    dst = testConfiguration.destination;
    message.push_back(dst | (src << 4));
    message.insert(message.end(), testConfiguration.message.begin(), testConfiguration.message.end());

    // Register Event Listener
    HDMICEC_REGISTER_EVENT_SUCCESS(gHDMICECHandle, eventListener);

    // Open HDMI CEC interface
    HDMICEC_OPEN_SUCCESS(gHDMICECHandle, controllerListener, hdmiCecController, true);

    // Add logical address
    HDMICEC_ADD_LOGICAL_ADDRESS_SUCCESS(hdmiCecController, testConfiguration.initiator);

    // Send message
    HDMICEC_SEND_MESSAGE_SUCCESS(hdmiCecController, message, msgStatus);

    // Wait for message to be processed and callback to be invoked
    this_thread::sleep_for(chrono::milliseconds(100));

    // Verify that the onMessageSent received to controllerListener
    controllerListener->getNextMessage(getMessage, getStatus);

    UT_ASSERT_EQUAL(getMessage.size(), message.size());

    if(getMessage.size() == message.size())
    {
        for (size_t i = 0; i < message.size(); i++)
        {
            UT_ASSERT_EQUAL(getMessage[i], message[i]);
        }
    }
    // Cast getStatus to int32_t to match with onMessageSentStatus for comparison
    UT_ASSERT_EQUAL(static_cast<int32_t>(getStatus), testConfiguration.onMessageSentStatus);

    // Wait for message to be processed and callback to be invoked
    this_thread::sleep_for(chrono::milliseconds(testConfiguration.waitResponseTimeoutMsec));

    getMessage.clear();

    // Verify that the onMessageReceived received to controllerListener
    controllerListener->getNextReceiveMessage(getMessage);
    UT_ASSERT_EQUAL(getMessage.size(), testConfiguration.responseMessage.size());

    if(getMessage.size() == testConfiguration.responseMessage.size())
    {
        for (size_t i = 0; i < testConfiguration.responseMessage.size(); i++)
        {
            UT_ASSERT_EQUAL(getMessage[i], testConfiguration.responseMessage[i]);
        }
    }

    getMessage.clear();

    // Verify that the onMessageReceived received to eventListener
    eventListener->getNextReceiveMessage(getMessage);
    UT_ASSERT_EQUAL(getMessage.size(), testConfiguration.responseMessage.size());

    if(getMessage.size() == testConfiguration.responseMessage.size())
    {
        for (size_t i = 0; i < testConfiguration.responseMessage.size(); i++)
        {
            UT_ASSERT_EQUAL(getMessage[i], testConfiguration.responseMessage[i]);
        }
    }

    // Remove logical address
    HDMICEC_REMOVE_LOGICAL_ADDRESS_SUCCESS(hdmiCecController, testConfiguration.initiator);

    // Close HDMI CEC interface
    HDMICEC_CLOSE_SUCCESS(gHDMICECHandle, hdmiCecController, true);

    // Unregister Event Listener
    HDMICEC_UNREGISTER_EVENT_SUCCESS(gHDMICECHandle, eventListener);

}

/** @} */ // End of HDMI_CEC_HALTEST L3 File
/** @} */ // End of HDMI_CEC_HALTEST
/** @} */ // End of HDMI_CEC Module
/** @} */ // End of HPK
