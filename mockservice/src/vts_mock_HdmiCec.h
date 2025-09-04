/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
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
* vts_mock_HdmiCec.h
*
* Header file for the HdmiCec service implementation
*/

#ifndef __HDMICEC_H__
#define __HDMICEC_H__

#include <com/rdk/hal/hdmicec/BnHdmiCec.h>
#include <com/rdk/hal/hdmicec/IHdmiCec.h>
#include <com/rdk/hal/hdmicec/BnHdmiCecController.h>
#include <binder/BinderService.h>
#include <optional>
#include <string>
#include <utils/StrongPointer.h>
#include <vector>
#include <mutex>

using namespace std;
namespace com
{
    namespace rdk
    {
        namespace hal
        {
            namespace hdmicec
            {

                class HdmiCec :public android::BinderService<HdmiCec>, public android::IBinder::DeathRecipient, public BnHdmiCec
                {
                public:
                    static char const* getServiceName() { return IHdmiCec::serviceName().c_str(); }

                    HdmiCec();
                    virtual ~HdmiCec();

                    android::binder::Status getState(State* _aidl_return) override;
                    android::binder::Status getProperty(hdmicec::Property property, optional<PropertyValue>* _aidl_return) override;
                    android::binder::Status getLogicalAddresses(vector<int32_t>* _aidl_return) override;
                    android::binder::Status open(const android::sp<hdmicec::IHdmiCecControllerListener>& cecControllerListener, android::sp<hdmicec::IHdmiCecController>* _aidl_return) override;
                    android::binder::Status close(const android::sp<hdmicec::IHdmiCecController>& hdmiCecController, bool* _aidl_return) override;
                    android::binder::Status registerEventListener(const android::sp<hdmicec::IHdmiCecEventListener>& cecEventListener, bool* _aidl_return) override;
                    android::binder::Status unregisterEventListener(const android::sp<hdmicec::IHdmiCecEventListener>& cecEventListener, bool* _aidl_return) override;
                public:
                    bool insertLogicalAddress(const vector<int32_t>& logicalAddresses);
                    bool removeLogicalAddress(const vector<int32_t>& logicalAddresses);
                    State getCurrentState();
                    bool findLogicalAddress(int32_t logicalAddress);
                private:
                    State mCurrentState = State::CLOSED;
                    std::mutex mMutex;
                    std::recursive_mutex mSafeLock;

                    vector<int32_t> mLogicalAddresses;
                    android::sp<IBinder> mOwner;

                    void binderDied(const ::android::wp<IBinder>& binder);
                    void changeState(com::rdk::hal::State newState);
                public:
                    android::sp<IHdmiCecControllerListener> mControllerListener;
                    std::map<::android::wp<IBinder>, ::android::sp<IHdmiCecEventListener>> mEventlistenerList;
                    uint64_t mDirectedMessagesSent = 0;
                    uint64_t mBroadcastMessagesSent = 0;
                    uint64_t mDirectedMessagesSentAck = 0;
                    uint64_t mBroadcastMessagesSentAck = 0;
                    uint64_t mArbitrationFailures = 0;
                };
            }   //namespace hdmicec
        }   //namespace hal
    }   //namespace rdk
}   //namespace com

#endif //__HDMICEC_H__
