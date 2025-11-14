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

        super().__init__(self.testName, qcId, log)

        self.sendMessageThreadObj = threading.Thread(target=self.sendMessageThread)

    def updateTestconfigurationFile(self):
        """
        Updates the HDMI CEC L3 ReceiveMessage YAML configuration file.
        """
        data = self.hdmiCecReadYaml(self.l3TestConfigFile)
        receive_msg = data["HdmiCec"]["L3"]

        self.sendMessageList = []
        for parameters in receive_msg.get("ReceiveMessage", []):
            parameters["Parameters"]["logical_address"] = [self.dutLogicalAddress]

            current_dest = parameters["Parameters"]["message"][0]

            dest_addr = int(current_dest, 16) & 0x0F if isinstance(current_dest, str) else current_dest & 0x0F
            # Use existing destination if it's 0xF, otherwise use self.dutLogicalAddress
            final_dest = dest_addr if dest_addr == 0x0F else self.dutLogicalAddress
            parameters["Parameters"]["message"][0] = f"0x{(self.controllerLogicalAddress << 4 | final_dest):02X}"
            self.sendMessageList.append({
                                        "sourceAddress": f"{self.controllerLogicalAddress:X}",
                                        "destAddress": f"{final_dest:X}",
                                        "opcode": parameters["Parameters"]["message"][1],
                                        "payload": parameters["Parameters"]["message"][2:]
                                    })
        # write to YAML
        self.hdmiCecWriteYaml(self.l3TestConfigFile, data)

    def sendMessageThread(self):
        """
        Thread function to send messages to DUT
        """
        time.sleep(1)  # Wait for DUT to be ready to receive messages
        for message in self.sendMessageList:
            self.cecDevice.sendMessage(message["sourceAddress"], message["destAddress"], message["opcode"], message["payload"])
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
        self.controllerLogicalAddress = self.pickValidLogicalAddressForController()

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