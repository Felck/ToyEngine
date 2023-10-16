#include "ToyEngine/ImGui/ImGuiLayer.hpp"

#include <GLFW/glfw3.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>
#include <imgui.h>
#include <imgui_internal.h>

#include "ToyEngine/Core/Application.hpp"
#include "ToyEngine/Events/Event.hpp"
#include "tepch.hpp"

namespace TE {

ImGuiLayer::ImGuiLayer() : Layer("ImGuiLayer") {}

void ImGuiLayer::onAttach() {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

  ImGui::StyleColorsDark();

  Application& app = Application::get();
  GLFWwindow* window =
      static_cast<GLFWwindow*>(app.getWindow().getNativeWindow());

  ImGui_ImplGlfw_InitForVulkan(window, true);
  // ImGui_ImplVulkan_Init();
}

void ImGuiLayer::onDetach() {
  ImGui_ImplVulkan_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}

void ImGuiLayer::onUpdate() {
  begin();
  static bool show = true;
  ImGui::ShowDemoWindow(&show);
  end();
}

void ImGuiLayer::onEvent(Event& e) {
  if (m_BlockEvents) {
    ImGuiIO& io = ImGui::GetIO();
    e.handled |= e.isInCategory(MouseCategory) & io.WantCaptureMouse;
    e.handled |= e.isInCategory(KeyboardCategory) & io.WantCaptureKeyboard;
  }
}

void ImGuiLayer::begin() {
  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
}

void ImGuiLayer::end() {
  ImGui::Render();
  // ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData());
}

}  // namespace TE