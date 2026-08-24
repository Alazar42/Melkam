#pragma once

#include "core/Memory.hpp"
#include "core/Node.hpp"
#include "core/Resource.hpp"
#include "helper/color/Color.hpp"
#include "helper/vectors/Vector2.hpp"

// 2D Scene Nodes
#include "nodes/2D/Camera2D.hpp"
#include "nodes/2D/Line2D.hpp"
#include "nodes/2D/Node2D.hpp"
#include "nodes/2D/PointLight2D.hpp"
#include "nodes/2D/Polygon2D.hpp"
#include "nodes/2D/Sprite2D.hpp"
#include "nodes/2D/CPUParticles2D.hpp"
#include "nodes/2D/physics/Area2D.hpp"
#include "nodes/2D/physics/CharacterBody2D.hpp"
#include "nodes/2D/physics/CollisionShape2D.hpp"
#include "nodes/2D/physics/StaticBody2D.hpp"
#include "nodes/2D/tilemap/TileMapLayer.hpp"

// Canvas UI Nodes
#include "nodes/UI/AspectRatioContainer.hpp"
#include "nodes/UI/BoxContainer.hpp"
#include "nodes/UI/Button.hpp"
#include "nodes/UI/CenterContainer.hpp"
#include "nodes/UI/CheckBox.hpp"
#include "nodes/UI/Control.hpp"
#include "nodes/UI/Dialogs.hpp"
#include "nodes/UI/FlowContainer.hpp"
#include "nodes/UI/GridContainer.hpp"
#include "nodes/UI/Label.hpp"
#include "nodes/UI/LineEdit.hpp"
#include "nodes/UI/LinkButton.hpp"
#include "nodes/UI/MenuButton.hpp"
#include "nodes/UI/NinePatchRect.hpp"
#include "nodes/UI/OptionButton.hpp"
#include "nodes/UI/Panel.hpp"
#include "nodes/UI/PanelContainer.hpp"
#include "nodes/UI/ProgressBar.hpp"
#include "nodes/UI/RichTextLabel.hpp"
#include "nodes/UI/ScrollBar.hpp"
#include "nodes/UI/ScrollContainer.hpp"
#include "nodes/UI/Separator.hpp"
#include "nodes/UI/Slider.hpp"
#include "nodes/UI/SpinBox.hpp"
#include "nodes/UI/SplitContainer.hpp"
#include "nodes/UI/TabContainer.hpp"
#include "nodes/UI/TextEdit.hpp"
#include "nodes/UI/TextureButton.hpp"
#include "nodes/UI/TextureProgressBar.hpp"
#include "nodes/UI/TextureRect.hpp"
#include "nodes/UI/UIWindow.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

// Serialized Node Descriptor
struct SceneNodeData {
  std::string name;
  std::string type;
  std::string parent; // "." for root, "NodeName" or "Parent/Child"
  std::map<std::string, std::string> properties;
};

// Serialized External Resource Descriptor
struct SceneExtResource {
  std::string type;
  std::string path;
  std::string id;
};

// Godot-style PackedScene (.tscn Text Format Serialization & Instantiator)
class PackedScene : public Resource {
public:
  using NodeFactoryFunc = std::function<Ref<Node>(const std::string &name)>;
  inline static std::map<std::string, NodeFactoryFunc> s_customNodeRegistry;

  static void registerCustomNode(const std::string &key, NodeFactoryFunc factory) {
    s_customNodeRegistry[key] = factory;
  }

  PackedScene() : Resource() {}
  explicit PackedScene(std::string path) : Resource(std::move(path)) {}

