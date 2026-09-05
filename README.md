# input-device

Arduino / ESP32 向けのハードウェア入力管理ライブラリです。  
ボタン、トグルスイッチ、ジョイスティック入力を扱う基本クラスと、状態管理しやすい仮想入力クラスを提供します。

- 基本入力クラス: `HardwareInput.h`
- 仮想入力クラス: `VirtualInput.h`

## 提供クラス

### `Kiban::Button`
- 1つのデジタル入力をボタンとして扱います
- `attach(pin, pullup)` で初期化し、`readPressed()` で押下状態を取得します

### `Kiban::Toggle`
- 2つのボタン入力から上/下のトグル入力を作ります
- `readTilted()` の戻り値は **上: 1 / 中立: 0 / 下: -1** です

### `Kiban::Joystick`
- X/Y のアナログ入力 + 押し込みボタンを扱います
- `setCenter()` で現在値を中心として補正できます
- `readX()`, `readY()` は中心との差分を返します

### `StateSelectorFunc<T>`
- 任意の入力関数を仮想プッシュスイッチとして扱うテンプレートクラスです
- `update()` をループ内で呼ぶと、押下エッジで状態を進めます
- `state_num` は循環する状態数、`retrriger_delay` は再入力を無視する時間[ms]です

### `StateSelectorMember<Btn, T>`
- 任意の入力クラスを仮想プッシュスイッチとして扱うテンプレートクラスです
- `update()` をループ内で呼ぶと、押下エッジで状態を進めます
- `state_num` は循環する状態数、`retrriger_delay` は再入力を無視する時間[ms]です

### `VectorStick<Joy>`
- ジョイスティック入力を更新し、デッドゾーン処理後の値で判定できます
- `calcRadius()`, `calcAngleRad()`, `calcAngleDeg()` で極座標値を取得できます
- `isInnerXY(...)`, `isInnerRTheta(...)` で範囲判定できます

## 基本的な使い方

```cpp
#include "HardwareInput.h"
#include "VirtualInput.h"

const uint8_t PIN_BUTTON = 2;
Kiban::Button button;
StateSelectorMember<Kiban::Button, bool> modeSwitch(button, 3, 20);

void setup() {
  button.attach(&PIN_BUTTON, true);
  modeSwitch.attachFunc(&Kiban::Button::readPressed);
}

void loop() {
  modeSwitch.update();
  int mode = modeSwitch.getState();
}
```

## ジョイスティックの例

```cpp
#include "HardwareInput.h"
#include "VirtualInput.h"

const uint8_t PIN_X = 34;
const uint8_t PIN_Y = 35;
const uint8_t PIN_SW = 25;

Kiban::Joystick joystick;
VectorStick<Kiban::Joystick> vjoy(joystick);

void setup() {
  joystick.attach(&PIN_X, &PIN_Y, &PIN_SW, true);
  joystick.setCenter();
  vjoy.attachFunc(&Kiban::Joystick::readX, &Kiban::Joystick::readY, 20, 20);
}

void loop() {
  vjoy.update();
  if (vjoy.isInnerRTheta(100, 2000, 315, 45)) {
    // 右方向の入力
  }
}
```

## 注意

- `update()` を定期的に呼ばないと仮想入力クラスの状態は更新されません
- `Kiban::Button` と `Kiban::Joystick` の押し込みボタン入力は、内蔵あるいは外部の抵抗を用いたプルアップ状態（押下時 LOW）を前提としています

---
最終更新日 2026-09-05  
更新者 Tomoooji  
