import sys
import time
import shutil
import hashlib
import argparse
from pathlib import Path
import serial
import serial.tools.list_ports
import psutil

# Raspberry Pi Pico (RP2040) の標準的な識別子
PICO_VID = 0x2E8A
PICO_PID = 0x0005  # MicroPython/CircuitPython実行時
BOOTSEL_LABEL = "RPI-RP2"
EXPECTED_HASH = "18ec42850c70eb06ca928e81704b967cb20eff54aec6b0293453fbca3ab270ca" 
HASH_ALGORITHM = "sha256" # "sha512" に変更も可能

def calculate_file_hash(file_path: str | Path) -> str:
    """ファイルのハッシュ値を計算する"""
    hasher = hashlib.new(HASH_ALGORITHM)
    with open(file_path, "rb") as f:
        while chunk := f.read(4096):
            hasher.update(chunk)
    return hasher.hexdigest()

def find_pico_serial():
    """現在接続されているPicoのシリアルポートを探す"""
    ports = serial.tools.list_ports.comports()
    for port in ports:
        if port.vid == PICO_VID:
            return port.device
    return None

def enter_bootsel_mode(port):
    """1200bpsリセットを実行してBOOTSELモードへ移行させる"""
    print(f"[*] Resetting Pico on {port} (1200bps trick)...")
    try:
        # 1200bpsで開いてすぐに閉じる
        ser = serial.Serial(port, baudrate=1200, timeout=0.1)
        ser.close()
        time.sleep(1)
    except Exception:
        # 移行時にデバイスが消えるため、エラーが出るのは正常（無視してOK）
        pass

def find_pico_drive(timeout=10):
    """RPI-RP2 ドライブがマウントされるのを待つ"""
    print(f"[*] Waiting for {BOOTSEL_LABEL} drive to mount...", end="", flush=True)
    start_time = time.time()
    while time.time() - start_time < timeout:
        for partition in psutil.disk_partitions():
            try:
                # INFO_UF2.TXT が存在するかどうかでPicoのドライブか判定
                info_path = Path(partition.mountpoint) / "INFO_UF2.TXT"
                if info_path.exists():
                    print("\n[+] Found Pico drive at:", partition.mountpoint)
                    return Path(partition.mountpoint)
            except (PermissionError, OSError):
                continue
        print(".", end="", flush=True)
        time.sleep(1)
    print("\n[!] Timeout: Pico drive not found.")
    return None

def flash_firmware(uf2_path, mount_point):
    """UF2ファイルをPicoにコピーする"""
    dest_path = mount_point / uf2_path.name
    print(f"[*] Copying {uf2_path.name} to {dest_path}...")
    try:
        # shutil.copy2 はメタデータも保持しようとするため、単純な copy でも可
        shutil.copy(uf2_path, dest_path)
        print("[SUCCESS] Flash complete! Pico should restart automatically.")
        return True
    except Exception as e:
        # 書き込み直後に再起動して切断されるため、エラーが出ても成功していることが多い
        # ドライブ自体が消えているかを確認する
        try:
            if not mount_point.exists():
                print("\n[OK] Flash finished (device rebooted and disconnected).")
                return True
        except OSError:
            # Windows等でデバイス消失後にアクセスするとOSErrorになる場合があるため成功とみなす
            print("\n[OK] Flash finished (device disconnected).")
            return True

        print(f"\n[!] Error during copy: {e}")
        return False

def main():
    parser = argparse.ArgumentParser(description="Pico Auto Flasher")
    parser.add_argument("file", nargs='?', help="Path to the .uf2 firmware file")
    args = parser.parse_args()

    if args.file:
        uf2_path = Path(args.file)
    else:
        # PyInstallerでビルドされた場合と、通常実行の場合でパスの取得方法を変える
        if getattr(sys, 'frozen', False):
            # PyInstallerで実行されている場合、実行ファイル(.exe)のあるディレクトリを取得
            base_dir = Path(sys.executable).parent
        else:
            # 通常のスクリプト実行の場合
            base_dir = Path(__file__).parent.resolve()
        
        uf2_path = base_dir / "firmware.uf2"
        print(f"[*] Looking for firmware at: {uf2_path}")

    if not uf2_path.exists():
        print(f"Error: File not found: {uf2_path}")
        sys.exit(1)

    # --- 0. ハッシュチェック（書き込み前に行う） ---
    # コマンドラインで指定されなかった場合にだけハッシュをチェック
    if not args.file:
        print(f"[*] Verifying firmware integrity ({HASH_ALGORITHM})...")
        actual_hash = calculate_file_hash(uf2_path)
        if actual_hash != EXPECTED_HASH:
            print("[X] CRITICAL ERROR: Hash mismatch!")
            print(f"    Expected: {EXPECTED_HASH}")
            print(f"    Actual:   {actual_hash}")
            print("[X] This firmware may be corrupted or tampered with. Aborting.")
            sys.exit(1)
        print("[+] Verification successful. Proceeding...")

    # 1. デバイス検知
    port = find_pico_serial()
    if port:
        # 2. BOOTSELモードへ移行
        enter_bootsel_mode(port)
    else:
        print("[!] Pico serial port not found. Checking if already in BOOTSEL mode...")

    # 3. ドライブ認識待ち
    mount_point = find_pico_drive()
    if not mount_point:
        sys.exit(1)

    # 4. 書き込み
    flash_firmware(uf2_path, mount_point)

if __name__ == "__main__":
    main()