  // Instantiates the serialized node hierarchy into a live Node instance
  Ref<Node> instantiate() const {
    if (m_nodes.empty()) return nullptr;

    std::map<std::string, Ref<Node>> createdNodes;
    Ref<Node> rootNode = nullptr;

    for (const auto &nodeData : m_nodes) {
      Ref<Node> node = nullptr;

      // 1. Check if node has an attached script (e.g. script = "res://scripts/Player.hpp")
      auto scriptIt = nodeData.properties.find("script");
      if (scriptIt != nodeData.properties.end()) {
        std::string script = cleanString(scriptIt->second);
        auto regIt = s_customNodeRegistry.find(script);
        if (regIt == s_customNodeRegistry.end()) {
          std::string base = std::filesystem::path(script).filename().string();
          regIt = s_customNodeRegistry.find(base);
          if (regIt == s_customNodeRegistry.end()) {
            std::string stem = std::filesystem::path(script).stem().string();
            regIt = s_customNodeRegistry.find(stem);
          }
        }
        if (regIt != s_customNodeRegistry.end()) {
          node = regIt->second(nodeData.name);
        }
      }

      // 2. Check custom type registry by node type name
      if (!node) {
        auto regIt = s_customNodeRegistry.find(nodeData.type);
        if (regIt != s_customNodeRegistry.end()) {
          node = regIt->second(nodeData.name);
        }
      }

      // 3. Fallback to built-in reflection factory
      if (!node) {
        node = createNodeByType(nodeData.type, nodeData.name);
      }
      if (!node) {
        node = makeRef<Node>(nodeData.name);
      }

      applyProperties(node.get(), nodeData.properties);

      if (nodeData.parent.empty() || rootNode == nullptr) {
        // Root node
        rootNode = node;
        createdNodes["."] = rootNode;
        createdNodes[nodeData.name] = rootNode;
      } else {
        // Find parent
        Ref<Node> parentNode = nullptr;
        if (nodeData.parent == ".") {
          parentNode = rootNode;
        } else {
          auto it = createdNodes.find(nodeData.parent);
          if (it != createdNodes.end()) {
            parentNode = it->second;
          } else {
            parentNode = rootNode;
          }
        }

        if (parentNode) {
          parentNode->addChild(node);
        }

        std::string fullPath = (nodeData.parent == ".") ? nodeData.name : (nodeData.parent + "/" + nodeData.name);
        createdNodes[fullPath] = node;
        createdNodes[nodeData.name] = node;
      }
    }

    return rootNode;
  }

  // Packs a live runtime Node hierarchy into this PackedScene resource
  bool pack(Node *root) {
    if (!root) return false;
    m_nodes.clear();
    m_extResources.clear();

    serializeNodeRecursive(root, "");
    return true;
  }

  // Loads and parses a .tscn file
  bool load(const std::string &path) override {
    std::string resolved = ResourceLoader::resolvePath(path);
    std::ifstream file(resolved);
    if (!file.is_open()) {
      return false;
    }

    std::stringstream ss;
    ss << file.rdbuf();
    return parseTSCN(ss.str());
  }

  // Saves this PackedScene to a .tscn file
  bool save(const std::string &path) const override {
    std::ofstream file(path);
    if (!file.is_open()) return false;
    file << encodeTSCN();
    return true;
  }

  // Encodes to Godot 4 .tscn text format
  std::string encodeTSCN() const {
    std::stringstream ss;
    ss << "[gd_scene format=3]\n\n";

    for (const auto &ext : m_extResources) {
      ss << "[ext_resource type=\"" << ext.type << "\" path=\"" << ext.path
         << "\" id=\"" << ext.id << "\"]\n";
    }
    if (!m_extResources.empty()) ss << "\n";

    for (const auto &node : m_nodes) {
      ss << "[node name=\"" << node.name << "\" type=\"" << node.type << "\"";
      if (!node.parent.empty()) {
        ss << " parent=\"" << node.parent << "\"";
      }
      ss << "]\n";

      for (const auto &[k, v] : node.properties) {
        ss << k << " = " << v << "\n";
      }
      ss << "\n";
    }

    return ss.str();
  }

  // Parses Godot 4 .tscn text format
  bool parseTSCN(const std::string &content) {
    m_nodes.clear();
    m_extResources.clear();

    std::stringstream ss(content);
    std::string line;
    SceneNodeData *currentNode = nullptr;

    while (std::getline(ss, line)) {
      line = trim(line);
      if (line.empty() || line[0] == ';') continue;

      if (line.rfind("[ext_resource", 0) == 0) {
        SceneExtResource ext;
        ext.type = extractAttribute(line, "type");
        ext.path = extractAttribute(line, "path");
        ext.id = extractAttribute(line, "id");
        m_extResources.push_back(ext);
        currentNode = nullptr;
        continue;
      }

      if (line.rfind("[node", 0) == 0) {
        SceneNodeData node;
        node.name = extractAttribute(line, "name");
        node.type = extractAttribute(line, "type");
        node.parent = extractAttribute(line, "parent");
        m_nodes.push_back(node);
        currentNode = &m_nodes.back();
        continue;
      }

      if (currentNode) {
        size_t eqPos = line.find('=');
        if (eqPos != std::string::npos) {
          std::string key = trim(line.substr(0, eqPos));
          std::string val = trim(line.substr(eqPos + 1));
          currentNode->properties[key] = val;
        }
      }
    }

    return !m_nodes.empty();
  }

