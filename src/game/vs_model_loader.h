//
// Created by Vilhelm Stokstad on 2022-01-29.
//
#pragma once

#include "vs_game_object.h"
#include <filesystem>
#include <map>

namespace vs::game {

class vs_model_loader {
public:
  explicit vs_model_loader(vs_device &device);


  vs_game_object spawnGameObject(const std::string &model_name = "",
                                 glm::vec3 position = glm::vec3(0.f),
                                 glm::vec3 rotation = glm::vec3(0.f),
                                 glm::vec3 scale = glm::vec3(1.f));


private:
  void loadModelsFromFolder(const std::string &models_folder_path,
                            vs_device &device_);
  void preloadModels(const std::vector<std::filesystem::path> &model_paths,
                     vs_device &device_);

  std::map<std::string, std::shared_ptr<vs_model_component>> loaded_models;
};

} // namespace vs::game
