#pragma once

#include "Application.hpp"
#include "tepch.hpp"

extern TE::Application *TE::createApplication();

int main() {  // NOLINT
  auto app = TE::createApplication();
  app->run();
  delete app;
}
