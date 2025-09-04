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
* vts_mock_HdmiCec.cpp
*
* Implementation of HdmiCec service
*/
#include <iostream>
#include <thread>
#include <chrono>
#include <ut.h>
#include <ut_log.h>

#include "vts_mock_HdmiCecInit.h"
#include "vts_mock_HdmiCec.h"
#include "vts_mock_HdmiCecController.h"

namespace com
{
    namespace rdk
    {
        namespace hal
        {
            namespace hdmicec
            {
                HdmiCec::HdmiCec()
                {

                }

                HdmiCec::~HdmiCec()
                {
                }

                android::binder::Status HdmiCec::getState(State* _aidl_return)
                {
                    if (!_aidl_return)
                    {
                        UT_LOG_ERROR("[HDMICEC_MOCK]getState: Invalid arguments");
                        return android::binder::Status::fromExceptionCode(android::binder::Status::EX_NULL_POINTER);
                    }
                    *_aidl_return = mCurrentState;
                    return android::binder::Status::ok();
                }
                android::binder::Status HdmiCec::getProperty(hdmicec::Property property, optional<PropertyValue>* _aidl_return)
                {
                    if (!_aidl_return)
                    {
                        UT_LOG_ERROR("[HDMICEC_MOCK]getProperty: Invalid arguments");
                        return android::binder::Status::fromExceptionCode(android::binder::Status::EX_NULL_POINTER);
                    }
                    com::rdk::hal::PropertyValue value;
                    switch (property)
                    {
                        case hdmicec::Property::HAL_CEC_VERSION:
                            value.value = com::rdk::hal::PropertyValue::Value::make<com::rdk::hal::PropertyValue::Value::intValue>(gHdmiCecConfig.version);
                            break;
                        case hdmicec::Property::METRIC_DIRECTED_MESSAGES_SENT:
                            value.value = com::rdk::hal::PropertyValue::Value::make<com::rdk::hal::PropertyValue::Value::intValue>(mDirectedMessagesSent);
                            break;
                        case hdmicec::Property::METRIC_BROADCAST_MESSAGES_SENT:
                            value.value = com::rdk::hal::PropertyValue::Value::make<com::rdk::hal::PropertyValue::Value::intValue>(mBroadcastMessagesSent);
                            break;
                        case hdmicec::Property::METRIC_DIRECTED_MESSAGES_SENT_AND_ACKED:
                            value.value = com::rdk::hal::PropertyValue::Value::make<com::rdk::hal::PropertyValue::Value::intValue>(mDirectedMessagesSentAck);
                            break;
                        case hdmicec::Property::METRIC_BROADCAST_MESSAGE_SENT_AND_ACKED:
                            value.value = com::rdk::hal::PropertyValue::Value::make<com::rdk::hal::PropertyValue::Value::intValue>(mBroadcastMessagesSentAck);
                            break;
                        case hdmicec::Property::METRIC_ARBITRATION_FAILURES:
                            value.value = com::rdk::hal::PropertyValue::Value::make<com::rdk::hal::PropertyValue::Value::intValue>(mArbitrationFailures);
                            break;
                        default:
                            *_aidl_return = std::nullopt;
                            return android::binder::Status::ok();
                    }
                    *_aidl_return = value;
                    return android::binder::Status::ok();
                }

                android::binder::Status HdmiCec::getLogicalAddresses(vector<int32_t>* _aidl_return)
                {
                    if (!_aidl_return)
                    {
                        UT_LOG_ERROR("[HDMICEC_MOCK]getLogicalAddresses: Invalid arguments");
                        return android::binder::Status::fromExceptionCode(android::binder::Status::EX_NULL_POINTER);
                    }

                    _aidl_return->clear();
                    for (const auto& addr : mLogicalAddresses)
                    {
                        if (addr != 0xF) // Exclude broadcast address
                            _aidl_return->push_back(addr);
                    }
                    return android::binder::Status::ok();
                }

