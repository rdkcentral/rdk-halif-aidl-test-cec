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
 * @file vts_hdmicec_l1_tests.cpp
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

    void TearDown() override
    {
    }
};

/**
 * @brief L1 Test for the HDMI CEC HAL getState() API (Positive Scenario)
 *
 * This test validates that the getState() API correctly reports the current
 * state of the HDMI CEC interface when first queried.
 *
 * The HAL specification guarantees that only CLOSED and STARTED states
 * are reported. Since no prior open() call has been made in this test,
 * the expected result is State::CLOSED.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 001@n
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
 * |Variation / Step|Description|Test Data|Expected Result|Notes|
 * |----------------|-----------|---------|---------------|-----|
 * |01|Call getState() on HAL|None|State::CLOSED|Initial state|
 */
UT_ADD_TEST(VTS_L1_HDMICEC, getState_pos)
{
    com::rdk::hal::State state;

    // Call getState and retrieve the current HDMI CEC interface state
    HDMICEC_GET_STATE(gHDMICECHandle, state);

    // Verify that the initial state is CLOSED
    UT_ASSERT_EQUAL(state, State::CLOSED);
}

/**
 * @brief L1 Positive Test for the HDMI CEC HAL getProperty() API
 *
 * This test verifies that the getProperty() API returns valid property values
 * for supported keys such as HAL_CEC_VERSION and metric counters.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 002@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:**
 * - No external HDMI devices connected to the DUT
 * - HDMI CEC interface must be available in CLOSED state initially
 *
 * **Dependencies:** gHDMICECHandle and gHdmiCecDeviceInfo must be valid@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * |Variation / Step|Description|Test Data|Expected Result|Notes|
 * |----------------|-----------|---------|---------------|-----|
 * |01|Query HAL_CEC_VERSION property|HAL_CEC_VERSION|Value should match device info (hfp version)|Mandatory property|
 * |02|Query metric counter properties|Metric properties|If implemented, return 0; if not implemented, return -1|Metrics counters start at 0|
 */
UT_ADD_TEST(VTS_L1_HDMICEC, getProperty_pos)
{
    std::optional<PropertyValue> cecVersion;

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
        cecVersion->value->get<PropertyValue::Value::Tag::intValue>(),
        gHdmiCecDeviceInfo.hal_cec_version);

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
}

/**
 * @brief L1 Negative Test for the HDMI CEC HAL getProperty() API
 *
 * This test verifies that the getProperty() API correctly handles
 * invalid or unsupported property keys and returns null.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 003@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:**
 * - No external HDMI devices connected to the DUT
 * - HDMI CEC interface must be available in CLOSED state initially
 *
 * **Dependencies:** gHDMICECHandle must be valid@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * |Variation / Step|Description|Test Data|Expected Result|Notes|
 * |----------------|-----------|---------|---------------|-----|
 * |01|Call getProperty() with invalid key |Invalid property|Return null (no value present)|Negative boundary check|
 */
UT_ADD_TEST(VTS_L1_HDMICEC, getProperty_neg)
{
    std::optional<PropertyValue> propertyValue;

    int32_t firstValue = static_cast<int32_t>(Property::HAL_CEC_VERSION);
    Property invalidProperty = static_cast<Property>(firstValue - 1);

    // Query invalid property
    HDMICEC_GET_PROPERTY_FAILURE(gHDMICECHandle, invalidProperty, propertyValue, Status::EX_NONE);

    // Expect no value for invalid property
    UT_ASSERT_FALSE(propertyValue.has_value());
}

/**
 * @brief L1 Positive Test for the HDMI CEC HAL getLogicalAddresses() API
 *
 * This test verifies that the getLogicalAddresses() API correctly returns
 * an empty vector when no logical addresses have been configured.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 004@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:**
 * - No external HDMI devices connected to the DUT
 * - HDMI CEC interface must be available in CLOSED state initially
 *
 * **Dependencies:** gHDMICECHandle must be valid@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * |Variation / Step|Description|Test Data|Expected Result|Notes|
 * |----------------|-----------|---------|---------------|-----|
 * |01|Call getLogicalAddresses()|None|Return an empty vector (no addresses present)|Initial state check|
 */
UT_ADD_TEST(VTS_L1_HDMICEC, getLogicalAddresses_pos)
{
    vector<int32_t> getLogicalAddresses;

    /* Get logical addresses, by default the list is empty */
    HDMICEC_GET_LOGICAL_ADDRESS_SUCCESS(gHDMICECHandle, getLogicalAddresses);

    UT_ASSERT_TRUE(getLogicalAddresses.empty());
}

