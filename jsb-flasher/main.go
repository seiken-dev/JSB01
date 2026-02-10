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

	"github.com/lxn/walk"
	. "github.com/lxn/walk/declarative"
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
	consoleFlag := flag.Bool("c", false, "Launch console")
	ignoreHashFlag := flag.Bool("i", false, "Ignore firmware hash check (not recommended)")
	flag.Parse()
	firmware := ""
	if len(flag.Args()) == 1 {
		firmware = flag.Args()[0]
	}

	if *consoleFlag {
		err := consoleApp(firmware, *portFlag, *ignoreHashFlag)
		if err != nil {
			os.Exit(1)
		}
		fmt.Printf("Firmware flashing Done.\n")
	} else {
		guiApp(firmware, *portFlag, *ignoreHashFlag)
	}
}

func guiApp(firmwarePath string, portName string, ignoreHash bool) error {
	var mainWindow *walk.MainWindow
	var portListBox *walk.ListBox
	var selectedPort string

	// Get available ports
	availablePorts := []string{}
	if portName != "" {
		availablePorts = append(availablePorts, portName)
		selectedPort = portName
	} else {
		availablePorts = findPicoSerial()
		if len(availablePorts) > 0 {
			selectedPort = availablePorts[0]
		}
	}

	// If no ports available, show error dialog
	if len(availablePorts) == 0 {
		walk.MsgBox(nil, "Error", "No JSB01 serial ports found", walk.MsgBoxIconError)
		return fmt.Errorf("no available ports")
	}

	MainWindow{
		AssignTo: &mainWindow,
		Title:    "JSB01 Firmware Flasher",
		Layout:   VBox{},
		Children: []Widget{
			Label{
				Text: "Select Serial Port:",
			},
			ListBox{
				AssignTo:     &portListBox,
				Model:        availablePorts,
				CurrentIndex: 0,
				OnCurrentIndexChanged: func() {
					if portListBox.CurrentIndex() >= 0 && portListBox.CurrentIndex() < len(availablePorts) {
						selectedPort = availablePorts[portListBox.CurrentIndex()]
					}
				},
			},
			Composite{
				Layout: HBox{},
				Children: []Widget{
					PushButton{
						Text: "OK",
						OnClicked: func() {
							// Execute flashing process with selectedPort
							uf2Path, err := resolveAndValidateUF2Path(firmwarePath, ignoreHash)
							if err != nil {
								walk.MsgBox(mainWindow, "Error", fmt.Sprintf("Error: %v", err), walk.MsgBoxIconError)
								return
							}
							// Enter BOOTSEL mode
							enterBootselMode(selectedPort)
							// Wait for Drive
							mountPoint := findPicoDrive(10 * time.Second)
							if mountPoint == "" {
								walk.MsgBox(mainWindow, "Error", "Failed to find Pico drive in BOOTSEL mode", walk.MsgBoxIconError)
								return
							}
							// Flash firmware
							err = flashFirmware(uf2Path, mountPoint)
							if err != nil {
								walk.MsgBox(mainWindow, "Error", fmt.Sprintf("Error: %v", err), walk.MsgBoxIconError)
								return
							}
							walk.MsgBox(mainWindow, "Success", "Firmware flashing completed!", walk.MsgBoxIconInformation)
							mainWindow.Close()
						},
					},
					PushButton{
						Text: "Cancel",
						OnClicked: func() {
							mainWindow.Close()
						},
					},
				},
			},
		},
	}.Run()
	return nil
}

