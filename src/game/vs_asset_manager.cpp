//
// Created by Vilhelm Stokstad on 2022-01-29.
//

#include "vs_asset_manager.h"
#include "profiler.h"
// std
#include <future>
#include <iostream>
#include <string>
#include <vector>

namespace vs::game {

vs::game::vs_asset_manager::vs_asset_manager(vs_device &device) {
  timer timer_;
  timer_.start();
  loadModelsFromFolder("models", device);
  timer_.stop();
  timer_.print_time();
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

void vs_asset_manager::loadModelsFromFolder(
    const std::string &models_folder_path, vs_device &device_) {

  std::vector<std::future<vs::vs_model_component::builder>> futures;
  std::size_t vsize = futures.size();
  std::vector<std::thread> threads;

  for (auto &entry :
       std::filesystem::recursive_directory_iterator(models_folder_path)) {
    if (entry.path().extension().string() == ".obj") {
      std::string path = entry.path().relative_path().string();
      std::string name = entry.path().filename().string();

      uintmax_t size = entry.file_size();


      // DO SOME ASYNC LOADING FOR LARGE FILES //
      if (size > 1000000) {
      //  continue;
        std::packaged_task<vs_model_component::builder(std::string name,
                                                       std::string path)>
            task([](std::string name, std::string path) {
              vs_model_component::builder builder{};
              builder.loadModel(path, path);

              assert(!builder.vertices.empty() && "builder failed");

              builder.name = name;
              return builder;
            });

        futures.emplace_back(task.get_future());
        threads.emplace_back(std::thread(std::move(task), name, path));
      } else {
        loadModelFromFileEnty(device_, entry);
      }
    }
  }

  for (auto &future : futures) {
    if (!future.valid())
      continue;
    future.wait();
    auto result = future.get();
    if (result.vertices.empty())
      continue;
    auto modelcomponent = std::make_unique<vs_model_component>(device_, result);
    std::cout << "loaded model: " << modelcomponent->string_name << std::endl;
    loaded_models.emplace(modelcomponent->string_name,
                          std::move(modelcomponent));
  }

  for (auto &t : threads) {
    t.join();
  }

  std::cout << "done loading!" << std::endl;
}

void vs_asset_manager::loadModelFromFileEnty(
    vs_device &device_, const std::filesystem::directory_entry &entry) {

  std::string rel_path = entry.path().relative_path().string();

  std::shared_ptr<vs_model_component> model =
      vs_model_component::createModelFromFile(device_, rel_path, rel_path);
  model->string_name =
      static_cast<std::string>(entry.path().filename().string());
  loaded_models.emplace(entry.path().filename().string(), std::move(model));
  std::cout << "loaded model: " << rel_path << std::endl;
}

bool vs_asset_manager::isModelLoaded(const std::string &model_name) {
  auto model = loaded_models.find(model_name);
  return (model != loaded_models.end());
}

} // namespace vs::game
