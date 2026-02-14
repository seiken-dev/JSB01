# ビルド手順

製品用ビルドの方法については、README.mdを参照してください。

## 単一ファイルとして発行

以下のコマンドで、単一の実行ファイルとして発行できます：

```powershell
dotnet publish -c Release -r win-x64 --self-contained true -p:PublishSingleFile=true
```

## 発行されたファイルの場所

発行されたファイルは以下のディレクトリに生成されます：
```
bin\Release\net10.0-windows\win-x64\publish\
```

## 配布ファイル

発行後、以下の2つのファイルを配布してください：
- `jsb-flasher.exe` - メインの実行ファイル（約70-100MB、.NETランタイム込み）
- `firmware.uf2` - Picoファームウェアファイル

**重要**: この2つのファイルは必ず同じディレクトリに配置してください。

## オプション: 軽量版（Framework依存）

.NET 10ランタイムがインストールされている環境向けの軽量版：

```powershell
dotnet publish -c Release -r win-x64 --self-contained false -p:PublishSingleFile=true
```

この場合、実行ファイルのサイズは約10MB程度になりますが、実行には.NET 10ランタイムが必要です。

## 簡易ビルドコマンド

開発用のデバッグビルド：
```powershell
dotnet build
```

リリースビルド：
```powershell
dotnet build -c Release
```
