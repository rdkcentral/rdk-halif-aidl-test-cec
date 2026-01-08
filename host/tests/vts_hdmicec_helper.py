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
import tarfile
import yaml
import time

dir_path = os.path.dirname(os.path.realpath(__file__))
sys.path.append(os.path.join(dir_path, "../"))

from raft.framework.plugins.ut_raft.configRead import ConfigRead
from raft.framework.plugins.ut_raft import utHelperClass
from raft.framework.core.logModule import logModule
from raft.framework.plugins.ut_raft.utSuiteNavigator import UTSuiteNavigatorClass
from raft.framework.plugins.ut_raft.utBaseUtils import utBaseUtils

HDMICEC_TEST_MENU_CONFIGURATION_FILE = os.path.join(dir_path, "vts_hdmicec_testMenuConfiguration.yaml")
HDMICEC_TESTCASES_CONFIGURATION_FILE = os.path.join(os.path.join(dir_path, "../configs"), "vts_hdmicec_testcase_configuration.yaml")
HDMICEC_L3_TEST_CONFIG_FILE = os.path.join(os.path.join(dir_path, "../../test_configuration"), "vts_l3_HdmiCec.yaml")

#TODO: remove once the vendor layer start handled by vDevice service
HDMICEC_BINDER_SERVICE_NAME = "Vendor-HdmiCec"

