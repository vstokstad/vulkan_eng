//
// Created by Vilhelm Stokstad on 2022-02-14.
//
#include "vs_asset.h"

namespace assets{
bool save_binary_file(const char *path, const vs_asset_file &file) {
  std::ofstream outfile;
  outfile.open(path, std::ios::binary | std::ios::out);
  outfile.write(file.type, 4);
  uint32_t version = file.version;
  // write version
  outfile.write((const char *)&version, sizeof(uint32_t));

  // json length
  uint32_t length = file.json.size();
  outfile.write((const char *)&length, sizeof(uint32_t));
  // write json stream
  outfile.write(file.json.data(), length);
  // write blob data
  outfile.write(file.binary_blob.data(), file.binary_blob.size());

  outfile.close();
  return true;
}
bool load_binary_file(const char *path, vs_asset_file &output_file) {
  std::ifstream infile;
  infile.open(path, std::ios::binary);

  if (!infile.is_open())
    return false;

  infile.seekg(0);
  infile.read((char *)&output_file.type, 4);
  infile.read((char *)&output_file.version, sizeof(uint32_t));
  // get lenghts
  uint32_t jsonlen = 0;
  infile.read((char *)&jsonlen, sizeof(uint32_t));

  uint32_t bloblen = 0;
  infile.read((char *)&bloblen, sizeof(uint32_t));

  // resize and read to outfile
  output_file.json.resize(jsonlen);
  infile.read(output_file.json.data(), jsonlen);

  output_file.binary_blob.resize(bloblen);
  infile.read(output_file.binary_blob.data(), bloblen);

  return true;
}
}