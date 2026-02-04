#pragma once

#include "mai_config.h"
#include "mai_vk.h"

#include <glm/ext.hpp>
#include <glm/glm.hpp>

struct Skybox {
  Skybox(MAI::Renderer *ren, MAI::Texture *depthTexture);
  ~Skybox();

  void draw(MAI::CommandBuffer *buff, float ratio, glm::mat4 proj,
            glm::mat4 view, glm::vec3 cameraPos = glm::vec3(0.0f, 1.0f, -1.5f));

private:
  MAI::Renderer *ren_;

  MAI::Pipeline *pipeline_ = nullptr;
  MAI::Texture *cubemapTex = nullptr;
};
