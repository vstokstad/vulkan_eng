//
// Created by Vilhelm Stokstad on 2022-01-29.
//

#include "vs_asset_manager.h"

// std
#include <future>
#include <iostream>
#include <string>


namespace vs {

vs_asset_manager::vs_asset_manager(vs_device &device) : device_(device) {

}

vs_game_object vs_asset_manager::spawnGameObject(const std::string &model_name,
                                                 glm::vec3 position,
                                                 glm::vec3 rotation,
                                                 glm::vec3 scale) {

  auto object = vs_game_object::createGameObject();
  if (!model_name.empty()) {
    auto model = loaded_models.find(model_name);
    if (model == loaded_models.end()) {
      std::cout << model_name << std::endl;
      throw std::runtime_error(
          "failed to create a game object, model was not preloaded.");
    }
    object.transform_comp.translation = position;
    object.transform_comp.rotation = rotation;
    object.transform_comp.scale = scale;
    object.color = {1.f, 1.f, 1.f};
    object.model_comp = model->second;
  }
  return object;
}


void vs_asset_manager::cleanup() {}

} // namespace vs
