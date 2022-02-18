//
// Created by Vilhelm Stokstad on 2022-01-29.
//
#pragma once

#include "vs_game_object.h"
#include <filesystem>
#include <map>

namespace vs {

class vs_asset_manager {
public:
  explicit vs_asset_manager(vs_device &device);
  ~vs_asset_manager() { cleanup(); };

  vs_game_object spawnGameObject(const std::string &model_name = "",
                                 glm::vec3 position = glm::vec3(0.f),
                                 glm::vec3 rotation = glm::vec3(0.f),
                                 glm::vec3 scale = glm::vec3(1.f));


  void cleanup();

  std::map<std::string, std::shared_ptr<vs_model_component>> loaded_models;
  std::map<std::string, std::shared_ptr<vs_texture>> loaded_textures;

  vs_device &device_;
};

} // namespace vs
