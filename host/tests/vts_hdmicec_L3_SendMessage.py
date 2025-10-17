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
import tarfile

dir_path = os.path.dirname(os.path.realpath(__file__))
sys.path.append(os.path.join(dir_path, "../"))

from raft.framework.plugins.ut_raft.configRead import ConfigRead
from raft.framework.core.logModule import logModule
from raft.framework.plugins.ut_raft import utHelperClass
from raft.framework.plugins.ut_raft.utSuiteNavigator import UTSuiteNavigatorClass
from raft.framework.plugins.ut_raft.utBaseUtils import utBaseUtils
from vts_hdmicec_helper import hdmiCecHelperClass
from deviceNetwork.HdmiCecNetwork import HdmiCecNetwork

HDMICEC_L3_TEST_SUITE_NAME = "VTS_L3_HDMICEC"

class  VTS_HDMICEC_L3_SendMessage(hdmiCecHelperClass):
    """
    Runs L3 SendMessage test
    """
    def __init__(self, log:logModule=None):
        """
        Initializes the test class
        """
        self.testName = "HdmiCec_L3_SendMessage"
        qcId = "1"
        self.listDirectOpcodesToTest = ["SetOsdName", "CecVersion", "SetOsdName", "CecVersion",
                                        "SetOsdName", "CecVersion", "SetOsdName", "CecVersion"]
        self.listBroadcastOpcodesToTest = ["SetMenuLanguage", "DeviceVendorId", "SetMenuLanguage", "DeviceVendorId",
                                           "SetMenuLanguage", "DeviceVendorId", "SetMenuLanguage", "DeviceVendorId"]

        self.setOSDPayload = ["0x52", "0x44", "0x4B", "0x20", "0x76", "0x44", "0x65", "0x76", "0x69", "0x63", "0x65", "0x20", "0x54", "0x56"]  # "RDK vDevice TV" in ASCII
        self.cecVersionPayload = ["0x05"]  # CEC Version 1.4a
        self.setMenuLanguagePayload = ["0x65", "0x6E", "0x67"]  # "eng" in ASCII
        self.deviceVendorIdPayload = ["0x00", "0x19", "0xA2"]  # RDK Vendor ID

        super().__init__(self.testName, qcId, log)

        self.vComponent_bin_path = os.path.join(self.cpe.get("target_directory"), self.moduleName, self.cpe.get("hdmicec_process_name"))
        self.cecDevice = HdmiCecNetwork("vDevice", self.device_type, self.hal_session, self.cpe.get("hdmicec_process_name"), self.vComponent_bin_path,
                                   self.cpe.get("prompt"), self.deviceTestSetup.get("controller_port"))

        self.connectedDevices = self.cecDevice.listDevices()

    def pickLogicalAddressForDut(self, possibleLogicalAddresses:list):
        """
        Picks a logical address from the possible logical addresses
        Args:
            possibleLogicalAddresses (list): list of possible logical addresses
        """
        if not possibleLogicalAddresses:
            return None
        # Find a logical address from possibleLogicalAddresses that is not already used by connectedDevices
        used_logical_addresses = {device.get("logical address") for device in self.connectedDevices}
        for addr in possibleLogicalAddresses:
            if addr not in used_logical_addresses:
                return addr
        return None

    def pickValidLogicalAddressForController(self):
        """
        Picks a valid logical address for the controller device
        """
        for device in self.connectedDevices:
            if device.get("logical address") != -1:
                return device.get("logical address")
        return None

    def updateTestconfigurationFile(self):
        """
        Updates the test configuration file
        """
        data = self.hdmiCecReadYaml(self.l3TestConfigFile)

        send_msg = data["HdmiCec"]["L3"]["SendMessage"]

        send_msg["source_logical_address"] = [self.dutLogicalAddress]

        # Build CEC messages based on opcode list
        new_cec_messages = []
        for opcode in self.listDirectOpcodesToTest:
            opcode_hex = self.opCodeToHex(opcode)
            if not opcode_hex:
                continue

            if self.destinationLogicalAddress != 0xF:
                directedMessagesSent = 1
                directedMessagesSentAndAcked = 1
                broadcastMessagesSent = 0
                broadcastMessagesSentAndAcked = 0
            else:
                directedMessagesSent = 0
                directedMessagesSentAndAcked = 0
                broadcastMessagesSent = 1
                broadcastMessagesSentAndAcked = 1
            # Compose initiator (source) and destination logical address into a single byte
            if self.dutLogicalAddress is not None and self.destinationLogicalAddress is not None:
                src = int(self.dutLogicalAddress)
                dst = int(self.destinationLogicalAddress)
                header_byte = (src << 4) | dst
                header_byte_str = f"0x{header_byte:02X}"
            else:
                header_byte_str = None  # or handle error as needed

            message = [header_byte_str, self.opCodeToHex(opcode)]
            if opcode == "CecVersion":
                message += self.cecVersionPayload
            elif opcode == "SetOsdName":
                message += self.setOSDPayload
            elif opcode == "SetMenuLanguage":
                message += self.setMenuLanguagePayload
            elif opcode == "DeviceVendorId":
                message += self.deviceVendorIdPayload

            new_cec_messages.append({
                "Parameters": {
                    "message": message
                },
                "Expected_Results": {
                    "metrics": {
                        "directedMessagesSent": directedMessagesSent,
                        "directedMessagesSentAndAcked": directedMessagesSentAndAcked,
                        "broadcastMessagesSent": broadcastMessagesSent,
                        "broadcastMessagesSentAndAcked": broadcastMessagesSentAndAcked,
                        "arbitrationFailures": 0
                    }
                }
            })

        # Replace or extend the list
        send_msg["cec_messages"] = new_cec_messages

        self.hdmiCecWriteYaml(self.l3TestConfigFile, data)

    def testFunction(self):
        """
        The main test function to run the L3 SendMessage.

        Returns:
            bool: Final result of the test.
        """
        self.testcase = "SendMessage"
        possibleLogicalAddresses = self.getPossibleLogicalAddresses(self.device_type)
        self.dutLogicalAddress = self.pickLogicalAddressForDut(possibleLogicalAddresses)
        self.destinationLogicalAddress = self.pickValidLogicalAddressForController()

        # Remove this
        self.utilities.sftpCopy(self.hal_session, "/home/sky/Bhanu/hdmicec_python/test_configuration/vts_l3_HdmiCec.yaml", "/tmp/HdmiCec/test_configuration/")

        self.updateTestconfigurationFile()

        self.utMenu.start()

        self.log.stepStart(f'Test Suit: {HDMICEC_L3_TEST_SUITE_NAME} Test Case: {self.testcase}')
        output = self.utMenu.select(HDMICEC_L3_TEST_SUITE_NAME, self.testcase, None, self.waitTime, False)

        results = self.utMenu.collect_results(output, True)
        if results == None:
            results = False

        self.log.stepResult(results, f'Test Suit: {HDMICEC_L3_TEST_SUITE_NAME} Test Case: {self.testcase}')

        return results

if __name__ == '__main__':
    summerLogName = os.path.splitext(os.path.basename(__file__))[0] + "_summery"
    summeryLog = logModule(summerLogName, level=logModule.INFO)
    test = VTS_HDMICEC_L3_SendMessage(summeryLog)
    test.run(False)