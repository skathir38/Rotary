/////////////////////////////////////////////////////////////////
/*
  ESP8266/Arduino Library for reading rotary encoder values.
  Copyright 2017-2021 Lennart Hennigs.
*/
/////////////////////////////////////////////////////////////////

#include "Rotary.h"

/////////////////////////////////////////////////////////////////

Rotary::Rotary() {
}

/////////////////////////////////////////////////////////////////

Rotary::Rotary(byte pin1, byte pin2, byte steps_per_click /* = 1 */,  int lower_bound /* = INT16_MIN */, int upper_bound /* = INT16_MAX */, int inital_pos /* = 0 */, int increment /* = 1 */) {
  begin(pin1, pin2, steps_per_click, lower_bound, upper_bound, inital_pos, increment);
}

/////////////////////////////////////////////////////////////////

void Rotary::begin(byte pin1, byte pin2, byte steps_per_click /* = 1 */,  int lower_bound /* = INT16_MIN */, int upper_bound /* = INT16_MAX */, int inital_pos /* = 0 */, int increment /* = 1 */) {
  this->pin1 = pin1;
  this->pin2 = pin2;
  pinMode(pin1, INPUT_PULLUP);
  pinMode(pin2, INPUT_PULLUP);

  setUpperBound(upper_bound);
  setLowerBound(lower_bound);
  setIncrement(increment);
  setStepsPerClick(steps_per_click);
  
  loop();
  resetPosition(inital_pos, false);
}

/////////////////////////////////////////////////////////////////

void Rotary::setUpperBound(int upper_bound) {
  this->upper_bound = (lower_bound < upper_bound) ? upper_bound: lower_bound;
}

/////////////////////////////////////////////////////////////////

void Rotary::setLowerBound(int lower_bound) {
  this->lower_bound = (lower_bound < upper_bound) ? lower_bound : upper_bound;
}

/////////////////////////////////////////////////////////////////

int Rotary::getUpperBound() {
  return this->upper_bound;
}

/////////////////////////////////////////////////////////////////

int Rotary::getLowerBound() {
  return this->lower_bound;
}

/////////////////////////////////////////////////////////////////

void Rotary::setChangedHandler(CallbackFunction f) {
  change_cb = f;
}

/////////////////////////////////////////////////////////////////

void Rotary::setRightRotationHandler(CallbackFunction f) {
  right_cb = f;
}

/////////////////////////////////////////////////////////////////

void Rotary::setLeftRotationHandler(CallbackFunction f) {
  left_cb = f;
}

/////////////////////////////////////////////////////////////////

void Rotary::setUpperOverflowHandler(CallbackFunction f) {
  upper_cb = f;
}

/////////////////////////////////////////////////////////////////

void Rotary::setLowerOverflowHandler(CallbackFunction f) {
  lower_cb = f;
}

/////////////////////////////////////////////////////////////////

void Rotary::resetPosition(int p /* = 0 */, bool fireCallback /* = true */) {
  if (p > upper_bound) {
    last_position = upper_bound * steps_per_click;
  } else {
    last_position = (lower_bound > p) ? lower_bound * steps_per_click : p * steps_per_click;
  }

  if (position != last_position) {
    position = last_position;
    if (fireCallback && change_cb != NULL) change_cb (*this);
  }
  direction = 0;
}

/////////////////////////////////////////////////////////////////

void Rotary::setIncrement(int inc) {
  increment = inc;
}

/////////////////////////////////////////////////////////////////

int Rotary::getIncrement() {
  return increment;
}

/////////////////////////////////////////////////////////////////

void Rotary::setStepsPerClick(int steps) {
  steps_per_click = (steps < 1) ? 1 : steps;
}

/////////////////////////////////////////////////////////////////

int Rotary::getStepsPerClick() {
  return steps_per_click;
}

/////////////////////////////////////////////////////////////////

byte Rotary::getDirection() {
  return direction;
}

/////////////////////////////////////////////////////////////////

String Rotary::directionToString(byte direction) {
  if (direction == RE_LEFT) {
    return "LEFT";
  } else {
    return "RIGHT";
  }
}

/////////////////////////////////////////////////////////////////

int Rotary::getPosition() {
  return position / steps_per_click;
}

/////////////////////////////////////////////////////////////////

void Rotary::loop() {
  int s = state & 3;
  if (digitalRead(pin1)) s |= 4;
  if (digitalRead(pin2)) s |= 8;

  switch (s) {
    case 0: case 5: case 10: case 15:
      break;
    case 1: case 7: case 8: case 14:
        position += increment; break;
    case 2: case 4: case 11: case 13:
      position -= increment; break;
    case 3: case 12:
      position += 2 * increment; break;
    default:
      position -= 2 * increment; break;
  }
  state = (s >> 2);

  if (position != last_position && (abs(position - last_position) >= steps_per_click * increment)) {
    int current_position = getPosition();
    if (current_position >= lower_bound && current_position <= upper_bound) {
      if (position > last_position) {
        direction = RE_RIGHT;
        if (right_cb != NULL) right_cb (*this);
      } else {
        direction = RE_LEFT;
        if (left_cb != NULL) left_cb (*this);
      }
      last_position = position;
      if (change_cb != NULL) change_cb (*this);
    } else {
      position = last_position;
      if (current_position < lower_bound && lower_cb != NULL) lower_cb (*this);
      if (current_position > upper_bound && upper_cb != NULL) upper_cb (*this);
    }
  }
}

/////////////////////////////////////////////////////////////////
