using System;
using System.Collections.Generic;
using System.IO;
using System.IO.Ports;
using System.Linq;
using System.Security.Cryptography;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using Microsoft.Win32;

namespace jsb_flasher;

/// <summary>
/// Pico Firmware Flasher - Raspberry Pi Pico ファームウェア書き込みユーティリティ
/// </summary>
public partial class MainWindow : Window
{
    private const string PicoVID = "VID_2E8A";
    private const string BootselLabel = "RPI-RP2";
    private const string ExpectedHash = "066562e5312239a85f2ba0415688c7a03c6d2d46cecc9045256d35c49d9191ce";

    private string _selectedPort = "";
    private string _firmwarePath = "";

    public MainWindow() : this(null)
    {
    }

    public MainWindow(string? firmwarePath)
    {
        InitializeComponent();
        _firmwarePath = firmwarePath ?? "";
        LoadAvailablePorts();
    }

    private void LoadAvailablePorts()
    {
        ItemListBox.Items.Clear();
        var ports = FindPicoSerial();

        if (ports.Count == 0)
        {
            MessageBox.Show("JSB01シリアルポートが見つかりません", "エラー", MessageBoxButton.OK, MessageBoxImage.Error);
            return;
        }

        foreach (var port in ports)
        {
            ItemListBox.Items.Add(port);
        }

        if (ItemListBox.Items.Count > 0)
        {
            ItemListBox.SelectedIndex = 0;
            _selectedPort = ItemListBox.Items[0]?.ToString() ?? "";
        }
    }

    private void OkButton_Click(object sender, RoutedEventArgs e)
    {
        if (ItemListBox.SelectedIndex < 0)
        {
            MessageBox.Show("シリアルポートを選択してください。", "警告", MessageBoxButton.OK, MessageBoxImage.Warning);
            return;
        }

        _selectedPort = ItemListBox.SelectedItem?.ToString() ?? "";

        // フラッシング処理を実行
        FlashFirmwareAsync();
    }

    private void CancelButton_Click(object sender, RoutedEventArgs e)
    {
        this.Close();
    }

    private async void FlashFirmwareAsync()
    {
        OkButton.IsEnabled = false;
        CancelButton.IsEnabled = false;

        try
        {
            // ファイルパスの検証
            var uf2Path = ResolveAndValidateUF2Path(_firmwarePath);
            // MessageBox.Show($"ファームウェア: {uf2Path}", "情報", MessageBoxButton.OK, MessageBoxImage.Information);

            // BOOTSEL モードに入る
            // MessageBox.Show("BUTTOOSELボタンを押しながらPicoをコンピュータに接続してください。", "確認", MessageBoxButton.OK, MessageBoxImage.Information);
            EnterBootselMode(_selectedPort);

            // Pico ドライブを検索
            // MessageBox.Show("Pico ドライブを検索しています...", "情報", MessageBoxButton.OK, MessageBoxImage.Information);
            var mountPoint = FindPicoDrive(TimeSpan.FromSeconds(10));

            if (string.IsNullOrEmpty(mountPoint))
            {
                MessageBox.Show("Pico ドライブが見つかりません。", "エラー", MessageBoxButton.OK, MessageBoxImage.Error);
                return;
            }

            // ファームウェアをフラッシュ
            MessageBox.Show($"ファームウェアを {mountPoint} に書き込みます...", "情報", MessageBoxButton.OK, MessageBoxImage.Information);
            await Task.Run(() => FlashFirmware(uf2Path, mountPoint));

            MessageBox.Show("ファームウェアの書き込みが完了しました！", "成功", MessageBoxButton.OK, MessageBoxImage.Information);
            this.Close();
        }
        catch (Exception ex)
        {
            MessageBox.Show($"エラー: {ex.Message}", "エラー", MessageBoxButton.OK, MessageBoxImage.Error);
        }
        finally
        {
            OkButton.IsEnabled = true;
            CancelButton.IsEnabled = true;
        }
    }

    /// <summary>
    /// ファイルのSHA256ハッシュを計算します
    /// </summary>
    private string CalculateFileHash(string filePath)
    {
        using (var sha256 = SHA256.Create())
        using (var fileStream = File.OpenRead(filePath))
        {
            var hash = sha256.ComputeHash(fileStream);
            return BitConverter.ToString(hash).Replace("-", "").ToLowerInvariant();
        }
    }

