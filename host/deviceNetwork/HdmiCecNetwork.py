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

class HdmiCecNetwork:
    """
    Wrapper class to instantiate the correct HDMI CEC implementation.
    """

    def __init__(self, mode: str, device_type: list, session, process_name: str="",bin_path: str="", prompt="~#", port=8080):
        """
        Initializes the class
        Args:
            mode (str): "vDevice" for virtual HDMI CEC, "aDevice" for actual HDMI CEC
            device_type (list): device type
            session: console session
            process_name (str, optional): Process name of HDMI CEC binary. Defaults to "".
            bin_path (str, optional): Path to HDMI CEC binary. Defaults to "".
            prompt (str, optional): Command prompt string. Defaults to "~#".
            port (int, optional): Port number for HDMI CEC communication. Defaults to 8080.
        """
        if mode == "vDevice":
            from .vHdmiCec import vHdmiCec
            self.cec = vHdmiCec(device_type, session, process_name, bin_path, prompt, port)
        else:
            #TODO: Add actual HDMI CEC implementation here
            raise NotImplementedError(f"HDMI CEC mode '{mode}' is not implemented yet.")

    def __getattr__(self, name):
        """Delegate method/attribute access to underlying instance."""
        return getattr(self.cec, name)