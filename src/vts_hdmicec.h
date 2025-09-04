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
 * @defgroup HDMI_CEC_HALTEST_HEADER HDMI CEC HAL Tests helper functions Header File
 * @{
 * @parblock
 *
 * ### Common functions/macros required for the hdmi cec :
 *
 * Common functions/macros required for the hdmi cec .
 *
 * **Pre-Conditions:**  None @n
 * **Dependencies:** None @n
 *
 * @endparblock
 *
 */

/**
 * @file vts_hdmicec.h
 *
 */

#ifndef __VTS_HDMICEC_H__
#define __VTS_HDMICEC_H__

#include <iostream>
#include <vector>
#include <string>
#include <sstream>

#include <ut.h>
#include <ut_log.h>
#include <ut_kvp_profile.h>
#include <ut_control_plane.h>
#include <chrono>
#include <thread>

#include <binder/IServiceManager.h>
#include <binder/ProcessState.h>
#include <binder/IPCThreadState.h>
#include <binder/Status.h>
#include <utils/String16.h>

#include "com/rdk/hal/hdmicec/BnHdmiCecControllerListener.h"
#include "com/rdk/hal/hdmicec/BnHdmiCecEventListener.h"
#include "com/rdk/hal/hdmicec/IHdmiCec.h"
#include "com/rdk/hal/hdmicec/IHdmiCecController.h"
#include "com/rdk/hal/hdmicec/IHdmiCecControllerListener.h"
#include "com/rdk/hal/hdmicec/IHdmiCecEventListener.h"

#include "vts_hdmicec_helper_functions.h"

using android::sp;
using android::String16;
using android::defaultServiceManager;
using android::binder::Status;
using namespace std;
using namespace std::chrono;

using namespace com::rdk::hal;
using namespace com::rdk::hal::hdmicec;

#define MAX_TRANSITORY_STATE_WAIT_MS  5
#define MAX_FINAL_STATE_WAIT_MS  500

#define HDMICEC_GET_STATE(hdmicecHandle, state) \
    do { \
        ::android::binder::Status status; \
        UT_LOG_DEBUG("Calling getState(OUT:state[]"); \
        status = (hdmicecHandle)->getState(&state); \
        UT_LOG_DEBUG("Result getState(OUT:state[%s]), status:[isOK:%s, exception:%s]", com::rdk::hal::toString(state).c_str(), \
                UT_Control_GetMapString(bool_mapTable_cec, status.isOk()), \
                UT_Control_GetMapString(statusHdmiCecException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
    } while (0)

#define HDMICEC_WAIT_FOR_STATE(hdmicecHandle, desiredState, lastState) \
    do { \
        int32_t sleep_time = 0; \
        do \
        { \
            this_thread::sleep_for(chrono::milliseconds(MAX_TRANSITORY_STATE_WAIT_MS)); \
            sleep_time += MAX_TRANSITORY_STATE_WAIT_MS; \
            /* Check the state of the decoder */ \
            HDMICEC_GET_STATE((hdmicecHandle), lastState); \
        } while (sleep_time < MAX_FINAL_STATE_WAIT_MS && lastState != desiredState); \
    } while (0)

#define HDMICEC_OPEN_SUCCESS(hdmicecHandle, controllerListener, hdmiCecController, stateCheck) \
    do { \
        android::binder::Status status; \
        \
        /* Open the Decoder */ \
        UT_LOG_DEBUG("Calling open(IN:controllerListener[%p], OUT:hdmiCecController[])", controllerListener); \
        status = (hdmicecHandle)->open(controllerListener, &hdmiCecController); \
        UT_LOG_DEBUG("Result open(IN:IN:controllerListener[%p], OUT:hdmiCecController[0x%X]), status:[isOK:%s, exception:%s]", \
            controllerListener, hdmiCecController.get(), \
            UT_Control_GetMapString(bool_mapTable_cec, status.isOk()), \
            UT_Control_GetMapString(statusHdmiCecException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE_FATAL(status.isOk()); \
        UT_ASSERT_NOT_EQUAL_FATAL(hdmiCecController.get(), nullptr); \
        UT_ASSERT_EQUAL_FATAL(status.exceptionCode(), Status::EX_NONE); \
        \
        if (stateCheck) \
        { \
            com::rdk::hal::State lastState; \
            HDMICEC_WAIT_FOR_STATE(hdmicecHandle, State::STARTED, lastState); \
            UT_ASSERT_EQUAL_FATAL(lastState, State::STARTED); \
        } \
    } while (0)

#define HDMICEC_OPEN_FAILURE(hdmicecHandle, controllerListener, hdmiCecController, exception) \
    do { \
        android::binder::Status status; \
        \
        /* Open the Decoder */ \
        UT_LOG_DEBUG("Calling open(IN:controllerListener[%p], OUT:hdmiCecController[])", controllerListener); \
        status = (hdmicecHandle)->open(controllerListener, &hdmiCecController); \
        UT_LOG_DEBUG("Result open(IN:IN:controllerListener[%p], OUT:hdmiCecController[0x%X]), status:[isOK:%s, exception:%s]", \
            controllerListener, hdmiCecController.get(), \
            UT_Control_GetMapString(bool_mapTable_cec, status.isOk()), \
            UT_Control_GetMapString(statusHdmiCecException_mapTable, status.exceptionCode())); \
        UT_ASSERT_FALSE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), exception); \
        UT_ASSERT_EQUAL(hdmiCecController.get(), nullptr); \
    } while (0)

#define HDMICEC_CLOSE_SUCCESS(hdmicecHandle, hdmiCecController, stateCheck) \
    do { \
        ::android::binder::Status status; \
        bool _aidl_return; \
        UT_LOG_DEBUG("Calling close(IN:hdmiCecController[%p], OUT:_aidl_return[])", hdmiCecController.get()); \
        status = hdmicecHandle->close(hdmiCecController, &_aidl_return); \
        UT_LOG_DEBUG("Result close(IN:hdmiCecController[%p], OUT:_aidl_return[%s %d]), status:[isOK:%s, exception:%s]", \
            hdmiCecController.get(), UT_Control_GetMapString(bool_mapTable_cec, _aidl_return), _aidl_return,\
            UT_Control_GetMapString(bool_mapTable_cec, status.isOk()), \
            UT_Control_GetMapString(statusHdmiCecException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE_FATAL(status.isOk()); \
        UT_ASSERT_TRUE_FATAL(_aidl_return); \
        UT_ASSERT_EQUAL_FATAL(status.exceptionCode(), Status::EX_NONE); \
        if (stateCheck) \
        { \
            ::com::rdk::hal::State lastState; \
            HDMICEC_WAIT_FOR_STATE(hdmicecHandle, State::CLOSED, lastState); \
            UT_ASSERT_EQUAL_FATAL(lastState, State::CLOSED); \
        } \
    } while (0)

#define HDMICEC_CLOSE_FAILURE(hdmicecHandle, hdmiCecController, exception) \
    do { \
        ::android::binder::Status status; \
        bool _aidl_return; \
        UT_LOG_DEBUG("Calling close(IN:hdmiCecController[%p], OUT:_aidl_return[])", hdmiCecController.get()); \
        status = hdmicecHandle->close(hdmiCecController, &_aidl_return); \
        UT_LOG_DEBUG("Result close(IN:hdmiCecController[%p], OUT:_aidl_return[%s]), status:[isOK:%s, exception:%s]", \
            hdmiCecController.get(), UT_Control_GetMapString(bool_mapTable_cec, _aidl_return), \
            UT_Control_GetMapString(bool_mapTable_cec, status.isOk()), \
            UT_Control_GetMapString(statusHdmiCecException_mapTable, status.exceptionCode())); \
        UT_ASSERT_FALSE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), exception); \
        UT_ASSERT_FALSE(_aidl_return); \
    } while (0)

#define HDMICEC_GET_PROPERTY_SUCCESS(hdmicecHandle, property, propertyValue) \
    do { \
        ::android::binder::Status status; \
        ::com::rdk::hal::State state; \
        UT_LOG_DEBUG("Calling getProperty(IN:property[%s], OUT:propertyValue[]", hdmicec::toString(property).c_str()); \
        status = (hdmicecHandle)->getProperty(property, &propertyValue); \
        if(propertyValue.has_value()) \
        { \
            switch (propertyValue->value.value().getTag()) \
            { \
                case PropertyValue::Value::Tag::intValue: \
                    UT_LOG_DEBUG("Result getProperty(IN:property[%s], OUT:propertyValue[%d]), status:[isOK:%s, exception:%s]", hdmicec::toString(property).c_str(), \
                    propertyValue->value->get<PropertyValue::Value::Tag::intValue>(), \
                    UT_Control_GetMapString(bool_mapTable_cec, status.isOk()), \
                    UT_Control_GetMapString(statusHdmiCecException_mapTable, status.exceptionCode())); \
                    break; \
                case PropertyValue::Value::Tag::stringValue: \
                    UT_LOG_DEBUG("Result getProperty(IN:property[%s], OUT:propertyValue[%s]), status:[isOK:%s, exception:%s]", hdmicec::toString(property).c_str(), \
                    propertyValue->value->get<PropertyValue::Value::Tag::stringValue>().string(), \
                    UT_Control_GetMapString(bool_mapTable_cec, status.isOk()), \
                    UT_Control_GetMapString(statusHdmiCecException_mapTable, status.exceptionCode())); \
                    break; \
            } \
        } \
        else \
        { \
            UT_LOG_DEBUG("Result getProperty(IN:property[%s], OUT:propertyValue[]), status:[isOK:%s, exception:%s]", hdmicec::toString(property).c_str(), \
                    UT_Control_GetMapString(bool_mapTable_cec, status.isOk()), \
                    UT_Control_GetMapString(statusHdmiCecException_mapTable, status.exceptionCode())); \
            UT_ASSERT_TRUE(false); \
        } \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
    } while (0)

#define HDMICEC_REGISTER_EVENT_SUCCESS(hdmicecHandle, eventListener) \
    do { \
        ::android::binder::Status status; \
        bool _aidl_return; \
        UT_LOG_DEBUG("Calling registerEventListener(IN:eventListener[%p], OUT:_aidl_return[])", eventListener.get()); \
        status = (hdmicecHandle)->registerEventListener(eventListener, &_aidl_return); \
        UT_LOG_DEBUG("Result registerEventListener(IN:eventListener[%p], OUT:_aidl_return[%s]), status:[isOK:%s, exception:%s]", \
            eventListener.get(), UT_Control_GetMapString(bool_mapTable_cec, _aidl_return), \
            UT_Control_GetMapString(bool_mapTable_cec, status.isOk()), \
            UT_Control_GetMapString(statusHdmiCecException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
        UT_ASSERT_TRUE(_aidl_return); \
    } while (0)

#define HDMICEC_REGISTER_EVENT_FAILURE(hdmicecHandle, eventListener, exception) \
    do { \
        ::android::binder::Status status; \
        bool _aidl_return; \
        UT_LOG_DEBUG("Calling registerEventListener(IN:eventListener[%p], OUT:_aidl_return[])", eventListener.get()); \
        status = (hdmicecHandle)->registerEventListener(eventListener, &_aidl_return); \
        UT_LOG_DEBUG("Result registerEventListener(IN:eventListener[%p], OUT:_aidl_return[%s]), status:[isOK:%s, exception:%s]", \
            eventListener.get(), UT_Control_GetMapString(bool_mapTable_cec, _aidl_return), \
            UT_Control_GetMapString(bool_mapTable_cec, status.isOk()), \
            UT_Control_GetMapString(statusHdmiCecException_mapTable, status.exceptionCode())); \
        if (exception == Status::EX_NONE) \
            UT_ASSERT_TRUE(status.isOk()); \
        else \
            UT_ASSERT_FALSE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), exception); \
        UT_ASSERT_FALSE(_aidl_return); \
    } while (0)

#define HDMICEC_UNREGISTER_EVENT_SUCCESS(hdmicecHandle, eventListener) \
    do { \
        ::android::binder::Status status; \
        bool _aidl_return; \
        UT_LOG_DEBUG("Calling unregisterEventListener(IN:eventListener[%p], OUT:_aidl_return[])", eventListener.get()); \
        status = hdmicecHandle->unregisterEventListener(eventListener, &_aidl_return); \
        UT_LOG_DEBUG("Result unregisterEventListener(IN:eventListener[%p], OUT:_aidl_return[%s]), status:[isOK:%s, exception:%s]", \
            eventListener.get(), UT_Control_GetMapString(bool_mapTable_cec, _aidl_return), \
            UT_Control_GetMapString(bool_mapTable_cec, status.isOk()), \
            UT_Control_GetMapString(statusHdmiCecException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_TRUE(_aidl_return); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
    } while (0)

#define HDMICEC_UNREGISTER_EVENT_FAILURE(hdmicecHandle, eventListener, exception) \
    do { \
        ::android::binder::Status status; \
        bool _aidl_return; \
        UT_LOG_DEBUG("Calling unregisterEventListener(IN:eventListener[%p], OUT:_aidl_return[])", eventListener.get()); \
        status = hdmicecHandle->unregisterEventListener(eventListener, &_aidl_return); \
        UT_LOG_DEBUG("Result unregisterEventListener(IN:eventListener[%p], OUT:_aidl_return[%s]), status:[isOK:%s, exception:%s]", \
            eventListener.get(), UT_Control_GetMapString(bool_mapTable_cec, _aidl_return), \
            UT_Control_GetMapString(bool_mapTable_cec, status.isOk()), \
            UT_Control_GetMapString(statusHdmiCecException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), exception); \
        UT_ASSERT_FALSE(_aidl_return); \
    } while (0)

#define HDMICEC_GET_LOGICAL_ADDRESS_SUCCESS(hdmicecHandle, logicalAddresses) \
    do { \
        ::android::binder::Status status; \
        UT_LOG_DEBUG("Calling getLogicalAddresses(OUT:logicalAddresses[])"); \
        status = hdmicecHandle->getLogicalAddresses(&logicalAddresses); \
        UT_LOG_DEBUG("Result getLogicalAddresses(OUT:logicalAddresses[%s]), status:[isOK:%s, exception:%s]", \
            vecIntToString(logicalAddresses).c_str(), \
            UT_Control_GetMapString(bool_mapTable_cec, status.isOk()), \
            UT_Control_GetMapString(statusHdmiCecException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
    } while (0)

#define HDMICEC_ADD_LOGICAL_ADDRESS_SUCCESS(hdmicecControlHandle, logicalAddresses) \
    do { \
        ::android::binder::Status status; \
        bool _aidl_return; \
        UT_LOG_DEBUG("Calling addLogicalAddresses(IN:logicalAddresses[%s], OUT:_aidl_return[])", vecIntToString(logicalAddresses).c_str()); \
        status = hdmiCecController->addLogicalAddresses(logicalAddresses, &_aidl_return); \
        UT_LOG_DEBUG("Result addLogicalAddresses(IN:logicalAddresses[%s], OUT:_aidl_return[%s]), status:[isOK:%s, exception:%s]", \
            vecIntToString(logicalAddresses).c_str(), \
            UT_Control_GetMapString(bool_mapTable_cec, _aidl_return), \
            UT_Control_GetMapString(bool_mapTable_cec, status.isOk()), \
            UT_Control_GetMapString(statusHdmiCecException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
    } while (0);

#define HDMICEC_REMOVE_LOGICAL_ADDRESS_SUCCESS(hdmicecControlHandle, logicalAddresses) \
    do { \
        ::android::binder::Status status; \
        bool _aidl_return; \
        UT_LOG_DEBUG("Calling removeLogicalAddresses(IN:logicalAddresses[%s], OUT:_aidl_return[])", vecIntToString(logicalAddresses).c_str()); \
        status = hdmiCecController->removeLogicalAddresses(logicalAddresses, &_aidl_return); \
        UT_LOG_DEBUG("Result removeLogicalAddresses(OUT:logicalAddresses[%s], OUT:_aidl_return[%s]), status:[isOK:%s, exception:%s]", \
            vecIntToString(logicalAddresses).c_str(), \
            UT_Control_GetMapString(bool_mapTable_cec, _aidl_return), \
            UT_Control_GetMapString(bool_mapTable_cec, status.isOk()), \
            UT_Control_GetMapString(statusHdmiCecException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
    } while (0)

#define HDMICEC_SEND_MESSAGE_SUCCESS(hdmiCecController, message, sendStatus) \
    do { \
        ::android::binder::Status status; \
        UT_LOG_DEBUG("Calling sendMessage(IN:message[%s], OUT:sendStatus[])", vecByteToHexString(message).c_str()); \
        status = hdmiCecController->sendMessage(message, &sendStatus); \
        UT_LOG_DEBUG("Result sendMessage(IN:message[%s], OUT:sendStatus[%s]), status:[isOK:%s, exception:%s]", \
            vecByteToHexString(message).c_str(), \
            hdmicec::toString(sendStatus).c_str(), \
            UT_Control_GetMapString(bool_mapTable_cec, status.isOk()), \
            UT_Control_GetMapString(statusHdmiCecException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
    } while (0)

/* Contoller Listener Class */
class HdmiCecControllerListener : public BnHdmiCecControllerListener
{
public:
    HdmiCecControllerListener() : BnHdmiCecControllerListener(){}
    virtual ~HdmiCecControllerListener() = default;
    android::binder::Status onMessageReceived(const ::std::vector<uint8_t>& message) override;
    android::binder::Status onStateChanged(::com::rdk::hal::State oldState, ::com::rdk::hal::State newState) override;
    android::binder::Status onMessageSent(const ::std::vector<uint8_t>& message, ::com::rdk::hal::hdmicec::SendMessageStatus status) override;
private:
    mutex mQueueMutex;
};

/* Event Listener Class */
class HdmiCecEventListener : public BnHdmiCecEventListener
{
public:
    HdmiCecEventListener(){}
    virtual ~HdmiCecEventListener() = default;

    android::binder::Status onMessageReceived(const ::std::vector<uint8_t>& message) override;
    android::binder::Status onStateChanged(::com::rdk::hal::State oldState, ::com::rdk::hal::State newState) override;
};

#endif // __VTS_HDMICEC_H__

/** @} */ // End of HDMI_CEC_HALTEST_HEADER
/** @} */ // End of HDMI_CEC_HALTEST
/** @} */ // End of HDMI_CEC Module
/** @} */ // End of HPK