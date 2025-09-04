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
* vts_mock_HdmiCecService.cpp
*
* Main entry point for the HdmiCec service
*/

#include <signal.h>
#include <getopt.h>
#include <ut.h>
#include <ut_log.h>

#include "vts_mock_HdmiCecInit.h"
#include "vts_mock_HdmiCec.h"

using namespace com::rdk::hal::hdmicec;

#define DEFAULT_MOCK_CONFIG_FILE "mock_configurations/vts_hdmi_cec_mock_v1.yaml"

int main(int argc, char** argv)
{
    std::string configFile = DEFAULT_MOCK_CONFIG_FILE;
    if(argc > 1)
    {
        std::string arg = argv[1];
        if(arg == "-h" || arg == "--help")
        {
            UT_LOG_INFO("[HDMICEC_MOCK]HdmiCecService -h | --help | -c | --config <file>");
            return 0;
        }
        else if((arg == "-c" || arg == "--config") && argc > 2)
        {
            configFile = argv[2];
        }
        else
        {
            UT_LOG_ERROR("[HDMICEC_MOCK]Unknown option: %s", arg);
            UT_LOG_ERROR("[HDMICEC_MOCK]Use -h or --help for usage information");
            return 1;
        }
    }

    UT_LOG_INFO("[HDMICEC_MOCK]Using configuration file: %s", configFile.c_str());

    if(HdmiCecInit((char *)configFile.c_str()) == false)
    {
        UT_LOG_ERROR("[HDMICEC_MOCK]Failed to initialize HdmiCec service with config file: %s", configFile.c_str());
        return -1;
    }

    signal(SIGPIPE, SIG_IGN);
    HdmiCec::publishAndJoinThreadPool();

    return 0;
}

