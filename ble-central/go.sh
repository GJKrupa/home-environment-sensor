#!/usr/bin/env bash
DIR=/home/pi/pi-ble
echo "Copying..."
scp -r src Makefile "pi@pi-ble:${DIR}/"
echo "Building..."
ssh pi@pi-ble "cd ${DIR} && make"
echo "Running..."
ssh pi@pi-ble "cd ${DIR} && sudo ./main garden"
