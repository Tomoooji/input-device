/**
 * @file VirtualInput.h
 * @brief 
 * 
 * @author Tomoooji (https://github.com/Tomoooji)
 * @version 0.1
 * @date 2026-09-05
 * @copyright Copyright (c) 2026
 */

#pragma once
#include <Arduino.h>
#include <HardwareInput.h>

template <typename T = bool>
class StateSelector{
protected:
  bool is_pressed = false;
  const int state_num;
  int current_state;
  const unsigned long retrriger_delay;
  unsigned long last_release_time;
  
public:
  StateSelector(const int state_num, const unsigned long retrriger_delay)
   : state_num(state_num), current_state(0), retrriger_delay(retrriger_delay), last_release_time(0) {}

  virtual void update() = 0;

  /**
   * @brief 現在状態を取得する
   * @return 現在状態
   */
  int getState() const {
    return this->current_state;
  }

protected:
  void _update(T reading){
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
  
};

template <typename T = bool>
class StateSelectorFunc : public StateSelector<T>{
private:
  T (*read_func)(void*);
  void* context;
public:
  StateSelectorFunc(const int state_num, const unsigned long retrriger_delay)
   : read_func(nullptr), context(nullptr),StateSelector<T>(state_num, retrriger_delay){}

  void attachFunc(){
    this->read_func = readFunction;
    this->context = context;
  }

  void update() override{
    if(this->read_func != nullptr){
      this->_update(this->read_func(this->context));
    }
  }
};


/**
 * @brief 任意入力クラスを仮想プッシュスイッチとして扱うテンプレートクラス
 * @tparam Btn 入力クラス型
 * @tparam T 入力値型（bool, int など）
 */
template<class Btn = Kiban::Button, typename T = bool>
class StateSelectorMember : public StateSelector<T> {
private:
  Btn& object;
  T(Btn::*read_func)() = nullptr;  //function pointer

public:
  /**
   * @brief コンストラクタ
   * @param object 入力読み取り元オブジェクト
   * @param state_num 状態数（状態は 0 〜 state_num-1 を循環）
   * @param retrriger_delay 押下判定後に再入力を無視する時間[ms]
   */
  StateSelectorMember(Btn& object, const int state_num = 2, const unsigned long retrriger_delay = 10)
    : object(object), StateSelector<T>(state_num, retrriger_delay) {
  }

  /**
   * @brief 入力読み取り用メンバ関数を登録する
   * @param read_function 入力を返すメンバ関数ポインタ
   */
  void attachFunc(T (Btn::*read_function)()) {
    this->read_func = read_function;
  }

  /**
   * @brief 押下状態を更新し、必要に応じて状態を遷移させる
   */
  void update() override {
    if (this->read_func != nullptr) {
      this->_update((object.*read_func)());
    }
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
class VectorStick {
private:
  Joy& joystick;
  int (Joy::*x_read_func)() = nullptr;
  int (Joy::*y_read_func)() = nullptr;
  int x_value = 0;
  int y_value = 0;
  int x_ignore_range = 20;
  int y_ignore_range = 20;

public:
  /**
   * @brief コンストラクタ
   * @param joystick 入力読み取り元ジョイスティックオブジェクト
   */
  VectorStick(Joy& joystick): joystick(joystick) {}

  /**
   * @brief X/Y入力読み取り関数とデッドゾーンを設定する
   * @param xReadFunction X軸入力読み取り関数
   * @param yReadFunction Y軸入力読み取り関数
   * @param x_ignore_range X軸デッドゾーン閾値
   * @param y_ignore_range Y軸デッドゾーン閾値
   */
  void attachFunc(int (Joy::*xReadFunction)(), int (Joy::*yReadFunction)(), int x_ignore_range = 20, int y_ignore_range = 20) {
    this->x_read_func = xReadFunction;
    this->y_read_func = yReadFunction;
    this->x_ignore_range = x_ignore_range;
    this->y_ignore_range = y_ignore_range;
  }

  /**
   * @brief X/Y入力値を更新し、デッドゾーン処理を適用する
   */
  void update() {
    if (this->x_read_func != nullptr && this->y_read_func != nullptr) {
      this->x_value = (joystick.*x_read_func)();
      this->x_value = (abs(this->x_value) < this->x_ignore_range) ? 0 : this->x_value;
      this->y_value = (joystick.*y_read_func)();
      this->y_value = (abs(this->y_value) < this->y_ignore_range) ? 0 : this->y_value;
    }
  }

  /**
   * @brief 現在の入力半径を計算する
   * @return 半径（sqrt(x^2 + y^2)）
   */
  float calcRadius() const {
    if (this->x_read_func != nullptr && this->y_read_func != nullptr) {
      #ifdef(ESP32)
        return hypotf(this->x_value, this->y_value);
      #else
        return sqrt(sq(this->x_value) + sq(this->y_value));
      #endif
    }
    return 0.0f;
  }

  /**
   * @brief 現在の入力角度をラジアンで計算する
   * @return 角度[rad]（0 〜 2π）
   */
  float calcAngleRad() const {
    if (this->x_read_func != nullptr && this->y_read_func != nullptr) {
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
  bool isInnerXY(int x_area[2], int y_area[2]){
    return this->isInnerXY(x_area[0],x_area[1],y_area[0],y_area[1]);
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
  bool isInnerrTheta(float r_range[2], float theta_range[2]){
    return this->isInnerRTheta(r_range[0],r_range[1],theta_range[0],theta_range[1]);
  }
};
