# Testing Suite For HDMI CEC HAL

## Build Steps

### Build linux binder

The test suite depends on the Linux binder with `AIDL` support. Follow these steps to build it:

For detail build instruction, please refer to [Linux Binder](https://github.com/rdk-e/linux_binder_idl/tree/main)

Here outline the simplest steps are

```bash
git clone -b develop 'https://github.com/rdkcentral/linux_binder_idl.git'

cd linux_binder_idl

./build-linux-binder-aidl.sh 64
```

### Build VTS test

Clone and build the test suite:

```bash
git clone git@github.com:rdkcentral/rdk-halif-binder-test-cec.git
```

```bash
cd rdk-halif-binder-test-cec
```

Set below environment variable:

- AIDL_BIN: Path to the `AIDL` compiler (e.g., /path/to/linux_binder_idl/out/aidl).

- LINUX_BINDER_OUT_DIR: Path to the Linux binder output directory (e.g., /path/to/linux_binder_idl/local).

- UT_CORE_VERSION: Specific version of the UT-Core framework (e.g., 4.7.0). Defaults to the latest tagged version.

- VCOMPONENT_HDMI_CEC_VERSTION: Specific version of the HDMI CEC Virtual Component (e.g 1.0.0). Defaults to the latest HEAD.

- RDK_HALIF_AIDL_VERSION: Specific version of the AIDL HAL Specification (e.g, 0.7.1). Defaults to the latest HEAD.

Example:

```bash
export UT_CORE_VERSION=4.7.0
export RDK_HALIF_AIDL_VERSION=0.7.1
export VCOMPONENT_HDMI_CEC_VERSTION=1.0.0
export AIDL_BIN=/path/linux_binder_idl/out/aidl
export LINUX_BINDER_OUT_DIR=/path//linux_binder_idl/local/
```

By default, UT logs are disabled. To enable them, Set below environment variable

```bash
export ENABLE_UT_INFO_LOGGING=1
```

Run the build script:

```bash
./build.sh Target=linux
```

The build.sh script:

- Clones the `rdk-halif-aidl` repository and UT-Core framework.

- Compiles the test suite.

- Generates binaries in the `build/bin` directory.

- Generates tar file of the bin folder `rdk-halif-binder-test-cec.tar.gz` in the `build/bin` directory.

## Run test

### Transfer tar file to Target Device:

Copy the tar-ball to the target device (e.g., using `scp`):

```bash
scp -P <port> rdk-halif-binder-test-cec.tar.gz root@target-device:/tmp/

```

### Extract on Target Device:

Log into the target device and extract the tar-ball:

```bash
ssh root@target-device
mkdir /tmp/HdmiCec
tar -zxf rdk-halif-binder-test-cec.tar.gz -C /tmp/HdmiCec
cd /tmp/HdmiCec
```

### Run the VTS Test on Target Device

If the target is vDevice and if `HdmiCec` service is not running, then run and configure the vComponent

```bash
#Run the vComponent service
./Vendor-HdmiCec &

#Configure the vComponent
./hdmicec_post_command.sh vcomponent_configurations/commands/hdmicec_device_map_init.yaml 8080
```

Run below command to execute the VTS Test:

```bash
./run.sh -p profiles/hfp-hdmicec_vcomponent.yaml
```

## Setting Python environment for running the L1, L2 and L3 automation test cases

- For running the L1 L2 and L3 test suite, a host PC with a Python environment is required.
- Install Python Environment and Activation Scripts as detailed in [Python Installation](https://github.com/rdkcentral/ut-raft/wiki/Guide-for-Setting-up-the-Python-RAFT-Testing-Suite)
- To run the `L1`, `L2` and `L3` test cases of AV Modules follow the `L1` `L2` and `L3` Test Procedure document - [vts_hdmicec_L1_L2_L3_TestProcedure.md](docs/pages/vts_hdmicec_L1_L2_L3_TestProcedure.md)
