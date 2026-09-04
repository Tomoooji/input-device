/**
 * @file HardwareInput.h
 * @brief Arduino / ESP32 向けハードウェア入力管理クラス群
 *
 * ボタン・トグル・ジョイスティックの入力クラスと、
 * それらを高レベルに扱う仮想入力クラス（vPushSwitch, vJoyStick）を提供します。
 *
 * @author Tomoooji (https://github.com/Tomoooji)
 * @version 0.1
 * @date 2026-09-03
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
  const uint8_t* pin;
  bool pulluped;

public:
  /**
   * @brief デフォルトコンストラクタ
   */
  Button() = default;

  /**
   * @brief ボタン入力ピンを設定する
   * @param pin 入力ピン番号へのポインタ
   * @param pullup 内部プルアップを有効化するか（デフォルト: true）
   */
  void attach(const uint8_t* pin, bool pullup = true) {
    this->pin = pin;
    this->pulluped = pullup;
#if defined(ESP32)
    pinMode(*pin, this->pulluped && *pin < 34 ? INPUT_PULLUP : INPUT);
#else
    pinMode(*pin, this->pulluped ? INPUT_PULLUP : INPUT);
#endif
  }

  /**
   * @brief ボタン押下状態を読み取る
   * @return 押下中なら true
   */
  bool readPressed() {
    return !digitalRead(*pin);
  }
};

/**
 * @class Toggle
 * @brief 2つのボタン入力を使ったトグル（上/下）入力クラス
 */
class Toggle {
private:
  Button button[2];

public:
  /**
   * @brief デフォルトコンストラクタ
   */
  Toggle()
    : button{} {}

  /**
   * @brief 上側/下側トグル入力ピンを設定する
   * @param pinUp 上側入力ピン番号へのポインタ
   * @param pinDown 下側入力ピン番号へのポインタ
   * @param pullupUp 上側入力の内部プルアップ有効化
   * @param pullupDown 下側入力の内部プルアップ有効化
   */
  void attach(const uint8_t* pinUp, const uint8_t* pinDown, bool pullupUp = true, bool pullupDown = true) {
    button[0].attach(pinUp, pullupUp);
    button[1].attach(pinDown, pullupDown);
  }

  /**
   * @brief 配列で上側/下側トグル入力ピンを設定する
   * @param pins 入力ピン配列（[0]:上, [1]:下）
   * @param pullupUp 上側入力の内部プルアップ有効化
   * @param pullupDown 下側入力の内部プルアップ有効化
   */
  void attach(const uint8_t pins[2], bool pullupUp, bool pullupDown) {
    this->attach(&pins[0], &pins[1], pullupUp, pullupDown);
  }

