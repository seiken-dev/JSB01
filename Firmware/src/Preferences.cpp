#include "Preferences.h"

#include "LittleFS.h"

Preferences::Preferences() {
  // コンストラクタ - 何もしない
}

Preferences::~Preferences() {
  // デストラクタ - LittleFSが初期化されていれば終了処理
  LittleFS.end();
}

void Preferences::begin() {
  // LittleFSを初期化
  if (!LittleFS.begin()) {
    Serial.println("LittleFS initialization failed!");
    return;
  }
  Serial.println("LittleFS initialized");
}

void Preferences::end() {
  // LittleFSを終了
  LittleFS.end();
}

void Preferences::put(const char* key, const char* value) {
  // ファイル名として/prefs/keyを使用
  String filepath = "/prefs/";
  filepath += key;

  // ディレクトリが存在しない場合は作成
  if (!LittleFS.exists("/prefs")) {
    LittleFS.mkdir("/prefs");
  }

  // ファイルに値を書き込み
  File file = LittleFS.open(filepath, "w");
  if (file) {
    file.print(value);
    file.close();
    Serial.printf("Saved: %s = %s\n", key, value);
  } else {
    Serial.printf("Failed to save: %s\n", key);
  }
}

bool Preferences::get(const char* key, char* value) {
  // ファイル名として/prefs/keyを使用
  String filepath = "/prefs/";
  filepath += key;

  // ファイルが存在しない場合はfalseを返す
  if (!LittleFS.exists(filepath)) {
    Serial.printf("Key not found: %s\n", key);
    return false;
  }

  // ファイルから値を読み込み
  File file = LittleFS.open(filepath, "r");
  if (file) {
    content = file.readString();
    file.close();

    // 読み込んだ内容をvalueにコピー
    strcpy(value, content.c_str());
    Serial.printf("Loaded: %s = %s\n", key, value);
    return true;
  } else {
    Serial.printf("Failed to load: %s\n", key);
    return false;
  }
}

Preferences pref;