/**
 * @brief L1 Positive Test for the HDMI CEC HAL open() API
 *
 * This test verifies that the open() API successfully returns a valid
 * controller handle and that the state transitions to STARTED.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 005@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:**
 * - No external HDMI devices connected to the DUT
 * - HDMI CEC interface must be available in CLOSED state initially
 *
 * **Dependencies:** gHDMICECHandle must be valid@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * |Variation / Step|Description|Test Data|Expected Result|Notes|
 * |----------------|-----------|---------|---------------|-----|
 * |01|Call open()|Valid listener|Valid IHdmiCecController handle returned; state=STARTED|should pass|
 * |02|Call close()|None|Return status=OK and state=CLOSED|should pass|
 */
UT_ADD_TEST(VTS_L1_HDMICEC, open_pos)
{
    sp<IHdmiCecControllerListener> controllerListener = new HdmiCecControllerListener();
    sp<IHdmiCecController> hdmiCecController = nullptr;

    /* Open the interface */
    HDMICEC_OPEN_SUCCESS(gHDMICECHandle, controllerListener, hdmiCecController, true);

    /* Close the interface */
    HDMICEC_CLOSE_SUCCESS(gHDMICECHandle, hdmiCecController, true);
}

/**
 * @brief L1 Negative Test for the HDMI CEC HAL open() API
 *
 * This test verifies that the open() API handles invalid parameters
 * and multiple open attempts as expected.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 006@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:**
 * - No external HDMI devices connected to the DUT
 * - HDMI CEC interface must be available in CLOSED state initially
 *
 * **Dependencies:** gHDMICECHandle must be valid@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * |Variation / Step|Description|Test Data|Expected Result|Notes|
 * |----------------|-----------|---------|---------------|-----|
 * |01|Call open() with null listener|NULL|Controller=null, status=EX_ILLEGAL_ARGUMENT, state=CLOSED|should pass|
 * |02|Call open() with valid listener|Valid|Valid IHdmiCecController handle returned; state=STARTED |should pass|
 * |03|Call open() again with same listener|Valid|Controller=null, status=EX_ILLEGAL_STATE, state=STARTED|should pass|
 * |04|Call close()|None|Return status=OK and state=CLOSED|should pass|
 */
UT_ADD_TEST(VTS_L1_HDMICEC, open_neg)
{
    sp<IHdmiCecControllerListener> invalidControllerListener = nullptr;
    sp<IHdmiCecControllerListener> controllerListener = new HdmiCecControllerListener();
    sp<IHdmiCecController> hdmiCecController_1 = nullptr;
    sp<IHdmiCecController> hdmiCecController_2 = nullptr;

    // Expect open to fail with null listener
    HDMICEC_OPEN_FAILURE(gHDMICECHandle, invalidControllerListener, hdmiCecController_1, State::CLOSED, Status::EX_NULL_POINTER);

    // Valid open
    HDMICEC_OPEN_SUCCESS(gHDMICECHandle, controllerListener, hdmiCecController_1, true);

    // Second open attempt should fail
    HDMICEC_OPEN_FAILURE(gHDMICECHandle, controllerListener, hdmiCecController_2, State::STARTED, Status::EX_ILLEGAL_STATE);

    // Clean up
    HDMICEC_CLOSE_SUCCESS(gHDMICECHandle, hdmiCecController_1, true);
}

/**
 * @brief L1 Positive Test for the HDMI CEC HAL close() API
 *
 * This test verifies that the close() API successfully closes the controller
 * and transitions the state from STARTED to CLOSED.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 007@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:**
 * - No external HDMI devices connected to the DUT
 * - HDMI CEC interface must be available in CLOSED state initially
 *
 * **Dependencies:** gHDMICECHandle must be valid@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * |Variation / Step|Description|Test Data|Expected Result|Notes|
 * |----------------|-----------|---------|---------------|-----|
 * |01|Call open()|Valid listener|Valid IHdmiCecController handle returned; state=STARTED|should pass|
 * |02|Call close()|Controller handle|Return status=true; state=CLOSED|should pass|
 */
UT_ADD_TEST(VTS_L1_HDMICEC, close_pos)
{
    sp<IHdmiCecControllerListener> controllerListener = new HdmiCecControllerListener();
    sp<IHdmiCecController> hdmiCecController = nullptr;

    /* Open first */
    HDMICEC_OPEN_SUCCESS(gHDMICECHandle, controllerListener, hdmiCecController, true);

    /* Then close */
    HDMICEC_CLOSE_SUCCESS(gHDMICECHandle, hdmiCecController, true);
}

