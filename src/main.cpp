#include "entities.h"
#include "maiApp.h"
#include "skybox.h"
#include <fonts.h>

#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <iostream>

#include "stbi_image.h"
#include "textures.h"

int main() {
  MaiApp *mai = new MaiApp();
  VkFormat format = mai->depthTexture->getDeptFormat();

  Skybox *skybox = new Skybox(mai->ren, format);

  Entities *entities = new Entities(mai->ren, mai->window, format);

  int currentAssets = 0;

  glm::vec3 cameraPos(0.0f, 1.0f, -1.5f);
  glm::vec3 cameraTarget(0.0f, 0.0f, 0.0f);

  auto draw = [&](MAI::CommandBuffer *buff, uint32_t width, uint32_t height,
                  float ratio, float deltaSecond) {
    glm::mat4 p = glm::perspective(glm::radians(60.0f), ratio, 0.1f, 1000.0f);
    p[1][1] *= -1;

    const glm::mat4 view = mai->camera->GetViewMatrix();

    skybox->draw({
        .buff = buff,
        .ratio = ratio,
        .proj = p,
        .view = view,
        .cameraPos = mai->camera->Position,
    });

    {
      buff->cmdBindDepthState({
          .depthWriteEnable = true,
          .compareOp = MAI::CompareOp::Less,
      });
      entities->draw({
          .buff = buff,
          .proj = p,
          .view = view,
          .cameraPos = cameraPos,
          .mouse_state = mai->mouse_state,
      });
    }

    // imgui
    if (const ImGuiViewport *v = ImGui::GetMainViewport()) {
      ImGui::SetNextWindowPos({v->WorkPos.x, v->WorkPos.y}, ImGuiCond_Always,
                              {0.0f, 0.0f});
      ImGui::SetNextWindowSize({v->WorkSize.x * 0.3f, v->WorkSize.y},
                               ImGuiCond_Always);
      ImGui::SetNextWindowBgAlpha(0.50f);
    }
    ImGui::Begin("Viewport");
    if (ImGui::Button("Save")) {
      entities->saveEntity();
    }
    ImGui::SameLine();
    if (ImGui::Button("Reset")) {
      entities->resetEntity();
    }

    ImGui::NewLine();

    mai->camera->camerGui();
    skybox->guiWidgets();
    entities->guiWidget();
    ImGui::End();

    entities->entityWidget();
  };

  auto beforeDraw = [&](MAI::CommandBuffer *buff, uint32_t width,
                        uint32_t height, float ratio, float deltaSecond) {};

  auto afterDraw = [&](MAI::CommandBuffer *buff, uint32_t width,
                       uint32_t height, float ratio, float deltaSecond) {};

  mai->run(draw, beforeDraw, afterDraw);

  delete entities;
  delete skybox;
  delete mai;

  return 0;
}
