#pragma once

namespace TE {

class Application {
 public:
  Application();
  virtual ~Application();

  void run();
};

// To be defined in client
Application *createApplication();
}  // namespace TE