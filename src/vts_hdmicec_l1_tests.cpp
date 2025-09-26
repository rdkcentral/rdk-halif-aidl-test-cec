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
 * @defgroup HDMI_CEC_HALTEST_L1 HDMI CEC HAL Tests L1 File
 * @{
 * @parblock
 *
 * ### L1 Test Cases for HDMI CEC HAL :
 *
 * This module includes Level 1 functional tests (success and failure scenarios).
 * This is to ensure that the API meets the operational requirements of the module across all vendors.
 *
 * **Pre-Conditions:** @n
 * **Dependencies:** None@n
 *
 * @endparblock
 *
 */

/**
 * @file vts_hdmicec_l1_tests.c
 *
 */
#include "vts_hdmicec_parse_hfp.h"
#include "vts_hdmicec.h"

extern sp<IHdmiCec> gHDMICECHandle;

/* Test class registration with UT_Core */
UT_ADD_TEST_TO_GROUP(VTS_L1_HDMICEC, UT_TESTS_L1)

class VTS_L1_HDMICEC : public UTCore
{
public:
    VTS_L1_HDMICEC() : UTCore() {}

    ~VTS_L1_HDMICEC() override = default;

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
* @brief Tests the getState API
*
* This test verifies that the getState API returns the expected state
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 001@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* |Variation / Step|Description|Test Data|Expected Result|Notes|
* |----------------|-----------|---------|---------------|-----|
* |01|Call getState()|None|State should be CLOSED|should pass|

*/
UT_ADD_TEST(VTS_L1_HDMICEC, getState_pos)
{
    com::rdk::hal::State state;

    HDMICEC_GET_STATE(gHDMICECHandle, state);

    UT_ASSERT_EQUAL(state, State::CLOSED);
}

/**
* @brief Tests the getProperty API
*
* This test verifies that the getProperty API returns expected property value
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 002@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* |Variation / Step|Description|Test Data|Expected Result|Notes|
* |----------------|-----------|---------|---------------|-----|
* |01|Call getProperty()|cec version from hfp|cec version should match with hfp and metrics should be 0 if implemented|should pass|

*/
UT_ADD_TEST(VTS_L1_HDMICEC, getProperty_pos)
{
    std::optional<PropertyValue> cecVersion;

    std::vector<Property> testProperties = {hdmicec::Property::METRIC_DIRECTED_MESSAGES_SENT,
                                            hdmicec::Property::METRIC_BROADCAST_MESSAGES_SENT,
                                            hdmicec::Property::METRIC_DIRECTED_MESSAGES_SENT_AND_ACKED,
                                            hdmicec::Property::METRIC_BROADCAST_MESSAGE_SENT_AND_ACKED,
                                            hdmicec::Property::METRIC_ARBITRATION_FAILURES};

    HDMICEC_GET_PROPERTY_SUCCESS(gHDMICECHandle, hdmicec::Property::HAL_CEC_VERSION, cecVersion);

    UT_ASSERT_TRUE(cecVersion.has_value());
    UT_ASSERT_EQUAL(cecVersion->value->get<PropertyValue::Value::Tag::intValue>(), gHdmiCecDeviceInfo.hal_cec_version);

    for (auto property : testProperties)
    {
        std::optional<PropertyValue> propertyValue;
        int32_t value = 0;
        HDMICEC_GET_PROPERTY_SUCCESS(gHDMICECHandle, property, propertyValue);
        UT_ASSERT_TRUE(propertyValue.has_value());
        value = propertyValue->value->get<PropertyValue::Value::Tag::intValue>();
        if(value != -1)
        {
            UT_ASSERT_EQUAL(value, 0);
        }
    }
}

/**
* @brief Tests the getProperty API for negative scenarios
*
* This test verifies that the getProperty API returns expected property value for invalid properties
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 003@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* |Variation / Step|Description|Test Data|Expected Result|Notes|
* |----------------|-----------|---------|---------------|-----|
* |01|Call getProperty() with invalid propery||should pass|

*/
UT_ADD_TEST(VTS_L1_HDMICEC, getProperty_neg)
{
    std::optional<PropertyValue> propertyValue;

    int32_t firstValue = static_cast<int32_t>(Property::HAL_CEC_VERSION);
    Property Invalidproperty = static_cast<Property>(firstValue - 1);

    HDMICEC_GET_PROPERTY_SUCCESS(gHDMICECHandle, Invalidproperty, propertyValue);

    UT_ASSERT_FALSE(propertyValue.has_value());
}

/**
* @brief Tests the getLogicalAddresses API
*
* This test verifies that the getLogicalAddresses API returns expected property value
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 004@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* |Variation / Step|Description|Test Data|Expected Result|Notes|
* |----------------|-----------|---------|---------------|-----|
* |01|Call getLogicalAddresses()||logical address vector should be empty|should pass|

*/
UT_ADD_TEST(VTS_L1_HDMICEC, getLogicalAddresses_pos)
{
    vector<int32_t> getlogicalAddresses;

    HDMICEC_GET_LOGICAL_ADDRESS_SUCCESS(gHDMICECHandle, getlogicalAddresses);

    UT_ASSERT_TRUE(getlogicalAddresses.empty());
}

/**
* @brief Tests the open API
*
* This test verifies that the open API returns expected status value and checks the state transitions
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 005@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* |Variation / Step|Description|Test Data|Expected Result|Notes|
* |----------------|-----------|---------|---------------|-----|
* |01|Call open()||should return proper handle for controller|should pass|
* |02|Call close()||return status should be ok|should pass|
*/
UT_ADD_TEST(VTS_L1_HDMICEC, open_pos)
{
    sp<IHdmiCecControllerListener> controllerListener = new HdmiCecControllerListener();
    sp<IHdmiCecController> hdmiCecController = nullptr;

    HDMICEC_OPEN_SUCCESS(gHDMICECHandle, controllerListener, hdmiCecController, true);

    HDMICEC_CLOSE_SUCCESS(gHDMICECHandle, hdmiCecController, true);
}

/**
* @brief Tests the open API for netagive scenarios
*
* This test verifies that the open API handles the negative scenarios
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 006@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* |Variation / Step|Description|Test Data|Expected Result|Notes|
* |----------------|-----------|---------|---------------|-----|
* |01|Call open() with null listener||controller should be null and return status should be EX_ILLEGAL_ARGUMENT|should pass|
* |02|Call open() with valid listener||should return proper handle for controller|should pass|
* |03|Call open() with valid listener again||should return null controller handle and status should be EX_ILLEGAL_STATE|should pass|
* |04|Call close()||return status should be ok|should pass|
*/
UT_ADD_TEST(VTS_L1_HDMICEC, open_neg)
{
    sp<IHdmiCecControllerListener> invalidControllerListener = nullptr;
    sp<IHdmiCecControllerListener> controllerListener = new HdmiCecControllerListener();
    sp<IHdmiCecController> hdmiCecController_1 = nullptr;
    sp<IHdmiCecController> hdmiCecController_2 = nullptr;

    // Expect open to fail with null listener
    HDMICEC_OPEN_FAILURE(gHDMICECHandle, invalidControllerListener, hdmiCecController_1, State::CLOSED, Status::EX_ILLEGAL_ARGUMENT);

    HDMICEC_OPEN_SUCCESS(gHDMICECHandle, controllerListener, hdmiCecController_1, true);
    HDMICEC_OPEN_FAILURE(gHDMICECHandle, controllerListener, hdmiCecController_2, State::STARTED, Status::EX_ILLEGAL_STATE);

    HDMICEC_CLOSE_SUCCESS(gHDMICECHandle, hdmiCecController_1, true);
}

/**
* @brief Tests the close API
*
* This test verifies that the close API returns expected status value and checks the state transitions
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 006@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* |Variation / Step|Description|Test Data|Expected Result|Notes|
* |----------------|-----------|---------|---------------|-----|
* |01|Call open()||should return proper handle for controller|should pass|
* |02|Call close()||return status should be ok|should pass|
*/
UT_ADD_TEST(VTS_L1_HDMICEC, close_pos)
{
    sp<IHdmiCecControllerListener> controllerListener = new HdmiCecControllerListener();
    sp<IHdmiCecController> hdmiCecController = nullptr;

    HDMICEC_OPEN_SUCCESS(gHDMICECHandle, controllerListener, hdmiCecController, true);

    HDMICEC_CLOSE_SUCCESS(gHDMICECHandle, hdmiCecController, true);
}

/**
* @brief Tests the close API for netagive scenarios
*
* This test verifies that the close API handles the negative scenarios
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 008@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* |Variation / Step|Description|Test Data|Expected Result|Notes|
* |----------------|-----------|---------|---------------|-----|
* |01|Call open() with null listener||controller should be null and return status should be EX_ILLEGAL_ARGUMENT|should pass|
* |02|Call open() with valid listener||should return proper handle for controller|should pass|
* |03|Call open() with valid listener again||should return null controller handle and status should be EX_ILLEGAL_STATE|should pass|
* |04|Call close()||return status should be ok|should pass|
*/
UT_ADD_TEST(VTS_L1_HDMICEC, close_neg)
{
    sp<IHdmiCecControllerListener> invalidControllerListener = nullptr;
    sp<IHdmiCecControllerListener> controllerListener = new HdmiCecControllerListener();
    sp<IHdmiCecController> hdmiCecController = nullptr;
    sp<IHdmiCecController> InvalidHdmiCecController = nullptr;

    HDMICEC_OPEN_SUCCESS(gHDMICECHandle, controllerListener, hdmiCecController, true);

    HDMICEC_CLOSE_FAILURE(gHDMICECHandle, InvalidHdmiCecController, State::STARTED, Status::EX_NONE);
    HDMICEC_CLOSE_SUCCESS(gHDMICECHandle, hdmiCecController, true);
    HDMICEC_CLOSE_FAILURE(gHDMICECHandle, hdmiCecController, State::CLOSED, Status::EX_NONE);
}

/**
* @brief Tests the registerEventListener API
*
* This test verifies that the registerEventListener API returns expected status value
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 009@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* |Variation / Step|Description|Test Data|Expected Result|Notes|
* |----------------|-----------|---------|---------------|-----|
* |01|Call registerEventListener()||should register event listener|should pass|
* |02|Call unregisterEventListener()||should un-register event listener|should pass|
*/
UT_ADD_TEST(VTS_L1_HDMICEC, registerEventListener_pos)
{
    sp<IHdmiCecEventListener> eventListener = new HdmiCecEventListener();

    HDMICEC_REGISTER_EVENT_SUCCESS(gHDMICECHandle, eventListener);

    HDMICEC_UNREGISTER_EVENT_SUCCESS(gHDMICECHandle, eventListener);
}

/**
* @brief Tests the registerEventListener API for netagive scenarios
*
* This test verifies that the registerEventListener API handles the negative scenarios
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 010@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* |Variation / Step|Description|Test Data|Expected Result|Notes|
* |----------------|-----------|---------|---------------|-----|
* |01|Call registerEventListener() with invalid listener||should return false|should pass|
* |02|Call registerEventListener() with valid listener||should return true|should pass|
* |03|Call registerEventListener() with valid listener again||should return false|should pass|
* |04|Call unregisterEventListener()||should un-register event listener|should pass|
*/
UT_ADD_TEST(VTS_L1_HDMICEC, registerEventListener_neg)
{
    sp<IHdmiCecEventListener> invalidEventListener = nullptr;
    sp<IHdmiCecEventListener> eventListener_1 = new HdmiCecEventListener();
    sp<IHdmiCecEventListener> eventListener_2 = new HdmiCecEventListener();

    HDMICEC_REGISTER_EVENT_FAILURE(gHDMICECHandle, invalidEventListener, Status::EX_NONE);

    HDMICEC_REGISTER_EVENT_SUCCESS(gHDMICECHandle, eventListener_1);
    HDMICEC_REGISTER_EVENT_FAILURE(gHDMICECHandle, eventListener_2, Status::EX_NONE);

    HDMICEC_UNREGISTER_EVENT_SUCCESS(gHDMICECHandle, eventListener_1);
}

/**
* @brief Tests the unregisterEventListener API
*
* This test verifies that the unregisterEventListener API returns expected status value
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 011@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* |Variation / Step|Description|Test Data|Expected Result|Notes|
* |----------------|-----------|---------|---------------|-----|
* |01|Call registerEventListener()||should register event listener|should pass|
* |02|Call unregisterEventListener()||should un-register event listener|should pass|
*/
UT_ADD_TEST(VTS_L1_HDMICEC, unregisterEventListener_pos)
{
    sp<IHdmiCecEventListener> eventListener = new HdmiCecEventListener();

    HDMICEC_REGISTER_EVENT_SUCCESS(gHDMICECHandle, eventListener);

    HDMICEC_UNREGISTER_EVENT_SUCCESS(gHDMICECHandle, eventListener);
}

/**
* @brief Tests the unregisterEventListener API for netagive scenarios
*
* This test verifies that the unregisterEventListener API handles the negative scenarios
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 012@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* |Variation / Step|Description|Test Data|Expected Result|Notes|
* |----------------|-----------|---------|---------------|-----|
* |01|Call unregisterEventListener() with invalid listener||should return false|should pass|
* |02|Call registerEventListener() with valid listener||should return true|should pass|
* |03|Call unregisterEventListener() with valid listener||should return true|should pass|
* |04|Call unregisterEventListener() with valid listener again||should return false|should pass|
*/
UT_ADD_TEST(VTS_L1_HDMICEC, unregisterEventListener_neg)
{
    sp<IHdmiCecEventListener> invalidEventListener = nullptr;
    sp<IHdmiCecEventListener> eventListener = new HdmiCecEventListener();

    HDMICEC_UNREGISTER_EVENT_FAILURE(gHDMICECHandle, invalidEventListener, Status::EX_NONE);
    HDMICEC_REGISTER_EVENT_SUCCESS(gHDMICECHandle, eventListener);
    HDMICEC_UNREGISTER_EVENT_SUCCESS(gHDMICECHandle, eventListener);
    HDMICEC_UNREGISTER_EVENT_FAILURE(gHDMICECHandle, eventListener, Status::EX_NONE);
}

/**
* @brief Tests the
*
* This test verifies that the
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 001@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* |Variation / Step|Description|Test Data|Expected Result|Notes|
* |----------------|-----------|---------|---------------|-----|
* |01||None||Should Pass|

*/
UT_ADD_TEST(VTS_L1_HDMICEC, test)
{
    com::rdk::hal::State state;
    sp<IHdmiCecController> hdmiCecController = nullptr;
    sp<IHdmiCecControllerListener> controllerListener = new HdmiCecControllerListener();
    sp<IHdmiCecEventListener> eventListener = new HdmiCecEventListener();
    std::optional<PropertyValue> cecVersion;
    vector<int32_t> logicalAddresses = {0, 3};
    vector<int32_t> remAddresses1 = {0};
    vector<int32_t> remAddresses2 = {3};
    vector<int32_t> getlogicalAddresses;
    vector<uint8_t> directMsg = {0x53, 0x04};
    vector<uint8_t> directMsg_res = {0x53, 0x46};
    vector<uint8_t> broadMsg = {0xF3, 0x32};
    hdmicec::SendMessageStatus msgStatus;


    HDMICEC_GET_STATE(gHDMICECHandle, state);

    HDMICEC_REGISTER_EVENT_SUCCESS(gHDMICECHandle, eventListener);

    HDMICEC_OPEN_SUCCESS(gHDMICECHandle, controllerListener, hdmiCecController, true);

    HDMICEC_GET_PROPERTY_SUCCESS(gHDMICECHandle, hdmicec::Property::HAL_CEC_VERSION, cecVersion);

    HDMICEC_GET_LOGICAL_ADDRESS_SUCCESS(gHDMICECHandle, getlogicalAddresses);

    HDMICEC_ADD_LOGICAL_ADDRESS_SUCCESS(hdmiCecController, logicalAddresses);
    HDMICEC_GET_LOGICAL_ADDRESS_SUCCESS(gHDMICECHandle, getlogicalAddresses);

    HDMICEC_SEND_MESSAGE_SUCCESS(hdmiCecController, directMsg, msgStatus);
    HDMICEC_SEND_MESSAGE_SUCCESS(hdmiCecController, directMsg_res, msgStatus);
    HDMICEC_SEND_MESSAGE_SUCCESS(hdmiCecController, broadMsg, msgStatus);

    this_thread::sleep_for(chrono::milliseconds(50));

    validateMetrics();

    HDMICEC_REMOVE_LOGICAL_ADDRESS_SUCCESS(hdmiCecController, remAddresses1);
    HDMICEC_GET_LOGICAL_ADDRESS_SUCCESS(gHDMICECHandle, getlogicalAddresses);

    HDMICEC_REMOVE_LOGICAL_ADDRESS_SUCCESS(hdmiCecController, remAddresses2);
    HDMICEC_GET_LOGICAL_ADDRESS_SUCCESS(gHDMICECHandle, getlogicalAddresses);

    HDMICEC_CLOSE_SUCCESS(gHDMICECHandle, hdmiCecController, true);
    HDMICEC_UNREGISTER_EVENT_SUCCESS(gHDMICECHandle, eventListener);

    validateMetrics();
}

/** @} */ // End of HDMI_CEC_HALTEST L1 File
/** @} */ // End of HDMI_CEC_HALTEST
/** @} */ // End of HDMI_CEC Module
/** @} */ // End of HPK
