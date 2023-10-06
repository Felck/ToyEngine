#include <ToyEngine.hpp>
#include <ToyEngine/Core/EntryPoint.hpp>
#include <iostream>

class Sandbox : public TE::Application {
 public:
  Sandbox() {}

  ~Sandbox() {}
};

TE::Application *TE::createApplication() { return new Sandbox(); }