/**
 * @brief L1 Negative Test for the HDMI CEC HAL close() API
 *
 * This test verifies that the close() API handles invalid parameters
 * and incorrect state transitions gracefully.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 008@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:**
 * - No external HDMI devices connected to the DUT
 * - HDMI CEC interface must be available in CLOSED state initially
 *
 * **Dependencies:** gHDMICECHandle must be valid@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * |Variation / Step|Description|Test Data|Expected Result|Notes|
 * |----------------|-----------|---------|---------------|-----|
 * |01|Call close() with null controller|NULL handle|Return status=false; state remains STARTED|should pass|
 * |02|Call close() with valid controller|Valid handle|Return status=true; state=CLOSED|should pass|
 * |03|Call close() again after already closed|Valid handle|Return status=false; state remains CLOSED|should pass|
 */
UT_ADD_TEST(VTS_L1_HDMICEC, close_neg)
{
    sp<IHdmiCecControllerListener> controllerListener = new HdmiCecControllerListener();
    sp<IHdmiCecController> hdmiCecController = nullptr;
    sp<IHdmiCecController> invalidHdmiCecController = nullptr;

    // Open first
    HDMICEC_OPEN_SUCCESS(gHDMICECHandle, controllerListener, hdmiCecController, true);

    // Invalid close attempt (null controller)
    HDMICEC_CLOSE_FAILURE(gHDMICECHandle, invalidHdmiCecController, State::STARTED, Status::EX_NULL_POINTER);

    // Valid close
    HDMICEC_CLOSE_SUCCESS(gHDMICECHandle, hdmiCecController, true);

    // Attempt to close again after already closed
    HDMICEC_CLOSE_FAILURE(gHDMICECHandle, hdmiCecController, State::CLOSED, Status::EX_NONE);
}

/**
 * @brief L1 Positive Test for the HDMI CEC HAL registerEventListener() API
 *
 * This test verifies that the registerEventListener() API successfully registers
 * a valid event listener and allows it to be unregistered.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 009@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:**
 * - No external HDMI devices connected to the DUT
 * - HDMI CEC interface must be available in CLOSED state initially
 *
 * **Dependencies:** gHDMICECHandle must be valid@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * |Variation / Step|Description|Test Data|Expected Result|Notes|
 * |----------------|-----------|---------|---------------|-----|
 * |01|Call registerEventListener()|Valid listener|Return status=true|should pass|
 * |02|Call unregisterEventListener()|Same listener|Return status=true (unregistered)|should pass|
 */
UT_ADD_TEST(VTS_L1_HDMICEC, registerEventListener_pos)
{
    sp<IHdmiCecEventListener> eventListener = new HdmiCecEventListener();

    /* Register the event listener */
    HDMICEC_REGISTER_EVENT_SUCCESS(gHDMICECHandle, eventListener);
    /* Unregister the event listener */
    HDMICEC_UNREGISTER_EVENT_SUCCESS(gHDMICECHandle, eventListener);
}

/**
 * @brief L1 Negative Test for the HDMI CEC HAL registerEventListener() API
 *
 * This test verifies that the registerEventListener() API handles invalid input
 * and duplicate registration attempts correctly.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 010@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:**
 * - No external HDMI devices connected to the DUT
 * - HDMI CEC interface must be available in CLOSED state initially
 *
 * **Dependencies:** gHDMICECHandle must be valid@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * |Variation / Step|Description|Test Data|Expected Result|Notes|
 * |----------------|-----------|---------|---------------|-----|
 * |01|Call registerEventListener() with invalid listener|NULL listener |Return status=false|should pass|
 * |02|Call registerEventListener() with valid listener  |Valid listener|Return status=true|should pass|
 * |03|Call registerEventListener() again with another valid listener|Valid listener|Return status=false (already registered)|should pass|
 * |04|Call unregisterEventListener()|Same listener |Return status=true (unregistered) |should pass|
 */
UT_ADD_TEST(VTS_L1_HDMICEC, registerEventListener_neg)
{
    sp<IHdmiCecEventListener> invalidEventListener = nullptr;
    sp<IHdmiCecEventListener> eventListener_1 = new HdmiCecEventListener();
    sp<IHdmiCecEventListener> eventListener_2 = new HdmiCecEventListener();

    /* Register the event listeners with invalid listener */
    HDMICEC_REGISTER_EVENT_FAILURE(gHDMICECHandle, invalidEventListener, Status::EX_NULL_POINTER);

    /* Register the event listeners with valid listener */
    HDMICEC_REGISTER_EVENT_SUCCESS(gHDMICECHandle, eventListener_1);
    /* Register the event listeners with another valid listener */
    HDMICEC_REGISTER_EVENT_FAILURE(gHDMICECHandle, eventListener_2, Status::EX_NONE);

    /* Unregister the first event listener */
    HDMICEC_UNREGISTER_EVENT_SUCCESS(gHDMICECHandle, eventListener_1);
}

