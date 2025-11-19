# HDMICEC High Level Test Specification Document

## Table of Contents

- [Acronyms, Terms and Abbreviations](#acronyms-terms-and-abbreviations)
- [Introduction](#introduction)
- [Module Description](#module-description)
- [L2 Test Specification](#l2-test-specification)
- [L3 Test Specification](#l3-test-specification)

## Acronyms, Terms and Abbreviations

- `HAL`    - Hardware Abstraction Layer
- `VTS`    - Vendor Test Suite
- `HDMI`   - High Definition Multimedia Interface
- `CEC`    - Consumer Electronics Control
- `L1`     - Level 1 Testing
- `L2`     - Level 2 Testing
- `L3`     - Level 3 Testing
- `DUT`    - Device Under Test
- `RAFT`   - Rapid Automation Framework for Testing
- `YAML`   - YAML Ain't Markup Language
- `AIDL`   - Android Interface Definition Language

## Introduction

Consumer Electronics Control (CEC) is a single-wire bidirectional bus within an HDMI system, facilitating communication among interconnected products. HDMI-CEC establishes a protocol enabling high-level control functions between audiovisual devices linked via an HDMI network, facilitating communication and control among them. Communication can occur in either Direct messaging mode or Broadcast mode.

- Hdmicec Interface Definitions - [Hdmicec HAL](https://github.com/rdkcentral/rdk-halif-aidl/tree/main/cec/current/com/rdk/hal/hdmicec)

- Hdmicec HFP - [Hdmicec Overview](https://github.com/rdkcentral/rdk-halif-aidl/blob/main/cec/current/hfp-cec.yaml)




## L2 Test Specification

|Test|Description|HAL APIs|
|----|-----------|--------|
|Verify state of cec|Ensure current state is received during the sequence: open()->close()->open()|open(), getState(), close()|
|Verify states using listener|Validate that the CEC event listener is registered, verify states using listener callback. Callbacks needs to be verified|open(),registerEventListener(), unregisterEventListener(), close(), HdmiCecEventListener.onStateChanged() , HdmiCecControllerListener.onStateChanged()|
|Validate property|Verify the property value for the input property key. If the metric is implemented by the vendor then the value should be zero else -1|getProperty()|
|Validate logical address|Verify the logical addresses that have been set. Note that, after calling removeLogicalAddresses(), getLogicalAddresses() should return empty array |open(), addLogicalAddresses(), getLogicalAddresses(),removeLogicalAddresses() close() |
|Negative test for sendmessage|Verify SendMessageStatus recieved is BUSY, when a sendMessage() is called without any connected devices and also no callbacks are received  |open(), registerEventListener(), sendMessage(), HdmiCecControllerListener.onMessageSent(), HdmiCecControllerListener.onMessageReceived(), HdmiCecEventListener.onMessageReceived(), unregisterEventListener(), close()|




## L3 Test Specification

|Test|	Description	|HAL APIs used|
|-----|------------|-------------|
|Send or Receive CEC Commands|	Send/Receive below commands 	|open(), registerEventListener(), sendMessage(), HdmiCecControllerListener.onMessageSent(), HdmiCecControllerListener.onMessageReceived(), HdmiCecEventListener.onMessageReceived(), unregisterEventListener(), close()|
|Stress Test|Send CEC messages at high frequency ex: 1000 messages per second|open(), registerEventListener(), sendMessage(), HdmiCecControllerListener.onMessageSent(), HdmiCecControllerListener.onMessageReceived(), HdmiCecEventListener.onMessageReceived(), unregisterEventListener(), close()|

|CEC Message| Validation Method|
|----|----|
Image View on|	Send the message and verify that the target device powers on and switches to the HDMI input of the sender.
Text View On|	Send the message and confirm the device powers on and displays the sender’s input with OSD (e.g., app name or source label).
Set Menu Language|	Change system language on the controller, send the message, and verify the DUT updates its menu language accordingly.
Standby	|Send the message and verify that the target device enters standby mode (powers off or sleeps).
Report OSD Name	|Not typically initiated by the controller. Validate only if the DUT sends this in response to a "Give OSD Name" request.
Set OSD String	|Send the message with a string payload and verify that the target device displays the string on screen (e.g., volume level, app name).
Active Source	|Send the message and verify that the DUT becomes the active source on the HDMI switch (TV switches to it).
Report Physical Address(0x84, Broadcast)	|Confirm the address matches the DUT’s HDMI port and is rceived during Power on of device
Device Vendor ID	|Validate only if received in response to a "Give Vendor ID" request. Confirm the vendor ID matches the DUT’s manufacturer.
Power Status	|Validate only if received in response to a "Give Power Status" request. Confirm the reported status matches the actual power state.
CEC Version	|Validate only if received in response to a "Get CEC Version" request. Confirm the version matches the DUT’s supported CEC version.
Report Physical Address	(0xA8, Direct)| Used in latency reporting. Validate only if received in response to "Request Latency". Confirm payload includes latency info.
Give OSD Name	|Send the message and verify that the DUT responds with "Report OSD Name" containing the correct OSD name.
Give Physical Address	|Send the message and verify that the DUT responds with "Report Physical Address" and correct device type.
Request Active Source	|Send the message and verify that the current active source responds with its physical address.
Give Vendor ID	|Send the message and verify that the DUT responds with "Device Vendor ID" and correct vendor ID.
Give Power Status	|Send the message and verify that the DUT responds with "Power Status" and the value matches the actual state.
Get Menu Language	|Send the message and verify that the DUT responds with "Set Menu Language" and the language matches the current setting.
Get CEC Version	|Send the message and verify that the DUT responds with "CEC Version" and the value matches the supported version.
Request Latency	|Send the message and verify that the DUT responds with "Report Physical Address" (0xA8) including latency information.
