// az-expander
// ATTiny826 を I2C 通信のIOエキスパンダにするファームウェア

#include <Wire.h>
#include <EEPROM.h>
#include <avr/wdt.h>
#include <tinyNeoPixel.h>

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

// ロータリーエンコーダカウント用
const short rotary_qem[16] = {0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0};

// NeoPixcel の最大接続数
#define NPX_MAX_LENGTH 128

// NeoPixcelクラス
tinyNeoPixel npx_obj;

// NeoPixcel ピン
short npx_pin = -1;

// NeoPixcel 接続数
uint8_t npx_count = 0;


uint8_t read_buf[16]; // キーの読み込みバッファ
uint8_t send_buf[16]; // キー送信用のデータ
uint8_t send_byte; // キー送信に何バイト送信するか
uint8_t direct_list[16]; // ダイレクトピンリスト
uint8_t col_list[16]; // colピンリスト
uint8_t row_list[16]; // rowピンリスト
uint8_t rotary_list[16]; // ロータリーエンコーダのピン(A,A,B,B,C,C....H,H)
uint8_t key_len; // 読み込むキーの数
uint8_t direct_len; // ダイレクトピンの数
uint8_t col_len; // colピンの数
uint8_t row_len; // rowピンの数
uint8_t req_cmd; // 要求されたコマンド
uint8_t rotary_read[8]; // ロータリーエンコーダの読み込みデータ
short rotary_count[8]; // ロータリーエンコーダの動作カウント
// 0.本体アドレス1バイト、1.読み込みモード1バイト(0=通常マトリックス、1=倍マトリックス)、2. NeoPixcel の接続数、 3～18.全ピンの設定16バイト(0=未使用、1=ダイレクト、2=COL、3=ROW)
uint8_t setting_data[19] = {
  I2C_SLAVE_ADD, 0x00, 0x00,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01};
// 各コマンド用の受け取ったデータ
uint8_t cmd_buf[CMD_BUF_SIZE];

// I2Cデータ要求を受け取った時の関数
void receiveEvent(int data_len); // データを受け取った
void requestEvent(); // データ要求を受け取った

// ロータリーエンコーダの読み込み
void read_rotary(int p) {
  int rotary_num = p / 2;
  uint8_t n = (digitalRead(rotary_list[p]) * 2) + digitalRead(rotary_list[p+1]); // 現在の状態取得
  uint8_t s = (rotary_read[rotary_num] * 4) + n; // 前回の状態と結合
  if (s < 16) rotary_count[rotary_num] += rotary_qem[s]; // 前回今回の状態を元にカウント計算
  rotary_read[rotary_num] = n; // 今回の状態を保持
}

// ロータリーエンコーダ割り込みアタッチ用関数
void read_rotary_a() { read_rotary(0); }
void read_rotary_b() { read_rotary(2); }
void read_rotary_c() { read_rotary(4); }
void read_rotary_d() { read_rotary(6); }
void read_rotary_e() { read_rotary(8); }
void read_rotary_f() { read_rotary(10); }
void read_rotary_g() { read_rotary(12); }
void read_rotary_h() { read_rotary(14); }