/**
 * @brief L1 Positive Test for the HDMI CEC HAL unregisterEventListener() API
 *
 * This test verifies that the unregisterEventListener() API successfully unregisters
 * a previously registered event listener.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 011@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:**
 * - No external HDMI devices connected to the DUT
 * - HDMI CEC interface must be available in CLOSED state initially
 *
 * **Dependencies:** gHDMICECHandle must be valid@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * |Variation / Step|Description|Test Data|Expected Result|Notes|
 * |----------------|-----------|---------|---------------|-----|
 * |01|Call registerEventListener() |Valid listener|Return status=true|should pass|
 * |02|Call unregisterEventListener()|Same listener|Return status=true (unregistered)|should pass|
 */
UT_ADD_TEST(VTS_L1_HDMICEC, unregisterEventListener_pos)
{
    sp<IHdmiCecEventListener> eventListener = new HdmiCecEventListener();

    /* Register the event listener */
    HDMICEC_REGISTER_EVENT_SUCCESS(gHDMICECHandle, eventListener);
    /* Unregister the event listener */
    HDMICEC_UNREGISTER_EVENT_SUCCESS(gHDMICECHandle, eventListener);
}

/**
 * @brief L1 Negative Test for the HDMI CEC HAL unregisterEventListener() API
 *
 * This test verifies that the unregisterEventListener() API handles invalid input
 * and multiple unregistration attempts correctly.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 012@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:**
 * - No external HDMI devices connected to the DUT
 * - HDMI CEC interface must be available in CLOSED state initially
 *
 * **Dependencies:** gHDMICECHandle must be valid@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * |Variation / Step|Description|Test Data|Expected Result|Notes|
 * |----------------|-----------|---------|---------------|-----|
 * |01|Call unregisterEventListener() with invalid listener|NULL listener|Return status=false|should pass|
 * |02|Call registerEventListener() with valid listener|Valid listener |Return status=true|should pass|
 * |03|Call unregisterEventListener() with valid listener|Same listener|Return status=true|should pass|
 * |04|Call unregisterEventListener() again with same listener|Same listener|Return status=false|should pass|
 */
UT_ADD_TEST(VTS_L1_HDMICEC, unregisterEventListener_neg)
{
    sp<IHdmiCecEventListener> invalidEventListener = nullptr;
    sp<IHdmiCecEventListener> eventListener = new HdmiCecEventListener();

    /* unregister without registering the listener */
    HDMICEC_UNREGISTER_EVENT_FAILURE(gHDMICECHandle, invalidEventListener, Status::EX_NULL_POINTER);

    /* register valid listener */
    HDMICEC_REGISTER_EVENT_SUCCESS(gHDMICECHandle, eventListener);
    /* unregister valid listener */
    HDMICEC_UNREGISTER_EVENT_SUCCESS(gHDMICECHandle, eventListener);
    /* unregister same listener again */
    HDMICEC_UNREGISTER_EVENT_FAILURE(gHDMICECHandle, eventListener, Status::EX_NONE);
}

/**
 * @brief L1 Positive Test for the HDMI CEC HAL addLogicalAddresses() API
 *
 * This test verifies that the addLogicalAddresses() API successfully adds valid logical
 * addresses, and they can be removed again. Multiple addresses and single addresses are tested.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 013@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:**
 * - No external HDMI devices connected to the DUT
 * - HDMI CEC interface must be available in CLOSED state initially
 *
 * **Dependencies:** gHDMICECHandle must be valid@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * |Variation / Step|Description|Test Data|Expected Result|Notes|
 * |----------------|-----------|---------|---------------|-----|
 * |01|Call open()|Valid controller listener|Should return handle|should pass|
 * |02|Call addLogicalAddresses()|{0–14} (all valid addresses)|Return status=true|should pass|
 * |03|Call removeLogicalAddresses()|{0–14}|Return status=true|should pass|
 * |04|Call addLogicalAddresses()|{0} (single valid address)|Return status=true|should pass|
 * |05|Call removeLogicalAddresses()|{0}|Return status=true|should pass|
 * |06|Call close()|Valid controller handle|Return status=true|should pass|
 */
