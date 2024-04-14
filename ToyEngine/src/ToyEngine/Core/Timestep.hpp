#pragma once

namespace TE {

class Timestep {
 public:
  Timestep(float time = 0.0f) : time(time) {}
  Timestep(double time = 0.0) : time((float)time) {}

  operator float() const { return time; }

  inline float seconds() const { return time; }
  inline float milliseconds() const { return time * 1000.0f; }

 private:
  float time;
};

}  // namespace TE