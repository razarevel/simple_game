#include "entities.h"
#include "imgui.h"
#include "maiApp.h"
#include <glm/ext.hpp>
#include <iostream>
#include <thread>

Entities::Entities(MAI::Renderer *ren, GLFWwindow *window, VkFormat formt)
    : ren_(ren), window(window), format(formt) {
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
  MAI::Shader *frag = ren_->createShader(SHADERS_PATH "spvs/model.fspv");
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
    EntityData &data = entity.entityData;
    if (data.disable)
      continue;

    // transformation
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, data.pos);
    model = glm::scale(model, data.scale);

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
          .tex = data.tm != nullptr ? data.tm->diffuse->getIndex() : 0,
          .vertx = ren_->gpuAddress(sm->vertBuff),
      };

      buff->cmdPushConstant(&pc);
      buff->bindIndexBuffer(sm->indexBuff, 0, MAI::IndexType::Uint16);
      buff->cmdDrawIndex(sm->indicesSize);
    }
  }
  undoCheck();
}

void Entities::undoCheck() {
  std::array<bool, 2> mods = MaiApp::getMods();

  if (actions.empty())
    return;

  if (mods[0]) {

    if (currAction < 0)
      return;

    Action action = actions[currAction];
    for (auto &it : entities) {
      if (it.id != action.id)
        continue;
      if (action.type == ENTITY) {
        it.entityData = action.data;
        currAction--;
        break;
      } else if (action.type == ADD) {
        it.entityData.disable = true;
        currAction--;
        break;
      }
    }

  }

  else if (mods[1]) {
    if (currAction != actions.size() - 1)
      currAction++;

    else if (currAction == actions.size())
      return;

    Action action = actions[currAction];

    for (auto &it : entities) {
      if (it.id != action.id)
        continue;
      if (action.type == ENTITY) {
        it.entityData = action.data;
        break;
      }
      if (action.type == ADD) {
        it.entityData.disable = false;
        break;
      }
    }

    // if (action.type == ENTITY) {
    // } else if (action.type == ADD) {
    //   for (auto &it : entities)
    //     if (it.id == action.id) {
    //       it.entityData.disable = false;
    //       break;
    //     }
    // }
  }
}

void Entities::actionAdd(uint32_t id, ActionType type, EntityData data) {
  if (actions.empty() || currAction == actions.size() - 1) {
    actions.emplace_back(Action{
        .type = type,
        .id = id,
        .data = data,
    });
    currAction++;
  } else {
    actions.resize(currAction + 1);
    actions.emplace_back(Action{
        .type = type,
        .id = id,
        .data = data,
    });
    currAction = actions.size() - 1;
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
      actionAdd(currentEntity);
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
        actionAdd(currentEntity);
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
    EntityData data = it.entityData;
    if (!data.disable) {
      std::string name = it.name + " " + std::to_string(it.id);
      if (ImGui::Button(name.c_str()))
        currentEntity = (int)it.id;
    }
  }
}

void Entities::entityWidget() {
  if (entities.empty())
    return;

  Entity *entity;
  for (auto &it : entities)
    if (it.id == currentEntity)
      entity = &it;

  EntityData &data = entity->entityData;

  if (data.disable)
    return;

  if (const ImGuiViewport *v = ImGui::GetMainViewport()) {
    ImGui::SetNextWindowPos(
        {v->WorkPos.x + v->WorkSize.x - 15.0f, v->WorkPos.y + 100.0f},
        ImGuiCond_Always, {1.0f, 0.0f});

    ImGui::SetNextWindowSize({v->WorkSize.x * 0.3f, 0}, ImGuiCond_Always);
  }
  ImGui::Begin(entity->name.c_str());

  if (ImGui::Button("Del")) {
    actionAdd(entity->id, ENTITY, entity->entityData);
    data.disable = true;
    ImGui::End();
    return;
  }

  ImGui::InputFloat3("Poisition", glm::value_ptr(data.pos));
  ImGui::InputFloat3("Rotate", glm::value_ptr(data.rotate));
  ImGui::InputFloat3("Scale", glm::value_ptr(data.scale));

  if (entity->type == SHAPE) {
    ImGui::Text("Textures");
    ImVec2 size = ImVec2(100, 100);
    auto texturesInfos = textures->getTextures();
    for (auto &it : texturesInfos) {
      if (ImGui::ImageButton(it.name.c_str(), it.diffuse->getIndex(), size)) {
        data.tm = &it;
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