UT_ADD_TEST(VTS_L1_HDMICEC, addLogicalAddresses_pos)
{
    sp<IHdmiCecControllerListener> controllerListener = new HdmiCecControllerListener();
    sp<IHdmiCecController> hdmiCecController = nullptr;
    vector<int32_t> logicalAddresses_1 = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14};
    vector<int32_t> logicalAddresses_2 = {0};

    /* Open HDMI CEC interface */
    HDMICEC_OPEN_SUCCESS(gHDMICECHandle, controllerListener, hdmiCecController, true);

    /* Add multiple logical addresses */
    HDMICEC_ADD_LOGICAL_ADDRESS_SUCCESS(hdmiCecController, logicalAddresses_1);
    /* Remove multiple logical addresses */
    HDMICEC_REMOVE_LOGICAL_ADDRESS_SUCCESS(hdmiCecController, logicalAddresses_1);

    /* Add single logical address */
    HDMICEC_ADD_LOGICAL_ADDRESS_SUCCESS(hdmiCecController, logicalAddresses_2);
    /* Remove single logical address */
    HDMICEC_REMOVE_LOGICAL_ADDRESS_SUCCESS(hdmiCecController, logicalAddresses_2);

    /* Close HDMI CEC interface */
    HDMICEC_CLOSE_SUCCESS(gHDMICECHandle, hdmiCecController, true);
}

/**
 * @brief L1 Negative Test for the HDMI CEC HAL addLogicalAddresses() API
 *
 * This test verifies that the addLogicalAddresses() API rejects invalid logical addresses,
 * duplicates, out-of-range values, and invalid state scenarios.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 014@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:**
 * - No external HDMI devices connected to the DUT
 * - HDMI CEC interface must be available in CLOSED state initially
 *
 * **Dependencies:** gHDMICECHandle must be valid@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * |Variation / Step|Description|Test Data|Expected Result|Notes|
 * |----------------|-----------|---------|---------------|-----|
 * |01|Create controller listener|None|should create listener|should pass|
 * |02|Call open()|Valid controller listener|Should return handle|should pass|
 * |03|Call addLogicalAddresses() with -1|{-1}|Return status=false|should pass|
 * |04|Call addLogicalAddresses() with 15|{15}|Return status=false|should pass|
 * |05|Call addLogicalAddresses() with 100|{100}|Return status=false|should pass|
 * |06|Call addLogicalAddresses() with duplicates|{3,3}|Return status=false|should pass|
 * |07|Call addLogicalAddresses() with mixed duplicates|{1,2,1,3,2}|Return status=false|should pass|
 * |08|Call addLogicalAddresses() with empty list|{}|Return status=false|should pass|
 * |09|Call addLogicalAddresses() with valid addresses|{1,2,3}|Return status=true|should pass|
 * |10|Call addLogicalAddresses() with same addresses|{1,2,3}|Return status=false (already added)|should pass|
 * |11|Call removeLogicalAddresses()|{1,2,3}|Return status=true|should pass|
 * |12|Call close()|Valid controller handle  |Return status=true|should pass|
 * |13|Call addLogicalAddresses() after close|{1,2,3}|Return status=false (ILLEGAL_STATE)|should pass|
 */
UT_ADD_TEST(VTS_L1_HDMICEC, addLogicalAddresses_neg)
{
    sp<IHdmiCecControllerListener> controllerListener = new HdmiCecControllerListener();
    sp<IHdmiCecController> hdmiCecController = nullptr;
    vector<int32_t> logicalAddresses = {1, 2, 3};
    vector<int32_t> invalidLogicalAddresses_1 = {-1};
    vector<int32_t> invalidLogicalAddresses_2 = {15};
    vector<int32_t> invalidLogicalAddresses_3 = {100};
    vector<int32_t> invalidLogicalAddresses_4 = {3, 3};
    vector<int32_t> invalidLogicalAddresses_5 = {1, 2, 1, 3, 2};
    vector<int32_t> invalidLogicalAddresses_6;

    /* Open HDMI CEC interface */
    HDMICEC_OPEN_SUCCESS(gHDMICECHandle, controllerListener, hdmiCecController, true);

    /* Attempt to add invalid logical addresses */
    HDMICEC_ADD_LOGICAL_ADDRESS_FAILURE(hdmiCecController, invalidLogicalAddresses_1, Status::EX_NONE);
    HDMICEC_ADD_LOGICAL_ADDRESS_FAILURE(hdmiCecController, invalidLogicalAddresses_2, Status::EX_NONE);
    HDMICEC_ADD_LOGICAL_ADDRESS_FAILURE(hdmiCecController, invalidLogicalAddresses_3, Status::EX_NONE);

    /* Attempt to add duplicate logical addresses */
    HDMICEC_ADD_LOGICAL_ADDRESS_FAILURE(hdmiCecController, invalidLogicalAddresses_4, Status::EX_NONE);
    HDMICEC_ADD_LOGICAL_ADDRESS_FAILURE(hdmiCecController, invalidLogicalAddresses_5, Status::EX_NONE);

    /* Attempt to add empty logical address list */
    HDMICEC_ADD_LOGICAL_ADDRESS_FAILURE(hdmiCecController, invalidLogicalAddresses_6, Status::EX_NONE);

    /* Add valid logical addresses */
    HDMICEC_ADD_LOGICAL_ADDRESS_SUCCESS(hdmiCecController, logicalAddresses);
    /* Attempt to add the same logical addresses again */
    HDMICEC_ADD_LOGICAL_ADDRESS_FAILURE(hdmiCecController, logicalAddresses, Status::EX_NONE);

    /* Remove logical addresses */
    HDMICEC_REMOVE_LOGICAL_ADDRESS_SUCCESS(hdmiCecController, logicalAddresses);

    /* Close HDMI CEC interface */
    HDMICEC_CLOSE_SUCCESS(gHDMICECHandle, hdmiCecController, true);

    /* Attempt to add logical addresses after close */
    HDMICEC_ADD_LOGICAL_ADDRESS_FAILURE(hdmiCecController, logicalAddresses, Status::EX_ILLEGAL_STATE);
}

