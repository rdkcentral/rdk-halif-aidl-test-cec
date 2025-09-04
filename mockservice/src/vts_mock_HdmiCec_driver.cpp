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
* vts_mock_HdmiCec_driver.cpp
*
* Implementation of mock HdmiCec driver
*/

#include "vts_mock_HdmiCecInit.h"
#include "vts_mock_HdmiCec_driver.h"

static std::function<void(const std::vector<uint8_t>&, hdmicec::SendMessageStatus*)> gSendMessageCallback = nullptr;
static std::function<void(const std::vector<uint8_t>&)> gReceiveMessageCallback = nullptr;
static std::queue<MessageTask> gMessageQueue;
static std::mutex gQueueMutex;
static std::atomic<bool> gThreadRunning{false};
static std::thread gWorkerThread;

static void hdmicec_mock_driver_worker()
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
        cecConfigCommands_t cmd;
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
                bool isPartialFaulty = std::find(gHdmiCecConfig.partialFaultyDevices.begin(), gHdmiCecConfig.partialFaultyDevices.end(), dest) != gHdmiCecConfig.partialFaultyDevices.end();

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

        vts_get_cec_command(opcode, cmd);
        // Mock sends back response listed in configuration file
        if(cmd.response.cecCommand != 0xFF)
        {
            this_thread::sleep_for(chrono::milliseconds(HDMIC_CEC_TRANSMISSION_TIME));

            // Mock implementaion: Find a logical address that is connected and not faulty or partial faulty
            uint8_t validLogicalAddress = 0xFF;

            for (auto addr : gHdmiCecConfig.connectedDevices)
            {
                if (std::find(gHdmiCecConfig.faultyDevices.begin(), gHdmiCecConfig.faultyDevices.end(), addr) == gHdmiCecConfig.faultyDevices.end() &&
                    std::find(gHdmiCecConfig.partialFaultyDevices.begin(), gHdmiCecConfig.partialFaultyDevices.end(), addr) == gHdmiCecConfig.partialFaultyDevices.end())
                {
                    validLogicalAddress = addr;
                    break;
                }
            }

            if (validLogicalAddress != 0xFF)
            {
                uint8_t response_dst = src;
                if(cmd.response.type != CEC_TYPE_DIRECT)
                {
                    response_dst = 0xF;
                }
                hdmicec_mock_driver_receiveMessage(cmd.response, validLogicalAddress, response_dst);
            }
        }
    }
}

void hdmicec_mock_driver_shutdown()
{
    gThreadRunning = false;
    if (gWorkerThread.joinable())
    {
        gWorkerThread.join();
    }
}

void hdmicec_mock_driver_init(
    std::function<void(const std::vector<uint8_t>&, hdmicec::SendMessageStatus*)> sendMessageCallback,
    std::function<void(const std::vector<uint8_t>&)> receiveMessageCallback)
{
    gSendMessageCallback = sendMessageCallback;
    gReceiveMessageCallback = receiveMessageCallback;
    gThreadRunning = true;
    gWorkerThread = std::thread(hdmicec_mock_driver_worker);
}

