// az-expander
// ATTiny826 を I2C 通信のIOエキスパンダにするファームウェア

#include <Wire.h>
#include <EEPROM.h>
#include <avr/wdt.h>

// ATTiny826 本体のアドレス
#define I2C_SLAVE_ADD 0x30

// I2C ピン
#define READ_PIN_SCL PIN_PB0
#define READ_PIN_SDA PIN_PB1

// I2Cで送られてきたコマンドを保持するバッファのサイズ
#define CMD_BUF_SIZE  24

// キー入力を保持するバッファのサイズ(MAXで8×8の倍マトリックスで128bit＝>16byte)
#define KEY_BUF_SIZE  16

// IOピンの番号
const uint8_t pin_num[16] = {
  PIN_PA4, PIN_PA5, PIN_PA6, PIN_PA7,
  PIN_PB5, PIN_PB4, PIN_PB3, PIN_PB2,
  PIN_PC0, PIN_PC1, PIN_PC2, PIN_PC3, 
  PIN_PA0, PIN_PA1, PIN_PA2, PIN_PA3
};

uint8_t read_buf[16]; // キーの読み込みバッファ
uint8_t send_buf[16]; // キー送信用のデータ
uint8_t send_byte; // キー送信に何バイト送信するか
uint8_t direct_list[16]; // ダイレクトピンリスト
uint8_t col_list[16]; // colピンリスト
uint8_t row_list[16]; // rowピンリスト
uint8_t key_len; // 読み込むキーの数
uint8_t direct_len; // ダイレクトピンの数
uint8_t col_len; // colピンの数
uint8_t row_len; // rowピンの数
uint8_t req_cmd; // 要求されたコマンド
// 0.本体アドレス1バイト、1.読み込みモード1バイト(0=通常マトリックス、1=倍マトリックス)、2～17.全ピンの設定16バイト(0=未使用、1=ダイレクト、2=COL、3=ROW)
uint8_t setting_data[18] = {
  I2C_SLAVE_ADD, 0x00,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01};
// 各コマンド用の受け取ったデータ
uint8_t cmd_buf[CMD_BUF_SIZE];

// I2Cデータ要求を受け取った時の関数
void receiveEvent(int data_len); // データを受け取った
void requestEvent(); // データ要求を受け取った

// ピンの設定情報を変数に格納
void set_pin_data() {
  int i, p;

  // 設定データクリア
  memset(direct_list, 0, sizeof(direct_list));
  memset(col_list, 0, sizeof(col_list));
  memset(row_list, 0, sizeof(row_list));
  direct_len = 0;
  col_len = 0;
  row_len = 0;
  
  // ピン情報を変数に格納
  for (i=0; i<16; i++) {
    p = setting_data[i+2];
    if (p == 1) {
      direct_list[direct_len] = pin_num[i];
      direct_len++;
    } else if (p == 2) {
      col_list[col_len] = pin_num[i];
      col_len++;
    } else if (p == 3) {
      row_list[row_len] = pin_num[i];
      row_len++;
    }
  }

  // 読み込むキー数を計算
  key_len = direct_len + (col_len * row_len);
  if (setting_data[1] == 1) key_len = direct_len + (col_len * row_len * 2);

  // キーデータを送信する時に送るバイト数計算
  send_byte = key_len / 8;
  if (key_len % 8) send_byte++;
}

void setup() {
    uint8_t c;
    int i;

    // 初めての起動の場合EPPROMにデフォルト設定を書き込む
    c = EEPROM.read(0); // 最初の0バイト目を読み込む
    if (c != 0x20) {
      EEPROM.write(0, 0x20); // 初期化したよを書き込む
      for (i=0; i<18; i++) {
        EEPROM.write(i+1, setting_data[i]); // デフォルトの設定データ書込み
      }
    }

    // EEPROMから設定を読み込む
    for (i=0; i<18; i++) {
      setting_data[i] = EEPROM.read(i+1);
    }

    // ピンの設定情報を変数に格納
    set_pin_data();
  
    // バッファクリア
    memset(read_buf, 0, sizeof(read_buf));
    memset(send_buf, 0, sizeof(send_buf));

    // I2C スレーブ初期化
    Wire.begin(setting_data[0]); // I2C初期化、アドレスは設定されたアドレス
    Wire.onReceive(receiveEvent); // データを受け取った
    Wire.onRequest(requestEvent); // データ要求を受け取った

}

