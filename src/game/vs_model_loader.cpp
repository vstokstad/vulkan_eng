//
// Created by Vilhelm Stokstad on 2022-01-29.
//

#include "vs_model_loader.h"

// std
#include <iostream>
#include <string>

namespace vs::game {

vs::game::vs_model_loader::vs_model_loader(vs_device &device) {
  loadModelsFromFolder("models", device);
}

vs_game_object vs_model_loader::spawnGameObject(const std::string &model_name,
                                                  glm::vec3 position,
                                                  glm::vec3 rotation,
                                                  glm::vec3 scale) {

  auto object = vs_game_object::createGameObject();
  if (!model_name.empty()) {
    auto model = loaded_models.find(model_name);
    if (model == loaded_models.end()) {
      throw std::runtime_error("failed to create a game object with "
                               "${model_name}, it was not preloaded.");
    }
    std::cout << "creating vs_game_object with model: " << model_name
              << std::endl;
    object.transform_comp.translation = position;
    object.transform_comp.rotation = rotation;
    object.transform_comp.scale = scale;
    object.color = {1.f, 1.f, 1.f};
    object.model_comp = model->second;
  }
  return object;
}

void vs_model_loader::loadModelsFromFolder(const std::string& models_folder_path,
                                             vs_device &device_) {

  std::vector<std::filesystem::path> model_paths;
  for (auto &entry : std::filesystem::directory_iterator(models_folder_path)) {
    if (entry.path().extension() == ".obj") {
      model_paths.emplace_back(
          std::move(entry.path().relative_path().string()));
      std::cout << "found model to load: " << entry.path() << std::endl;
    }
  }

  preloadModels(model_paths, device_);
}

void vs_model_loader::preloadModels(
    const std::vector<std::filesystem::path> &model_paths, vs_device &device) {

  for (auto &p : model_paths) {
    std::string rel_path = p.relative_path();
    std::cout << "loaded model: " << rel_path << std::endl;
    std::shared_ptr<vs_model_component> model =
        vs_model_component::createModelFromFile(device, rel_path,
                                                p.relative_path());
    model->string_name = static_cast<std::string>(p.filename());
    loaded_models.emplace(p.filename(), std::move(model));
  };
}

} // namespace vs::game