// ロータリーエンコーダの割り込みイベント登録
int attach_rotary(int p, uint8_t pin_i) {
  // 既に設定し終わっていたら無視
  if (rotary_list[p] != 0xFF && rotary_list[p+1] != 0xFF) {
    return 0;
  }
  // ここでピンモード設定しちゃう
  pinMode(pin_num[pin_i], INPUT_PULLUP);
  // 1つ目だけならピン番号格納して終わり
  if (rotary_list[p] == 0xFF) {
    rotary_list[p] = pin_num[pin_i];
    return 0;
  }
  // 2つ目ならば割り込みイベント登録
  rotary_list[p+1] = pin_num[pin_i]; // ピン番号保持
  // 現在のピンの状態を読み込んでおく
  rotary_read[(p / 2)] = (digitalRead(rotary_list[p]) << 1) + digitalRead(rotary_list[p+1]);
  // イベント登録
  int pin_a = digitalPinToInterrupt(rotary_list[p]);
  int pin_b = digitalPinToInterrupt(rotary_list[p+1]);
  if (p < 2) {
    attachInterrupt(pin_a, read_rotary_a, CHANGE);
    attachInterrupt(pin_b, read_rotary_a, CHANGE);
  } else if (p < 4) {
    attachInterrupt(pin_a, read_rotary_b, CHANGE);
    attachInterrupt(pin_b, read_rotary_b, CHANGE);
  } else if (p < 6) {
    attachInterrupt(pin_a, read_rotary_c, CHANGE);
    attachInterrupt(pin_b, read_rotary_c, CHANGE);
  } else if (p < 8) {
    attachInterrupt(pin_a, read_rotary_d, CHANGE);
    attachInterrupt(pin_b, read_rotary_d, CHANGE);
  } else if (p < 10) {
    attachInterrupt(pin_a, read_rotary_e, CHANGE);
    attachInterrupt(pin_b, read_rotary_e, CHANGE);
  } else if (p < 12) {
    attachInterrupt(pin_a, read_rotary_f, CHANGE);
    attachInterrupt(pin_b, read_rotary_f, CHANGE);
  } else if (p < 14) {
    attachInterrupt(pin_a, read_rotary_g, CHANGE);
    attachInterrupt(pin_b, read_rotary_g, CHANGE);
  } else if (p < 16) {
    attachInterrupt(pin_a, read_rotary_h, CHANGE);
    attachInterrupt(pin_b, read_rotary_h, CHANGE);
  }
  return 2;
}


// ピンの設定情報を変数に格納
void set_pin_data() {
  int i, p;
  int rc = 0;

  // 設定データクリア
  memset(direct_list, 0, sizeof(direct_list));
  memset(col_list, 0, sizeof(col_list));
  memset(row_list, 0, sizeof(row_list));
  memset(rotary_list, 0xFF, sizeof(rotary_list));
  direct_len = 0;
  col_len = 0;
  row_len = 0;
  npx_pin = -1;
  npx_count = setting_data[2];
  if (npx_count > NPX_MAX_LENGTH) npx_count = NPX_MAX_LENGTH;
  
  // ピン情報を変数に格納
  for (i=0; i<16; i++) {
    p = setting_data[i+3];
    if (p == 1) { // ダイレクト
      direct_list[direct_len] = pin_num[i];
      direct_len++;
    } else if (p == 2) { // COL
      col_list[col_len] = pin_num[i];
      col_len++;
    } else if (p == 3) { // ROW
      row_list[row_len] = pin_num[i];
      row_len++;
    } else if (p == 4) { // ロータリーA
      rc += attach_rotary(0, i);
    } else if (p == 5) { // ロータリーB
      rc += attach_rotary(2, i);
    } else if (p == 6) { // ロータリーC
      rc += attach_rotary(4, i);
    } else if (p == 7) { // ロータリーD
      rc += attach_rotary(6, i);
    } else if (p == 8) { // ロータリーE
      rc += attach_rotary(8, i);
    } else if (p == 9) { // ロータリーF
      rc += attach_rotary(10, i);
    } else if (p == 10) { // ロータリーG
      rc += attach_rotary(12, i);
    } else if (p == 11) { // ロータリーH
      rc += attach_rotary(14, i);
    } else if (p == 12) { // NeoPxcel
      npx_pin = pin_num[i];
    }
  }

  // 読み込むキー数を計算
  key_len = direct_len + (col_len * row_len) + rc; // 通常マトリックスの場合
  if (setting_data[1] == 1) key_len = direct_len + (col_len * row_len * 2) + rc; // 倍マトリックスの場合

  // キーデータを送信する時に送るバイト数計算
  send_byte = key_len / 8;
  if (key_len % 8) send_byte++;
}


