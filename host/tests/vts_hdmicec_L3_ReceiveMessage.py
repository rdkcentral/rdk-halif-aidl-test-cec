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

HDMICEC_L3_TEST_SUITE_NAME = "VTS_L3_HDMICEC"

class  VTS_HDMICEC_L3_ReceiveMessage(hdmiCecHelperClass):
    """
    Runs L3 ReceiveMessage test
    """
    def __init__(self, log:logModule=None, testMenuConfigFile: str=None, moduleName: str=None, artifactDirPath: str=None):
        """
        Initializes the test class
        """
        self.testName = "HdmiCec_L3_ReceiveMessage"
        qcId = "2"
        configureNetwork = True
        super().__init__(self.testName, configureNetwork, qcId, log, testMenuConfigFile, moduleName, artifactDirPath)

    def updateTestconfigurationFile(self, dutLogicalAddress:list=[], testDestinationAddress:int=None, broadcastRepeatCount:int=5):
        """
        Updates the HDMI CEC L3 ReceiveMessage YAML configuration file.
        """
        data = self.hdmiCecReadYaml(self.l3TestConfigFile)
        receive_msg = data["HdmiCec"]["L3"]

        self.sendMessageList = []
        for parameters in receive_msg.get("ReceiveMessage", []):
            parameters["Parameters"]["logical_address"] = dutLogicalAddress

            current_dest = parameters["Parameters"]["message"][0]

            dest_addr = int(current_dest, 16) & 0x0F if isinstance(current_dest, str) else current_dest & 0x0F
            # Use existing destination if it's 0xF, otherwise use testDestinationAddress
            if dest_addr == 0xF:
                final_dest = dest_addr
                parameters["Parameters"]["repeat"] = broadcastRepeatCount
            else:
                final_dest = testDestinationAddress

            parameters["Parameters"]["message"][0] = f"0x{(self.controllerLogicalAddress << 4 | final_dest):02X}"
            self.sendMessageList.append({
                                        "sourceAddress": f"{self.controllerLogicalAddress:X}",
                                        "destAddress": f"{final_dest:X}",
                                        "opcode": parameters["Parameters"]["message"][1],
                                        "payload": parameters["Parameters"]["message"][2:],
                                        "repeat": int(parameters["Parameters"].get("repeat", 1))
                                    })
        # write to YAML
        self.hdmiCecWriteYaml(self.l3TestConfigFile, data)

    def sendMessageThread(self):
        """
        Thread function to send messages to DUT
        """
        time.sleep(1)  # Wait for DUT to be ready to receive messages
        for message in self.sendMessageList:
            for _ in range(message["repeat"]):
                self.hdmiCECController.sendMessage(message["sourceAddress"], message["destAddress"], message["opcode"], message["payload"])
                time.sleep(1)  # Small delay between messages

    def receiveMessage(self, description:str="") -> bool:

        """
        The main test function to send messages to dut from controller.
        Args:
            description (str): Description of the test case.
        Returns:
            bool: Final result of the test.
        """

        self.sendMessageThreadObj = threading.Thread(target=self.sendMessageThread)

        self.utMenu.start()

        self.sendMessageThreadObj.start()

        self.log.stepStart(f'Test Suit: {HDMICEC_L3_TEST_SUITE_NAME} Test Case: {self.testcase} Description: {description}')
        output = self.utMenu.select(HDMICEC_L3_TEST_SUITE_NAME, self.testcase, None, self.waitTime, False)

        results = self.utMenu.collect_results(output, True)
        if results is None:
            results = False

        self.sendMessageThreadObj.join()

        self.log.stepResult(results, f'Test Suit: {HDMICEC_L3_TEST_SUITE_NAME} Test Case: {self.testcase} Description: {description}')

        self.utMenu.stop()

        return results

    def testFunction(self):
        """
        The main test function to run the L3 ReceiveMessage.

        Returns:
            bool: Final result of the test.
        """
        self.utilities.change_directory(self.hal_session, self.targetWorkspace)
        results = True
        self.testcase = "ReceiveMessage"

        possibleLogicalAddresses = self.getPossibleLogicalAddresses(self.device_type)
        self.dutLogicalAddress = self.pickLogicalAddressForDut(possibleLogicalAddresses)
        self.controllerLogicalAddress = self.pickValidLogicalAddressForController()

        if self.dutLogicalAddress is None or self.controllerLogicalAddress is None:
            self.log.stepResult(False, f'Test Suit: {HDMICEC_L3_TEST_SUITE_NAME} Test Case: {self.testcase} Description: Failed to assign DUT address')
            return False

        target_config_path = self.targetWorkspace + "/test_configuration/"
        # Scenario 1: Messages sent to DUT and broadcast from controller
        for addr in self.dutLogicalAddress:
            self.updateTestconfigurationFile(self.dutLogicalAddress, addr, 5)

            self.utilities.sftpCopy(self.hal_session, self.l3TestConfigFile, target_config_path)

            results &= self.receiveMessage(f"DUT:{self.dutLogicalAddress}, Destination:{addr}")

        # Scenario 2: Messages sent to DUT without assigning logical address to DUT
        for addr in self.dutLogicalAddress:
            self.updateTestconfigurationFile([], addr, 5)

            self.utilities.sftpCopy(self.hal_session, self.l3TestConfigFile, target_config_path)

            results &= self.receiveMessage(f"DUT:[], Destination:{addr}")

        # Scenario 3: Messages sent to different logical address than DUT
        # pick logical address not assigned to DUT
        allPossibleAddresses = set(range(0, 15))
        dutAddresses = set(self.dutLogicalAddress)
        availableAddresses = list(allPossibleAddresses - dutAddresses)

        if not availableAddresses:
            self.log.stepResult(False, f'Test Suit: {HDMICEC_L3_TEST_SUITE_NAME} Test Case: {self.testcase} Description: No available logical addresses for Scenario 3')
            return False

        otherAddr = availableAddresses[0]

        self.updateTestconfigurationFile(self.dutLogicalAddress, otherAddr, 0)
        self.utilities.sftpCopy(self.hal_session, self.l3TestConfigFile, target_config_path)
        results &= self.receiveMessage(f"DUT:{self.dutLogicalAddress}, Destination:{otherAddr}")

        return results

if __name__ == '__main__':
    summaryLogName = os.path.splitext(os.path.basename(__file__))[0] + "_summery"
    summaryLog  = logModule(summaryLogName, level=logModule.INFO)
    test = VTS_HDMICEC_L3_ReceiveMessage(summaryLog )
    test.run(False)