  const std::vector<SceneNodeData> &getNodes() const { return m_nodes; }
  const std::vector<SceneExtResource> &getExtResources() const { return m_extResources; }

private:
  static std::string trim(const std::string &s) {
    auto wsfront = std::find_if_not(s.begin(), s.end(), [](int c) { return std::isspace(c); });
    auto wsback = std::find_if_not(s.rbegin(), s.rend(), [](int c) { return std::isspace(c); }).base();
    return (wsback <= wsfront ? std::string() : std::string(wsfront, wsback));
  }

  static std::string extractAttribute(const std::string &line, const std::string &attr) {
    std::string key = attr + "=\"";
    size_t start = line.find(key);
    if (start == std::string::npos) return "";
    start += key.length();
    size_t end = line.find('"', start);
    if (end == std::string::npos) return "";
    return line.substr(start, end - start);
  }

  // Reflection Factory for Instantiating Nodes by Type Name
  static Ref<Node> createNodeByType(const std::string &type, const std::string &name) {
    // 2D Spatial Nodes
    if (type == "Node2D") return makeRef<Node2D>(name);
    if (type == "Sprite2D") return makeRef<Sprite2D>();
    if (type == "CharacterBody2D") return makeRef<CharacterBody2D>(name);
    if (type == "StaticBody2D") return makeRef<StaticBody2D>(name);
    if (type == "Area2D") return makeRef<Area2D>(name);
    if (type == "CollisionShape2D") return makeRef<CollisionShape2D>(Vector2(32.0f, 32.0f));
    if (type == "Camera2D") return makeRef<Camera2D>(name);
    if (type == "PointLight2D") return makeRef<PointLight2D>(120.0f);
    if (type == "CPUParticles2D") return makeRef<CPUParticles2D>(32);
    if (type == "Line2D") return makeRef<Line2D>();
    if (type == "Polygon2D") return makeRef<Polygon2D>();
    if (type == "TileMapLayer") return makeRef<TileMapLayer>();

    // Canvas UI Nodes
    if (type == "Control") return makeRef<Control>(name);
    if (type == "Button") return makeRef<Button>("Button");
    if (type == "Label") return makeRef<Label>("Label");
    if (type == "RichTextLabel") return makeRef<RichTextLabel>();
    if (type == "LineEdit") return makeRef<LineEdit>();
    if (type == "TextEdit") return makeRef<TextEdit>();
    if (type == "ProgressBar") return makeRef<ProgressBar>();
    if (type == "HSlider") return makeRef<HSlider>();
    if (type == "VSlider") return makeRef<VSlider>();
    if (type == "SpinBox") return makeRef<SpinBox>();
    if (type == "OptionButton") return makeRef<OptionButton>();
    if (type == "CheckButton") return makeRef<CheckButton>("CheckButton");
    if (type == "CheckBox") return makeRef<CheckBox>("CheckBox");
    if (type == "LinkButton") return makeRef<LinkButton>("LinkButton");
    if (type == "ColorRect") return makeRef<ColorRect>();
    if (type == "Panel") return makeRef<Panel>();
    if (type == "PanelContainer") return makeRef<PanelContainer>();
    if (type == "TextureRect") return makeRef<TextureRect>();
    if (type == "TabContainer") return makeRef<TabContainer>();
    if (type == "BoxContainer") return makeRef<BoxContainer>();
    if (type == "HBoxContainer") return makeRef<HBoxContainer>();
    if (type == "VBoxContainer") return makeRef<VBoxContainer>();
    if (type == "GridContainer") return makeRef<GridContainer>();
    if (type == "CenterContainer") return makeRef<CenterContainer>();
    if (type == "HFlowContainer") return makeRef<HFlowContainer>();
    if (type == "VFlowContainer") return makeRef<VFlowContainer>();
    if (type == "HSplitContainer") return makeRef<HSplitContainer>();
    if (type == "VSplitContainer") return makeRef<VSplitContainer>();
    if (type == "ScrollContainer") return makeRef<ScrollContainer>();
    if (type == "HSeparator") return makeRef<HSeparator>();
    if (type == "VSeparator") return makeRef<VSeparator>();
    if (type == "UIWindow") return makeRef<UIWindow>(name);
    if (type == "AcceptDialog") return makeRef<AcceptDialog>("Alert");
    if (type == "ConfirmationDialog") return makeRef<ConfirmationDialog>("Confirm");

    return makeRef<Node>(name);
  }

