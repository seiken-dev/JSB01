# JSB01 Firmware Flasher

JSB01のファームウェア書き込みツールです。
USBシリアル接続されたJSB01を自動的に検出し、BOOTSELモードへリセットしてから `.uf2` ファームウェアを書き込みます。

## 特徴

* **自動リセット**: 1200bps シリアル通信によるリセット（"1200bps touch"）を行い、BOOTSELボタンを押さなくてもマスストレージモードに移行させます。
* **自動書き込み**: マウントされたRPI-RP2ドライブを自動検出し、ファームウェアをコピーします。
* **配布用ビルド**: PyInstallerを使用して、単一の実行ファイル(`.exe`)として配布可能です。

## 必要要件

* Windows
* .NET 開発環境
* Task

## ビルド

``` bash
task build
```

## 使い方

``` powershell
jsb01-flasher.exe [options] <firmware.uf2>
```

GUIでは、シリアルポートの選択画面が出ます。


