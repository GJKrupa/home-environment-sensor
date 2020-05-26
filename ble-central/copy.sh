#!/usr/bin/env bash
DIR=/home/pi/pi-ble
echo "Copying..."
scp -r src Makefile "pi@pi-ble:${DIR}/"

