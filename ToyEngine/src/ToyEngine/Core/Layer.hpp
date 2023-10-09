#pragma once

#include "ToyEngine/Events/Event.hpp"
#include "tepch.hpp"

namespace TE {

class Layer {
 public:
  Layer(const std::string& name);
  virtual ~Layer();

  virtual void onAttach() {}
  virtual void onDetach() {}
  virtual void onUpdate() {}
  virtual void onEvent([[maybe_unused]] Event& event) {}

  inline const std::string& getName() const { return name; }

 protected:
  std::string name;
};

}  // namespace TE