                android::binder::Status HdmiCec::open(const android::sp<hdmicec::IHdmiCecControllerListener>& cecControllerListener,
                                                      android::sp<hdmicec::IHdmiCecController>* _aidl_return)
                {
                    if (!cecControllerListener || !_aidl_return)
                    {
                        UT_LOG_ERROR("[HDMICEC_MOCK]open: Invalid arguments");
                        return android::binder::Status::fromExceptionCode(android::binder::Status::EX_NULL_POINTER);
                    }
                    if(mCurrentState != State::CLOSED)
                    {
                        UT_LOG_ERROR("[HDMICEC_MOCK]open: Invalid state to open: %s", toString(mCurrentState));
                        return android::binder::Status::fromExceptionCode(android::binder::Status::EX_ILLEGAL_STATE);
                    }

                    mControllerListener = cecControllerListener;

                    android::sp<HdmiCecController> controller = new HdmiCecController(this);

                    mOwner = IInterface::asBinder(mControllerListener.get());
                    auto stat = mOwner->linkToDeath(this);
                    if (stat == android::INVALID_OPERATION)
                    {
                        UT_LOG_ERROR("[HDMICEC_MOCK]open: Failed to register death callback as interface is local");
                    }
                    else if (stat != android::OK)
                    {
                        UT_LOG_ERROR("[HDMICEC_MOCK]open: Failed to link to death: %d - %s", (int) stat, strerror(-stat));
                    }
                    else
                    {
                       UT_LOG_INFO("[HDMICEC_MOCK]open: Registered death callback");
                    }

                    changeState(State::STARTED);

                    *_aidl_return = controller;
                    return android::binder::Status::ok();
                }

                android::binder::Status HdmiCec::close(const android::sp<hdmicec::IHdmiCecController>& hdmiCecController, bool* _aidl_return)
                {
                    *_aidl_return = true;

                    if (!hdmiCecController || !_aidl_return)
                    {
                        UT_LOG_ERROR("[HDMICEC_MOCK]close: Invalid arguments");
                        *_aidl_return = false;
                        return android::binder::Status::fromExceptionCode(android::binder::Status::EX_NULL_POINTER);
                    }

                    if(mCurrentState != State::STARTED)
                    {
                        UT_LOG_ERROR("[HDMICEC_MOCK]close: Invalid state to close: %s", toString(mCurrentState).c_str());
                        *_aidl_return = false;
                        return android::binder::Status::fromExceptionCode(android::binder::Status::EX_ILLEGAL_STATE);
                    }

                    mDirectedMessagesSent = 0;
                    mBroadcastMessagesSent = 0;
                    mDirectedMessagesSentAck = 0;
                    mBroadcastMessagesSentAck = 0;
                    mArbitrationFailures = 0;

                    changeState(State::CLOSED);
                    mControllerListener = nullptr;
                    mLogicalAddresses.clear();

                    *_aidl_return = true;
                    return android::binder::Status::ok();
                }

                android::binder::Status HdmiCec::registerEventListener(const android::sp<hdmicec::IHdmiCecEventListener>& cecEventListener, bool* _aidl_return)
                {
                    if (!cecEventListener || !_aidl_return)
                    {
                        UT_LOG_ERROR("[HDMICEC_MOCK]registerEventListener: Invalid arguments");
                        *_aidl_return = false;
                        return android::binder::Status::fromExceptionCode(android::binder::Status::EX_NULL_POINTER);
                    }

                    auto asBinder = IInterface::asBinder(cecEventListener.get());
                    std::lock_guard lock(mMutex);
                    auto it = mEventlistenerList.find(asBinder);
                    auto stat = asBinder->linkToDeath(this);
                    if (stat == android::INVALID_OPERATION)
                    {
                        UT_LOG_ERROR("[HDMICEC_MOCK]registerEventListener: Failed to register death callback as interface is local");
                        *_aidl_return = false;
                    }
                    else if (stat != android::OK)
                    {
                        UT_LOG_ERROR("[HDMICEC_MOCK]registerEventListener: Failed to link to death: %d - %s", (int) stat, strerror(-stat));
                        *_aidl_return = false;
                    }

                    if (it == mEventlistenerList.end())
                    {
                        mEventlistenerList.emplace(::android::wp<IBinder>(asBinder), cecEventListener);
                        *_aidl_return = true;
                    }
                    else
                    {
                        UT_LOG_ERROR("[HDMICEC_MOCK]registerEventListener: IHdmiCecEventListener instance is already registered");
                        *_aidl_return = false;
                        return ::android::binder::Status::fromExceptionCode(::android::binder::Status::Exception::EX_ILLEGAL_STATE);
                    }

                    return android::binder::Status::ok();
                }

                android::binder::Status HdmiCec::unregisterEventListener(const android::sp<hdmicec::IHdmiCecEventListener>& cecEventListener, bool* _aidl_return)
                {
                    if (!cecEventListener || !_aidl_return)
                    {
                        UT_LOG_ERROR("[HDMICEC_MOCK]unregisterEventListener: Invalid arguments");
                        *_aidl_return = false;
                        return android::binder::Status::fromExceptionCode(android::binder::Status::EX_NULL_POINTER);
                    }

                    std::lock_guard lock(mMutex);
                    auto asBinder = IInterface::asBinder(cecEventListener.get());
                    auto it = mEventlistenerList.find(asBinder);
                    if (it != mEventlistenerList.end())
                    {
                        mEventlistenerList.erase(it);
                        *_aidl_return = true;
                    }
                    else
                    {
                        UT_LOG_ERROR("[HDMICEC_MOCK]unregisterEventListener: Failed to unregister as IHdmiCecEventListener instance has not registered for events");
                        *_aidl_return = false;
                        return ::android::binder::Status::fromExceptionCode(::android::binder::Status::Exception::EX_ILLEGAL_STATE);
                    }
                    asBinder->unlinkToDeath(this);

                    return android::binder::Status::ok();
                }

