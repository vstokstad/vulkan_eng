//
// Created by Vilhelm Stokstad on 2022-02-14.
//

#ifndef _VS_ASSET_H_
#define _VS_ASSET_H_
#include <fstream>
#include <string>
#include <vector>
namespace assets {
struct vs_asset_file {
  char type[4];
  int version;
  std::string json;
  std::vector<char> binary_blob;
};



bool save_binary_file(const char *path, const vs_asset_file &file);
bool load_binary_file(const char *path, vs_asset_file &output_file);

} // namespace vs::assets
#endif //_VS_ASSET_H_
