#include <ToyEngine.hpp>
#include <iostream>

class Sandbox : public TE::Application {
 public:
  Sandbox() { std::cout << "hello" << std::endl; }

  ~Sandbox() {}
};

TE::Application *TE::CreateApplication() { return new Sandbox(); }