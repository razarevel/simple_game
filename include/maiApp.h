#pragma once
#include "Camera.h"
#include "imguiRenderer.h"
#include "mai_config.h"
#include "mai_vk.h"
#include "utils.h"
#include <array>
#include <functional>

using DrawFrameFunc =
    std::function<void(MAI::CommandBuffer *buff, uint32_t width,
                       uint32_t height, float ratio, float deltaSecond)>;
struct FPS {
  FPS(float avgInterval = 0.5f) : avgInterval_(avgInterval) {}
  void tick(float deltaSecond) {
    numFrames++;
    accumulateTime += deltaSecond;
    if (accumulateTime > avgInterval_) {
      currentFPS_ = static_cast<float>(numFrames / accumulateTime);
      numFrames = 0;
      accumulateTime = 0;
    }
  }

public:
  float avgInterval_ = 0.5f;
  uint32_t numFrames = 0;
  double accumulateTime = 0;
  float currentFPS_ = 0.0f;
};

struct MaiApp {
  MaiApp();
  ~MaiApp();

  MAI::WindowInfo windowInfo;
  MAI::Renderer *ren = nullptr;
  GLFWwindow *window = nullptr;
  MAI::Texture *depthTexture = nullptr;
  Camera *camera = nullptr;
  ImGuiRenderer *imgui;
  MouseState mouse_state;
  float currentFPS;

  void run(DrawFrameFunc drawFrame, DrawFrameFunc beforeDraw,
           DrawFrameFunc afterDraw);

  static std::array<bool, 2> getMods();

private:
  void setMouseConfig();
  void updateMouseMovement();
};
