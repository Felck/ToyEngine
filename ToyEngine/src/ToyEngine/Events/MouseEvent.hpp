#pragma once

#include "Event.hpp"

namespace TE {

class MouseMovedEvent : public Event {
 public:
  IMPL_EVENT_CATEGORY(InputCategory | MouseCategory)
  IMPL_EVENT_TYPE(MouseMoved)

  MouseMovedEvent(float x, float y) : x(x), y(y) {}

  inline float getX() const { return x; }
  inline float getY() const { return y; }

 private:
  float x, y;
};

class MouseScrolledEvent : public Event {
 public:
  IMPL_EVENT_CATEGORY(InputCategory | MouseCategory)
  IMPL_EVENT_TYPE(MouseScrolled)

  MouseScrolledEvent(float xOffset, float yOffset)
      : xOffset(xOffset), yOffset(yOffset) {}

  inline float getXOffset() const { return xOffset; }
  inline float getYOffset() const { return yOffset; }

 private:
  float xOffset, yOffset;
};

class MouseButtonEvent : public Event {
 public:
  IMPL_EVENT_CATEGORY(InputCategory | MouseCategory)

  inline int getButton() const { return button; }

 protected:
  MouseButtonEvent(int button) : button(button) {}

  int button;
};

class MouseButtonPressedEvent : public MouseButtonEvent {
 public:
  IMPL_EVENT_TYPE(MouseButtonPressed)

  MouseButtonPressedEvent(int button) : MouseButtonEvent(button) {}
};

class MouseButtonReleasedEvent : public MouseButtonEvent {
 public:
  IMPL_EVENT_TYPE(MouseButtonReleased)

  MouseButtonReleasedEvent(int button) : MouseButtonEvent(button) {}
};

}  // namespace TE