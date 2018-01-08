BASE.DIR=$(PWD)
FX2PROGRAMMER.DIR=$(BASE.DIR)/cycfx2prog-0.47
FX2PROGRAMMER.BIN=$(FX2PROGRAMMER.DIR)/cycfx2prog
PREBUILTS=$(BASE.DIR)/prebuilts
FX2.TI.V17.FIRMWARE.BIN=$(PREBUILTS)/fx2/OPT8241-CDK-FX2-FW_0v17.iic
FIRMWARE.BUILD.DIR=$(BASE.DIR)/build.firmware
FIRMWARE.BIN=$(FIRMWARE.BUILD.DIR)/fx2_main.ihx
BUILD.PREFIX=$(BASE.DIR)/build
BUILD.LIBFTDI=$(BUILD.PREFIX).libftdi
SRC.LIBFTDI=$(BASE.DIR)/libftdi
CMAKE.BIN=cmake
HOST.INSTALLED=$(BASE.DIR)/host.installed
BIN.DIR=$(HOST.INSTALLED)/bin
SRC.EEPROM=$(BASE.DIR)/eeprom
BUILD.EEPROM=$(BUILD.PREFIX).eeprom
EEPROM.BIN=$(BIN.DIR)/eeprom
FTDIEEPROM.BIN=$(BIN.DIR)/ftdi_eeprom
FTDIEEPROM.CFG=$(SRC.FTDIEEPROM)/fx2.conf
BUILD.FTDIEEPROM=$(BUILD.PREFIX).ftdi_eeprom
SRC.FTDIEEPROM=$(BASE.DIR)/ftdi_eeprom
SRC.DOCOPT=$(BASE.DIR)/docopt
BUILD.DOCOPT=$(BUILD.PREFIX).docopt
SDCC.INSTALLED=$(HOST.INSTALLED)/sdcc
SRC.SDCC=$(BASE.DIR)/sdcc
SRC.FX2LIB=$(BASE.DIR)/fx2lib
TARGET.INSTALLED=$(BASE.DIR)/target.installed
SRC.GETTEXT=$(BASE.DIR)/gettext
SRC.LIBCONFUSE=$(BASE.DIR)/libconfuse
SRC.LIBUSB=$(BASE.DIR)/libusb
SRC.I2C=$(BASE.DIR)/ftdi-i2c
BUILD.I2C=$(BUILD.PREFIX).ftdi-i2c

#bootstrap: sdcc docopt hex2bix fx2lib

docopt: .FORCE
	rm -rf $(BUILD.DOCOPT) && mkdir $(BUILD.DOCOPT) && cd $(BUILD.DOCOPT) && $(CMAKE.BIN) -DCMAKE_INSTALL_PREFIX=$(HOST.INSTALLED) -DCMAKE_BUILT_TYPE=Debug -DCMAKE_INCLUDE_PATH=$(HOST.INSTALLED)/include -DCMAKE_LIBRARY_PATH=$(HOST.INSTALLED)/lib  $(SRC.DOCOPT) && make install

sdcc: .FORCE
	rm -rf $(SDCC.INSTALLED) && cd $(SRC.SDCC)/sdcc && ./configure --prefix=$(SDCC.INSTALLED) && make -j8 install

target.installed: $(TARGET.INSTALLED)/lib
	mkdir $(TARGET.INSTALLED)/lib

fx2lib: .FORCE 	
	cd $(SRC.FX2LIB)/fw && PATH=$(PATH):$(SDCC.INSTALLED)/bin make all && cp $(SRC.FX2LIB)/lib/fx2.lib $(TARGET.INSTALLED)/lib

flash.fx2.ti17: .FORCE
	sudo $(FX2PROGRAMMER.BIN) -id=0451.9105 reset prg:$(FX2.TI.V17.FIRMWARE.BIN) run

flash.fx2.density: .FORCE
	sudo $(FX2PROGRAMMER.BIN) -id=0451.9105 reset prg:$(FIRMWARE.BIN) run

gettext: .FORCE
	cd $(SRC.GETTEXT) && ./autogen.sh && ./configure --disable-java --prefix=$(HOST.INSTALLED) && make -j8 install

libconfuse: .FORCE
	cd $(SRC.LIBCONFUSE) && ./autogen.sh && ./configure --prefix=$(HOST.INSTALLED) && make -j8 install

libusb: .FORCE
	cd $(SRC.LIBUSB) && ./autogen.sh && ./configure  --prefix=$(HOST.INSTALLED) && make -j8 install

i2c: .FORCE
	rm -rf $(BUILD.I2C) && mkdir $(BUILD.I2C) && cd $(BUILD.I2C) && $(CMAKE.BIN) $(SRC.I2C) -DCMAKE_INSTALL_PREFIX=$(HOST.INSTALLED) -DBUILD_TESTS=OFF -DDOCUMENTATION=OFF -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INCLUDE_PATH=$(HOST.INSTALLED)/include -DCMAKE_CROSSCOMPILING=ON -DLIBFTDI_ROOT_DIR=$(HOST.INSTALLED) -DCMAKE_LIBRARY_PATH=$(HOST.INSTALLED)/lib  && make install



libftdi: .FORCE
	rm -rf $(BUILD.LIBFTDI) && mkdir $(BUILD.LIBFTDI) && cd $(BUILD.LIBFTDI) && $(CMAKE.BIN) $(SRC.LIBFTDI) -DCMAKE_INSTALL_PREFIX=$(HOST.INSTALLED) -DBUILD_TESTS=OFF -DDOCUMENTATION=OFF -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INCLUDE_PATH=$(HOST.INSTALLED)/include -DCMAKE_CROSSCOMPILING=ON -DLIBFTDI_ROOT_DIR=$(HOST.INSTALLED) -DCMAKE_LIBRARY_PATH=$(HOST.INSTALLED)/lib  && make install

eeprom: .FORCE
	rm -rf $(BUILD.EEPROM) && mkdir $(BUILD.EEPROM) && cd $(BUILD.EEPROM) && $(CMAKE.BIN) $(SRC.EEPROM) -DCMAKE_INSTALL_PREFIX=$(HOST.INSTALLED) -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INCLUDE_PATH=$(HOST.INSTALLED)/include && make install


run.eeprom.help: .FORCE
	LD_LIBRARY_PATH=$(HOST.INSTALLED)/lib $(FTDIEEPROM.BIN) --help

run.eeprom: .FORCE
	LD_LIBRARY_PATH=$(HOST.INSTALLED)/lib $(FTDIEEPROM.BIN) --device i:0x0403:0x6015 --read-eeprom $(FTDIEEPROM.CFG)

ftdi.eeprom: .FORCE
	rm -rf $(BUILD.FTDIEEPROM) && mkdir $(BUILD.FTDIEEPROM) && cd $(BUILD.FTDIEEPROM) && $(CMAKE.BIN) $(SRC.FTDIEEPROM) -DCMAKE_INSTALL_PREFIX=$(HOST.INSTALLED) -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INCLUDE_PATH=$(HOST.INSTALLED)/include && make install


.FORCE:

