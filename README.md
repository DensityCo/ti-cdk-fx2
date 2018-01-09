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
repo init -m fx2cdk.xml -u git://github.com/DensityCo/ti-cdk-fx2.git
repo sync
```

# Build Tool-chain
```bash
ant bootstrap
```

# Build FX2LIB Reference Firmware
```bash
ant build
```

# Build the FX2 Loader
```bash
ant build_loader
```

# Load the Firmware Onto FX2 Development Kit
Note that this step requires updated udev rules.
```bash
ant load
```
