#pragma once

#include "Event.hpp"

namespace TE {

class KeyEvent : public Event {
 public:
  IMPL_EVENT_CATEGORY(Input | Keyboard)

  inline int getKeyCode() const { return keyCode; }

 protected:
  explicit KeyEvent(int keyCode) : keyCode(keyCode) {}
  int keyCode;
};

class KeyPressedEvent : public KeyEvent {
 public:
  IMPL_EVENT_TYPE(KeyPressed)

  KeyPressedEvent(int keyCode, int repeatCount)
      : KeyEvent(keyCode), repeatCount(repeatCount) {}

  inline int getRepeatCount() const { return repeatCount; }

 private:
  int repeatCount;
};

class KeyReleasedEvent : public KeyEvent {
 public:
  IMPL_EVENT_TYPE(KeyReleased)

  KeyReleasedEvent(int keyCode) : KeyEvent(keyCode) {}
};
}  // namespace TE