// コマンドを受け取った
void receiveEvent(int data_len) {
  // 受け取ったデータ読み込み
  int i;
  for (i=0; i<CMD_BUF_SIZE; i++) {
    cmd_buf[i] = (Wire.available())? Wire.read(): 0x00;
  }
}

// データ要求を受け取った
void requestEvent() {
    int i, f;
    uint8_t cmd = cmd_buf[0];
    
    if (cmd == 0x01) {
      // 現在の設定と比較して違えば更新
      f = 0;
      for (i=0; i<18; i++) {
        if (setting_data[i] != cmd_buf[i+1]) f++;
      }
      if (f) {
        // 設定を変更
        for (i=0; i<18; i++) {
          EEPROM.write(i+1, cmd_buf[i+1]);
        }
        // 設定変更があれば2を返す
        Wire.write(0x02);
        Wire.write(cmd_buf[1]);
        // 60ミリ秒後に再起動
        wdt_enable(WDTO_60MS);
      } else {
        // 設定変更が無ければ1を返す
        Wire.write(0x01);
        Wire.write(cmd_buf[1]);
      }

    } else if (cmd == 0x02) {
        // 設定内容の取得
        for (i=0; i<18; i++) {
            Wire.write(setting_data[i]);
        }

    } else if (cmd == 0x03) {
        // 読み込むキーの数とバイト数を取得
        Wire.write(key_len);
        Wire.write(send_byte);

    } else if (cmd == 0x04) {
        // キー入力情報を取得
        for (i=0; i<send_byte; i++) {
            Wire.write(send_buf[i]);
        }

    } else {
      // 不明なコマンドの場合は自分のアドレスをエコーする
      Wire.write(setting_data[0]);
    }
}

// 読み込みバッファのn番目のキーをONにする
void set_read_buf(int n) {
  read_buf[(n / 8)] |= 0x01 << (n % 8);
}

void loop() {
  int i, j, t, r;

  // ピン初期化
  for (i=0; i<16; i++) {
    t = setting_data[i+2];
    if (t == 1) pinMode(pin_num[i], INPUT_PULLUP); // direct
    if (t == 2) pinMode(pin_num[i], OUTPUT); // col
    if (t == 3) pinMode(pin_num[i], INPUT_PULLUP); // row
  }

  // 前処理
  memset(read_buf, 0, sizeof(read_buf)); // 読み込みバッファクリア
  r = 0; // 読み込み位置

  // ダイレクトピンを読み取り
  for (i=0; i<direct_len; i++) {
    if (!digitalRead(direct_list[i])) set_read_buf(r);
    r++;
  }

  // マトリックス読み込み
  for (i=0; i<col_len; i++) {
    // 対象のcolのみ0にする
    for (j=0; j<col_len; j++) {
      digitalWrite(col_list[j], (i != j));
    }
    delayMicroseconds(50);
    // row読み込み
    for (j=0; j<row_len; j++) {
      if (!digitalRead(row_list[j])) set_read_buf(r);
      r++;
    }
  }

  // 倍マトリックス読み込み
  if (setting_data[1] == 1) {
    // ピン初期化
    for (i=0; i<16; i++) {
      t = setting_data[i+2];
      if (t == 2) pinMode(pin_num[i], INPUT_PULLUP); // col
      if (t == 3) pinMode(pin_num[i], OUTPUT); // row
    }
    // マトリックス読み込み
    for (i=0; i<row_len; i++) {
      // 対象のrowのみ0にする
      for (j=0; j<row_len; j++) {
        digitalWrite(row_list[j], (i != j));
      }
      delayMicroseconds(50);
      // col読み込み
      for (j=0; j<col_len; j++) {
        if (!digitalRead(col_list[j])) set_read_buf(r);
        r++;
      }
    }
  }

  // 読み込んだ内容を送信バッファにコピー
  memcpy(send_buf, read_buf, sizeof(send_buf));

  delay(5);
}
