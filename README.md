# JSB01 -- Johann Sebastian Bachってなに？

<p align="center">
<img alt="JSB01" width="50%" src="./images/JSB01.jpg">
</p>

視覚障害者の歩行支援デバイスです。距離センサー、地磁気センサー(コンパス)、照度センサーを備え、振動によって情報を伝えます。

## コンテンツ
- **Firmware**<br>
PlatformIOでビルドできるソースコードです(動作テスト用)。[XIAO RP2040](https://wiki.seeedstudio.com/XIAO-RP2040/)(Rev.1)と、基板実装のRP2040(Rev.3)に対応しています。
- **KiCad**<br>
[KiCad](https://www.kicad.org/)のプロジェクトです。Rev.1の**JSB01.kicad_pro**と、Rev.3の**JSB01R3.kicad_pro**の２つのプロジェクトファイルがあります。シンボルとフットプリントは共通です。
- **Blender**<br>
3Dプリントケース作成用のBlenderのプロジェクトファイルと、発注に使用したSTLファイルです。

※ **KiCad**と**Blender**のデータは、**Creative Commons** [**CC BY**](https://creativecommons.org/licenses/by/4.0/legalcode.ja)のライセンスの元で、自由に使用/改変/製作して頂いて構いません。ただし、データや製作物に対しては作者は一切の責任を負いません。

## 各ブランチはなんのため？

* c.mos: c.mosさん作成のコードです。ハードウェアのすべての機能を実装しています
* ATtiny: ATtiny X0Xシリーズをマイコンとして使うブランチ
* ESP32C3: XIAO ESP32C3をマイコンとして使って、esp-idfを開発環境にするブランチ
* RP2040: RP2040あるいは XIAO RP2040を使って、pico-sdkを開発環境にするブランチ
* Experimental: 各種実験ブランチ
* main: そのうちメインになるだろうブランチ

## その他

