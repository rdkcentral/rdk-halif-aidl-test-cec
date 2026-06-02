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
 * @defgroup HDMI_CEC_HALTEST_L2 HDMI CEC HAL Tests L2 File
 * @{
 * @parblock
 *
 * ### L2 Test Cases for HDMI CEC HAL :
 *
 * This module includes Level 2 functional tests (success and failure scenarios).
 * This is to ensure that the API meets the operational requirements of the module across all vendors.
 *
 * **Pre-Conditions:** @n
 * **Dependencies:** None@n
 *
 * @endparblock
 *
 */

/**
 * @file vts_hdmicec_l2_tests.cpp
 *
 */
#include "vts_hdmicec_parse_hfp.h"
#include "vts_hdmicec.h"

extern sp<IHdmiCec> gHDMICECHandle;

/* Test class registration with UT_Core */
UT_ADD_TEST_TO_GROUP(VTS_L2_HDMICEC, UT_TESTS_L2)

class VTS_L2_HDMICEC : public UTCore
{
public:
    VTS_L2_HDMICEC() : UTCore() {}

    ~VTS_L2_HDMICEC() override = default;

    void SetUp() override
    {
        android::ProcessState::self()->startThreadPool();
    }

    void TearDown() override
    {
    }
};

/**
 * @brief Tests HDMI CEC interface state transitions.
 *
 * This test validates the state machine behavior of the HDMI CEC interface.
 * It ensures that the interface transitions correctly between CLOSED and STARTED states,
 * and that state change notifications are received by the event listener.
 *
 * **Test Group ID:** Basic: 02@n
 * **Test Case ID:** 001@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:**
 * - No external HDMI devices connected to the DUT
 * - HDMI CEC interface must be available in CLOSED state initially
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

UT_ADD_TEST(VTS_L2_HDMICEC, StateTransitions)
{
    sp<HdmiCecEventListener> controllerListener = new HdmiCecEventListener();
    sp<IHdmiCecController> hdmiCecController = nullptr;
    com::rdk::hal::hdmicec::State state;
    pair<State, State> stateChanges;

    // Register event listener
    HDMICEC_REGISTER_EVENT_SUCCESS(gHDMICECHandle, controllerListener);

    // Call getState and retrieve the current HDMI CEC interface state
    HDMICEC_GET_STATE(gHDMICECHandle, state);

    // Verify that the initial state is CLOSED
    UT_ASSERT_EQUAL(state, State::CLOSED);

    // Ensure no spurious state changes before open()
    UT_ASSERT_FALSE(controllerListener->getNextStateChanges(stateChanges));

    /* multiple open and close test */
    for (int32_t i = 0; i < 2; i++)
    {
        // Open the HDMI CEC interface
        HDMICEC_OPEN_SUCCESS(gHDMICECHandle, controllerListener, hdmiCecController, true);

        // Verify that the onStateChanged for STARTED received to eventListener
        UT_ASSERT_TRUE(controllerListener->getNextStateChanges(stateChanges));
        UT_ASSERT_EQUAL(stateChanges.first, State::CLOSED);
        UT_ASSERT_EQUAL(stateChanges.second, State::STARTED);

        // Verify that the onStateChanged for STARTED received to eventListener
        UT_ASSERT_TRUE(controllerListener->getNextStateChanges(stateChanges));
        UT_ASSERT_EQUAL(stateChanges.first, State::CLOSED);
        UT_ASSERT_EQUAL(stateChanges.second, State::STARTED);

        // close the HDMI CEC interface
        HDMICEC_CLOSE_SUCCESS(gHDMICECHandle, hdmiCecController, true);

        // Verify that the onStateChanged for CLOSED received to eventListener
        UT_ASSERT_TRUE(controllerListener->getNextStateChanges(stateChanges));
        UT_ASSERT_EQUAL(stateChanges.first, State::STARTED);
        UT_ASSERT_EQUAL(stateChanges.second, State::CLOSED);

        // Verify that the onStateChanged for CLOSED received to eventListener
        UT_ASSERT_TRUE(controllerListener->getNextStateChanges(stateChanges));
        UT_ASSERT_EQUAL(stateChanges.first, State::STARTED);
        UT_ASSERT_EQUAL(stateChanges.second, State::CLOSED);

        // Ensure no spurious state changes after close()
        UT_ASSERT_FALSE(controllerListener->getNextStateChanges(stateChanges));
    }

    // Unregister event listener
    HDMICEC_UNREGISTER_EVENT_SUCCESS(gHDMICECHandle, controllerListener);
}

