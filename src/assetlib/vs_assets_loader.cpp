//
// Created by vstok on 2022-02-18.
//

#include "vs_assets_loader.h"
#include "profiler.h"

// std
#include <future>
#include <iostream>
#include <string>
using namespace vs;
vs_assets_loader::vs_assets_loader(vs_device &device) : device_(device) {
  loadModelsFromFolder(device_, models_path);
}
void vs_assets_loader::loadModelsFromFolder(
    vs_device &device_, const std::string &models_folder_path) {

  std::vector<std::future<vs::vs_model_component::builder>> futures;
  std::vector<std::thread> threads;

  for (auto &entry :
       std::filesystem::recursive_directory_iterator(models_folder_path)) {
    if (entry.path().extension().string() == ".obj") {
      std::string path = entry.path().relative_path().string();
      std::string name = entry.path().filename().string();

      uintmax_t size = entry.file_size();

      // DO SOME ASYNC LOADING FOR LARGE FILES //
      if (size > 10000) {
        //  continue;
        // don't load big models right, now, its annoying!
        std::packaged_task<vs_model_component::builder(std::string name,
                                                       std::string path)>
            task([](std::string name, std::string path) {
              vs_model_component::builder builder{};
              builder.loadModel(path, false);

              assert(!builder.vertices.empty() && "builder failed");

              return builder;
            });

        futures.emplace_back(task.get_future());
        threads.emplace_back(std::thread(std::move(task), name, path));
      } else {

        loadModelFromPath(device_, path);
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
    // loaded_models.emplace(modelcomponent->string_name,
    //                      std::move(modelcomponent));
  }

  for (auto &t : threads) {
    t.join();
  }
};

void vs_assets_loader::loadTexturesFromFolder(
    vs_device &device, const std::string &textures_folder_path) {
  for (auto &entry :
       std::filesystem::recursive_directory_iterator(textures_folder_path)) {
    if (entry.path().extension().string() == ".png") {
      std::string path = entry.path().relative_path().string();
      std::string name = entry.path().filename().string();

      uintmax_t size = entry.file_size();
    }
  }
}

void vs_assets_loader::loadModelFromPath(vs_device &device_,
                                         const std::string &path) {

  std::shared_ptr<vs_model_component> model =
      vs_model_component::createModelFromFile(device_, path);


//  loaded_models.emplace(path, std::move(model));

  std::cout << "loaded model: " << path << std::endl;
}

bool vs_assets_loader::isModelLoaded(const std::string &model_name) {
 // auto model = loaded_models.find(model_name);
 // return (model != loaded_models.end());
}

void vs_assets_loader::loadTextureFromPath(vs_device &device,
                                           const std::string &path) {
  std::shared_ptr<vs_texture> texture =
      vs_texture::createTextureFromFile(device, path);
}
