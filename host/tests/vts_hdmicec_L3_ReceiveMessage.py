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
import threading

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

class  VTS_HDMICEC_L3_ReceiveMessage(hdmiCecHelperClass):
    """
    Runs L3 ReceiveMessage test
    """
    def __init__(self, log:logModule=None):
        """
        Initializes the test class
        """
        self.testName = "HdmiCec_L3_ReceiveMessage"
        qcId = "2"
        self.listDirectOpcodesToTest = ["SetOsdName", "CecVersion", "SetOsdName", "CecVersion",
                                        "SetOsdName", "CecVersion", "SetOsdName", "CecVersion"]
        self.listBroadcastOpcodesToTest = ["SetMenuLanguage", "DeviceVendorId", "SetMenuLanguage", "DeviceVendorId",
                                           "SetMenuLanguage", "DeviceVendorId", "SetMenuLanguage", "DeviceVendorId"]

        self.setOSDPayload = ["0x52", "0x44", "0x4B", "0x20", "0x76", "0x44", "0x65", "0x76", "0x69", "0x63", "0x65", "0x20", "0x54", "0x56"]  # "RDK vDevice TV" in ASCII
        self.cecVersionPayload = ["0x05"]  # CEC Version 1.4a
        self.setMenuLanguagePayload = ["0x65", "0x6E", "0x67"]  # "eng" in ASCII
        self.deviceVendorIdPayload = ["0x00", "0x19", "0xA2"]  # RDK Vendor ID

        # Common opcode → payload map
        self.payload_map = {
            "CecVersion": self.cecVersionPayload,
            "SetOsdName": self.setOSDPayload,
            "SetMenuLanguage": self.setMenuLanguagePayload,
            "DeviceVendorId": self.deviceVendorIdPayload,
        }

        super().__init__(self.testName, qcId, log)

        self.vComponent_bin_path = os.path.join(self.cpe.get("target_directory"), self.moduleName, self.cpe.get("hdmicec_process_name"))
        self.cecDevice = HdmiCecNetwork("vDevice", self.device_type, self.hal_session_1, self.cpe.get("hdmicec_process_name"), self.vComponent_bin_path,
                                   self.cpe.get("prompt"), self.deviceTestSetup.get("controller_port"))

        self.connectedDevices = self.cecDevice.listDevices()

        self.sendMessageThreadObj = threading.Thread(target=self.sendMessageThread)

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
        Updates the HDMI CEC L3 ReceiveMessage YAML configuration file.
        """
        data = self.hdmiCecReadYaml(self.l3TestConfigFile)
        receive_msg = data["HdmiCec"]["L3"]

        receive_msg["ReceiveMessage"] = []
        for opcode in self.listDirectOpcodesToTest:
            src_addr = self.destinationLogicalAddress
            dst_addr = self.dutLogicalAddress
            msg = self._compose_message(opcode, src_addr, dst_addr,  self.payload_map)
            msg["Parameters"]["logical_address"] = [self.dutLogicalAddress]
            msg["Parameters"]["wait_for_response_timeout"] = 5000
            receive_msg["ReceiveMessage"].append(msg)

        # write to YAML
        self.hdmiCecWriteYaml(self.l3TestConfigFile, data)

    def sendMessageThread(self):
        """
        Thread function to send messages to DUT
        """
        time.sleep(1)  # Wait for DUT to be ready to receive messages
        for opcode in self.listDirectOpcodesToTest:
            self.cecDevice.sendMessage(self.destinationLogicalAddress, self.dutLogicalAddress, self.opCodeToHex(opcode), self.payload_map.get(opcode))
            time.sleep(1)  # Small delay between messages

    def testFunction(self):
        """
        The main test function to run the L3 ReceiveMessage.

        Returns:
            bool: Final result of the test.
        """
        self.utilities.change_directory(self.hal_session, self.targetWorkspace)
        results = False
        self.testcase = "ReceiveMessage"
        possibleLogicalAddresses = self.getPossibleLogicalAddresses(self.device_type)
        self.dutLogicalAddress = self.pickLogicalAddressForDut(possibleLogicalAddresses)
        self.destinationLogicalAddress = self.pickValidLogicalAddressForController()

        self.updateTestconfigurationFile()

        self.utilities.sftpCopy(self.hal_session, self.l3TestConfigFile, os.path.join(self.targetWorkspace, "test_configuration/"))

        self.utMenu.start()

        self.sendMessageThreadObj.start()

        self.log.stepStart(f'Test Suit: {HDMICEC_L3_TEST_SUITE_NAME} Test Case: {self.testcase}')
        output = self.utMenu.select(HDMICEC_L3_TEST_SUITE_NAME, self.testcase, None, self.waitTime, False)

        results = self.utMenu.collect_results(output, True)
        if results is None:
            results = False

        self.sendMessageThreadObj.join()

        self.log.stepResult(results, f'Test Suit: {HDMICEC_L3_TEST_SUITE_NAME} Test Case: {self.testcase}')

        return results

if __name__ == '__main__':
    summaryLogName = os.path.splitext(os.path.basename(__file__))[0] + "_summery"
    summaryLog  = logModule(summaryLogName, level=logModule.INFO)
    test = VTS_HDMICEC_L3_ReceiveMessage(summaryLog )
    test.run(False)