void setup() {
    uint8_t c;
    int i;

    // 初めての起動の場合EPPROMにデフォルト設定を書き込む
    c = EEPROM.read(0); // 最初の0バイト目を読み込む
    if (c != 0x22) {
      EEPROM.write(0, 0x22); // 初期化したよを書き込む
      for (i=0; i<19; i++) {
        EEPROM.write(i+1, setting_data[i]); // デフォルトの設定データ書込み
      }
    }

    // EEPROMから設定を読み込む
    for (i=0; i<19; i++) {
      setting_data[i] = EEPROM.read(i+1);
    }

    // ピンの設定情報を変数に格納
    set_pin_data();
  
    // バッファクリア
    memset(read_buf, 0, sizeof(read_buf));
    memset(send_buf, 0, sizeof(send_buf));
    memset(rotary_read, 0, sizeof(rotary_read));
    memset(rotary_count, 0, sizeof(rotary_count));

    // NeoPixel が定義されていればNeoPixel初期化
    if (npx_pin >= 0) {
      npx_obj = tinyNeoPixel(npx_count, npx_pin, NEO_GRB + NEO_KHZ800);
      npx_obj.begin();
      for (i=0; i<npx_count; i++) {
          npx_obj.setPixelColor(i, 0, 0, 0);
      }
      npx_obj.show();
    }

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
    int c, i, f, p;
    uint8_t cmd = cmd_buf[0];
    
    if (cmd == 0x01) {
      // 現在の設定と比較して違えば更新
      f = 0;
      for (i=0; i<19; i++) {
        if (setting_data[i] != cmd_buf[i+1]) f++;
      }
      if (f) {
        // 設定を変更
        for (i=0; i<19; i++) {
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
        for (i=0; i<19; i++) {
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
        // 一度読み込まれたらロータリーエンコーダの回したデータクリア
        for (i=0; i<8; i++) {
            rotary_count[i] = 0;
        }

    } else if (cmd == 0x05 || cmd == 0x06) {
        // NeoPixcel を全部同じ色に光らせる(0x05 = データ書き換えだけ, 0x06 = 光らせる処理も実行)
        if (npx_pin >= 0) {
            for (i=0; i<npx_count; i++) {
              npx_obj.setPixelColor(i, cmd_buf[1], cmd_buf[2], cmd_buf[3]);
            }
            if (cmd == 0x06) npx_obj.show();
        }
        // 0x00 を返す
        Wire.write(0x00);

    } else if (cmd == 0x07 || cmd == 0x08) {
        // NeoPixcel 5個まで色データを変更する(0x07 = データ書き換えだけ, 0x08 = 光らせる処理も実行)
        if (npx_pin >= 0) {
            c = cmd_buf[1]; // 送られてきたデータの数
            if (c > 5) c = 5; // 最大で5個まで
            f = cmd_buf[2]; // 書換え開始位置
            p = 3; // 現在のコマンドバッファ読み込み位置
            for (i=0; i<c; i++) {
                npx_obj.setPixelColor(f+i, cmd_buf[p], cmd_buf[p+1], cmd_buf[p+2]);
                p += 3;
            }
            if (cmd == 0x08) npx_obj.show();
        }
        // 0x00 を返す
        Wire.write(0x00);
      
    } else if (cmd == 0x09) {
        // NeoPixcel 現在のデータを LEDに送る
        npx_obj.show();
        // 0x00 を返す
        Wire.write(0x00);
        
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
    t = setting_data[i+3];
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
      t = setting_data[i+3];
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

  // ロータリーエンコーダ読み込み
  for (i=0; i<16; i+=2) {
    if (rotary_list[i] != 0xFF && rotary_list[i+1] != 0xFF) {
      t = i / 2;
      if (rotary_count[t] < 0) {
        set_read_buf(r);
      } else if (rotary_count[t] > 0) {
        set_read_buf(r+1);
      }
      r += 2;
    }
  }

  // 読み込んだ内容を送信バッファにコピー
  memcpy(send_buf, read_buf, sizeof(send_buf));

  delay(5);
}
