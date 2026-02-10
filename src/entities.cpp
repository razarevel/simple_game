#include "entities.h"
#include "imgui.h"
#include <glm/ext.hpp>
#include <iostream>
#include <thread>

Entities::Entities(MAI::Renderer *ren, VkFormat formt)
    : ren_(ren), format(formt) {
  std::thread t1([&]() { assets = new Assets(ren, formt); });
  std::thread t2([&] { textures = new Textures(ren); });
  std::thread t3([&] { shapes = new Shapes(ren, formt); });
  t2.join();
  t1.join();
  t3.join();
  preparePipelines();
}

void Entities::preparePipelines() {
  MAI::Shader *vert = ren_->createShader(SHADERS_PATH "spvs/model.vspv");
  MAI::Shader *frag = ren_->createShader(SHADERS_PATH "spvs/model.vspv");
  pipeline_ = ren_->createPipeline({
      .vert = vert,
      .frag = frag,
      .depthFormat = format,
      .cullMode = MAI::CullMode::Back,
  });
  delete vert;
  delete frag;
}

void Entities::draw(MAI::CommandBuffer *buff, glm::mat4 proj, glm::mat4 view,
                    glm::vec3 cameraPos) {
  for (auto &entity : entities) {
    // transformation
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, entity.pos);

    // draw
    buff->bindPipeline(pipeline_);
    if (entity.type == ASSET) {
      Model *md = assets->getModel(entity.name);
      md->draw(buff, proj, view, model);

    } else if (entity.type == SHAPE) {
      ShapeModule *sm = shapes->getShapeModule(entity.name);
      struct PushConstant {
        glm::mat4 proj;
        glm::mat4 view;
        glm::mat4 model;
        uint32_t tex;
        uint64_t vertx;
      } pc{
          .proj = proj,
          .view = view,
          .model = model,
          .tex = 0,
          .vertx = ren_->gpuAddress(sm->vertBuff),
      };
      buff->cmdPushConstant(&pc);
      buff->bindIndexBuffer(sm->indexBuff, 0, MAI::IndexType::Uint32);
      buff->cmdDrawIndex(sm->indicesSize);
    }
  }
}

void Entities::guiWidget() {
  // assets
  auto models = assets->getModelInfos();
  ImGui::NewLine();
  ImGui::Text("Assets");
  for (auto &it : models)
    if (ImGui::Button(it.name.c_str(), ImVec2(0, 50))) {
      currentEntity++;
      entities.emplace_back(Entity{
          .id = currentEntity,
          .name = it.name,
          .type = ASSET,
      });
    }
  // shapes

  ImGui::NewLine();
  auto shapesInfo = shapes->getShapesInfo();
  if (ImGui::TreeNodeEx("Shapes", ImGuiTreeNodeFlags_DefaultOpen)) {
    for (auto &it : shapesInfo)
      if (ImGui::Button(it.c_str(), ImVec2(50, 50))) {
        currentEntity++;
        entities.emplace_back(Entity{
            .id = currentEntity,
            .name = it,
            .type = SHAPE,
        });
      }
    ImGui::TreePop();
  }

  ImGui::NewLine();
  ImGui::TextWrapped("Entities");
  for (auto &it : entities) {
    std::string name = it.name + " " + std::to_string(it.id);
    if (ImGui::Button(name.c_str()))
      currentEntity = (int)it.id;
  }
}

void Entities::entityWidget() {
  if (entities.empty())
    return;

  Entity *entity;
  for (auto &it : entities)
    if (it.id == currentEntity)
      entity = &it;

  if (const ImGuiViewport *v = ImGui::GetMainViewport()) {
    ImGui::SetNextWindowPos(
        {v->WorkPos.x + v->WorkSize.x - 15.0f, v->WorkPos.y + 100.0f},
        ImGuiCond_Always, {1.0f, 0.0f});

    ImGui::SetNextWindowSize({v->WorkSize.x * 0.3f, 0}, ImGuiCond_Always);
  }
  ImGui::Begin(entity->name.c_str());
  ImGui::InputFloat3("Poisition", glm::value_ptr(entity->pos));
  ImGui::InputFloat3("Rotate", glm::value_ptr(entity->rotate));
  ImGui::InputFloat3("Scale", glm::value_ptr(entity->scale));

  if (entity->type == SHAPE) {
    ImGui::Text("Textures");
    ImVec2 size = ImVec2(100, 100);
    auto texturesInfos = textures->getTextures();
    for (auto &it : texturesInfos) {
      if (ImGui::ImageButton(it.name.c_str(), it.diffuse->getIndex(), size)) {
        std::cout << it.name << std::endl;
      }
      ImGui::SameLine();
    }
  }

  ImGui::End();
}

Entities::~Entities() {
  delete assets;
  delete textures;
  delete pipeline_;
  delete shapes;
}
