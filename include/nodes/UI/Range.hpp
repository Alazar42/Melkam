#pragma once

#include "nodes/UI/Control.hpp"
#include <algorithm>
#include <cmath>

// Base Abstract Range UI Node (inspired by Godot Range) for numeric sliders, progress bars, and scrollbars.
class Range : public Control {
public:
  // Signals
  Signal<float> value_changed;
  Signal<float> changed;

  float minValue = 0.0f;
  float maxValue = 100.0f;
  float value = 0.0f;
  float step = 1.0f;
  float page = 0.0f;
  bool rounded = false;
  bool allowGreater = false;
  bool allowLesser = false;

  Range() : Control("Range") {}
  explicit Range(std::string nodeName) : Control(std::move(nodeName)) {}

  virtual void setValue(float val) {
    float clamped = val;
    if (!allowLesser && clamped < minValue) clamped = minValue;
    if (!allowGreater && clamped > maxValue) clamped = maxValue;

    if (step > 0.0f) {
      clamped = minValue + std::round((clamped - minValue) / step) * step;
    }
    if (rounded) {
      clamped = std::round(clamped);
    }

    if (clamped != value) {
      value = clamped;
      value_changed.emit(value);
      changed.emit(value);
    }
  }

  float getValue() const { return value; }

  // Sets value by normalized ratio [0.0 .. 1.0]
  void setRatio(float ratio) {
    float clampedRatio = std::clamp(ratio, 0.0f, 1.0f);
    setValue(minValue + clampedRatio * (maxValue - minValue));
  }

  // Returns normalized value ratio [0.0 .. 1.0]
  float getRatio() const {
    if (maxValue <= minValue) return 0.0f;
    return std::clamp((value - minValue) / (maxValue - minValue), 0.0f, 1.0f);
  }

  void setMinValue(float min) {
    minValue = min;
    setValue(value);
  }

  void setMaxValue(float max) {
    maxValue = max;
    setValue(value);
  }

  void setStep(float s) {
    step = std::max(0.0f, s);
    setValue(value);
  }
};
