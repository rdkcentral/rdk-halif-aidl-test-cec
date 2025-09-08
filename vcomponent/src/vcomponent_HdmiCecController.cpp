/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2024 RDK Management
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

/** @brief
* vcomponent_HdmiCec.cpp
*
* Implementation of HdmiCec service
*/
#include <iostream>
#include <thread>
#include <chrono>
#include <ut.h>
#include <ut_log.h>

#include "vcomponent_HdmiCec_init.h"
#include "vcomponent_HdmiCec.h"
#include "vcomponent_HdmiCecController.h"
#include "vcomponent_HdmiCec_parse_config.h"
#include "vcomponent_HdmiCec_driver.h"

namespace com
{
    namespace rdk
    {
        namespace hal
        {
            namespace hdmicec
            {
                HdmiCecController::HdmiCecController(HdmiCec *parentHdmiCec)
                {
                    mParentHdmiCec = parentHdmiCec;
                    vcomponent_HdmiCec_driver_init(
                            [this](const std::vector<uint8_t>& msg, hdmicec::SendMessageStatus* status) {
                                this->sendMessageCallcback(msg, status);
                            },
                            [this](const std::vector<uint8_t>& msg) {
                                this->receiveMessageCallcback(msg);
                            }
                        );
                }

                HdmiCecController::~HdmiCecController()
                {
                    vcomponent_HdmiCec_driver_shutdown();
                }

                android::binder::Status HdmiCecController::addLogicalAddresses(const vector<int32_t>& logicalAddresses, bool* _aidl_return)
                {
                    if (!_aidl_return)
                    {
                        UT_LOG_ERROR("[VCOMPONENT_HDMICEC]addLogicalAddresses: Invalid arguments");
                        *_aidl_return = false;
                        return android::binder::Status::fromExceptionCode(android::binder::Status::EX_NULL_POINTER);
                    }

                    // Check state
                    if (mParentHdmiCec->getCurrentState() != State::STARTED)
                    {
                        UT_LOG_ERROR("[VCOMPONENT_HDMICEC]addLogicalAddresses: Invalid state, must be STARTED");
                        *_aidl_return = false;
                        return android::binder::Status::fromExceptionCode(android::binder::Status::EX_ILLEGAL_STATE);
                    }

                    *_aidl_return = mParentHdmiCec->insertLogicalAddress(logicalAddresses);

                    return android::binder::Status::ok();
                }

                android::binder::Status HdmiCecController::removeLogicalAddresses(const vector<int32_t>& logicalAddresses, bool* _aidl_return)
                {
                    if (!_aidl_return)
                    {
                        UT_LOG_ERROR("[VCOMPONENT_HDMICEC]removeLogicalAddresses: Invalid arguments");
                        *_aidl_return = false;
                        return android::binder::Status::fromExceptionCode(android::binder::Status::EX_NULL_POINTER);
                    }

                    if (mParentHdmiCec->getCurrentState() != State::STARTED)
                    {
                        UT_LOG_ERROR("[VCOMPONENT_HDMICEC]addLogicalAddresses: Invalid state, must be STARTED");
                        *_aidl_return = false;
                        return android::binder::Status::fromExceptionCode(android::binder::Status::EX_ILLEGAL_STATE);
                    }

                    *_aidl_return = mParentHdmiCec->removeLogicalAddress(logicalAddresses);

                    return android::binder::Status::ok();
                }

                android::binder::Status HdmiCecController::sendMessage(const vector<uint8_t>& message, hdmicec::SendMessageStatus* _aidl_return)
                {
                    if (!_aidl_return)
                    {
                        UT_LOG_ERROR("[VCOMPONENT_HDMICEC]sendMessage: Invalid arguments");
                        return android::binder::Status::fromExceptionCode(android::binder::Status::EX_NULL_POINTER);
                    }

                    // Check state
                    if (mParentHdmiCec->getCurrentState() != State::STARTED)
                    {
                        UT_LOG_ERROR("[VCOMPONENT_HDMICEC]sendMessage: Invalid state, must be STARTED");
                        return android::binder::Status::fromExceptionCode(android::binder::Status::EX_ILLEGAL_STATE);
                    }

                    // Validate message size
                    if (message.size() < 2 || message.size() > 16)
                    {
                        UT_LOG_ERROR("[VCOMPONENT_HDMICEC]sendMessage: Invalid message size: %zu", message.size());
                        return android::binder::Status::fromExceptionCode(android::binder::Status::EX_ILLEGAL_ARGUMENT);
                    }

                    // Validate logical address (destination): TODO: Check it this check required or not
                    uint8_t header = message[0];
                    uint8_t dest = (header & 0xF0) >> 4;
                    uint8_t src = header & 0x0F;
                    bool valid = true;

                    if (dest > 0xF)
                    {
                        valid = false;
                    }
                    if(!mParentHdmiCec->findLogicalAddress(src))
                    {
                        valid = false;
                    }

                    if (!valid)
                    {
                        UT_LOG_ERROR("[VCOMPONENT_HDMICEC]sendMessage: Source logical address 0x%X not registered", src);
                        return android::binder::Status::fromExceptionCode(android::binder::Status::EX_ILLEGAL_ARGUMENT);
                    }

                    vcomponent_HdmiCec_driver_sendMessage(message, _aidl_return);

                    return android::binder::Status::ok();
                }

                void HdmiCecController::sendMessageCallcback(const vector<uint8_t>& message, hdmicec::SendMessageStatus* status)
                {
                    uint8_t header = message[0];
                    uint8_t dest = (header & 0xF0) >> 4;
                    uint8_t src = header & 0x0F;

                    if(dest == 0x0F)
                    {
                        mParentHdmiCec->mBroadcastMessagesSent++;
                        if(*status ==  hdmicec::SendMessageStatus::ACK_STATE_0)
                        {
                            mParentHdmiCec->mDirectedMessagesSentAck++;
                        }
                    }
                    else
                    {
                        if(*status !=  hdmicec::SendMessageStatus::BUSY)
                        {
                            mParentHdmiCec->mDirectedMessagesSent++;
                            mParentHdmiCec->mDirectedMessagesSentAck++;
                        }
                        else
                        {
                            mParentHdmiCec->mArbitrationFailures++;
                        }
                    }
                    if (mParentHdmiCec->mControllerListener)
                    {
                        mParentHdmiCec->mControllerListener->onMessageSent(message, *status);
                    }
                }

                void HdmiCecController::receiveMessageCallcback(const vector<uint8_t>& message)
                {
                    if (mParentHdmiCec->mControllerListener)
                    {
                        mParentHdmiCec->mControllerListener->onMessageReceived(message);
                    }
                    for (const auto& [key, listener] : mParentHdmiCec->mEventlistenerList)
                    {
                        if (listener)
                        {
                            listener->onMessageReceived(message);
                        }
                    }
                }
            }   //namespace hdmicec
        }   //namespace hal
    }   //namespace rdk
}   //namespace com
