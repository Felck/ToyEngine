#pragma once

#include "Event.hpp"

namespace TE {

class WindowResizeEvent : public Event {
 public:
  IMPL_EVENT_CATEGORY(Window)
  IMPL_EVENT_TYPE(WindowResize)

  WindowResizeEvent(unsigned int width, unsigned int height)
      : width(width), height(height) {}

  unsigned int getWidth() const { return width; }
  unsigned int getHeight() const { return height; }

 private:
  unsigned int width, height;
};

class WindowCloseEvent : public Event {
 public:
  IMPL_EVENT_CATEGORY(Window)
  IMPL_EVENT_TYPE(WindowClose)
};
}  // namespace TE