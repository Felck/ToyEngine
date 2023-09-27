#pragma once

#include "Application.hpp"

extern TE::Application *TE::CreateApplication();

int main(void) {  // NOLINT
  auto app = TE::CreateApplication();
  app->Run();
  delete app;
}
