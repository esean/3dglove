#!/bin/bash
#
# Lights all LEDs on any gloves founds
#
# PASS
#   $1 = channel intensity 0-4095
#

NUM_CHAN=23 # 24-1
INTENSITY_TEST=${1:-100}

#########################################
die() { echo -e "\nERROR: $0: $@" >&2; exit 1; }
dlog() { echo -e "DEBUG: $@"; }
# PASS: $1 = intensity
# sets all channels to intensity
turn_all_channels_on_at()
{
    [ -z "$1" ] && die "ASSERT:turn_all_channels_on_at"
    TXT=''
    for i in `seq 0 $NUM_CHAN`; do
        TXT="${TXT}$i $1 "
    done
    echo $TXT
}

run_ftdi_cmd()
{
    dlog "Running FTDI cmd: ./set_ftdi $@"
    [ ! -f set_ftdi ] && die "Cannot find set_ftdi, run make"
    ./set_ftdi $@
}
#########################################


run_ftdi_cmd `turn_all_channels_on_at $INTENSITY_TEST`

