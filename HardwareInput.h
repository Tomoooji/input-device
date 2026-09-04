/**
 * @file HardwareInput.h
 * @brief 
 * 
 * @author Tomoooji (https://github.com/Tomoooji)
 * @version 0.1
 * @date 2026-09-03
 * @copyright Copyright (c) 2026
 */

#pragma once
#include <Arduino.h>

namespace Kiban{
class Button{
private:
  const uint8_t& pin;
  bool pulluped;
public:
  Button() = default;
  void attach(const uint8_t& pin, bool pullup = true) {
    this->pin = pin;
    this->pulluped = pullup;
    #if defined(ESP32)
      pinMode(pin, this->pulluped && pin < 34 ? INPUT_PULLUP : INPUT);
    #else
      pinMode(pin, this->pulluped ? INPUT_PULLUP : INPUT);
    #endif
  }
  bool isPlessed() {
    return digitalRead(pin) != this->pulluped;
  }
};

class Toggle{
private:
  Button button[2];
public:
  Toggle() = default;
  void attach(const uint8_t& pinUp, const uint8_t& pinDown, const bool pullupUp = true, const bool pullupDown = true) {
    button[0].attach(pinUp, pullupUp);
    button[1].attach(pinDown, pullupDown);
  }
  void attach(const uint8_t const pins[2], const bool pullups[2] = {true,true}) {
    button[0].attach(pins[0], pullups[0]);
    button[1].attach(pins[1], pullups[1]);
  }
  int getState() {
    return button[0].isPushed() - button[1].isPushed();
  }
};

#ifndef ADC_RESOLUTION
#if defined(ARDUINO_ARCH_ESP32)
#define ADC_RESOLUTION 12
#else
#define ADC_RESOLUTION 10
#endif
#endif

class Joystick{
private:
  Button button;
  const uint8_t& pinX;
  const uint8_t& pinY;
  int xCenter = 2 << (ADC_RESOLUTION - 1);
  int yCenter = 2 << (ADC_RESOLUTION - 1);
public:
  Joystick() = default;
  void attach(const uint8_t& pinX, const uint8_t& pinY, const uint8_t& pinButton, const bool pullupButton = true) {
    this->pinX = pinX;
    this->pinY = pinY;
    button.attach(pinButton, pullupButton);
  }
  void attach(const uint8_t const pins[3], const bool pullup = true) {
    this->pinX = pins[0];
    this->pinY = pins[1];
    button.attach(pins[2], pullup);
  }
  void setCenter(int xCenter, int yCenter) {
    this->xCenter = xCenter;
    this->yCenter = yCenter;
  }
  int Xvalue() {
    return analogRead(pinX);
  }
  int Yvalue() {
    return analogRead(pinY);
  }
  bool isPushed() {
    return button.isPushed();
  }
};
};

template <class Btn = Kiban::Button, typename T = bool>
class vPushSwitch {
private:
  Btn& object;
  T(Btn::*update_func)() = nullptr; //function pointer
  bool is_pressed = false;
  const int state_num;
  int current_state;
  const unsigned long ignore_time;
  unsigned long last_release_time;

public:
  vPushSwitch(Btn& object, T(Btn::*)() update_function ,const int state_num = 2, const unsigned long ignore_time = 10)
    : object(object), update_func(update_function), state_num(state_num), current_state(0), ignore_time(ignore_time), last_release_time(0) {
  }

  vPushSwitch(Btn& object, const int state_num = 2, const unsigned long ignore_time = 10)
    : object(object), state_num(state_num), current_state(0), ignore_time(ignore_time), last_release_time(0) {
  }

  void attachFunc(T(Btn::*)() update_function){
    this->update_func = update_function;
  }

  void update(){
    if(update_func != nullptr){
      T reading = (object.*update_func)();
      if(reading && !this->is_pressed){
        if (millis() - this->last_release_time > this->ignore_time) {
          this->current_state = (this->current_state + reading) % this->state_num;
          this->is_pressed = true;
        }
      } else if (!reading && this->is_pressed) {
        this->is_pressed = false;
        this->last_release_time = millis();
      }
    }
  }
  
  int getState() const {
    return this->current_state;
  }
};

float clip2pi(float rad) {
  return rad < 0 ? rad + 2 * PI : (rad >= 2 * PI ? rad - 2 * PI : rad);
}

template <class Joy = Kiban::Joystick>
class vJoyStick{
private:
  Joy& joystick;
  int (Joy::*x_update_func)() = nullptr;
  int (Joy::*y_update_func)() = nullptr;
  int x_value = 0;
  int y_value = 0;
  int x_ignore_range;
  int y_ignore_range;

public:
  vJoyStick(Joy& joystick) : joystick(joystick) {}

  void attachFunc(int(Joy::*x_update_function)(), int(Joy::*y_update_function)(), int x_ignore_range = 10, int y_ignore_range = 10) {
    this->x_update_func = x_update_function;
    this->y_update_func = y_update_function;
    this->x_ignore_range = x_ignore_range;
    this->y_ignore_range = y_ignore_range;
  }

  void update(){
    if(x_update_func != nullptr && y_update_func != nullptr){
      this->x_value = (joystick.*x_update_func)();
      this->x_value = (abs(this->x_value) < this->x_ignore_range) ? 0 : this->x_value;
      this->y_value = (joystick.*y_update_func)();
      this->y_value = (abs(this->y_value) < this->y_ignore_range) ? 0 : this->y_value;
    }
  }

  float calcRadius() const {
    if(x_update_func != nullptr && y_update_func != nullptr){
      return sqrt(pow(this->x_value, 2) + pow(this->y_value, 2));
    }
    return 0.0f;
  }

  float calcAngleRad() const {
    if(x_update_func != nullptr && y_update_func != nullptr){
      return clip2pi(atan2(this->y_value, this->x_value));
    }
    return 0.0f;
  }

  float calcAngleDeg() const {
    return degrees(calcAngleRad());
  }

  bool isInnerXY(int x_min, int x_max, int y_min, int y_max) const {
    return (this->x_value >= x_min && this->x_value <= x_max && this->y_value >= y_min && this->y_value <= y_max);
  }

  bool isInnerRTheta(float r_min, float r_max, float theta_min, float theta_max) const {
    float radius = calcRadius();
    float angle = calcAngleDeg();
    return (radius >= r_min && radius <= r_max) && (theta_max > theta_min ? (angle >= theta_min && angle <= theta_max) : (angle >= theta_min || angle <= theta_max));
  }
};
