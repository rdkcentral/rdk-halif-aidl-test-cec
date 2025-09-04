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
* vts_mock_HdmiCec_driver.h
*
* Header file for the HdmiCec driver implementation
*/

#ifndef __HDMICEC_DRIVER_H__
#define __HDMICEC_DRIVER_H__

#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>

#include <ut.h>
#include <ut_log.h>

#include "vts_mock_HdmiCecInit.h"
#include "vts_mock_HdmiCec.h"
#include "vts_mock_HdmiCecController.h"

using namespace com::rdk::hal;
using namespace std;

#define HDMIC_CEC_TRANSMISSION_TIME 5

struct MessageTask
{
    std::vector<uint8_t> message;
    hdmicec::SendMessageStatus status;
};

void hdmicec_mock_driver_init(
    std::function<void(const std::vector<uint8_t>&, hdmicec::SendMessageStatus*)> sendMessageCallback,
    std::function<void(const std::vector<uint8_t>&)> receiveMessageCallback);
void hdmicec_mock_driver_sendMessage(const vector<uint8_t>& message, hdmicec::SendMessageStatus* status);
void hdmicec_mock_driver_receiveMessage(cecCommands_t &response, uint8_t dst, uint8_t src);
void hdmicec_mock_driver_shutdown();

#endif //__HDMICEC_DRIVER_H__