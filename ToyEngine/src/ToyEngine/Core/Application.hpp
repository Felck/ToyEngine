#pragma once

namespace TE {

class Application {
 public:
  Application();
  virtual ~Application();

  void Run();
};

// To be defined in client
Application *CreateApplication();
}  // namespace TE