  static void applyProperties(Node *node, const std::map<std::string, std::string> &props) {
    if (!node) return;

    auto *n2d = dynamic_cast<Node2D *>(node);
    auto *ctrl = dynamic_cast<Control *>(node);

    for (const auto &[k, val] : props) {
      if (k == "visible") {
        node->visible = (val == "true");
      }

      // Node2D properties
      if (n2d) {
        if (k == "position") n2d->setPosition(parseVector2(val));
        else if (k == "rotation") n2d->setRotation(std::stof(val));
        else if (k == "scale") n2d->setScale(parseVector2(val));
      }

      // Control properties
      if (ctrl) {
        if (k == "position") ctrl->setPosition(parseVector2(val));
        else if (k == "size") ctrl->setSize(parseVector2(val));
        else if (k == "custom_minimum_size") ctrl->customMinimumSize = parseVector2(val);
        else if (k == "modulate") ctrl->modulate = parseColor(val);
      }

      // Sprite2D properties
      auto *sprite = dynamic_cast<Sprite2D *>(node);
      if (sprite) {
        if (k == "size") sprite->size = parseVector2(val);
        else if (k == "centered") sprite->centered = (val == "true");
        else if (k == "flip_h") sprite->flipH = (val == "true");
        else if (k == "flip_v") sprite->flipV = (val == "true");
        else if (k == "texture") {
          std::string clean = cleanString(val);
          if (!clean.empty()) sprite->texture = ResourceLoader::load<Texture2D>(clean);
        }
      }

      // TextureRect properties
      auto *texRect = dynamic_cast<TextureRect *>(node);
      if (texRect) {
        if (k == "texture") {
          std::string clean = cleanString(val);
          if (!clean.empty()) texRect->texture = ResourceLoader::load<Texture2D>(clean);
        }
      }

      // ColorRect properties
      auto *colorRect = dynamic_cast<ColorRect *>(node);
      if (colorRect) {
        if (k == "color") colorRect->color = parseColor(val);
      }

      // Panel properties
      auto *panel = dynamic_cast<Panel *>(node);
      if (panel) {
        if (k == "backgroundColor") panel->backgroundColor = parseColor(val);
        else if (k == "borderColor") panel->borderColor = parseColor(val);
        else if (k == "borderWidth") panel->borderWidth = std::stof(val);
        else if (k == "cornerRadius") panel->cornerRadius = std::stof(val);
      }

      // Button properties
      auto *btn = dynamic_cast<Button *>(node);
      if (btn) {
        if (k == "text") btn->text = cleanString(val);
        else if (k == "fontSize" || k == "font_size") btn->fontSize = std::stof(val);
      }

      // Label properties
      auto *lbl = dynamic_cast<Label *>(node);
      if (lbl) {
        if (k == "text") lbl->text = cleanString(val);
        else if (k == "fontSize" || k == "font_size") lbl->fontSize = std::stof(val);
        else if (k == "fontColor" || k == "font_color") lbl->fontColor = parseColor(val);
      }

      // ProgressBar properties
      auto *pbar = dynamic_cast<ProgressBar *>(node);
      if (pbar) {
        if (k == "value") pbar->setValue(std::stof(val));
        else if (k == "maxValue" || k == "max_value") pbar->setMaxValue(std::stof(val));
      }

      // Slider properties
      auto *hslider = dynamic_cast<HSlider *>(node);
      if (hslider) {
        if (k == "value") hslider->setValue(std::stof(val));
        else if (k == "minValue" || k == "min_value") hslider->setMinValue(std::stof(val));
        else if (k == "maxValue" || k == "max_value") hslider->setMaxValue(std::stof(val));
      }

      // Light properties
      auto *light = dynamic_cast<PointLight2D *>(node);
      if (light) {
        if (k == "radius") light->radius = std::stof(val);
        else if (k == "energy") light->energy = std::stof(val);
        else if (k == "color") light->color = parseColor(val);
      }

      // Camera properties
      auto *cam = dynamic_cast<Camera2D *>(node);
      if (cam) {
        if (k == "zoom") cam->setZoom(std::stof(val));
      }
    }
  }

  static std::string cleanString(const std::string &val) {
    std::string s = val;
    if (s.length() >= 2 && s.front() == '"' && s.back() == '"') {
      return s.substr(1, s.length() - 2);
    }
    return s;
  }

