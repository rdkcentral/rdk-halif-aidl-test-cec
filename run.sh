#!/bin/bash
#* ******************************************************************************
#*  If not stated otherwise in this file or this component's LICENSE
#*  file the following copyright and licenses apply:
#*
#*  Copyright 2025 RDK Management
#*
#*  Licensed under the Apache License, Version 2.0 (the License);
#*  you may not use this file except in compliance with the License.
#*  You may obtain a copy of the License at
#*
#*  http://www.apache.org/licenses/LICENSE-2.0
#*
#*  Unless required by applicable law or agreed to in writing, software
#*  distributed under the License is distributed on an AS IS BASIS,
#*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#*  See the License for the specific language governing permissions and
#*  limitations under the License.
#*
#* ******************************************************************************

RUN_DIR=$(realpath "$(dirname "$0")")
VCOMPONENT_SERVICE_NAME=VComponentHdmiCecService

# Function to check if a process is running
check_process()
{
    local process_name=$1
    pgrep -f "$process_name" > /dev/null
    return $?
}

prompt_yes_no()
{
    local prompt=$1
    while true; do
        read -p "$prompt (y/n): " response
        case $response in
            [Yy]* ) return 0;;
            [Nn]* ) return 1;;
            * ) echo "Please answer y or n";;
        esac
    done
}

prompt_string()
{
    local prompt=$1
    local response
    read -p "$prompt : " response
    echo "$response"
}

start_vcomponent_service()
{
    ${RUN_DIR}/$VCOMPONENT_SERVICE_NAME &
    vcomponentservice_pid=$!
    # Wait for service to be ready
    sleep 1
}

if check_process $VCOMPONENT_SERVICE_NAME; then
    echo "${VCOMPONENT_SERVICE_NAME} is already running"
    mocservice_pid=$(pgrep -f $VCOMPONENT_SERVICE_NAME)
    if prompt_yes_no "Do you want to re-start ${VCOMPONENT_SERVICE_NAME}?"; then
        echo "Killing existing ${VCOMPONENT_SERVICE_NAME} (PID: $mocservice_pid)"
        kill $mocservice_pid
        sleep 10
        start_vcomponent_service
    else
        echo "Continuing with existing ${VCOMPONENT_SERVICE_NAME} (PID: $mocservice_pid)"
    fi
else
    start_vcomponent_service
fi

echo "Executing [vts_hdmicec_hal_test]"
"${RUN_DIR}/vts_hdmicec_hal_test" "$@"
