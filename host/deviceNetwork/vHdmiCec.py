#!/usr/bin/env python3
#** *****************************************************************************
# *
# * If not stated otherwise in this file or this component's LICENSE file the
# * following copyright and licenses apply:
# *
# * Copyright 2024 RDK Management
# *
# * Licensed under the Apache License, Version 2.0 (the "License");
# * you may not use this file except in compliance with the License.
# * You may obtain a copy of the License at
# *
# *
# http://www.apache.org/licenses/LICENSE-2.0
# *
# * Unless required by applicable law or agreed to in writing, software
# * distributed under the License is distributed on an "AS IS" BASIS,
# * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# * See the License for the specific language governing permissions and
# * limitations under the License.
# *
#* ******************************************************************************

import os
import sys
import time
import re

dir_path = os.path.dirname(os.path.realpath(__file__))
sys.path.append(dir_path)
sys.path.append(os.path.join(dir_path, "../"))

HDMICEC_TV_VCOMPONENT_FILE = os.path.join(dir_path, "vts_cec_device_network_configuration_VTV.yaml")
HDMICEC_STB_VCOMPONENT_FILE = os.path.join(dir_path, "vts_cec_device_network_configuration_VSTB.yaml")
HDMICEC_PRINT_CEC_NETWORK_CONFIG_FILE = os.path.join(dir_path, "vts_cec_print_device_network_configuration.yaml")

from raft.framework.plugins.ut_raft.configRead import ConfigRead
from raft.framework.plugins.ut_raft.utBaseUtils import utBaseUtils
from HdmiCecDeviceInterface import HdmiCecDeviceInterface

class vHdmiCec(HdmiCecDeviceInterface):
    """
    HDMI CEC related utility functions
    """
    def __init__(self, device_type:list, session, process_name:str="", bin_path:str="", prompt: str="~#", port:int=8080):
        """
        Initializes the class
        """
        self.device_type = device_type
        self.port = port
        self.session = session
        self.commandPrompt = prompt
        self.processName = process_name
        self.utilities = utBaseUtils()
        self.target_directory = os.path.dirname(bin_path)

        self.cecDeviceNetworkConfigFile = HDMICEC_TV_VCOMPONENT_FILE
        if "TV" in self.device_type:
            self.cecDeviceNetworkConfigFile = HDMICEC_TV_VCOMPONENT_FILE
        elif "Tuner" in self.device_type or "PlaybackDevice" in self.device_type:
            self.cecDeviceNetworkConfigFile = HDMICEC_STB_VCOMPONENT_FILE

        self.cecDeviceNetworkConfig = ConfigRead(self.cecDeviceNetworkConfigFile, "HdmiCec")

        stdin, stdout, stderr = self.session.console.exec_command(f"pidof {self.processName}")
        out = stdout.read().decode().strip()
        rc = stdout.channel.recv_exit_status()

        if rc != 0 and not out:
            self.utilities.change_directory(self.session, self.target_directory)
            self.session.write(f"./{self.processName} &")
            time.sleep(2)

        self.target_configuration_file = os.path.join(self.target_directory, os.path.basename(self.cecDeviceNetworkConfigFile))
        self.target_print_configuration_file = os.path.join(self.target_directory, os.path.basename(HDMICEC_PRINT_CEC_NETWORK_CONFIG_FILE))

        self.utilities.sftpCopy(self.session, self.cecDeviceNetworkConfigFile, self.target_directory)
        self.utilities.sftpCopy(self.session, HDMICEC_PRINT_CEC_NETWORK_CONFIG_FILE, self.target_directory)

        self.loadCecDeviceNetworkConfiguration()

    def loadCecDeviceNetworkConfiguration(self):
        """
        Loads the HDMI CEC device network configuration file on to the vComponent.
        """

        # Prepare the curl command
        cmd = f'curl -X POST -H "Content-Type: application/x-yaml" --data-binary "@{self.target_configuration_file}" "http://localhost:{self.port}/api/postKVP"'

        self.session.write(cmd)


    def listDevices(self) -> list:
        """
        Lists the devices currently available on the HDMI CEC network.

        Returns:
            list: A list of dictionaries representing discovered devices with details.
            {
                "name": "name",
                "physical address": "0.0.0.0",
                "logical address": 0,
                "active source": 0,
            }
        """
        # Prepare the curl command
        cmd = f'curl -X POST -H "Content-Type: application/x-yaml" --data-binary "@{self.target_print_configuration_file}" "http://localhost:{self.port}/api/postKVP"'

        self.session.write(cmd)

        result = self.session.read_until(self.commandPrompt)

        result = re.sub(r'\x1b\[[0-9;]*m', '', result)         # remove ANSI color codes
        result = result.replace('\r', '')                      # normalize newlines
        result = re.sub(r'root@[\w\-\:\/# ]+', '', result)     # remove shell prompt lines
        result = re.sub(r'curl:.*?\n', '', result, flags=re.DOTALL)  # remove curl noise if any

        devices = []

        # Regex to match device lines
        pattern = re.compile(
            r"- Name:\s*(?P<name>[^,]+),.*?"
            r"Active Source:\s*(?P<active>\d+),.*?"
            r"Logical-1:\s*(?P<logical1>-?\d+),.*?"
            r"Physical:\s*(?P<physical>[\d\.]+)",
            re.MULTILINE
        )

        for match in pattern.finditer(result):
            devices.append({
                "name": match.group("name").strip(),
                "physical address": match.group("physical"),
                "logical address": int(match.group("logical1")),
                "active source": int(match.group("active")),
            })

        return devices

    def checkMessageReceived(self, sourceAddress: str, destAddress: str, opCode: str, timeout: int = 10, payload: list = None) -> bool:
        """
        This function checks to see if a specified opCode has been received.

        Args:
          sourceAddress (str): The logical address of the source device (0-9 or A-F).
          destAddress (str): The logical address of the destination device (0-9 or A-F).
          opCode (str): Operation code to send as an hexidecimal string e.g 0x81.
          timeout (int): The maximum amount of time, in seconds, that the method will
                           wait for the message to be received. Defaults to 10.
            payload (list): List of hexidecimal strings to be sent with the opCode. Optional.

        Returns:
          bool: True if the message was received, False otherwise.
        """
        # Note: for vComponent, this function always returns True as we cannot verify message receipt
        return True
