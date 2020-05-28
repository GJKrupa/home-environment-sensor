package main

import (
	"bytes"
	"context"
	"encoding/binary"
	"fmt"
	"log"
	"math"
	"net/http"
	"time"

	"github.com/go-ble/ble"
	"github.com/go-ble/ble/examples/lib/dev"
)

var done = make(chan struct{})

const TELEGRAF_ENDPOINT = "http://tick-stack.localdomain:8186/write"

const SERVICE_UUID = "4fafc2011fb5459e8fccc5c9c331914b"

var char_uuids = map[string]string{
	"beb5483e36e14688b7f5ea07361b26a8": "temperature",
	"5b1363f09f5711eaa61ff32afc9842aa": "battery",
	"7649df809f5911ea8bb5a35ae24ada32": "rain",
	"7cd6d0609f5911ea8bb5a35ae24ada32": "humidity",
	"8aabebd09f5911ea8bb5a35ae24ada32": "pressure"}

var names = map[string]string{}

func explore(cln ble.Client, p *ble.Profile) {
	name := names[cln.Addr().String()]
	fmt.Printf("Connected to %s\n", name)
	defer func() {
		if r := recover(); r != nil {
			fmt.Printf("Panic: %+v\n", r)
		}
	}()

	var values = map[string]float64{}

	for _, svc := range p.Services {
		if svc.UUID.String() == SERVICE_UUID {
			// Discovery characteristics
			for _, c := range svc.Characteristics {
				if char_uuids[c.UUID.String()] != "" {
					name := char_uuids[c.UUID.String()]
					// Read the characteristic, if possible.
					if (c.Property & ble.CharRead) != 0 {
						b, err := cln.ReadCharacteristic(c)
						bits := binary.LittleEndian.Uint64(b)
						float := math.Float64frombits(bits)
						if err != nil {
							fmt.Printf("Failed to read characteristic, err: %s\n", err)
							continue
						} else {
							fmt.Printf("Read characteristic %s\n", c.UUID.String())
						}

						values[name] = float
					} else {
						fmt.Printf("No permission to read characteristic %s\n", c.UUID.String())
					}
				} else {
					fmt.Printf("Unknown characteristic %s\n", c.UUID.String())
				}
			}
		}
	}

	go sendToTick(name, values)
}

func sendToTick(name string, values map[string]float64) {
	now := time.Now().UnixNano()
	for key, val := range values {
		msg := fmt.Sprintf("%s,room=%s value=%.2f %d", key, name, val, now)
		resp, _ := http.Post(TELEGRAF_ENDPOINT, "text/plain", bytes.NewBufferString(msg))
		fmt.Println(resp.Status)
		fmt.Println(msg)
	}
}

func main() {
	d, err := dev.NewDevice("default")
	if err != nil {
		log.Fatalf("can't new device : %s", err)
	}
	ble.SetDefaultDevice(d)

	filter := func(a ble.Advertisement) bool {
		for _, svc := range a.Services() {
			if svc.String() == SERVICE_UUID {
				fmt.Printf("Found %s (%s)\n", a.Addr().String(), a.LocalName())
				if a.LocalName() != "" {
					names[a.Addr().String()] = a.LocalName()
					return true
				}
			}
		}
		return false
	}

	for true {
		fmt.Println("Scanning...")
		ctx := ble.WithSigHandler(context.WithTimeout(context.Background(), 2*time.Minute))
		cln, err := ble.Connect(ctx, filter)
		if err != nil {
			log.Printf("can't connect : %s", err)
			continue
		}

		// Make sure we had the chance to print out the message.
		done := make(chan struct{})
		// Normally, the connection is disconnected by us after our exploration.
		// However, it can be asynchronously disconnected by the remote peripheral.
		// So we wait(detect) the disconnection in the go routine.
		go func() {
			<-cln.Disconnected()
			fmt.Printf("[ %s ] is disconnected \n", cln.Addr())
			close(done)
		}()

		fmt.Printf("Discovering profile...\n")
		p, err := cln.DiscoverProfile(true)
		if err != nil {
			log.Printf("can't discover profile on %s: %s", cln.Addr().String(), err)
		} else {
			// Start the exploration.
			explore(cln, p)
		}

		// Disconnect the connection. (On OS X, this might take a while.)
		fmt.Printf("Disconnecting [ %s ]... (this might take up to few seconds on OS X)\n", cln.Addr())
		cln.CancelConnection()

		<-done
		fmt.Println("Done")
	}
}
