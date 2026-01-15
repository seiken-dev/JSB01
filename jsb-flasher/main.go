package main

import (
	"crypto/sha256"
	"encoding/hex"
	"flag"
	"fmt"
	"io"
	"os"
	"path/filepath"
	"strings"
	"time"

	"go.bug.st/serial"
	"go.bug.st/serial/enumerator"
	"golang.org/x/sys/windows"
)

// Constants matching the Python script
const (
	PicoVID      = "2E8A" // Raspberry Pi Pico VID
	BootselLabel = "RPI-RP2"
	ExpectedHash = "45dda35abb9af37ac8a6531fe67b2391f7413ce485b9bb82eda4f4b4778b345c"
)

func main() {
	portFlag := flag.String("p", "", "Serial port name")
	flag.Parse()

	args := flag.Args()
	if len(args) > 1 {
		fmt.Println("Error: Too many arguments.")
		fmt.Printf("Usage: %s [-p PORT] [firmware.uf2]\n", filepath.Base(os.Args[0]))
		os.Exit(1)
	}

	var uf2Path string

	if len(args) == 1 {
		uf2Path = args[0]
	} else {
		// Try to find firmware.uf2 in the executable directory
		ex, err := os.Executable()
		if err != nil {
			panic(err)
		}
		baseDir := filepath.Dir(ex)
		uf2Path = filepath.Join(baseDir, "firmware.uf2")
		fmt.Printf("[*] Looking for firmware at: %s\n", uf2Path)
	}

	if _, err := os.Stat(uf2Path); os.IsNotExist(err) {
		fmt.Printf("Error: File not found: %s\n", uf2Path)
		os.Exit(1)
	}

	// --- 0. Hash Check (Only if file was automatically selected) ---
	if len(args) == 0 {
		fmt.Println("[*] Verifying firmware integrity (sha256)...")
		currentHash, err := calculateFileHash(uf2Path)
		if err != nil {
			fmt.Printf("Error calculating hash: %v\n", err)
			os.Exit(1)
		}

		if currentHash != ExpectedHash {
			fmt.Println("[X] CRITICAL ERROR: Hash mismatch!")
			fmt.Printf("    Expected: %s\n", ExpectedHash)
			fmt.Printf("    Actual:   %s\n", currentHash)
			fmt.Println("[X] This firmware may be corrupted or tampered with. Aborting.")
			os.Exit(1)
		}
		fmt.Println("[+] Verification successful. Proceeding...")
	}

	// 1. Find Serial Port
	var portName string
	if *portFlag != "" {
		portName = *portFlag
	} else {
		portName = findPicoSerial()
	}

	if portName != "" {
		// 2. Enter BOOTSEL mode
		enterBootselMode(portName)
	} else {
		fmt.Println("[!] Pico serial port not found. Checking if already in BOOTSEL mode...")
	}

	// 3. Wait for Drive
	mountPoint := findPicoDrive(10 * time.Second)
	if mountPoint == "" {
		os.Exit(1)
	}

	// 4. Flash Firmware
	flashFirmware(uf2Path, mountPoint)
}

func calculateFileHash(filePath string) (string, error) {
	f, err := os.Open(filePath)
	if err != nil {
		return "", err
	}
	defer f.Close()

	hasher := sha256.New()
	if _, err := io.Copy(hasher, f); err != nil {
		return "", err
	}

	return hex.EncodeToString(hasher.Sum(nil)), nil
}

func findPicoSerial() string {
	ports, err := enumerator.GetDetailedPortsList()
	if err != nil {
		fmt.Printf("Error enumerating ports: %v\n", err)
		return ""
	}

	var foundPorts []string
	for _, port := range ports {
		// Check both VID formats just in case (upper/lower)
		if strings.EqualFold(port.VID, PicoVID) {
			foundPorts = append(foundPorts, port.Name)
		}
	}

	if len(foundPorts) == 0 {
		return ""
	}

	if len(foundPorts) > 1 {
		fmt.Printf("[!] Error: Multiple RP2040 devices detected (%s).\n", strings.Join(foundPorts, ", "))
		fmt.Println("    Please connect only one device to ensure the correct one is flashed.")
		os.Exit(1)
	}

	return foundPorts[0]
}

func enterBootselMode(portName string) {
	fmt.Printf("[*] Resetting Pico on %s (1200bps trick)...\n", portName)
	mode := &serial.Mode{
		BaudRate: 1200,
	}

	// Open and close at 1200bps triggers BOOTSEL
	port, err := serial.Open(portName, mode)
	if err != nil {
		// In Python code, exception is ignored as device might disappear during init or race condition
		// We'll just log it lightly and proceed
		// fmt.Printf("Note: Failed to open port (might strictly be okay): %v\n", err)
	} else {
		port.Close()
	}
	time.Sleep(1 * time.Second)
}

func findPicoDrive(timeout time.Duration) string {
	fmt.Printf("[*] Waiting for %s drive to mount...", BootselLabel)
	deadline := time.Now().Add(timeout)

	for time.Now().Before(deadline) {
		// Check drives D...Z
		for _, drive := range "DEFGHIJKLMNOPQRSTUVWXYZ" {
			driveRoot := string(drive) + ":\\"

			// Check volume label
			label, err := getVolumeLabel(driveRoot)
			if err == nil && label == BootselLabel {
				fmt.Printf("\n[+] Found Pico drive at: %s\n", driveRoot)
				return driveRoot
			}
		}

		fmt.Print(".")
		time.Sleep(1 * time.Second)
	}
	fmt.Println("\n[!] Timeout: Pico drive not found.")
	return ""
}

func getVolumeLabel(driveRoot string) (string, error) {
	pathPtr, err := windows.UTF16PtrFromString(driveRoot)
	if err != nil {
		return "", err
	}

	var volumeName [256]uint16
	err = windows.GetVolumeInformation(
		pathPtr,
		&volumeName[0],
		uint32(len(volumeName)),
		nil,
		nil,
		nil,
		nil,
		0,
	)
	if err != nil {
		return "", err
	}

	return windows.UTF16ToString(volumeName[:]), nil
}

func flashFirmware(uf2Path, mountPoint string) {
	destPath := filepath.Join(mountPoint, filepath.Base(uf2Path))
	fmt.Printf("[*] Copying %s to %s...\n", filepath.Base(uf2Path), destPath)

	err := copyFile(uf2Path, destPath)
	if err == nil {
		fmt.Println("[SUCCESS] Flash complete! Pico should restart automatically.")
		return
	}

	// If error occurred, check if drive is gone (success case usually)
	if _, statErr := os.Stat(mountPoint); os.IsNotExist(statErr) {
		fmt.Println("\n[OK] Flash finished (device rebooted and disconnected).")
		return
	}

	// Also check checking volume label again might fail if device is gone
	if _, err := getVolumeLabel(mountPoint); err != nil {
		fmt.Println("\n[OK] Flash finished (device disconnected).")
		return
	}

	fmt.Printf("\n[!] Error during copy: %v\n", err)
}

func copyFile(src, dst string) error {
	source, err := os.Open(src)
	if err != nil {
		return err
	}
	defer source.Close()

	destination, err := os.Create(dst)
	if err != nil {
		return err
	}
	defer destination.Close()

	_, err = io.Copy(destination, source)
	if err != nil {
		return err
	}
	return nil
}
