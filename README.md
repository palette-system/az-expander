# az-expander(キースキャンに特化したIOエキスパンダ)

<img src="/images/pcb_f.png" height="400">　<img src="/images/pcb_b.png" height="400">
<br><br><br>

## 説明

az-expander は キーボードの入力に特化したエキスパンダモジュールです。  
基板にTRRSジャックが取り付けられているのが特徴で、TRRSを利用してI2C通信を行います。  
ピンの配列は可能な限りProMicroと互換するよう設計していますが一部利用できないピンがあるため注意して下さい。  
<br><br>
制御用の通信仕様については　<a href="/docs/command.md">こちら</a>　を参照してください。  
<br><br>


|  内容  |  説明  |
|  --  |  --  |
|  MCU  |  ATTiny826  |
|  動作電圧  |  1.8V ～ 5.5V  |
|  接続方法  |  I2C  |
|  I2Cアドレス  |  0x30 (コマンドにより変更可能)  |
|  I/O数  |  16  |
|  マトリックススキャン  |  Matrix、Duplex-Matrix  |
|  ロータリエンコーダ  |  最大8個  |
|  RGB_LED  |  最大128個  |

<br><br>

## TRRSジャック

<table>
  <tr>
    <td>1</td>
    <td>T</td>
    <td>SCL</td>
    <td rowspan="4"><img src="/images/pj320a.png" width="400"></td>
  </tr>
  <tr>
    <td>2</td>
    <td>R</td>
    <td>SDA</td>
  </tr>
  <tr>
    <td>3</td>
    <td>R</td>
    <td>VCC</td>
  </tr>
  <tr>
    <td>4</td>
    <td>S</td>
    <td>GND</td>
  </tr>
</table>
<br><br>


## ProMicroピン互換表

<table>
  <tr>
    <th>No</th>
    <th>ProMicro</th>
    <th>az-expander</th>
    <td rowspan="13">　</td>
    <th>No</th>
    <th>ProMicro</th>
    <th>az-expander</th>
  </tr>
  <tr>
    <td>1</td>
    <td>D3</td>
    <td>NC</td>
    <td>24</td>
    <td>RAW</td>
    <td>VCC</td>
  </tr>
  <tr>
    <td>2</td>
    <td>D2</td>
    <td>NC</td>
    <td>23</td>
    <td>GND</td>
    <td>GND</td>
  </tr>
  <tr>
    <td>3</td>
    <td>GND</td>
    <td>GND</td>
    <td>22</td>
    <td>RESET</td>
    <td>NC</td>
  </tr>
  <tr>
    <td>4</td>
    <td>GND</td>
    <td>GND</td>
    <td>21</td>
    <td>VCC</td>
    <td>VCC</td>
  </tr>
  <tr>
    <td>5</td>
    <td>D1</td>
    <td>P0</td>
    <td>20</td>
    <td>F4</td>
    <td>P15</td>
  </tr>
  <tr>
    <td>6</td>
    <td>D0</td>
    <td>P1</td>
    <td>19</td>
    <td>F5</td>
    <td>P14</td>
  </tr>
  <tr>
    <td>7</td>
    <td>D4</td>
    <td>P2</td>
    <td>18</td>
    <td>F6</td>
    <td>P13</td>
  </tr>
  <tr>
    <td>8</td>
    <td>C6</td>
    <td>P3</td>
    <td>17</td>
    <td>F7</td>
    <td>P12</td>
  </tr>
  <tr>
    <td>9</td>
    <td>D7</td>
    <td>P4</td>
    <td>16</td>
    <td>B1</td>
    <td>P11</td>
  </tr>
  <tr>
    <td>10</td>
    <td>E6</td>
    <td>P5</td>
    <td>15</td>
    <td>F3</td>
    <td>P10</td>
  </tr>
  <tr>
    <td>11</td>
    <td>B4</td>
    <td>P6</td>
    <td>14</td>
    <td>B2</td>
    <td>P9</td>
  </tr>
  <tr>
    <td>12</td>
    <td>B5</td>
    <td>P7</td>
    <td>13</td>
    <td>B6</td>
    <td>P8</td>
  </tr>
</table>
<br><br>

## ジャンパVOについて
az-expander はデフォルトで21ピン、24ピンへVCCは接続されていません。  
これはOLEDやRGB_LEDなどの電力消費が多いパーツが取り付けられていた場合電力が足りなくなる可能性があるからです。  
OLEDやRGB_LEDへ給電したい場合にジャンパVOをショートさせてください。  
<br><br>


## ATTiny826 へ書き込みする時のボード設定
<img src="/images/board.png" width="500"><br>
