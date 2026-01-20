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

- Hdmicec Interface Definitions - [Hdmicec HAL](https://github.com/rdkcentral/rdk-halif-aidl/tree/main/hdmicec/current/com/rdk/hal/hdmicec)

- Hdmicec HFP - [Hdmicec Overview](https://github.com/rdkcentral/rdk-halif-aidl/blob/main/hdmicec/current/hfp-hdmicec.yaml)




## L2 Test Specification

|Test|Description|HAL APIs|
|----|-----------|--------|
|Verify state of cec|Ensure current state is received during the sequence: open()->close()->open()|open(), getState(), close()|
|Verify states using listener|Validate that the CEC event listener is registered, verify states using listener callback. Callbacks needs to be verified|open(),registerEventListener(), unregisterEventListener(), close(), HdmiCecEventListener.onStateChanged() , HdmiCecEventListener.onStateChanged()|
|Validate property|Verify the property value for the input property key. If the metric is implemented by the vendor then the value should be zero else -1|getProperty()|
|Validate logical address|Verify the logical addresses that have been set. Note that, after calling removeLogicalAddresses(), getLogicalAddresses() should return empty array |open(), addLogicalAddresses(), getLogicalAddresses(),removeLogicalAddresses() close() |
|Negative test for sendmessage|Verify SendMessageStatus recieved is BUSY, when a sendMessage() is called without any connected devices and also no callbacks are received  |open(), registerEventListener(), sendMessage(), HdmiCecEventListener.onMessageSent(), HdmiCecEventListener.onMessageReceived(), HdmiCecEventListener.onMessageReceived(), unregisterEventListener(), close()|

## L3 Test Specification

|Test|	Description	|HAL APIs used|
|-----|------------|-------------|
|Send CEC Commands|	Send `cec` commands to controller and check whether the messages received at controller or not|open(), registerEventListener(), sendMessage(), HdmiCecEventListener.onMessageSent(), HdmiCecEventListener.onMessageReceived(), HdmiCecEventListener.onMessageReceived(), unregisterEventListener(), close()|
|Receive CEC Commands|	Send `cec` commands to `DUT` from controller and check whether the HAL triggers callbacks |open(), registerEventListener(), sendMessage(), HdmiCecEventListener.onMessageSent(), HdmiCecEventListener.onMessageReceived(), HdmiCecEventListener.onMessageReceived(), unregisterEventListener(), close()|
