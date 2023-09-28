#pragma once

#include "Application.hpp"
#include "tepch.hpp"

extern TE::Application *TE::CreateApplication();

int main(void) {  // NOLINT
  std::cout << "Hello TE!" << std::endl;

  auto app = TE::CreateApplication();
  app->Run();
  delete app;
}
