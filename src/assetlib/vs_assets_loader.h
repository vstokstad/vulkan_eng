//
// Created by vstok on 2022-02-18.
//
#pragma once
#include "vs_game_object.h"
// std
#include <filesystem>
#include <map>
namespace vs {


class vs_assets_loader {
  const std::string models_path = "assets/models";
  const std::string textures_path = "assets/textures";

  explicit vs_assets_loader(vs_device &device);

  bool isModelLoaded(const std::string &model_name);

private:
  void loadModelsFromFolder(vs_device &device_,
                            const std::string &models_folder_path);
  void loadTexturesFromFolder(vs_device &device,
                              const std::string &textures_folder_path);

  void loadModelFromPath(vs_device &device_, const std::string& path);

  void loadTextureFromPath(vs_device& device, const std::string& path);

  void cleanup();

//  std::map<std::string, std::shared_ptr<vs_model_component>> loaded_models;
  //std::map<std::string, std::shared_ptr<vs_texture>> loaded_textures;

  vs_device &device_;
};

} // namespace vs