func consoleApp(firmwarePath string, portName string, ignoreHash bool) error {
	uf2Path, err := resolveAndValidateUF2Path(firmwarePath, ignoreHash)
	if err != nil {
		fmt.Printf("Error: %v\n", err)
		return err
	}
	fmt.Printf("Flashing firmware %v\n", uf2Path)

	// 1. Find Serial Port
	fmt.Printf("Finding JSB01 serial port...\n")
	picoPort := portName
	if picoPort == "" {
		availablePorts := findPicoSerial()
		if len(availablePorts) > 0 {
			if len(availablePorts) > 1 {
				fmt.Printf("Multiple JSB01 devices found. Available ports:\n")
				for _, p := range availablePorts {
					fmt.Printf(" - %s\n", p)
				}
				return fmt.Errorf("multiple JSB01 devices found; please specify a port with -p")
			} else {
				picoPort = availablePorts[0]
				fmt.Printf("Using port %s\n", picoPort)
			}
		}
	}

	if picoPort != "" {
		// 2. Enter BOOTSEL mode
		fmt.Printf("Entering BOOTSEL mode on port %s...\n", picoPort)
		enterBootselMode(picoPort)
	} else {
		return fmt.Errorf("Pico serial port not found. Checking if already in BOOTSEL mode...")
	}

	// 3. Wait for Drive
	fmt.Printf("Waiting for Pico drive in BOOTSEL mode...\n")
	mountPoint := findPicoDrive(10 * time.Second)
	if mountPoint == "" {
		return fmt.Errorf("[!] Failed to find Pico drive in BOOTSEL mode.")
	}

	fmt.Printf("Flashing firmware to %s...\n", mountPoint)
	flashFirmware(uf2Path, mountPoint)
	return nil
}

// ファイルのSHA256ハッシュを計算します。
// ファイルが存在しない場合や読み取りエラーが発生した場合はエラーを返します。
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

// JSB01が接続されているシリアルポートのリストを返します。
// 通常複数見つかることはないのですが、複数RP2040を接続しているような環境で、意図せずJSB01ではないファームウェアを書き換えないようにするため、呼び出し側でポート選択ができるようにするためです。
func findPicoSerial() []string {
	picoPorts := []string{}
	ports, err := enumerator.GetDetailedPortsList()
	if err == nil {
		for _, port := range ports {
			// Check both VID formats just in case (upper/lower)
			if strings.EqualFold(port.VID, PicoVID) {
				picoPorts = append(picoPorts, port.Name)
			}
		}
	}
	return picoPorts
}

func enterBootselMode(portName string) {
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
	deadline := time.Now().Add(timeout)

	for time.Now().Before(deadline) {
		// Check drives D...Z
		for _, drive := range "DEFGHIJKLMNOPQRSTUVWXYZ" {
			driveRoot := string(drive) + ":\\"

			// Check volume label
			label, err := getVolumeLabel(driveRoot)
			if label != "" {
				// fmt.Printf("Label: %v\n", label)
			}
			if err == nil && label == BootselLabel {
				return driveRoot
			}
		}

		time.Sleep(1 * time.Second)
	}
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

func flashFirmware(uf2Path, mountPoint string) error {
	destPath := filepath.Join(mountPoint, filepath.Base(uf2Path))

	err := copyFile(uf2Path, destPath)
	if err == nil {
		return nil
	}

	// If error occurred, check if drive is gone (success case usually)
	if _, statErr := os.Stat(mountPoint); os.IsNotExist(statErr) {
		return nil
	}

	// Also check checking volume label again might fail if device is gone
	if _, err := getVolumeLabel(mountPoint); err != nil {
		return nil
	}

	return fmt.Errorf("\n[!] Error during copy: %v\n", err)
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

// resolveAndValidateUF2Path は、コマンドライン引数から uf2 ファイルパスを解決し、
// ファイル存在確認とハッシュ検証を行います。
//
// args: コマンドライン引数のスライス
//
// 戻り値: 完全な uf2 ファイルパスとエラー
// 自動選択の場合（args が空）、ハッシュ検証も行われます。
func resolveAndValidateUF2Path(firmware string, ignoreHash bool) (string, error) {
	var uf2Path string
	if firmware != "" {
		uf2Path = firmware
	} else {
		// Try to find firmware.uf2 in the executable directory
		ex, err := os.Executable()
		if err != nil {
			return "", err
		}
		baseDir := filepath.Dir(ex)
		uf2Path = filepath.Join(baseDir, "firmware.uf2")
	}

	// Check if file exists
	if _, err := os.Stat(uf2Path); os.IsNotExist(err) {
		return "", fmt.Errorf("file not found: %s", uf2Path)
	}

	// Hash Check (only if file was automatically selected)
	if firmware == "" && !ignoreHash {
		currentHash, err := calculateFileHash(uf2Path)
		if err != nil {
			return "", fmt.Errorf("error calculating hash: %v", err)
		}
		if currentHash != ExpectedHash {
			return "", fmt.Errorf("hash mismatch for %s: expected %s, got %s", uf2Path, ExpectedHash, currentHash)
		}
	}
	return uf2Path, nil
}
