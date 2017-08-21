# ti-cdk-fx2
TI CDK FX2 Firmware - top level project

# Setup Environment (Ubuntu 17.04)
```bash
sudo apt-get install build-essential repo ant git libboost-graph-dev libusb-1.0-0-dev
```

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

The reference fx2lib build artifact lives in fx2lib/fw/build.


