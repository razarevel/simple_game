#include "entities.h"
#include "json.hpp"
#include "maiApp.h"
#include <cstdio>
#include <fstream>
#include <glm/ext.hpp>
#include <iostream>
#include <string>
#include <thread>

using json = nlohmann::json;

std::string entityCacheStr =
    "entiyId = %d, addId = %d, type = %s, textureId = %d, tiling = "
    "%f, pos = [%f, %f, %f], "
    "scale = [%f, %f, %f], rotate = [%f, %f, %f]";

std::string entityCacheFile = RESOURCES_PATH "entity.json";

Entities::Entities(MAI::Renderer *ren, GLFWwindow *window, VkFormat formt)
    : ren_(ren), window(window), format(formt) {
  std::thread t1([&]() { assets = new Assets(ren, formt); });
  std::thread t2([&] { textures = new Textures(ren); });
  std::thread t3([&] { shapes = new Shapes(ren, formt); });
  t2.join();
  t1.join();
  t3.join();

  preparePipelines();
  loadEntity();
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

  vert = ren_->createShader(SHADERS_PATH "spvs/shap.vspv");
  frag = ren_->createShader(SHADERS_PATH "spvs/shap.fspv");
  ShapePipeline_ = ren_->createPipeline({
      .vert = vert,
      .frag = frag,
      .depthFormat = format,
      .cullMode = MAI::CullMode::Back,
  });
  delete vert;
  delete frag;
}

void Entities::draw(EntityDrawInfo info) {

  drawInfo_ = info;
  MAI::CommandBuffer *buff = info.buff;
  for (auto &entity : entities) {
    EntityData &data = entity.entityData;
    if (data.disable)
      continue;

    // transformation
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, data.pos);
    model = glm::scale(model, data.scale);
    // model = glm::toMat4(glm::quat(data.rotate));

    // draw
    if (entity.type == ASSET) {
      buff->bindPipeline(pipeline_);
      Model *md = assets->getModel(entity.addId);
      md->draw(buff, info.proj, info.view, model);

    } else if (entity.type == SHAPE) {
      buff->bindPipeline(ShapePipeline_);
      ShapeModule *sm = shapes->getShapeModule(entity.addId);
      TextureModel *tm = textures->getTextureModel(entity.entityData.textureId);
      struct PushConstant {
        glm::mat4 proj;
        glm::mat4 view;
        glm::mat4 model;
        float tiling;
        uint32_t tex;
        uint64_t vertx;
      } pc{
          .proj = info.proj,
          .view = info.view,
          .model = model,
          .tiling = data.tiling,
          .tex = tm != nullptr ? tm->diffuse->getIndex() : 0,
          .vertx = ren_->gpuAddress(sm->vertBuff),
      };

      buff->cmdPushConstant(&pc);
      buff->bindIndexBuffer(sm->indexBuff, 0, MAI::IndexType::Uint16);
      buff->cmdDrawIndex(sm->indicesSize);
    }
  }
  undoCheck();
}

void Entities::checkMouseClick() {
  MouseState state = drawInfo_.mouse_state;
  if (!state.pressedLeft)
    return;

  int width, height;
  glfwGetFramebufferSize(window, &width, &height);

  float x = (2.0f * state.pos.x) / width;
  float y = 1.0f - (2.0f * state.pos.y) / height;
  float z = 1.0f;

  // NDC -> view space
  glm::vec4 rayClip = glm::vec4(x, y, -1.0f, 1.0f);
  glm::vec4 rayEye = glm::inverse(drawInfo_.proj) * rayClip;
  rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);

  // View space -> world space
  glm::vec3 rayWorld =
      glm::normalize(glm::vec3(glm::inverse(drawInfo_.view) * rayEye));
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
  }
}

void Entities::saveEntity() {
  if (entities.empty())
    return;

  std::ofstream outfile(entityCacheFile.c_str());
  if (outfile.is_open()) {
    for (auto &entity : entities)
      if (!entity.entityData.disable) {

        std::string type;
        if (entity.type == ASSET)
          type = "ASSET";
        else if (entity.type == SHAPE)
          type = "SHAPE";

        float pos[3] = {entity.entityData.pos.x, entity.entityData.pos.y,
                        entity.entityData.pos.z};

        float scale[3] = {entity.entityData.scale.x, entity.entityData.scale.y,
                          entity.entityData.scale.z};
        float roate[3] = {entity.entityData.rotate.x,
                          entity.entityData.rotate.y,
                          entity.entityData.rotate.z};

        json j;
        j["entiyId"] = entity.id;
        j["addId"] = entity.addId;
        j["type"] = type;
        j["textureId"] = entity.entityData.textureId;
        j["tiling"] = entity.entityData.tiling;
        j["pos"] = pos;
        j["rotate"] = roate;
        j["scale"] = scale;

        outfile << j << std::endl;
      }
  }
  outfile.close();
  std::cout << "saved" << std::endl;
}