/**
 * @brief L1 Positive Test for the HDMI CEC HAL removeLogicalAddresses() API
 *
 * This test verifies that the removeLogicalAddresses() API successfully removes
 * previously added logical addresses. It covers removing all at once, removing subsets,
 * and removing single addresses.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 015@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:**
 * - No external HDMI devices connected to the DUT
 * - HDMI CEC interface must be available in CLOSED state initially
 *
 * **Dependencies:** gHDMICECHandle must be valid@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * |Variation / Step|Description|Test Data|Expected Result|Notes|
 * |----------------|-----------|---------|---------------|-----|
 * |01|Call open()|Valid controller listener|Return valid handle |should pass|
 * |02|Add full set of logical addresses|{0–14}|Return status=true|should pass|
 * |03|Remove full set of logical addresses|{0–14}|Return status=true|should pass|
 * |04|Add full set again|{0–14}|Return status=true|should pass|
 * |05|Remove subset of addresses|{0,1,2,3}|Return status=true|should pass|
 * |06|Remove single address|{4}|Return status=true|should pass|
 * |07|Remove another single address|{5}|Return status=true|should pass|
 * |08|Remove remaining subset|{6–14}|Return status=true|should pass|
 * |09|Add a single logical address|{0}|Return status=true|should pass|
 * |10|Remove that single logical address|{0}|Return status=true|should pass|
 * |11|Call close()|Valid controller handle|Return status=true  |should pass|
 */
UT_ADD_TEST(VTS_L1_HDMICEC, removeLogicalAddresses_pos)
{
    sp<IHdmiCecControllerListener> controllerListener = new HdmiCecControllerListener();
    sp<IHdmiCecController> hdmiCecController = nullptr;
    vector<int32_t> logicalAddresses_1 = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14};
    vector<int32_t> removeLogicalAddresses_1 = {0, 1, 2, 3};
    vector<int32_t> removeLogicalAddresses_2 = {4};
    vector<int32_t> removeLogicalAddresses_3 = {5};
    vector<int32_t> removeLogicalAddresses_4 = {6, 7, 8, 9, 10, 11, 12, 13, 14};
    vector<int32_t> logicalAddresses_2 = {0};

    /* Open HDMI CEC interface */
    HDMICEC_OPEN_SUCCESS(gHDMICECHandle, controllerListener, hdmiCecController, true);

    /* Add logical addresses */
    HDMICEC_ADD_LOGICAL_ADDRESS_SUCCESS(hdmiCecController, logicalAddresses_1);

    /* Remove all logical addresses */
    HDMICEC_REMOVE_LOGICAL_ADDRESS_SUCCESS(hdmiCecController, logicalAddresses_1);

    /* Add logical addresses again */
    HDMICEC_ADD_LOGICAL_ADDRESS_SUCCESS(hdmiCecController, logicalAddresses_1);

    /* Remove logical addresses separately */
    HDMICEC_REMOVE_LOGICAL_ADDRESS_SUCCESS(hdmiCecController, removeLogicalAddresses_1);
    HDMICEC_REMOVE_LOGICAL_ADDRESS_SUCCESS(hdmiCecController, removeLogicalAddresses_2);
    HDMICEC_REMOVE_LOGICAL_ADDRESS_SUCCESS(hdmiCecController, removeLogicalAddresses_3);
    HDMICEC_REMOVE_LOGICAL_ADDRESS_SUCCESS(hdmiCecController, removeLogicalAddresses_4);

    /* Add a single logical address */
    HDMICEC_ADD_LOGICAL_ADDRESS_SUCCESS(hdmiCecController, logicalAddresses_2);

    /* Remove that single logical address */
    HDMICEC_REMOVE_LOGICAL_ADDRESS_SUCCESS(hdmiCecController, logicalAddresses_2);

    /* Close HDMI CEC interface */
    HDMICEC_CLOSE_SUCCESS(gHDMICECHandle, hdmiCecController, true);
}

