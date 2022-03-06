//
// Created by Vilhelm Stokstad on 2022-02-14.
//
#include "vs_asset.h"
//libs
#include <nlohmann/json.hpp>
#include <lz4.h>

namespace vs{
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

compression_mode parse_compression(const char *f) {
  if (strcmp(f, "LZ4") == 0) {
    return compression_mode::LZ4;
  } else {
    return compression_mode::None;
  }
}
texture_format parse_format(const char *format) {
  if (strcmp(format, "RGBA8") == 0) {
    return texture_format::RGBA8;
  } else {
    return texture_format::Unknown;
  }
}
vs_asset_file pack_texture(texture_info *info, void *pixel_data) {

  // core file header
  vs_asset_file file{};
  file.type[0] = 'T';
  file.type[1] = 'E';
  file.type[2] = 'X';
  file.type[3] = 'I';
  file.version = 1;
  // json info
  nlohmann::json texture_metadata;
  texture_metadata["format"] = "RGBA8";
  texture_metadata["width"] = info->pixel_size[0];
  texture_metadata["height"] = info->pixel_size[1];
  texture_metadata["buffer_size"] = info->texture_size;
  texture_metadata["original_file"] = info->original_file;

  int compression_staging = LZ4_compressBound(info->texture_size);
  file.binary_blob.resize(compression_staging);
  int compressed_size =
      LZ4_compress_default((const char *)pixel_data, file.binary_blob.data(),
                           info->texture_size, compression_staging);
  file.binary_blob.resize(compressed_size);

  texture_metadata["compression"] = "LZ4";

  std::string stringified = texture_metadata.dump();
  file.json = stringified;
  return file;
}

void unpack_texture(texture_info *info, const char *source_buffer,
                    size_t source_size, char *destination) {

  if (info->compression == compression_mode::LZ4) {
    LZ4_decompress_safe(source_buffer, destination, source_size,
                        info->texture_size);
  } else {
    memcpy(destination, source_buffer, source_size);
  }
}
texture_info read_texture_info(vs_asset_file *asset) {

  texture_info info;
  nlohmann::json texture_metadata = nlohmann::json::parse(asset->json);
  std::string format_string = texture_metadata["format"];
  info.format = parse_format(format_string.c_str());
  std::string comp_string = texture_metadata["compression"];
  info.compression = vs::parse_compression(comp_string.c_str());

  info.pixel_size[0] = texture_metadata["width"];
  info.pixel_size[1] = texture_metadata["height"];
  info.texture_size = texture_metadata["buffer_size"];
  info.original_file = texture_metadata["original_file"];

  return info;
}
}