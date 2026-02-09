#include "model.h"
#include <filesystem>

namespace fs = std::filesystem;

#include <iostream>

#include "stbi_image.h"

#define flags                                                                  \
  aiProcess_Triangulate | aiProcess_JoinIdenticalVertices |                    \
      aiProcess_GenNormals | aiProcess_CalcTangentSpace |                      \
      aiProcess_ImproveCacheLocality | aiProcess_RemoveRedundantMaterials |    \
      aiProcess_ValidateDataStructure | aiProcess_SortByPType |                \
      aiProcess_FlipUVs

std::string setName(const char *filename) {
  std::string name = filename;
  name = name.substr(name.find_last_of('/') + 1);
  return name;
}

Model::Model(MAI::Renderer *ren, const char *filename)
    : ren_(ren), filename(filename) {
  name = setName(filename);
  std::string file;
  for (const auto &entry : fs::directory_iterator(filename)) {
    std::string str = entry.path();
    if (str.find(".gltf") != std::string::npos) {
      file = str;
      type = GLTF;
      break;
    } else if (str.find(".fbx") != std::string::npos) {
      file = str;
      type = FBX;
      break;
    } else if (str.find(".obj") != std::string::npos) {
      file = str;
      type = OBJ;
      break;
    }
  }

  if (file.empty()) {
    std::cerr << "no asset found" << std::endl;
    std::cerr << "path: " << filename << std::endl;
    assert(false);
  }

  const aiScene *scene = aiImportFile(file.c_str(), flags);
  if (!scene) {
    std::cout << "failed to load assert at path: " << filename << std::endl;
    assert(false);
  }

  processNodes(scene->mRootNode, scene);
  aiReleaseImport(scene);
}

void Model::processNodes(const aiNode *node, const aiScene *scene) {
  for (uint32_t i = 0; i < node->mNumMeshes; i++) {
    const aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
    processMeshes(mesh, scene);
  }
  for (uint32_t i = 0; i < node->mNumChildren; i++)
    processNodes(node->mChildren[i], scene);
}

void Model::processMeshes(const aiMesh *mesh, const aiScene *scene) {
  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;

  for (size_t i = 0; i != mesh->mNumVertices; i++) {
    const aiVector3D p = mesh->mVertices[i];
    const aiVector3D t =
        mesh->mTextureCoords[0] ? mesh->mTextureCoords[0][i] : aiVector3D(0.0f);
    const aiVector3D n = mesh->mNormals[i];
    vertices.emplace_back(Vertex{
        .pos = glm::vec3(p.x, p.y, p.z),
        .uv = glm::vec2(t.x, t.y),
        .norm = glm::vec3(n.x, n.y, n.z),
    });
  }

  for (size_t i = 0; i < mesh->mNumFaces; i++)
    for (size_t j = 0; j != 3; j++)
      indices.emplace_back(mesh->mFaces[i].mIndices[j]);

  MAI::Buffer *vertBuff = ren_->createBuffer({
      .usage = MAI::StorageBuffer,
      .storage = MAI::StorageType_Device,
      .size = sizeof(Vertex) * vertices.size(),
      .data = vertices.data(),
  });
  MAI::Buffer *indexBuff = ren_->createBuffer({
      .usage = MAI::IndexBuffer,
      .storage = MAI::StorageType_Device,
      .size = sizeof(uint32_t) * indices.size(),
      .data = indices.data(),
  });

  meshes.emplace_back(Mesh{
      .vertexBuffer = vertBuff,
      .indexBuffer = indexBuff,
      .indicesSize = (uint32_t)indices.size(),
  });

  if (mesh->mMaterialIndex >= 0) {
    const aiMaterial *mat = scene->mMaterials[mesh->mMaterialIndex];
    loadTextures(mat, aiTextureType_DIFFUSE, filename);
  }
}

void Model::draw(MAI::CommandBuffer *buff, glm::mat4 proj, glm::mat4 view,
                 glm::mat4 model) {
  struct PushConstant {
    glm::mat4 proj;
    glm::mat4 view;
    glm::mat4 model;
    uint32_t textId;
    uint64_t vertices;
  } pc{
      .proj = proj,
      .view = view,
      .model = model,
      .textId = textures[0]->getIndex(),
  };

  for (auto &it : meshes) {
    pc.vertices = ren_->gpuAddress(it.vertexBuffer);
    buff->cmdPushConstant(&pc);
    buff->bindIndexBuffer(it.indexBuffer, 0, MAI::IndexType::Uint32);
    buff->cmdDrawIndex(it.indicesSize);
  }
}

void Model::loadTextures(const aiMaterial *mat, aiTextureType type,
                         const char *dir) {

  aiString str;
  mat->GetTexture(type, 0, &str);
  std::string name = str.data;

  std::string path = dir;
  path += "/";
  path += str.data;

  for (auto &it : loaded)
    if (it == path)
      return;

  int w, h, comp;
  const stbi_uc *pixels = stbi_load(path.c_str(), &w, &h, &comp, 4);
  if (!pixels) {
    return;
  }
  MAI::Texture *texture = ren_->createImage({
      .type = MAI::TextureType_2D,
      .format = MAI::Format_RGBA_S8,
      .dimensions = {(uint32_t)w, (uint32_t)h},
      .data = pixels,
      .usage = MAI::Sampled_Bit,
  });
  stbi_image_free((void *)pixels);
  loaded.emplace_back(path);
  textures.emplace_back(texture);
}

Model::~Model() {
  if (!textures.empty())
    for (auto &it : textures)
      delete it;
  for (auto &it : meshes) {
    delete it.vertexBuffer;
    delete it.indexBuffer;
  }
}
