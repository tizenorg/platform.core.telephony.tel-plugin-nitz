#!/bin/sh
#------------------------
# tel-plugin-nitz
#------------------------

## Launch test0
function launch_test0() {
#	vconftool set -t int memory/private/telephony/nitz_test0 1 -i -f -s tizen::vconf::platform::rw
}

## Launch test1
function launch_test1() {
#	vconftool set -t string memory/private/telephony/nitz_test1 "$1" -i -f -s tizen::vconf::platform::rw
}

## Launch test2
function launch_test2() {
#	vconftool set -t int memory/private/telephony/nitz_test2 1 -i -f -s tizen::vconf::platform::rw
}

launch_test0

launch_test1 "310471"

launch_test2
