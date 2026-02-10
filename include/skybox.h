#pragma once

#include "mai_config.h"
#include "mai_vk.h"

#include <glm/ext.hpp>
#include <glm/glm.hpp>

struct Cubemap {
  std::string name;
  uint32_t id;
  MAI::Texture *tex;
};

struct DrawInfo {
  MAI::CommandBuffer *buff;
  float ratio;
  glm::mat4 proj;
  glm::mat4 view;
  glm::mat4 model = glm::mat4(1.0f);
  glm::vec3 cameraPos;
};

struct Skybox {
  Skybox(MAI::Renderer *ren, VkFormat format = VK_FORMAT_UNDEFINED);
  ~Skybox();

  void draw(const DrawInfo &info);
  const std::vector<Cubemap> &getSkyboxInfo() { return cubemaps; }

  void guiWidgets();

private:
  MAI::Renderer *ren_;
  MAI::Pipeline *pipeline_ = nullptr;
  std::vector<Cubemap> cubemaps;
  std::vector<int> currSkybox{0};
};
