#pragma once

#include "Application.hpp"
#include "tepch.hpp"

extern std::unique_ptr<TE::Application> TE::createApplication();

int main() {  // NOLINT
  auto app = TE::createApplication();
  app->run();
}
