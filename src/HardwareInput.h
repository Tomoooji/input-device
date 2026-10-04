/**
 * @file HardwareInput.h
 * @brief Arduino / ESP32 向けハードウェア入力管理クラス群
 *
 * ボタン・トグル・ジョイスティックの入力クラスを提供します。
 *
 * @author Tomoooji (https://github.com/Tomoooji)
 * @version 1.0
 * @date 2026-10-04
 * @copyright Copyright (c) 2026
 */

#pragma once
#include <utility>
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
   * @param pin ピンの参照
   * @param pullup 内部プルアップの有効化
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
   * @brief コンストラクタ(ボタンオブジェクト用)
   * @param up 上側のボタン入力オブジェクト(その場で宣言可能)
   * @param down 下側のボタン入力オブジェクト(その場で宣言可能)
   */
  Toggle(Button up, Button down) : up(std::move(up)), down(std::move)) {}
  
  /**
   * @brief コンストラクタ(個別ピン用)
   * @param pin_up 上側のボタン入力ピンの参照
   * @param pin_down 下側のボタン入力ピンの参照
   * @param pullup_up 上側のボタンの内部プルアップ有効化
   * @param pullup_down 下側のボタンの内部プルアップ有効化
   */
  Toggle(const uint8_t &pin_up, const uint8_t &pin_down, bool pullup_up = true, bool pullup_down = true)
     : up(pin_up, pullup_up), down(pin_down, pullup_down) {}

  /**
   * @brief コンストラクタ(ピン配列用)
   * @param pins ボタン入力ピン配列（[0]:上, [1]:下）の参照
   * @param pullups ボタンの内部プルアップ有効化配列（[0]:上, [1]:下）
   */
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
  const uint8_t &pinX;
  const uint8_t &pinY;
  int xCenter = 1 << (ADC_RESOLUTION - 1);
  int yCenter = 1 << (ADC_RESOLUTION - 1);

public:
  /**
   * @brief コンストラクタ(個別ピン用)
   * @param pinX X軸アナログ入力ピンの参照
   * @param pinY Y軸アナログ入力ピンの参照
   * @param pinButton 押し込みボタン入力ピンの参照
   * @param pullup ボタン入力の内部プルアップ有効化
   */
  Joystick(const uint8_t &pinX, const uint8_t &pinY, const uint8_t &pinButton, bool pullup = true)
     : pinX(pinX), pinY(pinY), button(pinButton, pullup) {}

  /**
   * @brief コンストラクタ(ピン配列用)
   * @param pins 入力ピン配列（[0]:X, [1]:Y, [2]:ボタン）の参照
   * @param pullup ボタン入力の内部プルアップ有効化
   */
  Joystick(const uint8_t (&pins)[3], bool pullup = true)
     : pinX(pins[0]), pinY(pins[1]), button(pins[2], pullup) {}

  /**
   * @brief ジョイスティックの各ピンを設定する
   */
  void begin() {
    button.begin();
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
    this->setCenter(analogRead(this->pinX), analogRead(this->pinY));
  }

  /**
   * @brief X軸の中心差分値を取得する
   * @return X軸差分値
   */
  int readX() {
    return analogRead(this->pinX) - this->xCenter;
  }

  /**
   * @brief Y軸の中心差分値を取得する
   * @return Y軸差分値
   */
  int readY() {
    return analogRead(this->pinY) - this->yCenter;
  }

  /**
   * @brief 押し込みボタン押下状態を取得する
   * @return 押下中なら true
   */
  bool readPressed() {
    return this->button.readPressed();
  }
};

enum ENCODER_TYPE {
  SINGLEEDGE, HALFQUAD, FULLQUAD
};

#ifdef ESP32
#include <ESP32Encoder.h>

template <ENCODER_TYPE TYPE = HALFQUAD>
class Encoder {
private:
  ESP32Encoder encoder_;
  const uint8_t &pinA;
  const uint8_t &pinB;
  const bool pulluped;
  const long lap_count;
public:
  Encoder(const uint8_t &pinA, const uint8_t &pinB, const bool pullup = true, const long lapCount = 0)
   : encoder_(), pinA(pinA), pinB(pinB), pulluped(pullup), lap_count(lapCount) {}
  Encoder(const uint8_t (&pins)[2], const bool pullup = true, const long lapCount = 0)
   : Encoder(pins[0], pins[1], pullup, lapCount) {}
  
  void begin() {
    if (this->pulluped) ESP32Encoder::useInternalWeakPullResistors = puType::up;
    switch (TYPE) {
      case SINGLEEDGE:
        this->encoder_.attachSingleEdge(this->pinA, pinB);
        break;
      case HALFQUAD:
        this->encoder_.attachHalfQuad(this->pinA, pinB);
        break;
      case FULLQUAD:
        this->encoder_.attachFullQuad(this->pinA, pinB);
        break;
    }
    this->encoder_.clearCount();
  }
  
  void reset() { this->encoder_.clearCount(); }
  
  int read() { return this->encoder.getCount(); }
  
  float getAngleDeg() { return this->read() / this->lap_count * 180; }
  
  float getAngleRad() { return this->read() / this->lap_count * TWO_PI; }
};

#else

template <uint8_t ID, ENCODER_TYPE TYPE>
class EncoderClass {
private:
  inline staic Encoder<TYPE, ID> *instance;
  const uint8_t &pinA;
  const uint8_t &pinB;
  const bool pulluped;
  consr long lap_count;
  std::atomic<long> count;
  
  static void countISR () {
    instance_->count.store(instance_->count.load() +
      (digitalRead(instance->pinA) == digitalRead(instance->pinB) ? 1 : -1)
    );
  }

piblic:
  Encoder(const uint8_t &pinA, const uint8_t &pinB, const bool pullup = true, const long lapCount = 0)
   : pinA(pinA), pinB(pinB), pulluped(pullup), lap_count(lapCount) {}
  
  Encoder(const uint8_t (&pins)[2], const bool pullup = true, const long lapCount = 0)
   : Encoder(pins[0], pins[1], pullup, lapCount) {}

  void begin() {
    pinMode(this->pinA, this->pulluped ? INPUT_PULLUP : INPUT);
    pinMode(this->pinB, this->pulluped ? INPUT_PULLUP : INPUT);
    switch (TYPE) {
      case SINGLEDGE:
        attachInterrupt(digitalPinToInterrupt(this->pinA), countISR, RASING);
        break;
      case HALFQUAD:
        attachInterrupt(digitalPinToInterrupt(this->pinA), countISR, CHANGE);
        break;
      case FULLQUAD:
        attachInterrupt(digitalPinToInterrupt(this->pinA), countISR, CHANGE);
        attachInterrupt(digitalPinToInterrupt(this->pinB), countISR, CHANGE);
        break;
    }
  }
  
  void reset() { return this->count.store(0); }
  
  int read() { return this->count.load(); }
  
  float getAngleDeg() { return this->read() / this->lap_count * 180; }
  
  float getAngleRad() { return this->read() / this->lap_count * TWO_PI; }
};

#define Encoder(ENCODER_TYPE) EncoderClass<__COUNTER__, ENCODER_TYPE>

#endif

};