/**
 * @brief L1 Negative Test for the HDMI CEC HAL removeLogicalAddresses() API
 *
 * This test verifies that the removeLogicalAddresses() API rejects invalid addresses,
 * addresses not previously added, out-of-range values, empty lists, and invalid state scenarios.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 016@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:**
 * - No external HDMI devices connected to the DUT
 * - HDMI CEC interface must be available in CLOSED state initially
 *
 * **Dependencies:** gHDMICECHandle must be valid@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * |Variation / Step|Description|Test Data|Expected Result|Notes|
 * |----------------|-----------|---------|---------------|-----|
 * |01|Create controller listener|None|should create listener|should pass|
 * |02|Call open()|Valid controller|Return valid handle|should pass|
 * |03|Call removeLogicalAddresses() not added|{1,2,3}|Return status=false|should pass|
 * |04|Add valid logical addresses|{1,2,3}|Return status=true |should pass|
 * |05|Remove invalid address (-1)|{-1}|Return status=false|should pass|
 * |06|Remove invalid address (15)|{15}|Return status=false|should pass|
 * |07|Remove empty list|{}|Return status=false|should pass|
 * |08|Remove address not added|{4}|Return status=false|should pass|
 * |09|Remove mix of added and not added|{1,2,3,4}|Return status=false|should pass|
 * |10|Remove valid logical addresses|{1,2,3}|Return status=true |should pass|
 * |11|Attempt to remove same addresses again|{1,2,3}|Return status=false|should pass|
 * |12|Call close()|Valid controller|Return status=true|should pass|
 * |13|Call removeLogicalAddresses() after close|{1,2,3}|Return status=false (ILLEGAL_STATE)|should pass|
 */
UT_ADD_TEST(VTS_L1_HDMICEC, removeLogicalAddresses_neg)
{
    sp<IHdmiCecControllerListener> controllerListener = new HdmiCecControllerListener();
    sp<IHdmiCecController> hdmiCecController = nullptr;
    vector<int32_t> logicalAddresses = {0, 1, 2, 3};
    vector<int32_t> invalidRemoveLogicalAddresses_1 = {-1};
    vector<int32_t> invalidRemoveLogicalAddresses_2 = {15};
    vector<int32_t> invalidRemoveLogicalAddresses_3;
    vector<int32_t> invalidRemoveLogicalAddresses_4 = {4};
    vector<int32_t> invalidRemoveLogicalAddresses_5 = {1, 2, 3, 4};

    /* Open HDMI CEC interface */
    HDMICEC_OPEN_SUCCESS(gHDMICECHandle, controllerListener, hdmiCecController, true);

    /* Remove logical addresses without adding them first */
    HDMICEC_REMOVE_LOGICAL_ADDRESS_FAILURE(hdmiCecController, logicalAddresses, Status::EX_NONE);

    /* Add valid logical addresses */
    HDMICEC_ADD_LOGICAL_ADDRESS_SUCCESS(hdmiCecController, logicalAddresses);

    /* Attempt to remove invalid logical addresses */
    HDMICEC_REMOVE_LOGICAL_ADDRESS_FAILURE(hdmiCecController, invalidRemoveLogicalAddresses_1, Status::EX_NONE);
    HDMICEC_REMOVE_LOGICAL_ADDRESS_FAILURE(hdmiCecController, invalidRemoveLogicalAddresses_2, Status::EX_NONE);
    HDMICEC_REMOVE_LOGICAL_ADDRESS_FAILURE(hdmiCecController, invalidRemoveLogicalAddresses_3, Status::EX_NONE);

    /* Attempt to remove which is not added */
    HDMICEC_REMOVE_LOGICAL_ADDRESS_FAILURE(hdmiCecController, invalidRemoveLogicalAddresses_4, Status::EX_NONE);
    HDMICEC_REMOVE_LOGICAL_ADDRESS_FAILURE(hdmiCecController, invalidRemoveLogicalAddresses_5, Status::EX_NONE);

    /* Remove valid logical addresses */
    HDMICEC_REMOVE_LOGICAL_ADDRESS_SUCCESS(hdmiCecController, logicalAddresses);

    /* Attempt to remove same addresses again */
    HDMICEC_REMOVE_LOGICAL_ADDRESS_FAILURE(hdmiCecController, logicalAddresses, Status::EX_NONE);

    /* Close HDMI CEC interface */
    HDMICEC_CLOSE_SUCCESS(gHDMICECHandle, hdmiCecController, true);

    /* Attempt to remove addresses after close */
    HDMICEC_REMOVE_LOGICAL_ADDRESS_FAILURE(hdmiCecController, logicalAddresses, Status::EX_ILLEGAL_STATE);
}