void Entities::resetEntity() {
  entities.clear();
  actions.clear();
}

void Entities::loadEntity() {
  std::ifstream file(entityCacheFile);
  if (!file.is_open())
    return;

  std::string line;
  while (std::getline(file, line)) {
    Entity entity;
    json j = json::parse(line);
    entity.id = j["entiyId"].get<uint32_t>();
    entity.addId = j["addId"].get<uint32_t>();
    std::string type = j["type"].get<std::string>();
    entity.entityData.textureId = j["textureId"].get<uint32_t>();
    entity.entityData.tiling = j["tiling"].get<float>();
    auto pos = j["pos"];
    auto rotate = j["rotate"];
    auto scale = j["scale"];

    if (type == "ASSET")
      entity.type = ASSET;
    else if (type == "SHAPE")
      entity.type = SHAPE;

    entity.entityData.pos = glm::vec3(pos[0], pos[1], pos[2]);
    entity.entityData.scale = glm::vec3(scale[0], scale[1], scale[2]);
    entity.entityData.rotate = glm::vec3(rotate[0], rotate[1], rotate[2]);
    entities.emplace_back(entity);
  }
  file.close();
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
          .addId = it.id,
          .type = ASSET,
      });
    }
  // shapes

  ImGui::NewLine();
  auto shapesInfo = shapes->getShapesInfo();
  if (ImGui::TreeNodeEx("Shapes", ImGuiTreeNodeFlags_DefaultOpen)) {
    for (auto &it : shapesInfo)
      if (ImGui::Button(it.name.c_str(), ImVec2(50, 50))) {
        currentEntity++;
        actionAdd(currentEntity);
        entities.emplace_back(Entity{
            .id = currentEntity,
            .addId = it.id,
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
      std::string name = "Entity " + std::to_string(it.id);
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

  EntityData data = entity->entityData;

  if (data.disable)
    return;

  if (const ImGuiViewport *v = ImGui::GetMainViewport()) {
    ImGui::SetNextWindowPos(
        {v->WorkPos.x + v->WorkSize.x - 15.0f, v->WorkPos.y + 100.0f},
        ImGuiCond_Always, {1.0f, 0.0f});

    ImGui::SetNextWindowSize({v->WorkSize.x * 0.3f, 0}, ImGuiCond_Always);
  }

  std::string name = "Entities " + std::to_string(entity->id);
  ImGui::Begin(name.c_str());

  if (ImGui::Button("Del")) {
    actionAdd(entity->id, ENTITY, entity->entityData);
    data.disable = true;
    ImGui::End();
    entity->entityData = data;
    return;
  }

  auto inputFloat3WithCommit = [](const char *label, glm::vec3 &value) {
    ImGui::InputFloat3(label, glm::value_ptr(value));
    return ImGui::IsItemDeactivatedAfterEdit();
  };

  if (inputFloat3WithCommit("Position", data.pos)) {
    actionAdd(entity->id, ENTITY, entity->entityData);
    entity->entityData = data;
  }

  if (inputFloat3WithCommit("Rotate", data.rotate)) {
    actionAdd(entity->id, ENTITY, entity->entityData);
    entity->entityData = data;
  }

  if (inputFloat3WithCommit("Scale", data.scale)) {
    actionAdd(entity->id, ENTITY, entity->entityData);
    entity->entityData = data;
  }

  ImGui::NewLine();
  ImGui::InputFloat("Tiling", &data.tiling);
  if (ImGui::IsItemDeactivatedAfterEdit()) {
    actionAdd(entity->id, ENTITY, entity->entityData);
    entity->entityData = data;
  }

  if (entity->type == SHAPE) {
    ImGui::Text("Textures");
    ImVec2 size = ImVec2(100, 100);
    auto texturesInfos = textures->getTextures();
    for (auto &it : texturesInfos) {
      if (ImGui::ImageButton(it.name.c_str(), it.diffuse->getIndex(), size)) {
        actionAdd(entity->id, ENTITY, entity->entityData);
        data.textureId = it.id;
        entity->entityData = data;
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
  delete ShapePipeline_;
  delete shapes;
}
