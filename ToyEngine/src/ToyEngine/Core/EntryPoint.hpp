#pragma once

#include "Application.hpp"

extern TE::Application *TE::createApplication();

int main() {  // NOLINT
  std::cout << "Hello TE!" << std::endl;

  auto app = TE::createApplication();
  app->run();
  delete app;
}
