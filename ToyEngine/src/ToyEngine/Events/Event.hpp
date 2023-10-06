#pragma once

namespace TE {
// clang-format off
enum class EventType {
  None = 0,
  WindowClose, WindowResize, WindowFocus, WindowLostFocus,
  KeyPressed, KeyReleased,
  MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
};

enum EventCategory {
  None                = 0,
  WindowCategory      = 1 << 0,
  InputCategory       = 1 << 1,
  KeyboardCategory    = 1 << 2,
  MouseCategory       = 1 << 3,
  MouseButtonCategory = 1 << 4
};
// clang-format on

#define IMPL_EVENT_TYPE(type)                                  \
  static EventType getStaticType() { return EventType::type; } \
  virtual EventType getEventType() const override { return getStaticType(); }

#define IMPL_EVENT_CATEGORY(category) \
  virtual int getCategories() const override { return category; }

#define BIND_EVENT_FN(fn)                                   \
  [this](auto&&... args) -> decltype(auto) {                \
    return this->fn(std::forward<decltype(args)>(args)...); \
  }

class Event {
 public:
  bool handled = false;

  virtual EventType getEventType() const = 0;
  virtual int getCategories() const = 0;

  inline bool isInCategory(EventCategory category) {
    return getCategories() & category;
  }

  template <typename T, typename F>
  bool dispatch(const F& func) {
    if (this->getEventType() == T::GetStaticType()) {
      this->handled |= func(static_cast<T&>(this));
      return true;
    }
    return false;
  }
};

}  // namespace TE
