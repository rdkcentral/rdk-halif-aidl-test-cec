# HDMI CEC L2 Low Level Test Specification

## Table of Contents

- [Overview](#overview)
- [Acronyms, Terms and Abbreviations](#acronyms-terms-and-abbreviations)
- [References](#references)
- [Level 2 Test Specification](#level-2-test-specification)

## Overview

This document describes the level 2 low level test specification for the `HDMI` `CEC` module.

### Acronyms, Terms and Abbreviations

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

### References

- `High Level Test Specification` - [hdmicec_High-Level_TestSpecification.md](hdmicec_High-Level_TestSpecification.md)

## Level 2 Test Specification

The following functions are intended to test the `HDMI` `CEC` `HAL` module according to the `L2` Test specification.

### Test 1

| Title | Details |
| -- | -- |
| Function Name | `StateTransitions` |
| Description |This test validates the state machine behaviour of the HDMI CEC interface. It ensures that the interface transitions correctly between CLOSED and STARTED states, and that state change notifications are received by both the controller listener and the event listener|
| Test Group | 02 |
| Test Case ID | 001 |
| Priority | High |

**Pre-Conditions :**
None

**Dependencies :**
None

**User Interaction :**
If user chose to run the test in interactive mode, then the test case has to be selected via console.

**Test Procedure :**

| Variation / Steps | Description | Test Data | Expected Result | Notes |
|-------------------|-------------|-----------|----------------|-------|
| 1 | Register Event Listeners | None | Register the event listener ||
| 2 | Get the State before Open | None | State should be CLOSED ||
| 3 | Open the HDMI CEC with controller listener | controller listener handle | Function should return valid controller handle ||
| 4 | Get the State | None | State should be STARTED | |
| 5 | Check the event listener callback | | Previous State should be CLOSED and current state should be STARTED  ||
| 6 | Check the controller listener callback | | Previous State should be CLOSED and current state should be STARTED  ||
| 7 | Close the HDMI CEC | | Function should return `True` ||
| 8 | Get the State | None | State should be CLOSED | |
| 9 | Check the event listener callback | | Previous State should be STARTED and current state should be CLOSED  ||
| 10 | Check the controller listener callback | | Previous State should be STARTED and current state should be CLOSED  ||
| 11 | Unregister Event Listeners | | Function should return `True` ||

### Test 2

| Title | Details |
| -- | -- |
| Function Name | `metricValidation` |
| Description | This test validates HDMI CEC metric properties and HAL CEC version. It ensures that HAL_CEC_VERSION property is correctly reported and all defined metric properties (message counters, arbitration failures, etc.) are available and initialized to expected default values |
| Test Group | 02 |
| Test Case ID | 002 |
| Priority | High |

**Pre-Conditions :**
None

**Dependencies :**
None

**User Interaction :**
If user chose to run the test in interactive mode, then the test case has to be selected via console.

**Test Procedure :**

| Variation / Steps | Description | Test Data | Expected Result | Notes |
|-------------------|-------------|-----------|----------------|-------|
| 1 | Get HAL_CEC_VERSION property | None | Property should have valid value matching the HAL specification ||
| 2 | Get METRIC_DIRECTED_MESSAGES_SENT property | None | Property should be available, value should be 0 or -1 (not implemented) ||
| 3 | Get METRIC_BROADCAST_MESSAGES_SENT property | None | Property should be available, value should be 0 or -1 (not implemented) ||
| 4 | Get METRIC_DIRECTED_MESSAGES_SENT_AND_ACKED property | None | Property should be available, value should be 0 or -1 (not implemented) ||
| 5 | Get METRIC_BROADCAST_MESSAGE_SENT_AND_ACKED property | None | Property should be available, value should be 0 or -1 (not implemented) ||
| 6 | Get METRIC_ARBITRATION_FAILURES property | None | Property should be available, value should be 0 or -1 (not implemented) ||
| 7 | Open the HDMI CEC with controller listener | controller listener handle | Function should return valid controller handle ||
| 8 | Close the HDMI CEC | | Function should return `True` ||
| 9 | Get METRIC_DIRECTED_MESSAGES_SENT property | None | Property value should remain 0 or -1 (unchanged) ||
| 10 | Get METRIC_BROADCAST_MESSAGES_SENT property | None | Property value should remain 0 or -1 (unchanged) ||
| 11 | Get METRIC_DIRECTED_MESSAGES_SENT_AND_ACKED property | None | Property value should remain 0 or -1 (unchanged) ||
| 12 | Get METRIC_BROADCAST_MESSAGE_SENT_AND_ACKED property | None | Property value should remain 0 or -1 (unchanged) ||
| 13 | Get METRIC_ARBITRATION_FAILURES property | None | Property value should remain 0 or -1 (unchanged) ||

### Test 3

| Title | Details |
| -- | -- |
| Function Name | `logicalAddressesValidation` |
| Description | This test validates add/remove/get logical addresses functionality. It ensures that logical addresses can be added to the controller, retrieved via getLogicalAddresses(), and removed selectively or fully. It also verifies that the retrieved list matches expectations after each operation |
| Test Group | 02 |
| Test Case ID | 003 |
| Priority | High |

**Pre-Conditions :**
None

**Dependencies :**
None

**User Interaction :**
If user chose to run the test in interactive mode, then the test case has to be selected via console.

**Test Procedure :**

| Variation / Steps | Description | Test Data | Expected Result | Notes |
|-------------------|-------------|-----------|----------------|-------|
| 1 | Get logical addresses before opening | None | Logical addresses list should be empty ||
| 2 | Open the HDMI CEC with controller listener | controller listener handle | Function should return valid controller handle ||
| 3 | Add logical addresses | {0, 1} | Function should return `True` ||
| 4 | Get logical addresses | None | Should return {0, 1} ||
| 5 | Remove logical address | {0} | Function should return `True` ||
| 6 | Get logical addresses | None | Should return {1} only ||
| 7 | Remove logical address | {1} | Function should return `True` ||
| 8 | Get logical addresses | None | Logical addresses list should be empty ||
| 9 | Add logical addresses | {2, 3} | Function should return `True` ||
| 10 | Get logical addresses | None | Should return {2, 3} ||
| 11 | Remove logical addresses | {2, 3} | Function should return `True` ||
| 12 | Get logical addresses | None | Logical addresses list should be empty ||
| 13 | Add logical addresses | {0, 1} | Function should return `True` ||
| 14 | Get logical addresses | None | Should return {0, 1} ||
| 15 | Close the HDMI CEC | | Function should return `True` ||
| 16 | Get logical addresses after closing | None | Logical addresses list should be empty ||

### Test 4

| Title | Details |
| -- | -- |
| Function Name | `sendMessageValidation` |
| Description | This test validates sendMessage API with message transmission and callback verification. It ensures that messages of varying sizes can be sent over HDMI CEC after assigning logical addresses, and that the sent messages and status are correctly received via the HdmiCecEventListener callback. It also validates that messages up to maximum payload length (16 bytes) are supported, the status received via the callback matches expectations, and the message contents are preserved without corruption |
| Test Group | 02 |
| Test Case ID | 004 |
| Priority | High |

**Pre-Conditions :**
None

**Dependencies :**
None

**User Interaction :**
If user chose to run the test in interactive mode, then the test case has to be selected via console.

**Test Procedure :**

| Variation / Steps | Description | Test Data | Expected Result | Notes |
|-------------------|-------------|-----------|----------------|-------|
| 1 | Open the HDMI CEC with controller listener | controller listener handle | Function should return valid controller handle ||
| 2 | Add logical address | {0} | Function should return `True` ||
| 3 | Send directed CEC message | Message: {0x05, 0x04} | Function should return success status ||
| 4 | Wait for message callback | | onMessageSent callback should be triggered with message and status ||
| 5 | Verify callback message | | Message should match {0x05, 0x04} ||
| 6 | Verify callback status | | Status should be ACK_STATE_1 ||
| 7 | Send maximum length directed message | Message: {0x05, 0x64, 0x00, ..., 0x00} (16 bytes) | Function should return success status ||
| 8 | Wait for message callback | | onMessageSent callback should be triggered with message and status ||
| 9 | Verify callback message | | Message should match the 16-byte message sent ||
| 10 | Verify callback status | | Status should be ACK_STATE_1 ||
| 11 | Send broadcast CEC message | Message: {0x0F, 0x85} | Function should return success status ||
| 12 | Wait for message callback | | onMessageSent callback should be triggered with message and status ||
| 13 | Verify callback message | | Message should match {0x0F, 0x85} ||
| 14 | Verify callback status | | Status should be ACK_STATE_0 ||
| 15 | Remove logical address | {0} | Function should return `True` ||
| 16 | Close the HDMI CEC | | Function should return `True` ||