  /**
   * @brief 現在の傾き状態を取得する
   * @return 上:1 / 中立:0 / 下:-1
   */
  int readTilted() {
    return button[0].readPressed() - button[1].readPressed();
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
  Joystick(): button{} {}

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
  void attach(const uint8_t pins[2], bool pullup = true) {
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
    this->setCenter(analogRead(*pinX), analogRead(*pinY));
  }

  /**
   * @brief X軸の中心差分値を取得する
   * @return X軸差分値
   */
  int readX() {
    return analogRead(*pinX) - this->xCenter;
  }

  /**
   * @brief Y軸の中心差分値を取得する
   * @return Y軸差分値
   */
  int readY() {
    return analogRead(*pinY) - this->yCenter;
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

/**
 * @brief 任意入力クラスを仮想プッシュスイッチとして扱うテンプレートクラス
 * @tparam Btn 入力クラス型
 * @tparam T 入力値型（bool, int など）
 */
template<class Btn = Kiban::Button, typename T = bool>
class vPushSwitch {
private:
  Btn& object;
  T(Btn::*update_func)() = nullptr;  //function pointer
  bool is_pressed = false;
  const int state_num;
  int current_state;
  const unsigned long ignore_time;
  unsigned long last_release_time;

public:
  /**
   * @brief コンストラクタ
   * @param object 入力読み取り元オブジェクト
   * @param state_num 状態数（状態は 0 〜 state_num-1 を循環）
   * @param ignore_time 押下判定後に再入力を無視する時間[ms]
   */
  vPushSwitch(Btn& object, const int state_num = 2, const unsigned long ignore_time = 10)
    : object(object), state_num(state_num), current_state(0), ignore_time(ignore_time), last_release_time(0) {
  }

  /**
   * @brief 入力読み取り用メンバ関数を登録する
   * @param update_function 入力を返すメンバ関数ポインタ
   */
  void attachFunc(T (Btn::*update_function)()) {
    this->update_func = update_function;
  }

  /**
   * @brief 押下状態を更新し、必要に応じて状態を遷移させる
   */
  void update() {
    if (update_func != nullptr) {
      T reading = (object.*update_func)();
      if (reading && !this->is_pressed) {
        if (millis() - this->last_release_time > this->ignore_time) {
          this->current_state = (this->state_num + this->current_state + reading) % this->state_num;
          this->is_pressed = true;
        }
      } else if (!reading && this->is_pressed) {
        this->is_pressed = false;
        this->last_release_time = millis();
      }
    }
  }

  /**
   * @brief 現在状態を取得する
   * @return 現在状態
   */
  int getState() const {
    return this->current_state;
  }
};

/**
 * @brief 角度[rad]を 0 〜 2π に正規化する
 * @param rad 角度[rad]
 * @return 正規化後の角度[rad]
 */
float clip2pi(float rad) {
  return rad < 0 ? rad + 2 * PI : (rad >= 2 * PI ? rad - 2 * PI : rad);
}

/**
 * @brief ジョイスティック入力を仮想的に扱うテンプレートクラス
 * @tparam Joy ジョイスティッククラス型
 */
template<class Joy = Kiban::Joystick>
class vJoyStick {
private:
  Joy& joystick;
  int (Joy::*x_update_func)() = nullptr;
  int (Joy::*y_update_func)() = nullptr;
  int x_value = 0;
  int y_value = 0;
  int x_ignore_range;
  int y_ignore_range;

public:
  /**
   * @brief コンストラクタ
   * @param joystick 入力読み取り元ジョイスティックオブジェクト
   */
  vJoyStick(Joy& joystick): joystick(joystick) {}

  /**
   * @brief X/Y入力読み取り関数とデッドゾーンを設定する
   * @param x_update_function X軸入力読み取り関数
   * @param y_update_function Y軸入力読み取り関数
   * @param x_ignore_range X軸デッドゾーン閾値
   * @param y_ignore_range Y軸デッドゾーン閾値
   */
  void attachFunc(int (Joy::*x_update_function)(), int (Joy::*y_update_function)(), int x_ignore_range = 10, int y_ignore_range = 10) {
    this->x_update_func = x_update_function;
    this->y_update_func = y_update_function;
    this->x_ignore_range = x_ignore_range;
    this->y_ignore_range = y_ignore_range;
  }

  /**
   * @brief X/Y入力値を更新し、デッドゾーン処理を適用する
   */
  void update() {
    if (x_update_func != nullptr && y_update_func != nullptr) {
      this->x_value = (joystick.*x_update_func)();
      this->x_value = (abs(this->x_value) < this->x_ignore_range) ? 0 : this->x_value;
      this->y_value = (joystick.*y_update_func)();
      this->y_value = (abs(this->y_value) < this->y_ignore_range) ? 0 : this->y_value;
    }
  }

  /**
   * @brief 現在の入力半径を計算する
   * @return 半径（sqrt(x^2 + y^2)）
   */
  float calcRadius() const {
    if (x_update_func != nullptr && y_update_func != nullptr) {
      return sqrt(pow(this->x_value, 2) + pow(this->y_value, 2));
    }
    return 0.0f;
  }

  /**
   * @brief 現在の入力角度をラジアンで計算する
   * @return 角度[rad]（0 〜 2π）
   */
  float calcAngleRad() const {
    if (x_update_func != nullptr && y_update_func != nullptr) {
      return clip2pi(atan2(this->y_value, this->x_value));
    }
    return 0.0f;
  }

  /**
   * @brief 現在の入力角度を度数で計算する
   * @return 角度[deg]
   */
  float calcAngleDeg() const {
    return degrees(calcAngleRad());
  }

  /**
   * @brief 直交座標系の矩形範囲内にあるか判定する
   * @param x_min X下限
   * @param x_max X上限
   * @param y_min Y下限
   * @param y_max Y上限
   * @return 範囲内なら true
   */
  bool isInnerXY(int x_min, int x_max, int y_min, int y_max) const {
    return (this->x_value >= x_min && this->x_value <= x_max && this->y_value >= y_min && this->y_value <= y_max);
  }

  /**
   * @brief 極座標系の扇形範囲内にあるか判定する
   * @param r_min 半径下限
   * @param r_max 半径上限
   * @param theta_min 角度下限[deg]
   * @param theta_max 角度上限[deg]
   * @return 範囲内なら true
   */
  bool isInnerRTheta(float r_min, float r_max, float theta_min, float theta_max) const {
    float radius = calcRadius();
    float angle = calcAngleDeg();
    return (radius >= r_min && radius <= r_max) && (theta_max > theta_min ? (angle >= theta_min && angle <= theta_max) : (angle >= theta_min || angle <= theta_max));
  }
};