/**
 * @brief Validates HDMI CEC metric properties and HAL CEC version
 *
 * This test ensures that:
 * - The HAL_CEC_VERSION property is correctly reported.
 * - All defined metric properties (message counters, arbitration failures, etc.)
 *   are available and initialized to expected default values.
 *
 * **Test Group ID:** Basic: 02@n
 * **Test Case ID:** 002@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:**
 * - No external HDMI devices connected to the DUT
 * - HDMI CEC interface must be available in CLOSED state initially
 *
 * **Dependencies:** A valid gHDMICECHandle must be available@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * Refer to UT specification documentation [hdmicec_High-Level_TestSpecification.md](../docs/pages/hdmicec_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L2_HDMICEC, metricValidation)
{
    std::optional<PropertyValue> cecVersion;
    sp<IHdmiCecEventListener> controllerListener = new HdmiCecEventListener();
    sp<IHdmiCecController> hdmiCecController = nullptr;

    // List of metric properties to validate
    std::vector<Property> testProperties = {
        hdmicec::Property::METRIC_DIRECTED_MESSAGES_SENT,
        hdmicec::Property::METRIC_BROADCAST_MESSAGES_SENT,
        hdmicec::Property::METRIC_DIRECTED_MESSAGES_SENT_AND_ACKED,
        hdmicec::Property::METRIC_BROADCAST_MESSAGE_SENT_AND_ACKED,
        hdmicec::Property::METRIC_ARBITRATION_FAILURES};

    // Validate HAL_CEC_VERSION property
    HDMICEC_GET_PROPERTY_SUCCESS(gHDMICECHandle, hdmicec::Property::HAL_CEC_VERSION, cecVersion);
    UT_ASSERT_TRUE(cecVersion.has_value());
    UT_ASSERT_EQUAL(
        cecVersion->value->get<PropertyValue::Value::Tag::byteValue>(),
        static_cast<int8_t>(gHdmiCecDeviceInfo.hal_cec_version));

    // Validate metric properties
    for (auto property : testProperties)
    {
        std::optional<PropertyValue> propertyValue;
        int32_t value = 0;

        HDMICEC_GET_PROPERTY_SUCCESS(gHDMICECHandle, property, propertyValue);
        UT_ASSERT_TRUE(propertyValue.has_value());

        value = propertyValue->value->get<PropertyValue::Value::Tag::intValue>();
        if (value != -1) // -1 indicates not implemented
        {
            UT_ASSERT_EQUAL(value, 0); // Initial counters must be zero
        }
    }

    // Open controller
    HDMICEC_OPEN_SUCCESS(gHDMICECHandle, controllerListener, hdmiCecController, true);

    // Close controller
    HDMICEC_CLOSE_SUCCESS(gHDMICECHandle, hdmiCecController, true);

    // Validate metric properties again to ensure no changes after open/close
    for (auto property : testProperties)
    {
        std::optional<PropertyValue> propertyValue;
        int32_t value = 0;

        HDMICEC_GET_PROPERTY_SUCCESS(gHDMICECHandle, property, propertyValue);
        UT_ASSERT_TRUE(propertyValue.has_value());

        value = propertyValue->value->get<PropertyValue::Value::Tag::intValue>();
        if (value != -1) // -1 indicates not implemented
        {
            UT_ASSERT_EQUAL(value, 0); // Initial counters must be zero
        }
    }
}

/**
 * @brief Validates add/remove/get logical addresses functionality
 *
 * This test ensures that logical addresses can be:
 * - Added to the controller
 * - Retrieved via getLogicalAddresses()
 * - Removed selectively or fully
 *
 * It also verifies that the retrieved list matches expectations after
 * each operation.
 *
 * **Test Group ID:** Basic: 02@n
 * **Test Case ID:** 003@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:**
 * - No external HDMI devices connected to the DUT
 * - HDMI CEC interface must be available in CLOSED state initially
 *
 * **Dependencies:** A valid gHDMICECHandle must be available@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * Refer to UT specification documentation [hdmicec_High-Level_TestSpecification.md](../docs/pages/hdmicec_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L2_HDMICEC, logicalAddressesValidation)
{
    sp<IHdmiCecEventListener> controllerListener = new HdmiCecEventListener();
    sp<IHdmiCecController> hdmiCecController = nullptr;
    vector<int32_t> logicalAddresses_1 = {0, 1};
    vector<int32_t> logicalAddresses_2 = {2, 3};
    vector<int32_t> removeLogicalAddresses_1 = {0};
    vector<int32_t> removeLogicalAddresses_2 = {1};
    vector<int32_t> removeLogicalAddresses_3 = {2, 3};
    vector<int32_t> getLogicalAddresses;

    // Validate initial logical addresses (should be empty before opening)
    HDMICEC_GET_LOGICAL_ADDRESS_SUCCESS(gHDMICECHandle, getLogicalAddresses);
    UT_ASSERT_TRUE(getLogicalAddresses.empty());

    // Open controller
    HDMICEC_OPEN_SUCCESS(gHDMICECHandle, controllerListener, hdmiCecController, true);

    // Add addresses {0,1} and verify
    HDMICEC_ADD_LOGICAL_ADDRESS_SUCCESS(hdmiCecController, logicalAddresses_1);
    HDMICEC_GET_LOGICAL_ADDRESS_SUCCESS(gHDMICECHandle, getLogicalAddresses);
    UT_ASSERT_EQUAL(getLogicalAddresses.size(), logicalAddresses_1.size());
    for (auto addr : logicalAddresses_1)
    {
        UT_ASSERT_TRUE(std::find(getLogicalAddresses.begin(), getLogicalAddresses.end(), addr) != getLogicalAddresses.end());
    }

    // Remove {0}, verify only {1} remains
    HDMICEC_REMOVE_LOGICAL_ADDRESS_SUCCESS(hdmiCecController, removeLogicalAddresses_1);
    HDMICEC_GET_LOGICAL_ADDRESS_SUCCESS(gHDMICECHandle, getLogicalAddresses);
    UT_ASSERT_EQUAL(getLogicalAddresses.size(), (logicalAddresses_1.size() - removeLogicalAddresses_1.size()));
    for (auto addr : removeLogicalAddresses_1)
    {
        UT_ASSERT_TRUE(std::find(getLogicalAddresses.begin(), getLogicalAddresses.end(), addr) == getLogicalAddresses.end());
    }

    // Remove {1}, verify list is empty
    HDMICEC_REMOVE_LOGICAL_ADDRESS_SUCCESS(hdmiCecController, removeLogicalAddresses_2);
    HDMICEC_GET_LOGICAL_ADDRESS_SUCCESS(gHDMICECHandle, getLogicalAddresses);
    UT_ASSERT_TRUE(getLogicalAddresses.empty());

    // Add addresses {2,3} and verify
    HDMICEC_ADD_LOGICAL_ADDRESS_SUCCESS(hdmiCecController, logicalAddresses_2);
    HDMICEC_GET_LOGICAL_ADDRESS_SUCCESS(gHDMICECHandle, getLogicalAddresses);
    UT_ASSERT_EQUAL(getLogicalAddresses.size(), logicalAddresses_2.size());
    for (auto addr : logicalAddresses_2)
    {
        UT_ASSERT_TRUE(std::find(getLogicalAddresses.begin(), getLogicalAddresses.end(), addr) != getLogicalAddresses.end());
    }

    // Remove {2,3}, verify list is empty
    HDMICEC_REMOVE_LOGICAL_ADDRESS_SUCCESS(hdmiCecController, removeLogicalAddresses_3);
    HDMICEC_GET_LOGICAL_ADDRESS_SUCCESS(gHDMICECHandle, getLogicalAddresses);
    UT_ASSERT_TRUE(getLogicalAddresses.empty());

    // Add addresses {0,1} and to verify close is removing all addresses
    HDMICEC_ADD_LOGICAL_ADDRESS_SUCCESS(hdmiCecController, logicalAddresses_1);
    HDMICEC_GET_LOGICAL_ADDRESS_SUCCESS(gHDMICECHandle, getLogicalAddresses);
    UT_ASSERT_EQUAL(getLogicalAddresses.size(), logicalAddresses_1.size());
    for (auto addr : logicalAddresses_1)
    {
        UT_ASSERT_TRUE(std::find(getLogicalAddresses.begin(), getLogicalAddresses.end(), addr) != getLogicalAddresses.end());
    }

    // Close controller
    HDMICEC_CLOSE_SUCCESS(gHDMICECHandle, hdmiCecController, true);

    // validate the logical addresses (should be empty after closing)
    HDMICEC_GET_LOGICAL_ADDRESS_SUCCESS(gHDMICECHandle, getLogicalAddresses);
    UT_ASSERT_TRUE(getLogicalAddresses.empty());

}

/**
 * @brief Validates sendMessage API with message transmission and callback verification
 *
 * This test ensures that messages of varying sizes can be sent over HDMI CEC
 * after assigning logical addresses, and that the sent messages and status are
 * correctly received via the HdmiCecEventListener callback.
 *
 * It also validates that:
 * - Messages up to maximum payload length (16 bytes) are supported
 * - The status received via the callback matches expectations
 * - The message contents are preserved without corruption
 *
 * **Test Group ID:** Basic: 03@n
 * **Test Case ID:** 004@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:**
 * - No external HDMI devices connected to the DUT
 * - HDMI CEC interface must be available in CLOSED state initially
 *
 * **Dependencies:** A valid gHDMICECHandle must be available@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * Refer to UT specification documentation [hdmicec_High-Level_TestSpecification.md](../docs/pages/hdmicec_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(VTS_L2_HDMICEC, sendMessageValidation)
{
    sp<HdmiCecEventListener> controllerListener = new HdmiCecEventListener();
    sp<IHdmiCecController> hdmiCecController = nullptr;
    vector<int32_t> logicalAddresses = {0};
    vector<uint8_t> directMsg_1 = {0x05, 0x04};
    vector<uint8_t> directMsg_2 = {0x05, 0x64, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    vector<uint8_t> broadcastMsg = {0x0F, 0x85};
    hdmicec::SendMessageStatus msgStatus;
    vector<uint8_t> getMessage;
    hdmicec::SendMessageStatus getStatus;

    // Open HDMI CEC interface
    HDMICEC_OPEN_SUCCESS(gHDMICECHandle, controllerListener, hdmiCecController, true);

    // Add logical address
    HDMICEC_ADD_LOGICAL_ADDRESS_SUCCESS(hdmiCecController, logicalAddresses);

    // Send message
    HDMICEC_SEND_MESSAGE_SUCCESS(hdmiCecController, directMsg_1, msgStatus);

    // Wait for message to be processed and callback to be invoked
    this_thread::sleep_for(chrono::milliseconds(MAX_MESSAGE_SENT_WAIT_MS));

    // Verify that the onMessageSent received to eventListener
    controllerListener->getNextMessage(getMessage, getStatus);

    UT_ASSERT_EQUAL(getMessage.size(), directMsg_1.size());

    if(getMessage.size() == directMsg_1.size())
    {
        for (size_t i = 0; i < directMsg_1.size(); i++)
        {
            UT_ASSERT_EQUAL(getMessage[i], directMsg_1[i]);
        }
    }
    UT_ASSERT_EQUAL(getStatus, hdmicec::SendMessageStatus::ACK_STATE_1);

    // Send another message
    HDMICEC_SEND_MESSAGE_SUCCESS(hdmiCecController, directMsg_2, msgStatus);

    // Wait for message to be processed and callback to be invoked
    this_thread::sleep_for(chrono::milliseconds(MAX_MESSAGE_SENT_WAIT_MS));

    // Verify that the onMessageSent received to eventListener
    controllerListener->getNextMessage(getMessage, getStatus);
    UT_ASSERT_EQUAL(getMessage.size(), directMsg_2.size());
    if(getMessage.size() == directMsg_2.size())
    {
        for (size_t i = 0; i < directMsg_2.size(); i++)
        {
            UT_ASSERT_EQUAL(getMessage[i], directMsg_2[i]);
        }
    }
    UT_ASSERT_EQUAL(getStatus, hdmicec::SendMessageStatus::ACK_STATE_1);

    // Send broadcast message
    HDMICEC_SEND_MESSAGE_SUCCESS(hdmiCecController, broadcastMsg, msgStatus);

    // Wait for message to be processed and callback to be invoked
    this_thread::sleep_for(chrono::milliseconds(MAX_MESSAGE_SENT_WAIT_MS));

    // Verify that the onMessageSent received to eventListener
    controllerListener->getNextMessage(getMessage, getStatus);
    UT_ASSERT_EQUAL(getMessage.size(), broadcastMsg.size());
    if(getMessage.size() == broadcastMsg.size())
    {
        for (size_t i = 0; i < broadcastMsg.size(); i++)
        {
            UT_ASSERT_EQUAL(getMessage[i], broadcastMsg[i]);
        }
    }
    UT_ASSERT_EQUAL(getStatus, hdmicec::SendMessageStatus::ACK_STATE_0);

    // Remove logical address
    HDMICEC_REMOVE_LOGICAL_ADDRESS_SUCCESS(hdmiCecController, logicalAddresses);

    // Close HDMI CEC interface
    HDMICEC_CLOSE_SUCCESS(gHDMICECHandle, hdmiCecController, true);
}

/** @} */ // End of HDMI_CEC_HALTEST L2 File
/** @} */ // End of HDMI_CEC_HALTEST
/** @} */ // End of HDMI_CEC Module
/** @} */ // End of HPK