                void HdmiCec::changeState(com::rdk::hal::State newState)
                {
                    std::scoped_lock lock(mSafeLock, mMutex);
                    UT_LOG_INFO("[HDMICEC_MOCK]State Changed from %s to %s", toString(mCurrentState).c_str(), toString(newState).c_str());

                    const State oldState = mCurrentState;
                    mCurrentState = newState;

                    for (const auto& [key, listener] : mEventlistenerList)
                    {
                        listener->onStateChanged(oldState, mCurrentState);
                    }

                    if (mControllerListener)
                    {
                        mControllerListener->onStateChanged(oldState, mCurrentState);
                    }

                    if (newState == State::CLOSED && mOwner)
                    {
                        mOwner->unlinkToDeath(this);
                        mOwner.clear();
                    }
                }

                void HdmiCec::binderDied(const ::android::wp<IBinder>& binder)
                {
                    std::lock_guard lock(mMutex);
                    mEventlistenerList.erase(binder);
                    if (mOwner == binder)
                    {
                        UT_LOG_ERROR("[HDMICEC_MOCK]Client who opened the sink died.. Resetting the state");
                        mControllerListener = nullptr;
                        mCurrentState = State::CLOSED;
                    }
                }

                State HdmiCec::getCurrentState()
                {
                    std::lock_guard lock(mMutex);
                    return mCurrentState;
                }

                bool HdmiCec::insertLogicalAddress(const vector<int32_t>& logicalAddresses)
                {
                    std::lock_guard lock(mMutex);
                    for (const auto& address : logicalAddresses)
                    {
                        if (address < 0 || address > 0xE) {
                            UT_LOG_ERROR("[HDMICEC_MOCK]addLogicalAddresses: Address %d out of range", address);
                            return false;
                        }
                        if (std::find(mLogicalAddresses.begin(), mLogicalAddresses.end(), address) != mLogicalAddresses.end())
                        {
                            UT_LOG_ERROR("[HDMICEC_MOCK]addLogicalAddresses: Address %d already present", address);
                            return false;
                        }
                        if (std::find(gHdmiCecConfig.connectedDevices.begin(), gHdmiCecConfig.connectedDevices.end(), address) != gHdmiCecConfig.connectedDevices.end())
                        {
                            UT_LOG_ERROR("[HDMICEC_MOCK]addLogicalAddresses: Address %d already Assigned to other device", address);
                            return false;
                        }
                    }
                    // All checks passed, add all addresses
                    mLogicalAddresses.insert(mLogicalAddresses.end(), logicalAddresses.begin(), logicalAddresses.end());
                    return true;
                }

                bool HdmiCec::removeLogicalAddress(const vector<int32_t>& logicalAddresses)
                {
                    bool status = true;
                    std::lock_guard lock(mMutex);
                    for (const auto& address : logicalAddresses)
                    {
                        if (address < 0 || address > 0xE)
                        {
                            UT_LOG_ERROR("[HDMICEC_MOCK]removeLogicalAddresses: Address %d out of range", address);
                            return false;
                        }
                        if (std::find(mLogicalAddresses.begin(), mLogicalAddresses.end(), address) == mLogicalAddresses.end())
                        {
                            UT_LOG_ERROR("[HDMICEC_MOCK]removeLogicalAddresses: Address %d not present", address);
                            return false;
                        }
                    }
                    // All checks passed, remove all addresses
                    for (const auto& address : logicalAddresses)
                    {
                        auto it = std::find(mLogicalAddresses.begin(), mLogicalAddresses.end(), address);
                        if (it != mLogicalAddresses.end())
                            mLogicalAddresses.erase(it);
                    }
                    return status;
                }

                bool HdmiCec::findLogicalAddress(int32_t logicalAddress)
                {
                    std::lock_guard lock(mMutex);
                    return (std::find(mLogicalAddresses.begin(), mLogicalAddresses.end(), logicalAddress) != mLogicalAddresses.end());
                }
            }   //namespace hdmicec
        }   //namespace hal
    }   //namespace rdk
}   //namespace com
