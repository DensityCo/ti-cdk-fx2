#!/bin/bash -e
# $1 iic file to program into the fx2.
# Note - You may need sudo access to run this script and have permission
# to connect and program over USB, just depends on how your udev rules are setup.
# example script call sudo ./opt9221PrgmBurnInTest.sh 001 050 OPT9221_0v29.tie

export QUIT=0

ctrl_c() {
	QUIT=1
}

run_test() {
	counter=1
	while [ $QUIT -eq 0 ]
	do
		echo "**************************** Test Pass $counter *****************************************"
		if ./cycfx2prog -id=0451.9105 prgFX2:$2 && ./cycfx2prog -id=0451.9105 prgFX2:$1
		then
			echo "Reading the EEPROM back to see what actually was programmed"
			./cycfx2prog -id=0451.9105 readfx2:11058,0,eeprom_contents.bin
			if ! diff eeprom_contents.bin $1
			then
				echo "  Files failed to compare. Test Failed."
				break
			fi

		    echo "  Test passed."
		else
		    echo "  Test failed."
		    break
		fi
		((counter++))
	done
}

# This trap allows ctrl-c to stop the test and report time.
trap ctrl_c INT

# Create a blank file of zeroes, so we can clear out the program each time
blank_file_size=$(stat -c%s "$1")
dd if=/dev/zero of=blank.iic bs=1 count=$blank_file_size

time run_test $1 blank.iic
# do one last programming cycle to make sure the board isn't left in a blank state
./cycfx2prog -id=0451.9105 prgFX2:$1
