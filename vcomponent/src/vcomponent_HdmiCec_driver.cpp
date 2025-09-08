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
* vcomponent_HdmiCec_driver.cpp
*
* Implementation of vcomponent HdmiCec driver
*/

#include "vcomponent_HdmiCec_parse_config.h"
#include "vcomponent_HdmiCec_driver.h"

static std::function<void(const std::vector<uint8_t>&, hdmicec::SendMessageStatus*)> gSendMessageCallback = nullptr;
static std::function<void(const std::vector<uint8_t>&)> gReceiveMessageCallback = nullptr;
static std::queue<MessageTask> gMessageQueue;
static std::mutex gQueueMutex;
static std::atomic<bool> gThreadRunning{false};
static std::thread gWorkerThread;

static void vcomponent_HdmiCec_driver_worker()
{
    while (gThreadRunning)
    {
        MessageTask task;
        {
            std::lock_guard<std::mutex> lock(gQueueMutex);
            if (gMessageQueue.empty())
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                continue;
            }
            task = gMessageQueue.front();
            gMessageQueue.pop();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(HDMIC_CEC_TRANSMISSION_TIME));
        cecResponse_t cmd;
        uint8_t opcode = task.message[1];
        uint8_t header = task.message[0];
        uint8_t dest = (header & 0xF0) >> 4;
        uint8_t src = header & 0x0F;

        if (gSendMessageCallback)
        {
            if (dest == 0xF)
            {
                gSendMessageCallback(task.message, &task.status);
            }
            else // Direct message
            {
                bool isPartialFaulty = std::find(gCecConfiguration.par_fault_devices.begin(), gCecConfiguration.par_fault_devices.end(), dest) != gCecConfiguration.par_fault_devices.end();

                if(task.status ==  hdmicec::SendMessageStatus::ACK_STATE_0)
                {
                    gSendMessageCallback(task.message, &task.status);
                }
                else
                {
                    this_thread::sleep_for(chrono::milliseconds(HDMIC_CEC_TRANSMISSION_TIME));
                    if (isPartialFaulty)
                    {
                        task.status = hdmicec::SendMessageStatus::ACK_STATE_0;
                        gSendMessageCallback(task.message, &task.status);
                    }
                    else
                    {
                        task.status = hdmicec::SendMessageStatus::BUSY;
                        gSendMessageCallback(task.message, &task.status);
                    }
                }
            }
        }

        vcomponent_HdmiCec_get_cec_command(opcode, cmd);

        // vcomponent sends back response listed in configuration file
        if(cmd.response.opcode != 0xFF)
        {
            vcomponent_HdmiCec_driver_receiveMessage(cmd.response, src, dest);
        }
    }
}

void vcomponent_HdmiCec_driver_shutdown()
{
    gThreadRunning = false;
    if (gWorkerThread.joinable())
    {
        gWorkerThread.join();
    }
}

void vcomponent_HdmiCec_driver_init(
    std::function<void(const std::vector<uint8_t>&, hdmicec::SendMessageStatus*)> sendMessageCallback,
    std::function<void(const std::vector<uint8_t>&)> receiveMessageCallback)
{
    gSendMessageCallback = sendMessageCallback;
    gReceiveMessageCallback = receiveMessageCallback;
    gThreadRunning = true;
    gWorkerThread = std::thread(vcomponent_HdmiCec_driver_worker);
}

void vcomponent_HdmiCec_driver_sendMessage(const std::vector<uint8_t>& message, hdmicec::SendMessageStatus* status)
{
    cecResponse_t cmd;
    vector<uint8_t> par_fault_devices;
    vector<uint8_t> fault_devices;
    // Parse opcode
    uint8_t opcode = message[1];
    uint8_t header = message[0];
    uint8_t dest = (header & 0xF0) >> 4;
    uint8_t src = header & 0x0F;

    vcomponent_HdmiCec_get_cec_command(opcode, cmd);

    // Default to BUSY if command not supported
    if (cmd.command.opcode == 0xFF)
    {
        UT_LOG_ERROR("[VCOMPONENT_HDMICEC]sendMessageDriver: Unsupported opcode 0x%X", opcode);
        *status = hdmicec::SendMessageStatus::BUSY;
        return;
    }

    // Broadcast message
    if (dest == 0xF)
    {
        // If any faulty device is present, treat as rejected (ACK_STATE_0)
        if (gCecConfiguration.par_fault_devices.size() > 0 ||
            gCecConfiguration.fault_devices.size() > 0)
            *status = hdmicec::SendMessageStatus::ACK_STATE_0;
        else
            *status = hdmicec::SendMessageStatus::ACK_STATE_1;
    }
    else // Direct message
    {
        // If dest is not connected, or is faulty, treat as not acknowledged (ACK_STATE_1)
        if (gCecConfiguration.par_fault_devices.size() > 0 ||
            gCecConfiguration.fault_devices.size() > 0)
        {
            UT_LOG_ERROR("[VCOMPONENT_HDMICEC]sendMessageDriver: Destination 0x%X not responding", dest);
            *status = hdmicec::SendMessageStatus::ACK_STATE_1;
        }
        else
        {
            *status = hdmicec::SendMessageStatus::ACK_STATE_0;
        }
    }
    {
        std::lock_guard<std::mutex> lock(gQueueMutex);
        gMessageQueue.push({message, *status});
    }
}

void vcomponent_HdmiCec_driver_receiveMessage(cecOpcode_t  &response, uint8_t actual_src, uint8_t actual_dst)
{
    vector<uint8_t> message;
    uint8_t dst = actual_src;
    uint8_t src = actual_dst;
    bool status;

    if(src != 0xF)
    {
        if(std::find(gCecConfiguration.fault_devices.begin(), gCecConfiguration.fault_devices.end(), src) != gCecConfiguration.fault_devices.end())
            return;
    }
    else
    {
        if(gCecConfiguration.working_devices.size() == 0)
        {
            return;
        }
        src = gCecConfiguration.working_devices[0];
    }

    if(response.type == CEC_TYPE_BROADCAST)
    {
        dst = 0xF;
    }

    status = vcomponent_HdmiCec_get_cec_response(src, dst, response, message);

    if(status == false)
    {
        UT_LOG_ERROR("[VCOMPONENT_HDMICEC]Failed to send response from %d with opcode: 0x%X", src, response.opcode);
        return;
    }
    // Call the receiveMessage callback if set
    if (gReceiveMessageCallback)
    {
        gReceiveMessageCallback(message);
    }
}