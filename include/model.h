#pragma once
#include "mai_config.h"
#include "mai_vk.h"
#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <glm/glm.hpp>

enum ModelType : uint8_t {
  GLTF = 0x01,
  FBX = 0x02,
  OBJ = 0x04,
};

struct Vertex {
  glm::vec3 pos;
  glm::vec2 uv;
  glm::vec3 norm;
};

struct Mesh {
  MAI::Buffer *vertexBuffer;
  MAI::Buffer *indexBuffer;
  uint32_t indicesSize;
};

struct Model {
  Model(MAI::Renderer *ren, const char *filename);
  ~Model();

  void draw(MAI::CommandBuffer *buff, glm::mat4 proj, glm::mat4 view,
            glm::mat4 model = glm::mat4(1.0f));

  std::string name;
  uint32_t id;

private:
  MAI::Renderer *ren_ = nullptr;
  std::vector<Mesh> meshes;
  std::vector<MAI::Texture *> textures;
  std::vector<std::string> loaded;
  ModelType type;
  const char *filename;

  void loadTextures(const aiMaterial *mat, aiTextureType type,
                    const char *dir = nullptr);
  void processNodes(const aiNode *node, const aiScene *scene);
  void processMeshes(const aiMesh *mesh, const aiScene *scene);
};
