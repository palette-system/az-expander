# az-expander(キースキャンに特化したIOエキスパンダ)

## 基本仕様
|  内容  |  説明  |
|  --  |  --  |
|  MCU  |  ATTiny826  |
|  動作電圧  |  1.8V ～ 5.5V  |
|  接続方法  |  I2C  |
|  I2Cアドレス  |  0x30 (コマンドにより変更可能)  |
|  I/O数  |  16  |

<br><br>

## コマンド

### 0x01　コンフィグの設定

<b>■ 送信 ： 20 byte </b>
|  内容  |  説明  |  サイズ(byte)  |
|  --  |  --  |  --  |
|  コマンドタイプ  |  0x01 (固定)  |  1  |
|  変更後のI2Cアドレス  |  接続先のアドレスを変更できます  |  1  |
|  マトリックススキャンのタイプ  |  0 = 通常マトリックス<br>1 = 倍マトリックス  |  1  |
|  RGB_LEDに接続されているLEDの数  |  0～128  |  1  |
|  各ピンのタイプ  |  1バイト1ピンで16ピン分のI/Oを設定します<br><br>0 = 未使用<br>1 = ダイレクトインプット<br>2 = COL インプット<br>3 = ROW インプット<br>4 = ロータリーエンコーダA<br>5 = ロータリーエンコーダB<br>6 = ロータリーエンコーダC<br>7 = ロータリーエンコーダD<br>8 = ロータリーエンコーダE<br>9 = ロータリーエンコーダF<br>10 = ロータリーエンコーダG<br>11 = ロータリーエンコーダH<br>12 = RGB_LED  |  16  |

<b>■ レスポンス ： 2 byte </b>
|  内容  |  説明  |  サイズ(byte)  |
|  --  |  --  |  --  |
|  ステータス  |  1 = 変更なし<br>2 = 変更あり  |  1  |
|  変更後のI2Cアドレス  |  　  |  1  |

<br><br>

### 0x02　現在のコンフィグ取得

<b>■ 送信 ： 1 byte </b>
|  内容  |  説明  |  サイズ(byte)  |
|  --  |  --  |  --  |
|  コマンドタイプ  |  0x02 (固定)  |  1  |

<b>■ レスポンス ： 19 byte </b>
|  内容  |  説明  |  サイズ(byte)  |
|  --  |  --  |  --  |
|  AZ-EXPANDER本体のI2Cアドレス  |  　  |  1  |
|  マトリックススキャンのタイプ  |  ※ 詳細はコンフィグの設定参照  |  1  |
|  RGB_LEDに接続されているLEDの数  |  　  |  1  |
|  各ピンのタイプ  |  ※ 詳細はコンフィグの設定参照  |  16  |

<br><br>

### 0x03　現在の読み込むキーの数とバイト数を取得

<b>■ 送信 ： 1 byte </b>
|  内容  |  説明  |  サイズ(byte)  |
|  --  |  --  |  --  |
|  コマンドタイプ  |  0x03 (固定)  |  1  |

<b>■ レスポンス ： 2 byte </b>
|  内容  |  説明  |  サイズ(byte)  |
|  --  |  --  |  --  |
|  読み込むキーの数  |  0 ～ 128  |  1  |
|  レスポンスのバイト数  |  0 ～ 16  |  1  |

<br><br>

### 0x04　現在のスキャンデータを受け取る

<b>■ 送信 ： 1 byte </b>
|  内容  |  説明  |  サイズ(byte)  |
|  --  |  --  |  --  |
|  コマンドタイプ  |  0x04 (固定)  |  1  |

<b>■ レスポンス ： 0 ～ 16 byte </b>
|  内容  |  説明  |  サイズ(byte)  |
|  --  |  --  |  --  |
|  スキャンした結果  |  1キー1bitでコンフィグで設定されているキー分データを返します<br><br>0 = OFF<br>1 = ON  |  0 ～ 16  |

<br><br>

### 0x05　全ての RGB_LED の色を指定した色にする（データを書き換えるのみ）

