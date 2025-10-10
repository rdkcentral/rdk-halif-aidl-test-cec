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

class  VTS_HDMICEC_L1_L2_Tests(utHelperClass):
    """
    Runs automated L1, L2 HDMICEC tests on the target device.
    Handles setup, service restarts, asset management.
    Collects and aggregates test results for pass/fail reporting.
    """
    def __init__(self, log:logModule=None):
        """
        Initializes the test class
        """
        self.moduleName = "HdmiCec"
        qcId = "1"
        super().__init__(self.moduleName, qcId, log)

        self.log = log
        if log is None:
            self.log = logModule(self.__class__.__name__)
            self.log.setLevel(self.log.INFO)

        deviceTestSetup = self.cpe.get("test")
        self.targetWorkspace = os.path.join(self.cpe.get("target_directory"), self.moduleName)

        self.moduleHFPFile = os.path.join(dir_path, deviceTestSetup.get("profile"))
        self.testConfigFile = os.path.join(dir_path, "vts_hdmicec_testConfig.yml")

        self.testConfig    = ConfigRead(self.testConfigFile, self.moduleName)

        self.hal_session = self.dut.getConsoleSession("ssh_hal_test")

        self.utMenu = UTSuiteNavigatorClass(self.testConfig, None, self.hal_session)
        self.utilities = utBaseUtils()

        self.utilities.change_directory(self.hal_session, self.targetWorkspace)

        for artifact in self.testConfig.test.artifacts:
            filesPath = os.path.join(dir_path, artifact)
            self.utilities.sftpCopy(self.hal_session, filesPath, self.targetWorkspace)

            if tarfile.is_tarfile(filesPath):
                target_tarfile = os.path.join(self.targetWorkspace, os.path.basename(artifact))
                self.utilities.untar(self.hal_session, target_tarfile, self.targetWorkspace)

        testSetupPath = os.path.join(dir_path, "vts_hdmicec_setup.yml")
        self.testSetup = ConfigRead(testSetupPath, self.moduleName)

        profileOnTarget = os.path.join(self.targetWorkspace, "profiles")
        profileOnTarget = os.path.join(profileOnTarget, os.path.basename(self.moduleHFPFile))
        self.testConfig.test.execute = self.testConfig.test.execute + f" -p {profileOnTarget}"
        self.waitTime = self.cpe.get("wait_time")

    def runTest(self, testSuite, test_case:str=None):
        """
        Runs the test case passed to this funtion
        Args:
            test_case (str, optional): test case name to run, default runs all test
        Returns:
            bool: True - test pass, False - test fails
        """
        testsuite_name = testSuite.get("name")

        output = self.utMenu.select(testsuite_name, test_case, None, self.waitTime, False)
        results = self.utMenu.collect_results(output, True)
        if results == None:
            results = False
        return results

    def testFunction(self):
        """
        The main test function to run the L1 and L2 tests.

        Returns:
            bool: Final result of the test.
        """
        self.utMenu.start()

        testsuites = self.testSetup.fields.get('test_suites')
        finalresult = True

        for testsuite in testsuites:
            testsuite_name = testsuite.get("name")
            test_cases = testsuite.get("test_cases")

            if len(test_cases) == 1 and test_cases[0] == "all":
                self.log.stepStart(f'Test Suit: {testsuite_name} Run all Tests cases')
                # If 'all' test case mentioned in list, run all tests with 'r' option
                result = self.runTest(testsuite)
                self.log.stepResult(result, f'Test Suit: {testsuite_name} Run all Tests cases')
            else:
                for test_case in testsuite.get("test_cases"):
                    self.log.stepStart(f'Test Suit: {testsuite_name} Test Case: {test_case}')
                    result = self.runTest(testsuite, test_case)
                    if not result:
                        finalresult = False
                    self.log.stepResult(result, f'Test Suit: {testsuite_name} Test Case: {test_case}')
        return finalresult

if __name__ == '__main__':
    summerLogName = os.path.splitext(os.path.basename(__file__))[0] + "_summery"
    summeryLog = logModule(summerLogName, level=logModule.INFO)
    test = VTS_HDMICEC_L1_L2_Tests(summeryLog)
    test.run(False)