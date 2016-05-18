#include <libgen.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <zlib.h>

#include <string>
#include <vector>

#include <mysya/ioevent/logger.h>

#include "tutorial/orcas/combat/base64.h"
#include "tutorial/orcas/deps/tinyxml/tinyxml.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace test {

class BlockConfig {
 public:
  BlockConfig();
  ~BlockConfig();

  bool Load(const std::string &file);

  int GetHeight() const;
  int GetWidth() const;
  const std::string &GetBlocks() const;

 private:
  int height_;
  int width_;
  std::string blocks_;
};

BlockConfig::BlockConfig()
  : height_(0), width_(0) {}
BlockConfig::~BlockConfig() {}

bool BlockConfig::Load(const std::string &file) {
  TiXmlDocument doc;
  if (doc.LoadFile(file.data()) == false) {
    MYSYA_ERROR("Load config(%s) failed.", file.data());
    return false;
  }

  TiXmlElement *map_node = doc.FirstChildElement("map");
  if (map_node == NULL) {
    MYSYA_ERROR("Config file(%s) /map not found.", file.data());
    return false;
  }

  const char *attr = NULL;
  TiXmlElement *path_layer_node = NULL;

  TiXmlElement *layer_node = map_node->FirstChildElement("layer");
  while (layer_node != NULL) {
    attr = layer_node->Attribute("name");
    if (attr == NULL) {
      MYSYA_ERROR("Config file(%s) /map/layer[name] not found.",
          file.data());
      return false;
    }

    if (std::string(attr) == "path") {
      path_layer_node = layer_node;
      break;
    }

    layer_node = layer_node->NextSiblingElement("layer");
  }

  if (path_layer_node == NULL) {
    MYSYA_ERROR("Config file(%s) not found path layer.",
        file.data());
    return false;
  }

  attr = path_layer_node->Attribute("width");
  if (attr == NULL) {
    MYSYA_ERROR("Config file(%s) not found /map/layer[width] not found.",
        file.data());
    return false;
  }
  this->width_ = atoi(attr);

  attr = path_layer_node->Attribute("height");
  if (attr == NULL) {
    MYSYA_ERROR("Config file(%s) not found /map/layer[height] not found.",
        file.data());
    return false;
  }
  this->height_ = atoi(attr);

  TiXmlElement *data_node = path_layer_node->FirstChildElement("data");
  if (data_node == NULL) {
    MYSYA_ERROR("Config file(%s) /map/layer/data not found.",
        file.data());
    return false;
  }

  attr = data_node->Attribute("encoding");
  if (attr == NULL) {
    MYSYA_ERROR("Config file(%s) /map/layer/data[encoding] not found.",
        file.data());
    return false;
  }
  if (std::string(attr) != "base64") {
    MYSYA_ERROR("Config file(%s) /map/layer/data[encoding(%s)] is not base64.",
        file.data(), attr);
    return false;
  }

  attr = data_node->Attribute("compression");
  if (attr == NULL) {
    MYSYA_ERROR("Config file(%s) /map/layer/data[compression] not found.",
        file.data());
    return false;
  }
  if (std::string(attr) != "zlib") {
    MYSYA_ERROR("Config file(%s) /map/layer/data[compression(%s)] is not zlib.",
        file.data(), attr);
    return false;
  }

  attr = data_node->GetText();
  int attr_size = strlen(attr);

  std::string compressed_blocks = base64_decode(attr, attr_size);
  uLongf compressed_size = compressed_blocks.size();

  uLongf buffer_size = this->width_ * this->height_ * 4;

  std::vector<char> buffer;
  buffer.resize(buffer_size);

  uLongf uncompressed_size = buffer_size;
  int ret = ::uncompress((Bytef *)&buffer[0], (uLongf *)&uncompressed_size,
      (const Bytef *)compressed_blocks.data(), compressed_size);
  if (ret != Z_OK) {
    MYSYA_ERROR("::uncompress() failed, error=%d[Z_DATA_ERROR(%d), Z_MEM_ERROR(%d), Z_BUF_ERROR(%d)].",
        ret, Z_DATA_ERROR, Z_MEM_ERROR, Z_BUF_ERROR);
    return false;
  }

  MYSYA_DEBUG("uncompressed_size=%d", uncompressed_size);

  for (size_t i = 0; i < uncompressed_size; i += sizeof(int)) {
    if (*(int *)(&buffer[i]) > 0) {
      this->blocks_ += "0";
    } else {
      this->blocks_ += "1";
    }
  }

  return true;
}

const std::string &BlockConfig::GetBlocks() const {
  return this->blocks_;
}

int BlockConfig::GetHeight() const {
  return this->height_;
}

int BlockConfig::GetWidth() const {
  return this->width_;
}

}  // namespace test
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial

class CommandLineOption {
 public:
  CommandLineOption() {}
  ~CommandLineOption() {}

  bool Parse(int argc, char *argv[]);

  std::string error_;
  std::string conf_file_;
  std::string cmd_dirname_;
  std::string cmd_basename_;
};

bool CommandLineOption::Parse(int argc, char *argv[]) {
  int opt = -1;

  std::vector<char> buffer(argv[0], argv[0] + strlen(argv[0]) + 1);
  this->cmd_dirname_ = ::dirname(&buffer[0]);
  this->cmd_basename_ = ::dirname(&buffer[0]);

  opterr = 0;

  while ((opt = ::getopt(argc, argv, "c:")) != -1) {
    switch (opt) {
      case 'c':
        this->conf_file_ = optarg;
        break;
      default:
        this->error_ = "invalid option";
        return false;
    }
  }

  if (this->conf_file_.empty() == true) {
    this->error_ = "invalid option c";
    return false;
  }

  return true;
}

int main(int argc, char *argv[]) {
  CommandLineOption option;
  if (option.Parse(argc, argv) == false) {
    MYSYA_ERROR("Option::Parse failed(%s).", option.error_.data());
    return -1;
  }

  ::tutorial::orcas::combat::test::BlockConfig conf;
  if (conf.Load(option.conf_file_) == false) {
    MYSYA_ERROR("Load Config file(%s) failed.",
        option.conf_file_.data());
    return -1;
  }

  int height = conf.GetHeight();
  int width = conf.GetWidth();
  const std::string &blocks = conf.GetBlocks();

  for (int i = 0; i < height; ++i) {
    for (int j = 0; j < width; ++j) {
      size_t pos = i * width + j;
      // size_t pos = i * height + j;
      if (pos >= blocks.size()) {
        MYSYA_ERROR("error pos(%lu), i=%d, j=%d",
          pos, i, j);
        return -1;
      }
      printf("%c", blocks.data()[pos]);
    }

    printf("\n");
  }

  return 0;
}
