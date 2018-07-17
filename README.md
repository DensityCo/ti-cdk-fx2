# ti-cdk-fx2
TI CDK FX2 Firmware - top level project

# Setup Environment (Ubuntu 17.04)
```bash
sudo apt-get install build-essential repo ant git bison flex gputils texinfo cmake libboost-graph-dev libusb-1.0-0-dev libusb-dev gperf autopoint
```

# Setup Udev Rules
Create file /etc/udev/rules.d and add contents:
```bash
SUBSYSTEM=="usb", ATTR{idVendor}=="0451", MODE="0664", GROUP="plugdev"
```

Add your user to plugdev and restart udev.
```bash
sudo adduser $USER plugdev
sudo service udev restart
```

# Setup Environment (OSX)
TBD

# Get the source
```bash
repo init -m fx2cdk.xml -u git://github.com/DensityCo/ti-cdk-fx2.git -b trunk
repo sync
```

# Build Tool-chain
```bash
ant bootstrap
```

# Build Firmware for Dev purposes
To build the fully functional ram-loadable version of the FX2 firmware run the following build command.
```bash
ant firmware.ramload
```

# Build Firmware for Release
To build an eye-safe version of the FX2 firmware for uncalibrated TOF modules, run the following build command.
```bash
ant firmware.safe.eeprom
```

The resulting FX2 image will be placed under the directory build.firmware

The resulting host executable binary is located under the directory host.installed/bin

# Load the Firmware Onto FX2 Development Kit
Note that this step requires updated udev rules.
```bash
make flash.fx2.density
```

# Push up your changes
```bash
git add files
git commit
git push githubssh HEAD:yourbranch
```