    /// <summary>
    /// Pico が接続されているシリアルポートのリストを返します
    /// </summary>
    private List<string> FindPicoSerial()
    {
        var picoPorts = new List<string>();

        try
        {
            // 現在利用可能なシリアルポートを取得
            var availablePorts = SerialPort.GetPortNames().ToHashSet();

            // レジストリからVIDに一致するUSBデバイスを検索
            using (var usbKey = Registry.LocalMachine.OpenSubKey(@"SYSTEM\CurrentControlSet\Enum\USB"))
            {
                if (usbKey != null)
                {
                    foreach (var deviceKeyName in usbKey.GetSubKeyNames())
                    {
                        // VIDが一致するデバイスを探す
                        if (deviceKeyName.StartsWith(PicoVID, StringComparison.OrdinalIgnoreCase))
                        {
                            using (var deviceKey = usbKey.OpenSubKey(deviceKeyName))
                            {
                                if (deviceKey != null)
                                {
                                    // インスタンスIDを走査
                                    foreach (var instanceId in deviceKey.GetSubKeyNames())
                                    {
                                        using (var instanceKey = deviceKey.OpenSubKey(instanceId))
                                        {
                                            if (instanceKey != null)
                                            {
                                                // Device Parameters からポート名を取得
                                                using (var paramsKey = instanceKey.OpenSubKey("Device Parameters"))
                                                {
                                                    if (paramsKey != null)
                                                    {
                                                        var portName = paramsKey.GetValue("PortName") as string;
                                                        // 実際に利用可能で、まだリストにないポートのみ追加
                                                        if (!string.IsNullOrEmpty(portName) && 
                                                            availablePorts.Contains(portName) && 
                                                            !picoPorts.Contains(portName))
                                                        {
                                                            picoPorts.Add(portName);
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        catch (Exception ex)
        {
            MessageBox.Show($"レジストリアクセスエラー: {ex.Message}", "エラー", MessageBoxButton.OK, MessageBoxImage.Error);
        }

        return picoPorts;
    }

    /// <summary>
    /// BOOTSEL モードに入ります（1200bps で開閉）
    /// </summary>
    private void EnterBootselMode(string portName)
    {
        try
        {
            using (var port = new SerialPort(portName, 1200))
            {
                port.Open();
                port.Close();
            }
        }
        catch (Exception)
        {
            // ポート接続エラーはここでは無視
        }

        Thread.Sleep(1000);
    }

    /// <summary>
    /// BOOTSEL モードのPico ドライブを検索します
    /// </summary>
    private string FindPicoDrive(TimeSpan timeout)
    {
        var deadline = DateTime.Now.Add(timeout);

        while (DateTime.Now < deadline)
        {
            // ドライブ D～Z をチェック
            for (char drive = 'D'; drive <= 'Z'; drive++)
            {
                var driveRoot = $"{drive}:\\";

                try
                {
                    if (Directory.Exists(driveRoot))
                    {
                        var driveInfo = new DriveInfo(driveRoot.TrimEnd('\\'));
                        if (driveInfo.VolumeLabel == BootselLabel)
                        {
                            return driveRoot;
                        }
                    }
                }
                catch (Exception)
                {
                    // ドライブチェック失敗は無視
                }
            }

            Thread.Sleep(1000);
        }

        return "";
    }

    /// <summary>
    /// ファームウェアをPico に書き込みます
    /// </summary>
    private void FlashFirmware(string uf2Path, string mountPoint)
    {
        var destPath = Path.Combine(mountPoint, Path.GetFileName(uf2Path));

        try
        {
            File.Copy(uf2Path, destPath, true);
        }
        catch (Exception ex)
        {
            // ドライブが消えた場合は成功と判定（Pico が再起動した場合）
            try
            {
                if (!Directory.Exists(mountPoint))
                {
                    return; // ドライブが消えた = 成功
                }
            }
            catch (Exception)
            {
            }

            throw new Exception($"ファームウェアのコピーエラー: {ex.Message}", ex);
        }
    }

    /// <summary>
    /// コマンドラインからUF2ファイルパスを解決して検証します
    /// </summary>
    private string ResolveAndValidateUF2Path(string firmware)
    {
        var uf2Path = firmware;
        var isCommandLineSpecified = !string.IsNullOrEmpty(firmware);

        if (string.IsNullOrEmpty(uf2Path))
        {
            // 実行ファイルのディレクトリで firmware.uf2 を探す
            // 単一ファイルアプリの場合はAppContext.BaseDirectoryを使用
            var baseDir = AppContext.BaseDirectory;
            uf2Path = Path.Combine(baseDir, "firmware.uf2");
        }

        // ファイル存在確認
        if (!File.Exists(uf2Path))
        {
            throw new FileNotFoundException($"ファイルが見つかりません: {uf2Path}");
        }

        // ハッシュ検証（コマンドラインで指定されていない場合のみ実施）
        if (!isCommandLineSpecified)
        {
            var currentHash = CalculateFileHash(uf2Path);
            if (currentHash != ExpectedHash)
            {
                throw new InvalidOperationException(
                    $"ハッシュが一致しません。期待値: {ExpectedHash}, 実際: {currentHash}");
            }
        }

        return uf2Path;
    }
}