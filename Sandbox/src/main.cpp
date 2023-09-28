#include <ToyEngine.hpp>
#include <ToyEngine/Core/EntryPoint.hpp>
#include <iostream>

class Sandbox : public TE::Application {
 public:
  Sandbox() { std::cout << "Hello Sandbox!" << std::endl; }

  ~Sandbox() {}
};

TE::Application *TE::CreateApplication() { return new Sandbox(); }