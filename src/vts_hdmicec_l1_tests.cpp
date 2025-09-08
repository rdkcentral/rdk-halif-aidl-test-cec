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
UT_ADD_TEST(VTS_L1_HDMICEC, getState_pos)
{
    com::rdk::hal::State state;
    sp<IHdmiCecController> hdmiCecController = nullptr;
    sp<IHdmiCecControllerListener> controllerListener = new HdmiCecControllerListener();
    sp<IHdmiCecEventListener> eventListener = new HdmiCecEventListener();
    std::optional<PropertyValue> cecVersion;
    vector<int32_t> logicalAddresses = {3, 6};
    vector<int32_t> remAddresses1 = {3};
    vector<int32_t> remAddresses2 = {6};
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
