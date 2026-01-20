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
 * @defgroup HDMI_CEC_HALTEST_CLASS HDMI CEC HAL Tests helper functions Header File
 * @{
 * @parblock
 *
 * ### Common functions/macros required for the HDMI CEC module :
 *
 * Common functions/macros required for the HDMI CEC module .
 *
 * **Pre-Conditions:**  None @n
 * **Dependencies:** None @n
 *
 * @endparblock
 *
 */

/**
 * @file vts_hdmicec.cpp
 *
 */

#include "vts_hdmicec.h"
#include "vts_hdmicec_helper_functions.h"

/* Implementation of HdmiCecEventListener methods */
android::binder::Status HdmiCecEventListener::onMessageReceived(const ::std::vector<uint8_t>& message)
{
    {
        std::lock_guard<std::mutex> lock(mMsgReceiveMutex);
        mMessageReceiveQueue.emplace(message);
    }
    UT_LOG_INFO("HdmiCecEventListener::onMessageReceived called with message[%s]",vecByteToHexString(message).c_str());
    return android::binder::Status::ok();
}

android::binder::Status HdmiCecEventListener::onStateChanged(::com::rdk::hal::hdmicec::State oldState, ::com::rdk::hal::hdmicec::State newState)
{
    UT_LOG_INFO("HdmiCecEventListener::onStateChanged called with oldState:%s, newState:%s",
                com::rdk::hal::hdmicec::toString(oldState).c_str(), com::rdk::hal::hdmicec::toString(newState).c_str());
    {
        lock_guard<mutex> lock(mStateMutex);
        mStateChanges.push({oldState, newState});
    }
    return android::binder::Status::ok();
}

android::binder::Status HdmiCecEventListener::onMessageSent(const ::std::vector<uint8_t> &message, ::com::rdk::hal::hdmicec::SendMessageStatus status)
{
    {
        std::lock_guard<std::mutex> lock(mMsgSentMutex);
        mMessageSentQueue.emplace(message, status);
    }

    UT_LOG_INFO("HdmiCecEventListener::onMessageSent called with message: [%s] status[%s]",vecByteToHexString(message).c_str(), hdmicec::toString(status).c_str());
    return android::binder::Status::ok();
}

bool HdmiCecEventListener::getNextStateChanges(pair<State, State>& stateChanges)
{
    lock_guard<mutex> lock(mStateMutex);

    if (mStateChanges.empty())
        return false;

    stateChanges = mStateChanges.front();
    mStateChanges.pop();
    return true;
}

bool HdmiCecEventListener::getNextMessage(std::vector<uint8_t>& outMessage,
                                               hdmicec::SendMessageStatus& outStatus)
{
    std::lock_guard<std::mutex> lock(mMsgSentMutex);

    if (mMessageSentQueue.empty())
    {
        return false;
    }

    auto entry = std::move(mMessageSentQueue.front());
    mMessageSentQueue.pop();

    outMessage = std::move(entry.first);
    outStatus  = entry.second;

    return true;
}

bool HdmiCecEventListener::getReceiveMessageStatus()
{
    std::lock_guard<std::mutex> lock(mMsgReceiveMutex);

    return !mMessageReceiveQueue.empty();
}

bool HdmiCecEventListener::getNextReceiveMessage(std::vector<uint8_t>& outMessage)
{
    std::lock_guard<std::mutex> lock(mMsgReceiveMutex);

    if (mMessageReceiveQueue.empty())
    {
        return false;
    }

    auto entry = std::move(mMessageReceiveQueue.front());
    mMessageReceiveQueue.pop();

    outMessage = std::move(entry);

    return true;
}

void HdmiCecEventListener::resetStateQueue()
{
    std::lock_guard<std::mutex> lock(mStateMutex);
    std::queue<pair<State, State>> empty;
    std::swap(mStateChanges, empty);
}

void HdmiCecEventListener::resetSendMessageQueue()
{
    std::lock_guard<std::mutex> lock(mMsgSentMutex);
    std::queue<std::pair<std::vector<uint8_t>, hdmicec::SendMessageStatus>> empty;
    std::swap(mMessageSentQueue, empty);
}

void HdmiCecEventListener::resetReceiveMessageQueue()
{
    std::lock_guard<std::mutex> lock(mMsgReceiveMutex);
    std::queue<std::vector<uint8_t>> empty;
    std::swap(mMessageReceiveQueue, empty);
}

/** @} */ // End of HDMI_CEC_HALTEST_HEADER
/** @} */ // End of HDMI_CEC_HALTEST
/** @} */ // End of HDMI_CEC
/** @} */ // End of HPK