<b>■ 送信 ： 4 byte </b>
|  内容  |  説明  |  サイズ(byte)  |
|  --  |  --  |  --  |
|  コマンドタイプ  |  0x05 (固定)  |  1  |
|  RGBの色データ  |  各1バイトずつ赤緑青  |  3  |

<b>■ レスポンス ： 1 byte </b>
|  内容  |  説明  |  サイズ(byte)  |
|  --  |  --  |  --  |
|  ステータス  |  0x00（固定）  |  1  |

<br><br>

### 0x06　全ての RGB_LED の色を指定した色にする（データ書き換え後表示もする）

<b>■ 送信 ： 4 byte </b>
|  内容  |  説明  |  サイズ(byte)  |
|  --  |  --  |  --  |
|  コマンドタイプ  |  0x06 (固定)  |  1  |
|  RGBの色データ  |  各1バイトずつ赤緑青  |  3  |

<b>■ レスポンス ： 1 byte </b>
|  内容  |  説明  |  サイズ(byte)  |
|  --  |  --  |  --  |
|  ステータス  |  0x00（固定）  |  1  |

<br><br>

### 0x07　指定した RGB_LED の色を指定した色にする（データを書き換えるのみ）

<b>■ 送信 ： 5 ～ 17 byte </b>
|  内容  |  説明  |  サイズ(byte)  |
|  --  |  --  |  --  |
|  コマンドタイプ  |  0x07 (固定)  |  1  |
|  RGB_LED個数色を変えるか  |  1 ～ 5  |  1  |
|  何個目から書き換えるか  |  0スタートの接続されているRGB_LED以下の数字  |  1  |
|  1個目のRGBの色データ  |  各1バイトずつ赤緑青  |  3  |
|  2個目のRGBの色データ  |  各1バイトずつ赤緑青  |  3  |
|  3個目のRGBの色データ  |  各1バイトずつ赤緑青  |  3  |
|  4個目のRGBの色データ  |  各1バイトずつ赤緑青  |  3  |
|  5個目のRGBの色データ  |  各1バイトずつ赤緑青  |  3  |

<b>■ レスポンス ： 1 byte </b>
|  内容  |  説明  |  サイズ(byte)  |
|  --  |  --  |  --  |
|  ステータス  |  0x00（固定）  |  1  |

<br><br>

### 0x08　指定した RGB_LED の色を指定した色にする（表示も行う）

<b>■ 送信 ： 5 ～ 17 byte </b>
|  内容  |  説明  |  サイズ(byte)  |
|  --  |  --  |  --  |
|  コマンドタイプ  |  0x08 (固定)  |  1  |
|  RGB_LED個数色を変えるか  |  1 ～ 5  |  1  |
|  何個目から書き換えるか  |  0スタートの接続されているRGB_LED以下の数字  |  1  |
|  1個目のRGBの色データ  |  各1バイトずつ赤緑青  |  3  |
|  2個目のRGBの色データ  |  各1バイトずつ赤緑青  |  3  |
|  3個目のRGBの色データ  |  各1バイトずつ赤緑青  |  3  |
|  4個目のRGBの色データ  |  各1バイトずつ赤緑青  |  3  |
|  5個目のRGBの色データ  |  各1バイトずつ赤緑青  |  3  |

<b>■ レスポンス ： 1 byte </b>
|  内容  |  説明  |  サイズ(byte)  |
|  --  |  --  |  --  |
|  ステータス  |  0x00（固定）  |  1  |

<br><br>

### 0x09　メモリに確保しているデータを RGB_LED に送信する（表示する）

<b>■ 送信 ： 1 byte </b>
|  内容  |  説明  |  サイズ(byte)  |
|  --  |  --  |  --  |
|  コマンドタイプ  |  0x09 (固定)  |  1  |

<b>■ レスポンス ： 1 byte </b>
|  内容  |  説明  |  サイズ(byte)  |
|  --  |  --  |  --  |
|  ステータス  |  0x00（固定）  |  1  |

<br><br>

## ボード設定
<img src="/images/board.png" width="500"><br>
