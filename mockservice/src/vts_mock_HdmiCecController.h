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

#ifndef __HDMICEC_CONTROLLER_H__
#define __HDMICEC_CONTROLLER_H__

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
                class HdmiCecController :public android::BinderService<HdmiCecController>,
                                              public BnHdmiCecController
                {
                private:
                    HdmiCec *mParentHdmiCec;
                public:
                    HdmiCecController(HdmiCec *parentHdmiCec);
                    virtual ~HdmiCecController();

                    android::binder::Status addLogicalAddresses(const vector<int32_t>& logicalAddresses, bool* _aidl_return) override;
                    android::binder::Status removeLogicalAddresses(const vector<int32_t>& logicalAddresses, bool* _aidl_return);
                    android::binder::Status sendMessage(const vector<uint8_t>& message, hdmicec::SendMessageStatus* _aidl_return) override;
                public:
                    void sendMessageCallcback(const vector<uint8_t>& message, hdmicec::SendMessageStatus* status);
                    void receiveMessageCallcback(const vector<uint8_t>& message);
                };
            }   //namespace hdmicec
        }   //namespace hal
    }   //namespace rdk
}   //namespace com

#endif //__HDMICEC_CONTROLLER_H__
