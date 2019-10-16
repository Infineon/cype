# Power Estimator
Version 1.1.0

### Overview 
The Power Estimator (CyPE) tool provides an estimate of power consumed by a target device (also called platform). The tool is supported on Windows, Linux, and macOS for CY8CKIT_062S2_43012.
CyPE is supported on Mbed OS.

### Features
* Capture feature displays the plot of power-consuming events with respect to time on the target device.
* Import feature is used to import a power log file that was created by the live analysis (capture feature) in a separate session. The tool creates the power plot using the power events captured in the log file for offline analysis.
* Supports power estimation for PSoC 6 MCU and Wi-Fi only. The power estimation for USER_LEDs is not supported.
* CyPE tool estimates only power that is not actual power. To get an actual power number, use a hardware tool such as Agilent.

### Quick Start
1. From the command-line, import the mbed-os application. For example, mbed-os-example-blinky.

2. Clone the CyPE repo that houses the Cypress Power Estimator.

```
cd mbed-os-example-blinky/mbed-os/targets/TARGET_Cypress/TARGET_PSOC6/
git clone https://github.com/cypresssemiconductorco/cype.git TARGET_CYPE
```

3. Enable `“CYPE”` as shown in mbed-os-example-blinky\mbed-os\targets\targets.json

```
"CY8CMOD_062S2_43012": {
        "inherits": ["MCU_PSOC6_M4"],
        "features": ["BLE"],
        "device_has_remove": ["ANALOGOUT"],
        "extra_labels_add": [
            "PSOC6_02",
            "CM0P_SLEEP",
            "CYPE",
            "WHD",
            "43012",
            "CYW43XXX",
            "CORDIO"
        ],
        "macros_add": ["CY8C624ABZI_D44", "CYBSP_WIFI_CAPABLE"],
        "public": false,
        "overrides": {
            "network-default-interface-type": "WIFI"
        }
    },
```
4. To perform Wi-Fi power estimation, set `“cype-wifi-estimation-enabled”` to true as shown in TARGET_CYPE\mbed_lib.json

```
   "target_overrides": {
        "*": {
            "cpu-stats-enabled": true,
            "cype-macro-enabled": true,
	        "cype-wifi-estimation-enabled": true
	    }
    }
```
Enable this flag for only Wi-Fi apps. Power estimation for Wi-Fi apps can be done after connecting to an AP.

5. Build and flash the mbed-os-example-blinky app with CyPE support (.hex binary).

Invoke `mbed compile`, and specify the name of your platform (CY8CKIT_062S2_43012) and your favorite toolchain (`GCC_ARM`, `ARM`, `IAR`). For example, for the GCC_ARM Compiler:

```
mbed compile --target CY8CKIT_062S2_43012 --toolchain GCC_ARM -f
```
6. Launch the CyPE Tool. 

* ModusToolbox 2.0 or above should be used to obtain Power Estimator tool.
* To run the CyPE tool, navigate to the install location and run the executable. The default install location on Windows is: <MTB_install_dir>\tools_<version>\cype-tool
* Go to Help->View help to get the user guide. 
* The platform power estimator database file is available at mbed-os-example-blinky/mbed-os/targets/TARGET_Cypress/TARGET_PSOC6/TARGET_CYPE/platforms/TARGET_CY8CKIT_062S2_43012/cype_power_data/CY8CKIT_062S2_43012.xml.
* CyPE locks the deepsleep until power logging is started.

### Measuring Real Current on CY8CKIT_062S2_43012

* User can measure current for PSoC 6 at J15. Ensure that J25 is removed to prevent current leakage. PSoC 6 current includes current consumption by USER_LEDs also.
* User can measure current for CYW43012 at J8.
* Refer CY8CKIT_062S2_43012 schematics and user guide for more details.

### More information

 * [ModusToolbox Software Environment, Quick Start Guide, Documentation, and Videos](https://www.cypress.com/products/modustoolbox-software-environment)
 * [Power Estimator User Guide] (http://www.cypress.com/ModusToolboxCyPEConfig)

---
© Cypress Semiconductor Corporation, 2019.