class hdmiCecHelperClass(utHelperClass):

    def __init__(self, testName:str, configureNetwork:bool, qcId:str, log:logModule=None, testMenuConfigFile:str=None, moduleName:str=None, artifactDirPath:str=None):
        """
        Initializes the test class
        """
        if moduleName is None:
            self.moduleName = "HdmiCec"
        else:
            self.moduleName = moduleName
        super().__init__(testName, qcId, log)

        self.log = log
        if log is None:
            self.log = logModule(self.__class__.__name__)
            self.log.setLevel(self.log.INFO)

        # Parse device configuration
        self.deviceTestSetup = self.cpe.get("test")
        self.targetWorkspace = os.path.join(self.cpe.get("target_directory"), self.moduleName)
        self.moduleHFPFile = os.path.join(dir_path, self.deviceTestSetup.get("profile"))
        profileOnTarget = os.path.join(self.targetWorkspace, "profiles")
        profileOnTarget = os.path.join(profileOnTarget, os.path.basename(self.moduleHFPFile))
        profileOnTarget = profileOnTarget.replace("\\", "/")
        self.waitTime = self.cpe.get("wait_time")
        self.device_type = self.cpe.get("device_type")
        self.platform = self.cpe.get("platform")

        # Load test menu and test case configuration files
        self.l3TestConfigFile = HDMICEC_L3_TEST_CONFIG_FILE
        if testMenuConfigFile is not None:
                hdmiCECTEstMenuConfigFile = testMenuConfigFile
        else:
                hdmiCECTEstMenuConfigFile= HDMICEC_TEST_MENU_CONFIGURATION_FILE

        self.testMenuConfiguration = ConfigRead(hdmiCECTEstMenuConfigFile, self.moduleName)
        if moduleName is None:
            self.testCaseConfiguration = ConfigRead(HDMICEC_TESTCASES_CONFIGURATION_FILE, self.moduleName) # Only needed while running individual tests 
        self.testMenuConfiguration.test.execute = self.testMenuConfiguration.test.execute + f" -p {profileOnTarget}"

        # Establish HAL session
        self.hal_session = self.dut.getConsoleSession("ssh_hal_test")
        self.hal_session_1 = self.dut.getConsoleSession("ssh_hal_test_1")

        # Initialize UT menu navigator and utilities
        self.utMenu = UTSuiteNavigatorClass(self.testMenuConfiguration, None, self.hal_session)
        self.utilities = utBaseUtils()

        # Copy required artifacts to target device
        self.utilities.change_directory(self.hal_session, self.targetWorkspace)
        for artifact in self.testMenuConfiguration.test.artifacts:
            if artifactDirPath is None:
                filesPath = os.path.join(dir_path, artifact)
            else:
                filesPath = os.path.join(artifactDirPath, artifact)
            self.hal_session.write("ls " + self.targetWorkspace + "/" + os.path.basename(artifact) + "\n")
            res = self.hal_session.read_until(self.hal_session.prompt, timeout=5)
            if "No such file or directory" not in res:
                continue  # File already exists on target, skip copying and exit from loop
            self.utilities.sftpCopy(self.hal_session, filesPath, self.targetWorkspace)

            if tarfile.is_tarfile(filesPath):
                target_tarfile = os.path.join(self.targetWorkspace, os.path.basename(artifact))
                target_tarfile = target_tarfile.replace('\\', '/')
                self.utilities.untar(self.hal_session, target_tarfile, self.targetWorkspace)

        #TODO: remove once the vendor layer start handled by vDevice service
        self.utilities.change_directory(self.hal_session_1, self.targetWorkspace)
        self.hal_session_1.write(f"./{HDMICEC_BINDER_SERVICE_NAME} &")
        time.sleep(2)

        if configureNetwork:
            self.hdmiCECController.start()

            self.connectedDevices = self.hdmiCECController.listDevices()

    def pickLogicalAddressForDut(self, possibleLogicalAddresses:list) -> list:
        """
        Picks a logical address from the possible logical addresses
        Args:
            possibleLogicalAddresses (list): list of possible logical addresses
        """
        if not possibleLogicalAddresses:
            return None
        # Find a logical address from possibleLogicalAddresses that is not already used by connectedDevices
        used_logical_addresses = {device.get("logical address") for device in self.connectedDevices}

        available_addresses = [addr for addr in possibleLogicalAddresses if addr not in used_logical_addresses]
        return available_addresses

    def pickValidLogicalAddressForController(self):
        """
        Picks a valid logical address for the controller device
        """
        for device in self.connectedDevices:
            if device.get("logical address") != -1:
                return device.get("logical address")
        return None

    def getPossibleLogicalAddresses(self, device_type:list):
        """
        Returns the possible logical addresses based on the device type
        Args:
            device_type (list): device type
        Returns:
            list: list of possible logical addresses
        """
        logical_addresses = []
        for dtype in device_type:
            if dtype == "TV":
                logical_addresses.append(0)
            elif dtype == "RecordingDevice":
                logical_addresses.extend([1, 2, 9])
            elif dtype == "Tuner":
                logical_addresses.extend([3, 6, 7, 10])
            elif dtype == "PlaybackDevice":
                logical_addresses.extend([4, 8, 11])
            elif dtype == "AudioSystem":
                logical_addresses.append(5)
            elif dtype == "VideoProcessor":
                logical_addresses.append(14)
            elif dtype == "PureCecSwitch":
                logical_addresses.append(13)
            else:
                logical_addresses.extend([11, 12])
        return logical_addresses

    def hexToOpCode(self, hex_value:str):
        """
        Converts hex value to opcode
        Args:
            hex_value (str): hex value of the opcode
        Returns:
            str: opcode string
        """
        return {
            "0x47": "SetOsdName",
            "0x9F": "CecVersion",
            "0x32": "SetMenuLanguage",
            "0x84": "ReportPhysicalAddress",
            "0x87": "DeviceVendorId"
        }.get(hex_value, None)

    def opCodeToHex(self, opcode:str):
        """
        Converts the opcode to hex value
        Args:
            opcode (str): opcode string
        Returns:
            str: hex value of the opcode
        """
        #TODO: Expand this dictionary as needed
        opcode_dict = {
            "SetOsdName": "0x47",
            "CecVersion": "0x9F",
            "SetMenuLanguage": "0x32",
            "ReportPhysicalAddress": "0x84",
            "DeviceVendorId": "0x87"
        }
        return opcode_dict.get(opcode, None)

    def _compose_message(self, opcode: str, src_addr: int, dst_addr: int, payload_map: dict, metrics: dict = None) -> dict:
        """Helper to build one CEC message entry."""
        opcode_hex = self.opCodeToHex(opcode)
        if not opcode_hex:
            return None

        # Compose header byte (initiator << 4 | destination)
        header_str = f"0x{(src_addr << 4 | dst_addr):02X}" if src_addr is not None and dst_addr is not None else None

        # Construct message payload
        message = [header_str, opcode_hex] + payload_map.get(opcode, [])

        if metrics is None:
            return {
                "Parameters": {"message": message}
            }
        else:
            return {
                "Parameters": {"message": message},
                "Expected_Results": {"metrics": metrics},
            }

    def hdmiCecReadYaml(self, yaml_path: str) -> dict:
        """
        Reads a YAML file and returns its content as a dictionary.
        Args:
            yaml_path (str): Path to the YAML file
        Returns:
            dict: Content of the YAML file as a dictionary
        """
        with open(yaml_path, "r") as f:
            return yaml.safe_load(f)


    def hdmiCecWriteYaml(self, yaml_path: str, data: dict):
        """
        Writes a dictionary to a YAML file.
        Args:
            yaml_path (str): Path to the YAML file
            data (dict): Content to write to the YAML file
        """
        with open(yaml_path, "w") as f:
            yaml.dump(data, f, sort_keys=False, default_flow_style=None)

    def updateNestedDictionaries(self, original: dict, updates: dict):
        """
        Recursively updates nested dictionaries.
        Args:
            original (dict): Original dictionary to be updated
            updates (dict): Dictionary with updates
        """
        for key, value in updates.items():
            if isinstance(value, dict) and key in original:
                self.updateNestedDictionaries(original[key], value)
            else:
                original[key] = value

    def testEndFunction(self, powerOff=True):
        """
        Ends the test case.
        Args:
            powerOff (bool): Whether to power off the device
        """
        super().testEndFunction(powerOff)