  void serializeNodeRecursive(Node *node, const std::string &parentPath) {
    if (!node) return;

    SceneNodeData data;
    data.name = node->name;
    data.type = getNodeTypeName(node);
    data.parent = parentPath;

    // Record properties
    if (!node->visible) data.properties["visible"] = "false";

    auto *n2d = dynamic_cast<Node2D *>(node);
    if (n2d) {
      if (n2d->getPosition() != Vector2(0.0f, 0.0f)) {
        data.properties["position"] = "Vector2(" + std::to_string(n2d->getPosition().x) + ", " + std::to_string(n2d->getPosition().y) + ")";
      }
      if (n2d->getRotation() != 0.0f) {
        data.properties["rotation"] = std::to_string(n2d->getRotation());
      }
      if (n2d->getScale() != Vector2(1.0f, 1.0f)) {
        data.properties["scale"] = "Vector2(" + std::to_string(n2d->getScale().x) + ", " + std::to_string(n2d->getScale().y) + ")";
      }
    }

    auto *ctrl = dynamic_cast<Control *>(node);
    if (ctrl) {
      if (ctrl->getPosition() != Vector2(0.0f, 0.0f)) {
        data.properties["position"] = "Vector2(" + std::to_string(ctrl->getPosition().x) + ", " + std::to_string(ctrl->getPosition().y) + ")";
      }
      if (ctrl->getSize() != Vector2(0.0f, 0.0f)) {
        data.properties["size"] = "Vector2(" + std::to_string(ctrl->getSize().x) + ", " + std::to_string(ctrl->getSize().y) + ")";
      }
    }

    m_nodes.push_back(data);

    std::string currentPath = parentPath.empty() ? "." : (parentPath == "." ? node->name : (parentPath + "/" + node->name));
    for (const auto &child : node->getChildren()) {
      serializeNodeRecursive(child.get(), currentPath);
    }
  }

  static std::string getNodeTypeName(Node *node) {
    if (dynamic_cast<CharacterBody2D *>(node)) return "CharacterBody2D";
    if (dynamic_cast<StaticBody2D *>(node)) return "StaticBody2D";
    if (dynamic_cast<Area2D *>(node)) return "Area2D";
    if (dynamic_cast<CollisionShape2D *>(node)) return "CollisionShape2D";
    if (dynamic_cast<Camera2D *>(node)) return "Camera2D";
    if (dynamic_cast<Sprite2D *>(node)) return "Sprite2D";
    if (dynamic_cast<PointLight2D *>(node)) return "PointLight2D";
    if (dynamic_cast<CPUParticles2D *>(node)) return "CPUParticles2D";
    if (dynamic_cast<Line2D *>(node)) return "Line2D";
    if (dynamic_cast<Polygon2D *>(node)) return "Polygon2D";
    if (dynamic_cast<TileMapLayer *>(node)) return "TileMapLayer";
    if (dynamic_cast<Button *>(node)) return "Button";

    if (dynamic_cast<Label *>(node)) return "Label";
    if (dynamic_cast<RichTextLabel *>(node)) return "RichTextLabel";
    if (dynamic_cast<LineEdit *>(node)) return "LineEdit";
    if (dynamic_cast<TextEdit *>(node)) return "TextEdit";
    if (dynamic_cast<ProgressBar *>(node)) return "ProgressBar";
    if (dynamic_cast<HSlider *>(node)) return "HSlider";
    if (dynamic_cast<SpinBox *>(node)) return "SpinBox";
    if (dynamic_cast<OptionButton *>(node)) return "OptionButton";
    if (dynamic_cast<TabContainer *>(node)) return "TabContainer";
    if (dynamic_cast<ColorRect *>(node)) return "ColorRect";
    if (dynamic_cast<TextureRect *>(node)) return "TextureRect";
    if (dynamic_cast<Panel *>(node)) return "Panel";
    if (dynamic_cast<Node2D *>(node)) return "Node2D";
    if (dynamic_cast<Control *>(node)) return "Control";
    return "Node";
  }

  static Vector2 parseVector2(const std::string &val) {
    if (val.rfind("Vector2(", 0) == 0 && val.back() == ')') {
      std::string inner = val.substr(8, val.length() - 9);
      size_t comma = inner.find(',');
      if (comma != std::string::npos) {
        float x = std::stof(trim(inner.substr(0, comma)));
        float y = std::stof(trim(inner.substr(comma + 1)));
        return Vector2(x, y);
      }
    }
    return Vector2(0.0f, 0.0f);
  }

  static Color parseColor(const std::string &val) {
    if (val.rfind("Color(", 0) == 0 && val.back() == ')') {
      std::string inner = val.substr(6, val.length() - 7);
      std::stringstream ss(inner);
      std::string part;
      std::vector<float> parts;
      while (std::getline(ss, part, ',')) {
        parts.push_back(std::stof(trim(part)));
      }
      if (parts.size() >= 3) {
        float a = parts.size() >= 4 ? parts[3] : 1.0f;
        return Color(parts[0], parts[1], parts[2], a);
      }
    }
    return Color::WHITE;
  }

  std::vector<SceneNodeData> m_nodes;
  std::vector<SceneExtResource> m_extResources;
};