/**
 * @brief L1 Positive Test for HDMI CEC HAL sendMessage() API
 *
 * Verifies that sendMessage() successfully transmits a direct message
 * when the HDMI CEC interface is in STARTED state.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 017@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:**
 * - No external HDMI devices connected to the DUT
 * - HDMI CEC interface must be available in CLOSED state initially
 *
 * **Dependencies:** gHDMICECHandle must be valid@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * |Variation / Step|Description|Test Data|Expected Result|Notes|
 * |----------------|-----------|---------|---------------|-----|
 * |01|Call open()|Valid listener|Return valid controller|should pass|
 * |02|Send a direct CEC message|{0x53, 0x04}|Status should be SUCCESS|should pass|
 * |03|Call close()|Valid controller|Return status=true|should pass|
 */
UT_ADD_TEST(VTS_L1_HDMICEC, sendMessage_pos)
{
    sp<IHdmiCecControllerListener> controllerListener = new HdmiCecControllerListener();
    sp<IHdmiCecController> hdmiCecController = nullptr;
    vector<uint8_t> directMsg = {0x53, 0x04};
    hdmicec::SendMessageStatus msgStatus;

    // Open HDMI CEC interface
    HDMICEC_OPEN_SUCCESS(gHDMICECHandle, controllerListener, hdmiCecController, true);

    // Send direct message
    HDMICEC_SEND_MESSAGE_SUCCESS(hdmiCecController, directMsg, msgStatus);

    // Close HDMI CEC interface
    HDMICEC_CLOSE_SUCCESS(gHDMICECHandle, hdmiCecController, true);
}

/**
 * @brief L1 Negative Test for HDMI CEC HAL sendMessage() API
 *
 * Verifies that sendMessage() fails in illegal states, such as when
 * the HDMI CEC interface is not STARTED or after the controller is closed.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 018@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:**
 * - No external HDMI devices connected to the DUT
 * - HDMI CEC interface must be available in CLOSED state initially
 *
 * **Dependencies:** gHDMICECHandle must be valid@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * |Variation / Step|Description|Test Data|Expected Result|Notes|
 * |----------------|-----------|---------|---------------|-----|
 * |01|Create controller listener|None|should create listener|should pass|
 * |02|Call open()|Valid listener|Return valid controller|should pass|
 * |03|Call open()|Valid listener|Return valid controller|should pass|
 * |04|Call sendMessage()|send empty message|Return status|should pass|
 * |05|Call sendMessage()|message size greater than 16|Return status|should pass|
 * |06|Send message after close()|{0x53,0x04}|Return status EX_ILLEGAL_STATE |should pass|
 */
UT_ADD_TEST(VTS_L1_HDMICEC, sendMessage_neg)
{
    sp<IHdmiCecControllerListener> controllerListener = new HdmiCecControllerListener();
    sp<IHdmiCecController> hdmiCecController = nullptr;
    vector<uint8_t> directMsg = {0x53, 0x04};
    vector<uint8_t> invalidMsg_1 = {};
    vector<uint8_t> invalidMsg_2 = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    hdmicec::SendMessageStatus msgStatus;

    // Open HDMI CEC interface
    HDMICEC_OPEN_SUCCESS(gHDMICECHandle, controllerListener, hdmiCecController, true);

    // Send empty message
    HDMICEC_SEND_MESSAGE_FAILURE(hdmiCecController, invalidMsg_1, msgStatus, Status::EX_ILLEGAL_ARGUMENT);

    // Send empty message with size more than 16 bytes
    HDMICEC_SEND_MESSAGE_FAILURE(hdmiCecController, invalidMsg_2, msgStatus, Status::EX_ILLEGAL_ARGUMENT);

    // Close HDMI CEC interface
    HDMICEC_CLOSE_SUCCESS(gHDMICECHandle, hdmiCecController, true);

    // Send message after close() should fail
    HDMICEC_SEND_MESSAGE_FAILURE(hdmiCecController, directMsg, msgStatus, Status::EX_ILLEGAL_STATE);
}

/** @} */ // End of HDMI_CEC_HALTEST L1 File
/** @} */ // End of HDMI_CEC_HALTEST
/** @} */ // End of HDMI_CEC Module
/** @} */ // End of HPK
