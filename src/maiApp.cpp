#include "maiApp.h"
#include "imgui.h"

const glm::vec3 KInitialCameraPos = glm::vec3(0.0f, 1.0f, -1.5f);
const glm::vec3 kInitialCameraTarget = glm::vec3(0.0f, 0.5f, 0.0f);

CameraPositioner_FirstPerson positioner(KInitialCameraPos, kInitialCameraTarget,
                                        glm::vec3(0.0f, 1.0f, 0.0f));

struct MouseState {
  glm::vec2 pos = glm::vec2(0.0f);
  bool presedLeft = false;
} mouseState;

MaiApp::MaiApp() {
  camera = new Camera(positioner);
  windowInfo = {
      .width = 1200,
      .height = 800,
      .appName = "Demo Game",
  };
  window = MAI::initWindow(windowInfo);
  ren = MAI::initVulkanWithSwapChain(window, windowInfo.appName);

  depthTexture = ren->createImage({
      .type = MAI::TextureType_2D,
      .format = MAI::Format_Z_F32,
      .dimensions = {(uint32_t)windowInfo.width, windowInfo.height},
      .usage = MAI::Attachment_Bit,
  });

  setMouseConfig();
  setKeyboardConfig();

  imgui = new ImGuiRenderer(ren, window, depthTexture->getDeptFormat());
}

void MaiApp::setMouseConfig() {
  glfwSetCursorPosCallback(window, [](auto *window, double x, double y) {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    mouseState.pos.x = static_cast<float>(x / width);
    mouseState.pos.y = 1.0f - static_cast<float>(y / height);
  });

  glfwSetMouseButtonCallback(
      window, [](auto *window, int button, int action, int mods) {
        if (button == GLFW_MOUSE_BUTTON_LEFT) {
          mouseState.presedLeft = action == GLFW_PRESS;
        }
      });
}

void MaiApp::setKeyboardConfig() {
  glfwSetKeyCallback(window, [](GLFWwindow *window, int key, int scancode,
                                int action, int mods) {
    const bool pressed = action != GLFW_RELEASE;
    if (key == GLFW_KEY_ESCAPE && pressed)
      glfwSetWindowShouldClose(window, GLFW_TRUE);
    if (key == GLFW_KEY_W)
      positioner.movement_.forward_ = pressed;
    if (key == GLFW_KEY_S)
      positioner.movement_.backward_ = pressed;
    if (key == GLFW_KEY_A)
      positioner.movement_.left_ = pressed;
    if (key == GLFW_KEY_D)
      positioner.movement_.right_ = pressed;
    if (key == GLFW_KEY_1)
      positioner.movement_.up_ = pressed;
    if (key == GLFW_KEY_2)
      positioner.movement_.down_ = pressed;
    if (mods & GLFW_MOD_SHIFT)
      positioner.movement_.fastSpeed_ = pressed;
    if (key == GLFW_KEY_SPACE) {
      positioner.lookAt(KInitialCameraPos, kInitialCameraTarget,
                        glm::vec3(0.0f, 1.0f, 0.0f));
      positioner.setSpeed(glm::vec3(0));
    }
  });
}

void MaiApp::run(DrawFrameFunc drawFrame, DrawFrameFunc beforeDraw,
                 DrawFrameFunc afterDraw) {
  float deltaSecond = 0.0f;
  double timeStamp = glfwGetTime();
  FPS fps;

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    if (!width || !height)
      continue;
    float ratio = width / (float)height;
    const double newTimeStamp = glfwGetTime();
    deltaSecond = static_cast<float>(newTimeStamp - timeStamp);
    timeStamp = newTimeStamp;

    positioner.update(deltaSecond, mouseState.pos,
                      ImGui::GetIO().WantCaptureMouse ? false
                                                      : mouseState.presedLeft);
    fps.tick(deltaSecond);
    currentFPS = fps.currentFPS_;

    MAI::CommandBuffer *buff = ren->acquireCommandBuffer();
    beforeDraw(buff, width, height, ratio, deltaSecond);
    // draw
    buff->cmdBeginRendering({.texture = depthTexture});
    imgui->beginFrame({(uint32_t)width, (uint32_t)height});
    drawFrame(buff, width, height, ratio, deltaSecond);
    // fps
    {
      if (const ImGuiViewport *v = ImGui::GetMainViewport()) {
        ImGui::SetNextWindowPos(
            {v->WorkPos.x + v->WorkSize.x - 15.0f, v->WorkPos.y + 15.0f},
            ImGuiCond_Always, {1.0f, 0.0f});
      }
      ImGui::SetNextWindowBgAlpha(0.30f);
      ImGui::SetNextWindowSize(
          ImVec2(ImGui::CalcTextSize("FPS : _______").x, 0));
      if (ImGui::Begin("##FPS", nullptr,
                       ImGuiWindowFlags_NoDecoration |
                           ImGuiWindowFlags_AlwaysAutoResize |
                           ImGuiWindowFlags_NoSavedSettings |
                           ImGuiWindowFlags_NoFocusOnAppearing |
                           ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove)) {
        ImGui::Text("FPS : %i", (int)currentFPS);
        ImGui::Text("Ms  : %.1f", 1000.0 / currentFPS);
        ImGui::End();
      }
    }
    // ending
    imgui->endFrame(buff);
    buff->cmdEndRendering();
    ren->submit();
    afterDraw(buff, width, height, ratio, deltaSecond);
    delete buff;
  }
  ren->waitDeviceIdle();
}

MaiApp::~MaiApp() {
  delete imgui;
  delete camera;
  delete depthTexture;
  glfwDestroyWindow(window);
  glfwTerminate();
  delete ren;
}
