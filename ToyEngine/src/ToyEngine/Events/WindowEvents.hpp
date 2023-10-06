#pragma once

#include "Event.hpp"

namespace TE {

class WindowResizeEvent : public Event {
 public:
  IMPL_EVENT_CATEGORY(WindowCategory)
  IMPL_EVENT_TYPE(WindowResize)

  WindowResizeEvent(uint32_t width, uint32_t height)
      : width(width), height(height) {}

  uint32_t getWidth() const { return width; }
  uint32_t getHeight() const { return height; }

 private:
  uint32_t width, height;
};

class WindowCloseEvent : public Event {
 public:
  IMPL_EVENT_CATEGORY(WindowCategory)
  IMPL_EVENT_TYPE(WindowClose)
};
}  // namespace TE