#!/bin/bash
# $1 is the three digit USB bus ID.
# $2 is the three digit USB Device ID.
# Note - You may need sudo access to run this script and have permission
# to connect and program over USB, just depends on how your udev rules are setup.
# example script call sudo ./opt9221PrgmBurnInTest.sh 001 050

export QUIT=0

ctrl_c() {
	QUIT=1
}

run_test() {
	counter=1
	while [ $QUIT -eq 0 ]
	do
		echo "**************************** Test Pass $counter *****************************************"
		if ./cycfx2prog -d=$1.$2 prg9221:/home/cfisher/density/emb.opt9221firmware/firmware/OPT9221_Fw_0v29/OPT9221_0v29.tie
		then
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

time run_test $1 $2