void hdmicec_mock_driver_sendMessage(const std::vector<uint8_t>& message, hdmicec::SendMessageStatus* status)
{
    cecConfigCommands_t cmd;
    // Parse opcode
    uint8_t opcode = message[1];
    uint8_t header = message[0];
    uint8_t dest = (header & 0xF0) >> 4;
    uint8_t src = header & 0x0F;

    vts_get_cec_command(opcode, cmd);

    // Default to BUSY if command not supported
    if (cmd.command.cecCommand == 0xFF)
    {
        UT_LOG_ERROR("[HDMICEC_MOCK]sendMessageDriver: Unsupported opcode 0x%X", opcode);
        *status = hdmicec::SendMessageStatus::BUSY;
        return;
    }
    // Broadcast message
    if (dest == 0xF)
    {
        // If any faulty device is present, treat as rejected (ACK_STATE_0)
        bool anyRejected = false;
        bool isPartialFaulty = gHdmiCecConfig.partialFaultyDevices.size() > 0;

        if (isPartialFaulty)
            *status = hdmicec::SendMessageStatus::ACK_STATE_0;
        else
            *status = hdmicec::SendMessageStatus::ACK_STATE_1;
    }
    else // Direct message
    {
        // If dest is not connected, or is faulty, treat as not acknowledged (ACK_STATE_1)
        bool isConnected = std::find(gHdmiCecConfig.connectedDevices.begin(), gHdmiCecConfig.connectedDevices.end(), dest) != gHdmiCecConfig.connectedDevices.end();
        bool isFaulty = std::find(gHdmiCecConfig.faultyDevices.begin(), gHdmiCecConfig.faultyDevices.end(), dest) != gHdmiCecConfig.faultyDevices.end();
        bool isPartialFaulty = std::find(gHdmiCecConfig.partialFaultyDevices.begin(), gHdmiCecConfig.partialFaultyDevices.end(), dest) != gHdmiCecConfig.partialFaultyDevices.end();

        if (!isConnected || isFaulty || isPartialFaulty)
        {
            UT_LOG_ERROR("[HDMICEC_MOCK]sendMessageDriver: Destination 0x%X not responding", dest);
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
/*void hdmicec_mock_driver_sendMessage(const vector<uint8_t>& message, hdmicec::SendMessageStatus* status)
{
    cecConfigCommands_t cmd;
    // Parse opcode
    uint8_t opcode = message[1];
    uint8_t header = message[0];
    uint8_t dest = (header & 0xF0) >> 4;
    uint8_t src = header & 0x0F;

    vts_get_cec_command(opcode, cmd);

    // Default to BUSY if command not supported
    if (cmd.command.cecCommand == 0xFF)
    {
        UT_LOG_ERROR("[HDMICEC_MOCK]sendMessageDriver: Unsupported opcode 0x%X", opcode);
        *status = hdmicec::SendMessageStatus::BUSY;
        return;
    }

    // Broadcast message
    if (dest == 0xF)
    {
        // If any faulty device is present, treat as rejected (ACK_STATE_0)
        bool anyRejected = false;
        bool isPartialFaulty = gHdmiCecConfig.partialFaultyDevices.size() > 0;

        if (isPartialFaulty)
            *status = hdmicec::SendMessageStatus::ACK_STATE_0;
        else
            *status = hdmicec::SendMessageStatus::ACK_STATE_1;

        gSendMessageCallback(message, status);
    }
    else // Direct message
    {
        // If dest is not connected, or is faulty, treat as not acknowledged (ACK_STATE_1)
        bool isConnected = std::find(gHdmiCecConfig.connectedDevices.begin(), gHdmiCecConfig.connectedDevices.end(), dest) != gHdmiCecConfig.connectedDevices.end();
        bool isFaulty = std::find(gHdmiCecConfig.faultyDevices.begin(), gHdmiCecConfig.faultyDevices.end(), dest) != gHdmiCecConfig.faultyDevices.end();
        bool isPartialFaulty = std::find(gHdmiCecConfig.partialFaultyDevices.begin(), gHdmiCecConfig.partialFaultyDevices.end(), dest) != gHdmiCecConfig.partialFaultyDevices.end();

        if (!isConnected || isFaulty || isPartialFaulty)
        {
            UT_LOG_ERROR("[HDMICEC_MOCK]sendMessageDriver: Destination 0x%X not responding", dest);
            *status = hdmicec::SendMessageStatus::ACK_STATE_1;
        }
        else
        {
            *status = hdmicec::SendMessageStatus::ACK_STATE_0;
        }

        this_thread::sleep_for(chrono::milliseconds(HDMIC_CEC_TRANSMISSION_TIME));

        // Call the sendMessage callback if set
        if (gSendMessageCallback)
        {
            if(*status ==  hdmicec::SendMessageStatus::ACK_STATE_0)
            {
                gSendMessageCallback(message, status);
            }
            else
            {
                this_thread::sleep_for(chrono::milliseconds(HDMIC_CEC_TRANSMISSION_TIME));
                if (isPartialFaulty)
                {
                    *status = hdmicec::SendMessageStatus::ACK_STATE_0;
                    gSendMessageCallback(message, status);
                }
                else
                {
                    *status = hdmicec::SendMessageStatus::BUSY;
                    gSendMessageCallback(message, status);
                }
            }
        }
    }

    // Mock sends back response listed in configuration file
    if(cmd.response.cecCommand != 0xFF)
    {
        this_thread::sleep_for(chrono::milliseconds(HDMIC_CEC_TRANSMISSION_TIME));

        // Mock implementaion: Find a logical address that is connected and not faulty or partial faulty
        uint8_t validLogicalAddress = 0xFF;

        for (auto addr : gHdmiCecConfig.connectedDevices)
        {
            if (std::find(gHdmiCecConfig.faultyDevices.begin(), gHdmiCecConfig.faultyDevices.end(), addr) == gHdmiCecConfig.faultyDevices.end() &&
                std::find(gHdmiCecConfig.partialFaultyDevices.begin(), gHdmiCecConfig.partialFaultyDevices.end(), addr) == gHdmiCecConfig.partialFaultyDevices.end())
            {
                validLogicalAddress = addr;
                break;
            }
        }

        if (validLogicalAddress != 0xFF)
        {
            uint8_t response_dst = src;
            if(cmd.response.type != CEC_TYPE_DIRECT)
            {
                response_dst = 0xF;
            }
            hdmicec_mock_driver_receiveMessage(cmd.response, validLogicalAddress, response_dst);
        }
    }
}*/

void hdmicec_mock_driver_receiveMessage(cecCommands_t &response, uint8_t src, uint8_t dst)
{
    vector<uint8_t> message;

    message.push_back(src | (dst << 4));

    message.push_back(response.cecCommand);
    for (size_t i = 0; i < response.payload.size(); i++)
    {
        message.push_back(response.payload[i]);
    }

    // Call the receiveMessage callback if set
    if (gReceiveMessageCallback)
    {
        gReceiveMessageCallback(message);
    }
}