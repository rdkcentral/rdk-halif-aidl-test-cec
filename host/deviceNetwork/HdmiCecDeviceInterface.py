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

from abc import ABCMeta, abstractmethod

class HdmiCecDeviceInterface(metaclass=ABCMeta):
    """
    Abstract base class defining HDMI CEC interface.
    Both virtual and actual HDMI CEC implementations must inherit this.
    """

    def __init__(self, device_type: list, session, prompt: str = "~#", port: int = 8080):
        """
        Initializes the vHdmiCec class for HDMI CEC device communication.

        Args:
            device_type (list): Type of device(s) (e.g., ["TV"], ["Tuner"], ["PlaybackDevice"]).
            session: The session object for device communication.
            prompt (str, optional): Command prompt string. Defaults to "~#".
            port (int, optional): Port number for ut-controller communication. Defaults to 8080.
        """
        self.device_type = device_type
        self.session = session
        self.prompt = prompt
        self.port = port

    @abstractmethod
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
        pass

    @abstractmethod
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
        pass

    @abstractmethod
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
        pass