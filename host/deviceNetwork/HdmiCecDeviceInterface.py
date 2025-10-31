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

    def __init__(self, device_type: list, session, process_name: str="", bin_path: str="", prompt: str = "~#", port: int = 8080):
        self.device_type = device_type
        self.session = session
        self.bin_path = bin_path
        self.process_name = process_name
        self.prompt = prompt
        self.port = port

    @abstractmethod
    def loadCecDeviceNetworkConfiguration(self):
        """Load HDMI CEC network configuration onto target."""
        pass

    @abstractmethod
    def listDevices(self) -> list:
        """List discovered CEC devices on the network."""
        pass

    @abstractmethod
    def checkMessageReceived(self, sourceAddress: str, destAddress: str, opCode: str, timeout: int = 10, payload: list = None) -> bool:
        """Check if a specific CEC message is received."""
        pass

    @abstractmethod
    def sendMessage(self, sourceAddress: str, destAddress: str, opCode: str, payload: list = None) -> bool:
        """Send a specific CEC message."""
        pass