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

extern sp<IHdmiCec> gHDMICECHandle;

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

UT_ADD_TEST(VTS_L3_HDMICEC, StateTransitions)
{
    sp<HdmiCecControllerListener> controllerListener = new HdmiCecControllerListener();
    sp<IHdmiCecEventListener> eventListener = new HdmiCecEventListener();
    sp<IHdmiCecController> hdmiCecController = nullptr;
    vector<int32_t> logicalAddresses = {0};
    vector<uint8_t> directMsg = {0x05, 0x46};
    hdmicec::SendMessageStatus msgStatus;

    // Register Event Listener
    HDMICEC_REGISTER_EVENT_SUCCESS(gHDMICECHandle, eventListener);

    // Open HDMI CEC interface
    HDMICEC_OPEN_SUCCESS(gHDMICECHandle, controllerListener, hdmiCecController, true);

    // Add logical address
    HDMICEC_ADD_LOGICAL_ADDRESS_SUCCESS(hdmiCecController, logicalAddresses);

    // Send message
    HDMICEC_SEND_MESSAGE_SUCCESS(hdmiCecController, directMsg, msgStatus);

    // Remove logical address
    HDMICEC_REMOVE_LOGICAL_ADDRESS_SUCCESS(hdmiCecController, logicalAddresses);

    // Close HDMI CEC interface
    HDMICEC_CLOSE_SUCCESS(gHDMICECHandle, hdmiCecController, true);

    // Unregister Event Listener
    HDMICEC_UNREGISTER_EVENT_SUCCESS(gHDMICECHandle, eventListener);

}

/** @} */ // End of HDMI_CEC_HALTEST L3 File
/** @} */ // End of HDMI_CEC_HALTEST
/** @} */ // End of HDMI_CEC Module
/** @} */ // End of HPK
