BASE.DIR=$(PWD)
FX2PROGRAMMER.DIR=$(BASE.DIR)/cycfx2prog-0.47
FX2PROGRAMMER.BIN=$(FX2PROGRAMMER.DIR)/cycfx2prog
PREBUILTS=$(BASE.DIR)/prebuilts
FX2.TI.V17.FIRMWARE.BIN=$(PREBUILTS)/fx2/OPT8241-CDK-FX2-FW_0v17.iic

flash.fx2.ti17: .FORCE
	sudo $(FX2PROGRAMMER.BIN) -id=0451.9105 reset prg:$(FX2.TI.V17.FIRMWARE.BIN) run

.FORCE:

