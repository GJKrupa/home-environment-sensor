package main

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"log"
	"math"
	"net/http"
	"time"

	"github.com/paypal/gatt"
	"github.com/paypal/gatt/examples/option"
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

func onStateChanged(d gatt.Device, s gatt.State) {
	fmt.Println("State:", s)
	switch s {
	case gatt.StatePoweredOn:
		fmt.Println("Scanning...")
		d.Scan([]gatt.UUID{}, false)
		return
	default:
		d.StopScanning()
	}
}

func onPeriphDiscovered(p gatt.Peripheral, a *gatt.Advertisement, rssi int) {

	var isOk = false
	for _, svc := range a.Services {
		if svc.String() == SERVICE_UUID {
			isOk = true
		}
	}

	if isOk {
		fmt.Printf("Connecting to: '%s'...\n", p.Name())
		// Stop scanning once we've got the peripheral we're looking for.
		p.Device().StopScanning()

		fmt.Printf("\nPeripheral ID:%s, NAME:(%s)\n", p.ID(), p.Name())
		fmt.Println("  Local Name        =", a.LocalName)
		fmt.Println("  TX Power Level    =", a.TxPowerLevel)
		fmt.Println("  Manufacturer Data =", a.ManufacturerData)
		fmt.Println("  Service Data      =", a.ServiceData)
		fmt.Println("")

		p.Device().Connect(p)
	}
}

func onPeriphConnected(p gatt.Peripheral, err error) {
	fmt.Println("Connected")
	defer p.Device().CancelConnection(p)

	if err := p.SetMTU(500); err != nil {
		fmt.Printf("Failed to set MTU, err: %s\n", err)
	}

	// Discovery services
	ss, err := p.DiscoverServices(nil)
	if err != nil {
		fmt.Printf("Failed to discover services, err: %s\n", err)
		return
	}

	if len(ss) == 0 {
		fmt.Printf("No services found!")
	}

	var values = map[string]float64{}

	for _, s := range ss {
		if s.UUID().String() == SERVICE_UUID {
			// Discovery characteristics
			cs, err := p.DiscoverCharacteristics(nil, s)
			if err != nil {
				fmt.Printf("Failed to discover characteristics, err: %s\n", err)
				continue
			}

			if len(cs) == 0 {
				fmt.Printf("No characteristics found!")
			}

			for _, c := range cs {
				if char_uuids[c.UUID().String()] != "" {
					name := char_uuids[c.UUID().String()]
					// Read the characteristic, if possible.
					if (c.Properties() & gatt.CharRead) != 0 {
						b, err := p.ReadCharacteristic(c)
						bits := binary.LittleEndian.Uint64(b)
						float := math.Float64frombits(bits)
						if err != nil {
							fmt.Printf("Failed to read characteristic, err: %s\n", err)
							continue
						}

						values[name] = float

					}
				}
			}

			fmt.Println()
		}
	}

	go sendToTick(p.Name(), values)
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

func onPeriphDisconnected(p gatt.Peripheral, err error) {
	fmt.Println("Disconnected")
	p.Device().Scan([]gatt.UUID{}, false)
}

func main() {
	d, err := gatt.NewDevice(option.DefaultClientOptions...)
	if err != nil {
		log.Fatalf("Failed to open device, err: %s\n", err)
		return
	}

	// Register handlers.
	d.Handle(
		gatt.PeripheralDiscovered(onPeriphDiscovered),
		gatt.PeripheralConnected(onPeriphConnected),
		gatt.PeripheralDisconnected(onPeriphDisconnected),
	)

	d.Init(onStateChanged)
	<-done
	fmt.Println("Done")
}
