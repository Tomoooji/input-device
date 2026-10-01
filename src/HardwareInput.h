/**
 * @file HardwareInput.h
 * @brief Arduino / ESP32 向けハードウェア入力管理クラス群
 *
 * ボタン・トグル・ジョイスティックの入力クラスを提供します。
 *
 * @author Tomoooji (https://github.com/Tomoooji)
 * @version 0.1
 * @date 2026-09-05
 * @copyright Copyright (c) 2026
 */

#pragma once
#include <Arduino.h>

/**
 * @namespace Kiban
 * @brief 基本入力デバイスクラスをまとめる名前空間
 */
namespace Kiban {

/**
 * @class Button
 * @brief デジタル入力1点をプッシュボタンとして扱うクラス
 */
class Button {
private:
  const uint8_t &pin;
  bool pulluped;

public:
  /**
   * @brief コンストラクタ
   */
  Button(const uint8_t& pin, bool pullup = true) : pin(pin), pulluped(pullup) {}

  /**
   * @brief ボタン入力ピンを設定する
   */
  void begin() {
#ifdef ESP32
    pinMode(this->pin, this->pulluped && thiz->pin < 34 ? INPUT_PULLUP : INPUT);
#else
    pinMode(this->pin, this->pulluped ? INPUT_PULLUP : INPUT);
#endif
  }

  /**
   * @brief ボタン押下状態を読み取る
   * @return 押下中なら true
   */
  bool readPressed() {
    return digitalRead(this->pin) == this->pulluped;
  }
};

/**
 * @class Toggle
 * @brief 2つのボタン入力を使ったトグル（上/下）入力クラス
 */
class Toggle {
private:
  Button up, bown;

public:
  /**
   * @brief コンストラクタ
   */
  Toggle(Button &&up, Button &&down) : up(std::move(up)), down(std::move)) {}
  Toggle(const uint8_t &pin_up, const uint8_t &pin_down, bool pullup_up = true, bool pullup_down = true)
     : up(pin_up, pullup_up), down(pin_down, pullup_down) {}
  Toggle(const uint8_t (&pins)[2], bool pullups[2] = nullptr)
     : up(pin_up, pullups == nullptr ? true : pullups[0]), down(pin_down, pullups == nullptr ? true : pullups[1]) {}

  /**
   * @brief 上側/下側トグル入力ピンを設定する
   */
  void begin() {
    up.begin();
    down.begin();
  }

  /**
   * @brief 現在の傾き状態を取得する
   * @return 上:1 / 中立:0 / 下:-1
   */
  int readTilted() {
    return up.readPressed() - down.readPressed();
  }
};

#ifndef ADC_RESOLUTION
#if defined(ARDUINO_ARCH_ESP32)
#define ADC_RESOLUTION 12
#else
#define ADC_RESOLUTION 10
#endif
#endif

/**
 * @class Joystick
 * @brief アナログ2軸 + ボタンのジョイスティック入力クラス
 */
class Joystick {
private:
  Button button;
  const uint8_t* pinX;
  const uint8_t* pinY;
  int xCenter = 1 << (ADC_RESOLUTION - 1);
  int yCenter = 1 << (ADC_RESOLUTION - 1);

public:
  /**
   * @brief デフォルトコンストラクタ
   */
  Joystick() : button{} {}

  /**
   * @brief ジョイスティックの各ピンを設定する
   * @param pinX X軸アナログ入力ピンへのポインタ
   * @param pinY Y軸アナログ入力ピンへのポインタ
   * @param pinButton 押し込みボタン入力ピンへのポインタ
   * @param pullupButton ボタン入力の内部プルアップ有効化
   */
  void attach(const uint8_t* pinX, const uint8_t* pinY, const uint8_t* pinButton, bool pullupButton = true) {
    this->pinX = pinX;
    this->pinY = pinY;
    button.attach(pinButton, pullupButton);
  }

  /**
   * @brief 配列でジョイスティックの各ピンを設定する
   * @param pins 入力ピン配列（[0]:X, [1]:Y, [2]:ボタン）
   * @param pullup ボタン入力の内部プルアップ有効化
   */
  void attach(const uint8_t pins[3], bool pullup = true) {
    this->pinX = &pins[0];
    this->pinY = &pins[1];
    button.attach(&pins[2], pullup);
  }

  /**
   * @brief ジョイスティック中心値を手動設定する
   * @param xCenter X軸中心値
   * @param yCenter Y軸中心値
   */
  void setCenter(int xCenter, int yCenter) {
    this->xCenter = xCenter;
    this->yCenter = yCenter;
  }

  /**
   * @brief 現在のアナログ値を中心値として設定する
   */
  void setCenter() {
    if (this->pinX != nullptr && this->pinY != nullptr) {
      this->setCenter(analogRead(*pinX), analogRead(*pinY));
    }
  }

  /**
   * @brief X軸の中心差分値を取得する
   * @return X軸差分値
   */
  int readX() {
    return this->pinX == nullptr ? 0 : analogRead(*pinX) - this->xCenter;
  }

  /**
   * @brief Y軸の中心差分値を取得する
   * @return Y軸差分値
   */
  int readY() {
    return this->pinY == nullptr ? 0 : analogRead(*pinY) - this->yCenter;
  }

  /**
   * @brief 押し込みボタン押下状態を取得する
   * @return 押下中なら true
   */
  bool readPressed() {
    return button.readPressed();
  }
};

};
