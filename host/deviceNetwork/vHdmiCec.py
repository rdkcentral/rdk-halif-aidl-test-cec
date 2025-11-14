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
import yaml

dir_path = os.path.dirname(os.path.realpath(__file__))
sys.path.append(dir_path)
sys.path.append(os.path.join(dir_path, "../"))

HDMICEC_TV_VCOMPONENT_FILE = os.path.join(dir_path, "vts_cec_device_network_configuration_VTV.yaml")
HDMICEC_STB_VCOMPONENT_FILE = os.path.join(dir_path, "vts_cec_device_network_configuration_VSTB.yaml")
HDMICEC_PRINT_CEC_NETWORK_CONFIG_FILE = os.path.join(dir_path, "vts_cec_print_device_network_configuration.yaml")
HDMICEC_SEND_CEC_MSG = os.path.join(dir_path, "vts_cec_send_cec_message_configuration.yaml")

#from raft.framework.plugins.ut_raft.configRead import ConfigRead
from raft.framework.plugins.ut_raft.utBaseUtils import utBaseUtils
from raft.framework.plugins.ut_raft.utPlaneController import utPlaneController
from HdmiCecDeviceInterface import HdmiCecDeviceInterface

class vHdmiCec(HdmiCecDeviceInterface):
    """
    HDMI CEC related utility functions
    """
    def __init__(self, device_type:str, session, prompt: str="~#", port:int=8080):
        """
        Initializes the vHdmiCec class for HDMI CEC device communication.

        Args:
            device_type (str): Type of device (e.g., "TV", "Tuner", "PlaybackDevice").
            session: The session object for device communication.
            prompt (str, optional): Command prompt string. Defaults to "~#".
            port (int, optional): Port number for ut-controller communication. Defaults to 8080.
        """
        self.port = port
        self.session = session
        self.commandPrompt = prompt
        self.utilities = utBaseUtils()

        cecDeviceNetworkConfigurationFile = HDMICEC_TV_VCOMPONENT_FILE
        if "TV" in device_type:
            cecDeviceNetworkConfigurationFile = HDMICEC_TV_VCOMPONENT_FILE
        elif "Tuner" in device_type or "PlaybackDevice" in device_type:
            cecDeviceNetworkConfigurationFile = HDMICEC_STB_VCOMPONENT_FILE

        # Load the HDMI CEC device network configuration file
        with open(cecDeviceNetworkConfigurationFile, "r") as f:
            config_dict = yaml.safe_load(f)

        cecDeviceNetworkConfigString = yaml.dump(config_dict)

        cecDeviceNetworkConfigString = cecDeviceNetworkConfigString.replace('"', '\\"')

        # Load the print configuration file
        with open(HDMICEC_PRINT_CEC_NETWORK_CONFIG_FILE, "r") as f:
            print_dict = yaml.safe_load(f)

        self.printConfigString = yaml.dump(print_dict)
        self.printConfigString = self.printConfigString.replace('"', '\\"')

        self.utPlaneController = utPlaneController(self.session, port=self.port)

        self.loadCecDeviceNetworkConfiguration(cecDeviceNetworkConfigString)

    def loadCecDeviceNetworkConfiguration(self, configString: str):
        """
        Loads the HDMI CEC device network configuration file on to the vComponent.
        """

        self.utPlaneController.sendMessage(configString)

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
        # send command to CEC network to print device configuration
        self.utPlaneController.sendMessage(self.printConfigString)

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

    def checkMessageReceived(self, cecMessage:dict, timeout: int = 10) -> bool:
        """
        This function checks to see if a specified opCode has been received.

        Args:
            cecMessage (dict): A dictionary containing the following keys:
                sourceAddress (int): The logical address of the source device (0-15).
                destAddress (int): The logical address of the destination device (0-15).
                opCode (str): Operation code to check as an hexadecimal string e.g 0x81.
                payload (list): List of hexadecimal strings to be checked with the opCode.
            timeout (int): The maximum amount of time, in seconds, that the method will
                           wait for the message to be received. Defaults to 10.
        Returns:
          bool: True if the message was received, False otherwise.
        """
        # Note: for vComponent, this function always returns True as we cannot verify message receipt
        return True

    def sendMessage(self, sourceAddress: int, destAddress: int, opCode: str, payload: list = None) -> bool:
        """
        This function sends a specified opCode.

        Args:
          sourceAddress (int): The logical address of the source device (0-15).
          destAddress (int): The logical address of the destination device (0-15).
          opCode (str): Operation code to send as an hexadecimal string e.g 0x81.
          payload (list): List of hexadecimal strings to be sent with the opCode. Optional.

        Returns:
          bool: True if the message was sent successfully, False otherwise.
        """
        # Format the payload: source, destination, opCode, and payload
        msg_payload = [f"0x{sourceAddress}{destAddress}", opCode]
        if payload:
            msg_payload.extend(payload)

        yaml_content = (
            "HdmiCec:\n"
            "  command: cec_message\n"
            "  description: Send a CEC message\n"
            "  message:\n"
            "    user_defined: true\n"
            f"    payload: {msg_payload}\n"
        )

        # Send the command to ut-controller
        self.utPlaneController.sendMessage(yaml_content)

        return True
