#include <libgen.h>
#include <math.h>
#include <signal.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <zlib.h>

#include <algorithm>
#include <map>
#include <string>
#include <vector>

#include "tinyxml/tinyxml.h"

namespace tutorial {
namespace generate_path_config {

void Print(const char *format, ...) {
  va_list args;
  va_start(args, format);
  ::vfprintf(stdout, format, args);
  ::fprintf(stdout, "\n");
  va_end(args);
}

#define DEBUG(c, ...) \
    Print("[DEBUG] " c, ##__VA_ARGS__)
#define ERROR(c, ...) \
    Print("%s:%d (%s) [ERROR] " c, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)

static const char s_encode_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                     "abcdefghijklmnopqrstuvwxyz"
                                     "0123456789+/";
static const char s_decode_table[] = {
    62, -1, -1, -1, 63, 52, 53, 54, 55, 56,
    57, 58, 59, 60, 61, -1, -1, -1, -2, -1,
    -1, -1,  0,  1,  2,  3,  4,  5,  6,  7,
     8,  9, 10, 11, 12, 13, 14, 15, 16, 17,
    18, 19, 20, 21, 22, 23, 24, 25, -1, -1,
    -1, -1, -1, -1, 26, 27, 28, 29, 30, 31,
    32, 33, 34, 35, 36, 37, 38, 39, 40, 41,
    42, 43, 44, 45, 46, 47, 48, 49, 50, 51
};

int base64_decode(const char *in, size_t in_size,
    char *out, size_t out_size) {
  unsigned char b[4];
  const char *out_start = out;
  const char *out_end = out + out_size;

  for (; in_size > 0;) {
    if (in_size < 4) {
      return -1;
    }

    int b_len = 0;

    for (int i = 0; i < 4; ++i) {
      char c = *in++;
      if (c < 43 || c > 122 || '=' == c) {
        b[i] = 0;
      } else {
        b[i] = s_decode_table[c - 43];
        ++b_len;
      }
    }

    if (b_len < 2) {
      return -1;
    }

    // overflow
    if (out_end - out < b_len - 1) {
      return -1;
    }

    *out++ = b[0] << 2 | b[1] >> 4;
    if (b_len > 2) {
      *out++ = b[1] << 4 | b[2] >> 2;
    }
    if (b_len > 3) {
      *out++ = b[2] << 6 | b[3];
    }

    in_size -= 4;
  }

  return out - out_start;
}

std::string base64_decode(const char *buffer, size_t size) {
  std::vector<char> output(size);
  int count = base64_decode(buffer, size, &output[0], output.size());
  if (count <= 0) {
    return std::string();
  } else {
    return std::string(&output[0], count);
  }
}

std::string base64_decode(const std::string &str) {
  return base64_decode(str.c_str(), str.size());
}

class CommandLineOption {
 public:
  CommandLineOption() : min_path_length_(5) {}
  ~CommandLineOption() {}

  bool Parse(int argc, char *argv[]);

  std::string error_;
  std::string conf_file_;
  std::string export_file_;
  int32_t min_path_length_;
  std::string cmd_dirname_;
  std::string cmd_basename_;
};

bool CommandLineOption::Parse(int argc, char *argv[]) {
  int opt = -1;

  std::vector<char> buffer(argv[0], argv[0] + strlen(argv[0]) + 1);
  this->cmd_dirname_ = ::dirname(&buffer[0]);
  this->cmd_basename_ = ::dirname(&buffer[0]);

  opterr = 0;

  while ((opt = ::getopt(argc, argv, "c:o:m:")) != -1) {
    switch (opt) {
      case 'c':
        this->conf_file_ = optarg;
        break;
      case 'o':
        this->export_file_ = optarg;
        break;
      case 'm':
        this->min_path_length_ = atoi(optarg);
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
  if (this->export_file_.empty() == true) {
    this->error_ = "invalid option o";
    return false;
  }

  return true;
}

struct Position {
  int32_t x() const { return this->x_; }
  void set_x(int32_t value) { this->x_ = value; }
  int32_t y() const { return this->y_; }
  void set_y(int32_t value) { this->y_ = value; }

  int32_t x_;
  int32_t y_;
};

struct PathConf {
  typedef std::vector<Position> PathVector;
  Position p1_;
  Position p2_;
  PathVector paths_;
};

struct MapConf {
  int32_t id_;
  int32_t height_;
  int32_t width_;
  std::string blocks_;
};

class Config {
 public:
  typedef std::map<int32_t, MapConf> MapConfMap;

  Config();
  ~Config();

  bool LoadConfig(const std::string &file);
  bool GeneratePathConfig(const std::string &file, int32_t min_path_length);

  const MapConfMap &GetMaps() const;

 private:
  bool LoadBlockConfig(int32_t map_id, const std::string &file);

  std::string path_;
  MapConfMap maps_;
};

class Node {
 public:
  bool operator <(const Node &other) const;
  int NeighbroGCost(const Node &other) const;
  int HeursiticConstEstimate(const Node &other) const;

  Position pos_;
  bool walkable_;
  int f_;
  int g_;
  int open_list_pos_;
  int close_list_pos_;
  Node *parent_;
};

class Scene {
 public:
  typedef std::vector<Node> NodeVector;
  typedef std::vector<Node *> NodePtrVector;
  typedef std::vector<Position> PositionVector;

  explicit Scene(const MapConf &map_conf);
  ~Scene();

  bool SearchPath(const Position &pos1, const Position &pos2,
      PositionVector &paths);
  void PrintSearchPath(const Position &src_pos, const Position &dest_pos);

 private:
  Node *GetNode(const Position &pos);
  Node *GetNode(int32_t x, int32_t y);
  void GetNeighborNodes(const Node *node, NodePtrVector &neighbor_nodes);
  bool IsInOpenList(const Node *node) const;
  bool IsOpenListEmpty() const;
  void InsertOpenList(Node *node);
  Node *GetMinFScoreNodeInOpenList();
  void DeleteMinFScoreNodeInOpenList();
  void ResortOpenList(Node *node);
  bool IsInCloseList(const Node *node) const;
  void InsertCloseList(Node *node);
  void ConstructResultPath(PositionVector &result);

  int32_t height_;
  int32_t width_;
  std::string blocks_;

  NodeVector nodes_;
  Node *start_node_;
  Node *end_node_;
  NodePtrVector open_list_;
};

Config::Config() {}
Config::~Config() {}

bool Config::LoadConfig(const std::string &file) {
  std::vector<char> file_copy(file.size());
  ::strcpy(&file_copy[0], file.data());
  this->path_ = ::dirname(&file_copy[0]);

  TiXmlDocument doc;
  if (doc.LoadFile(file.data()) == false) {
    ERROR("file(%s) TiXmlDocument::LoadFile() failed.", file.data());
    return false;
  }

  TiXmlElement *resultset_node = doc.FirstChildElement("resultset");
  if (resultset_node == NULL) {
    ERROR("file(%s) /resultset not found.", file.data());
    return false;
  }

  const char *attr = NULL;
  const char *attr_name = NULL;

  TiXmlElement *row_node = resultset_node->FirstChildElement("row");
  while (row_node != NULL) {
    int32_t map_id = 0;
    std::string block_file_name;

    TiXmlElement *field_node = row_node->FirstChildElement("field");
    while (field_node != NULL) {
      attr_name = field_node->Attribute("name");
      if (attr_name == NULL) {
        ERROR("file(%s) /resultset/row/field[name] not found.", file.data());
        return false;
      }

      attr = field_node->GetText();
      if (attr == NULL) {
        ERROR("file(%s) /resultset/row/field[text] null.", file.data());
        return false;
      }

      std::string attr_name_str = attr_name;
      if (attr_name_str == "id") {
        map_id = atoi(attr);
      } else if (attr_name_str == "map_build_id") {
      } else if (attr_name_str == "tilemap") {
        block_file_name = this->path_ + "/" + attr + ".tmx";
      } else {
      }

      field_node = field_node->NextSiblingElement("field");
    }

    if (this->LoadBlockConfig(map_id, block_file_name) == false) {
      ERROR("LoadBlockConfig(%d, %s) failed.", map_id, block_file_name.data());
      return false;
    }

    row_node = row_node->NextSiblingElement("row");
  }

  return true;
}

bool Config::LoadBlockConfig(int32_t map_id, const std::string &file) {
  TiXmlDocument doc;
  if (doc.LoadFile(file.data()) == false) {
    ERROR("file(%s) TiXmlDocument::LoadFile() failed.", file.data());
    return false;
  }

  TiXmlElement *map_node = doc.FirstChildElement("map");
  if (map_node == NULL) {
    ERROR("file(%s) /map not found.", file.data());
    return false;
  }

  const char *attr = NULL;
  TiXmlElement *path_layer_node = NULL;

  TiXmlElement *layer_node = map_node->FirstChildElement("layer");
  while (layer_node != NULL) {
    attr = layer_node->Attribute("name");
    if (attr == NULL) {
      ERROR("file(%s) /map/layer[name] not found.", file.data());
      return false;
    }

    if (std::string(attr) == "path") {
      path_layer_node = layer_node;
      break;
    }

    layer_node = layer_node->NextSiblingElement("layer");
  }

  if (path_layer_node == NULL) {
    ERROR("flie(%s) not found path layer.", file.data());
    return false;
  }

  MapConf conf;
  conf.id_ = map_id;

  attr = path_layer_node->Attribute("width");
  if (attr == NULL) {
    ERROR("file(%s) /map/layer[width] not found.", file.data());
    return false;
  }
  conf.width_ = atoi(attr);

  attr = path_layer_node->Attribute("height");
  if (attr == NULL) {
    ERROR("file(%s) /map/layer[height] not found.", file.data());
    return false;
  }
  conf.height_ = atoi(attr);

  TiXmlElement *data_node = path_layer_node->FirstChildElement("data");
  if (data_node == NULL) {
    ERROR("file(%s) /map/layer/data not found.", file.data());
    return false;
  }

  attr = data_node->Attribute("encoding");
  if (attr == NULL) {
    ERROR("file(%s) /map/layer/data[encoding] not found.", file.data());
    return false;
  }
  if (std::string(attr) != "base64") {
    ERROR("file(%s) /map/layer/data[encoding(%s)] is not base64.", file.data());
    return false;
  }

  attr = data_node->Attribute("compression");
  if (attr == NULL) {
    ERROR("file(%s) /map/layer/data[compression] is not zlib.", file.data());
    return false;
  }
  if (std::string(attr) != "zlib") {
    ERROR("file(%s) /map/layer/data[compression(%s)] is not zlib.", file.data());
    return false;
  }

  attr = data_node->GetText();
  int attr_size = strlen(attr);

  std::string decode_str = base64_decode(attr, attr_size);
  uLongf decode_str_size = decode_str.size();

  uLongf buffer_size = conf.width_ * conf.height_ * 5;
  std::vector<char> buffer;
  buffer.resize(buffer_size);

  int ret = ::uncompress((Bytef *)&buffer[0], &buffer_size,
      (const Bytef *)decode_str.data(), decode_str_size);
  if (ret != Z_OK) {
    ERROR("file(%s) ::uncompress() failed(%d).", file.data(), ret);
    return false;
  }

  for (size_t i = 0; i < buffer_size; i += sizeof(int)) {
    if (*(int *)(&buffer[i]) > 0) {
      conf.blocks_ += "0";
    } else {
      conf.blocks_ += "1";
    }
  }

  this->maps_.insert(std::make_pair(conf.id_, conf));

  return true;
}

bool Config::GeneratePathConfig(const std::string &file, int32_t min_path_length) {
  for (MapConfMap::const_iterator iter = this->maps_.begin();
      iter != this->maps_.end(); ++iter) {
    Scene scene(iter->second);

    typedef std::vector<Position> PositionVector;
    PositionVector walkable_poses;

    for (int y = 0; y < iter->second.height_; ++y) {
      for (int x = 0; x < iter->second.width_; ++x) {
        if (iter->second.blocks_[y * iter->second.width_ + x] == '0') {
          Position pos;
          pos.x_ = x;
          pos.y_ = y;
          walkable_poses.push_back(pos);
        }
      }
    }

    TiXmlDocument doc;

    TiXmlDeclaration dec("1.0", "utf-8", "");
    doc.InsertEndChild(dec);

    char buffer[4096] = "0";
    size_t count = 0;

    for (size_t i = 0; i < walkable_poses.size(); ++i) {
      for (size_t j = i + 1; j < walkable_poses.size(); ++j) {
        typedef Scene::PositionVector PositionVector;
        PositionVector result;
        if (scene.SearchPath(walkable_poses[i], walkable_poses[j], result) == false) {
          ERROR("map(%d) SearchPath([%d,%d], [%d,%d]) failed.",
              iter->second.id_, walkable_poses[i].x(), walkable_poses[i].y(),
              walkable_poses[j].x(), walkable_poses[j].y());
          return false;
        }

        if (result.size() < (size_t)min_path_length) {
          continue;
        }

        TiXmlElement node("n");

        count = 0;
        count += snprintf(buffer + count, sizeof(buffer) - count, "{%d,%d}",
            walkable_poses[i].x(), walkable_poses[i].y());
        buffer[count] = '\0';
        node.SetAttribute("b", buffer);

        count = 0;
        count += snprintf(buffer + count, sizeof(buffer) - count, "{%d,%d}",
            walkable_poses[j].x(), walkable_poses[j].y());
        buffer[count] = '\0';
        node.SetAttribute("d", buffer);

        count = 0;
        for (PositionVector::const_iterator iter = result.begin();
            iter != result.end(); ++iter) {
          count += snprintf(buffer + count, sizeof(buffer) - count, "{%d,%d},",
              iter->x(), iter->y());
        }
        buffer[count] = '\0';
        node.SetAttribute("p", buffer);

        doc.InsertEndChild(node);
      }
    }

    count = 0;
    count += snprintf(buffer + count, sizeof(buffer) - count, "%s/map%d_%s",
        this->path_.data(), iter->second.id_, file.data());
    buffer[count] = '\0';

    doc.SaveFile(buffer);
  }

  return true;
}

const Config::MapConfMap &Config::GetMaps() const {
  return this->maps_;
}

bool Node::operator <(const Node &other) const {
  // if (this->pos_.x() == other.pos_.x() ||
  //     this->pos_.y() == other.pos_.y()) {
  //   return 10;
  // } else {
  //   return 14;
  // }
  return this->f_ < other.f_;
}

int Node::NeighbroGCost(const Node &other) const {
  return 10;
}

int Node::HeursiticConstEstimate(const Node &other) const {
  int dx = this->pos_.x() - other.pos_.x();
  int dy = this->pos_.y() - other.pos_.y();

  return (int)(sqrt(dx *dx + dy * dy) * 10);
}

Scene::Scene(const MapConf &map_conf)
  : height_(map_conf.height_),
    width_(map_conf.width_),
    blocks_(map_conf.blocks_) {
  for (int y = 0; y < this->height_; ++y) {
    for (int x = 0; x < this->width_; ++x) {
      Node node;
      node.pos_.set_x(x);
      node.pos_.set_y(y);
      node.walkable_ = (this->blocks_[y * this->width_ + x] == '0');
      this->nodes_.push_back(node);
    }
  }
}

Scene::~Scene() {}

bool Scene::SearchPath(const Position &begin_pos, const Position &end_pos,
    PositionVector &paths) {
  if (begin_pos.x() < 0 || begin_pos.x() >= this->width_ ||
      begin_pos.y() < 0 || begin_pos.y() >= this->height_) {
    ERROR("[SCENE] begin_pos(%d,%d) is invalid.",
        begin_pos.x(), begin_pos.y());
    return false;
  }
  if (end_pos.x() < 0 || end_pos.x() >= this->width_ ||
      end_pos.y() < 0 || end_pos.y() >= this->height_) {
    ERROR("[SCENE] end_pos(%d,%d) is invalid.",
        end_pos.x(), end_pos.y());
    return false;
  }

  for (size_t i = 0; i < this->nodes_.size(); ++i) {
    Node *node = &this->nodes_[i];
    node->f_ = 0;
    node->g_ = 0;
    node->open_list_pos_ = 0;
    node->close_list_pos_ = 0;
    node->parent_ = NULL;
  }

  this->start_node_ = this->GetNode(begin_pos);
  this->end_node_ = this->GetNode(end_pos);
  this->open_list_.clear();
  this->open_list_.push_back(NULL);

  this->start_node_->g_ = 0;
  this->start_node_->f_ = this->start_node_->g_ +
      this->start_node_->HeursiticConstEstimate(*this->end_node_);
  this->InsertOpenList(this->start_node_);

  NodePtrVector neighbor_nodes;
  neighbor_nodes.reserve(8);

  while (this->IsOpenListEmpty() == false) {
    Node *cur_node = this->GetMinFScoreNodeInOpenList();
    if (cur_node == this->end_node_) {
      this->ConstructResultPath(paths);
      return true;
    }

    this->DeleteMinFScoreNodeInOpenList();
    this->InsertCloseList(cur_node);

    this->GetNeighborNodes(cur_node, neighbor_nodes);

    for (size_t i = 0; i < neighbor_nodes.size(); ++i) {
      Node *neighbor = neighbor_nodes[i];

      bool is_in_open_list = this->IsInOpenList(neighbor);
      bool is_in_close_list = this->IsInCloseList(neighbor);
      int g_cal = cur_node->g_ + cur_node->NeighbroGCost(*neighbor);

      if (is_in_close_list && g_cal >= neighbor->g_) {
        continue;
      }

      if (is_in_open_list == false || g_cal < neighbor->g_) {
        neighbor->parent_ = cur_node;
        neighbor->g_ = g_cal;
        neighbor->f_ = neighbor->g_ +
          neighbor->HeursiticConstEstimate(*this->end_node_);

        if (is_in_open_list == false) {
          this->InsertOpenList(neighbor);
        } else {
          this->ResortOpenList(neighbor);
        }
      }
    }
  }

  return false;
}

void Scene::PrintSearchPath(const Position &begin_pos, const Position &end_pos) {
  PositionVector result;
  if (this->SearchPath(begin_pos, end_pos, result) == false) {
    ERROR("[SCENE] SearchPath([%d,%d],[%d,%d]) failed.",
        begin_pos.x(), begin_pos.y(), end_pos.x(), end_pos.y());
    return;
  }

  std::string map_string = this->blocks_;
  if (result.empty() == true) {
    ERROR("[SCENE] SearchPath([%d,%d],[%d,%d]) failed.",
        begin_pos.x(), begin_pos.y(), end_pos.x(), end_pos.y());
    return;
  }

  for (size_t i = 0; i < result.size(); ++i) {
    if (i == 0) {
      map_string[result[i].y() * this->width_ + result[i].x()] = 'B';
    } else if (i == result.size() - 1) {
      map_string[result[i].y() * this->width_ + result[i].x()] = 'E';
    } else {
      map_string[result[i].y() * this->width_ + result[i].x()] = 'X';
    }
  }

  for (int y = 0; y < this->height_; ++y) {
    for (int x = 0; x < this->width_; ++x) {
      char c = map_string[y * this->width_ + x];

      if (c == 'X' || c == 'B' || c == 'E') {
        ::printf("\033[;32m");
        ::printf("%c", c);
        ::printf("\033[0m");
      } else {
        ::printf("%c", c);
      }
    }
    printf("\n");
  }
}

Node *Scene::GetNode(const Position &pos) {
  return this->GetNode(pos.x(), pos.y());
}

Node *Scene::GetNode(int32_t x, int32_t y) {
  return &this->nodes_[y * this->width_ + x];
}

void Scene::GetNeighborNodes(const Node *node, NodePtrVector &neighbor_nodes) {
  neighbor_nodes.clear();

  int start_x = std::max(node->pos_.x() - 1, 0);
  int end_x = std::min(node->pos_.x() + 1, this->width_ - 1);
  int start_y = std::max(node->pos_.y() - 1, 0);
  int end_y = std::min(node->pos_.y() + 1, this->height_ - 1);

  for (int y = start_y; y <= end_y; ++y) {
    for (int x = start_x; x <= end_x; ++x) {
      if (x != node->pos_.x() || y != node->pos_.y()) {
        Node *neighbor = this->GetNode(x, y);
        if (neighbor->walkable_) {
          neighbor_nodes.push_back(neighbor);
        }
      }
    }
  }
}

bool Scene::IsInOpenList(const Node *node) const {
  return node->open_list_pos_;
}

bool Scene::IsOpenListEmpty() const {
  return this->open_list_.size() <= 1;
}

void Scene::InsertOpenList(Node *node) {
  size_t cur_index = this->open_list_.size();
  this->open_list_.push_back(node);
  node->open_list_pos_ = cur_index;

  for (;;) {
    size_t parent_index = cur_index / 2;

    if (parent_index == 0) {
      break;
    }

    if (*this->open_list_[parent_index] < *this->open_list_[cur_index]) {
      break;
    }

    this->open_list_[parent_index]->open_list_pos_ = cur_index;
    this->open_list_[cur_index]->open_list_pos_ = parent_index;
    std::swap(this->open_list_[parent_index], this->open_list_[cur_index]);

    cur_index = parent_index;
  }
}

Node *Scene::GetMinFScoreNodeInOpenList() {
  if (this->IsOpenListEmpty() == true) {
    return NULL;
  }

  return this->open_list_[1];
}

void Scene::DeleteMinFScoreNodeInOpenList() {
  if (this->IsOpenListEmpty()) {
    return;
  }

  this->open_list_[1]->open_list_pos_ = 0;
  this->open_list_[1] = this->open_list_.back();
  this->open_list_.pop_back();

  size_t cur_index = 1;

  for (;;) {
    size_t child_index = cur_index * 2;

    if (child_index >= this->open_list_.size()) {
      break;
    }

    if (child_index + 1 < this->open_list_.size() &&
        *this->open_list_[child_index + 1] < *this->open_list_[child_index]) {
      ++child_index;
    }

    if (*this->open_list_[cur_index] < *this->open_list_[child_index]) {
      break;
    }

    this->open_list_[cur_index]->open_list_pos_ = child_index;
    this->open_list_[child_index]->open_list_pos_ = cur_index;
    std::swap(this->open_list_[cur_index], this->open_list_[child_index]);

    cur_index = child_index;
  }
}

void Scene::ResortOpenList(Node *node) {
  size_t cur_index = node->open_list_pos_;

  for (;;) {
    size_t parent_index = cur_index / 2;

    if (0 == parent_index) {
      break;
    }
    if (*this->open_list_[parent_index] < *this->open_list_[cur_index]) {
      break;
    }

    this->open_list_[parent_index]->open_list_pos_ = cur_index;
    this->open_list_[cur_index]->open_list_pos_ = parent_index;
    std::swap(this->open_list_[parent_index], this->open_list_[cur_index]);

    cur_index = parent_index;
  }
}

bool Scene::IsInCloseList(const Node *node) const {
  return node->close_list_pos_;
}

void Scene::InsertCloseList(Node *node) {
  node->close_list_pos_ = 1;
}

void Scene::ConstructResultPath(PositionVector &result) {
  for (Node *node = end_node_;
      node != this->start_node_ && node != NULL;
      node = node->parent_) {
    result.push_back(node->pos_);
  }
  std::reverse(result.begin(), result.end());
}

}  // namespace generate_path_config
}  // namespace tutorial

int main(int argc, char *argv[]) {
  using namespace ::tutorial::generate_path_config;

  CommandLineOption option;
  if (option.Parse(argc, argv) == false) {
    ERROR("Option::Parse failed(%s).", option.error_.data());
    return -1;
  }

  Config config;
  if (config.LoadConfig(option.conf_file_) == false) {
    ERROR("Config::LoadConfig(%s) failed.", option.conf_file_.data());
    return -1;
  }

  if (config.GeneratePathConfig(option.export_file_, option.min_path_length_) == false) {
    ERROR("Config::GeneratePathConfig(%s, %d) failed.",
        option.export_file_.data(), option.min_path_length_);
    return -1;
  }

  return 0;
}
