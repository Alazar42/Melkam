#pragma once

// =============================================================================
// MelkamEngine Master Header
// Godot-inspired modular C++ Game Engine with ECS Backend & Box2D Physics.
// =============================================================================

// MSL - Melkam Standard Library (Color, Vector2, Vector3, Rect2, String, Array)
#include "helper/msl.hpp"
#include "helper/Rect2.hpp"

// Memory & Smart Pointer Aliases (Ref<T>, Shared<T>, Scope<T>, Unique<T>, Weak<T>)
#include "core/Memory.hpp"

// Input, Time & Core Node
#include "input.hpp"
#include "time.hpp"
#include "core/EventTracer.hpp"
#include "core/Node.hpp"
#include "core/Signal.hpp"
#include "core/Task.hpp"
#include "window.hpp"
#include "audio/Audio.hpp"
#include "ECS.hpp"

// 2D Physics & Box2D Server
#include "physics/2D/Collision2D.hpp"
#include "physics/2D/PhysicsServer2D.hpp"

// Scene Management
#include "core/SceneTree.hpp"

// Resource & Serialization System (Godot-Inspired Save/Load, .tscn, .tres, JSON, ConfigFile)
#include "core/Resource.hpp"
#include "core/ConfigFile.hpp"
#include "core/JSON.hpp"
#include "core/ProjectSettings.hpp"
#include "scene/PackedScene.hpp"


// 2D Spatial Nodes & Visuals
#include "nodes/2D/Camera2D.hpp"
#include "nodes/2D/Node2D.hpp"
#include "nodes/2D/MeshInstance2D.hpp"
#include "nodes/2D/Shape2D.hpp"
#include "nodes/2D/Sprite2D.hpp"
#include "nodes/2D/SpriteFrames.hpp"
#include "nodes/2D/AnimatedSprite2D.hpp"
#include "nodes/2D/Transform2D.hpp"
#include "nodes/2D/Line2D.hpp"
#include "nodes/2D/Polygon2D.hpp"
#include "nodes/2D/CPUParticles2D.hpp"
#include "nodes/2D/Parallax2D.hpp"
#include "nodes/2D/Marker2D.hpp"
#include "nodes/2D/RemoteTransform2D.hpp"
#include "nodes/2D/VisibleOnScreenNotifier2D.hpp"

// 2D Paths & Splines
#include "nodes/2D/Curve2D.hpp"
#include "nodes/2D/Path2D.hpp"
#include "nodes/2D/PathFollow2D.hpp"

// 2D Lighting & Shadows
#include "nodes/2D/CanvasModulate.hpp"
#include "nodes/2D/Light2D.hpp"
#include "nodes/2D/PointLight2D.hpp"
#include "nodes/2D/DirectionalLight2D.hpp"
#include "nodes/2D/OccluderPolygon2D.hpp"
#include "nodes/2D/LightOccluder2D.hpp"

// Audio Nodes (Godot-Inspired)
#include "nodes/audio/AudioStreamPlayer.hpp"
#include "nodes/audio/AudioStreamPlayer2D.hpp"

// 2D Physics Nodes (Godot-Inspired)
#include "nodes/2D/RayCast2D.hpp"
#include "nodes/2D/physics/CollisionShape2D.hpp"
#include "nodes/2D/physics/CollisionPolygon2D.hpp"
#include "nodes/2D/physics/CollisionObject2D.hpp"
#include "nodes/2D/physics/StaticBody2D.hpp"
#include "nodes/2D/physics/CharacterBody2D.hpp"
#include "nodes/2D/physics/Area2D.hpp"
#include "nodes/2D/physics/RigidBody2D.hpp"


// 2D Graphics, Textures & Fonts
#include "renderers/Renderer2D.hpp"
#include "renderers/Texture2D.hpp"
#include "renderers/Font.hpp"

// UI & Control Nodes (Godot-Inspired Canvas UI Layer)
#include "nodes/UI/Theme.hpp"
#include "nodes/UI/StyleBox.hpp"
#include "nodes/UI/Control.hpp"
#include "nodes/UI/Range.hpp"
#include "nodes/UI/Icons.hpp"

#include "nodes/UI/Label.hpp"
#include "nodes/UI/RichTextLabel.hpp"
#include "nodes/UI/CheckBox.hpp"
#include "nodes/UI/Button.hpp"
#include "nodes/UI/LinkButton.hpp"
#include "nodes/UI/MenuButton.hpp"
#include "nodes/UI/TextureButton.hpp"
#include "nodes/UI/OptionButton.hpp"
#include "nodes/UI/PopupMenu.hpp"
#include "nodes/UI/MenuBar.hpp"
#include "nodes/UI/LineEdit.hpp"
#include "nodes/UI/TextEdit.hpp"
#include "nodes/UI/ProgressBar.hpp"
#include "nodes/UI/TextureProgressBar.hpp"
#include "nodes/UI/Slider.hpp"
#include "nodes/UI/ScrollBar.hpp"
#include "nodes/UI/SpinBox.hpp"
#include "nodes/UI/Panel.hpp"
#include "nodes/UI/TextureRect.hpp"
#include "nodes/UI/NinePatchRect.hpp"
#include "nodes/UI/ReferenceRect.hpp"
#include "nodes/UI/Separator.hpp"
#include "nodes/UI/BoxContainer.hpp"
#include "nodes/UI/GridContainer.hpp"
#include "nodes/UI/CenterContainer.hpp"
#include "nodes/UI/PanelContainer.hpp"
#include "nodes/UI/AspectRatioContainer.hpp"
#include "nodes/UI/SplitContainer.hpp"
#include "nodes/UI/TabContainer.hpp"
#include "nodes/UI/FlowContainer.hpp"
#include "nodes/UI/ScrollContainer.hpp"
#include "nodes/UI/UIWindow.hpp"
#include "nodes/UI/Dialogs.hpp"
#include "nodes/UI/CanvasLayer.hpp"



// Animation & Tweening System (Godot-Inspired)
#include "animation/Tween.hpp"
#include "animation/Animation.hpp"
#include "animation/AnimationPlayer.hpp"

// 2D TileMap & TileSet System (Godot-Inspired)
#include "nodes/2D/tilemap/TileSet.hpp"
#include "nodes/2D/tilemap/TileMapLayer.hpp"

// ECS 2D Components & Simulation Systems
#include "components/Components2D.hpp"
#include "systems/Systems2D.hpp"

// 3D Math & Spatial Foundation (Godot-Inspired)
#include "nodes/3D/Quaternion.hpp"
#include "nodes/3D/Basis.hpp"
#include "nodes/3D/Transform3D.hpp"
#include "nodes/3D/AABB.hpp"
#include "nodes/3D/Plane.hpp"
#include "nodes/3D/Frustum.hpp"

// 3D Materials & Meshes
#include "nodes/3D/StandardMaterial3D.hpp"
#include "nodes/3D/meshes/Mesh.hpp"
#include "nodes/3D/meshes/PrimitiveMesh.hpp"

// 3D Visual & Geometry Hierarchy (Godot-Inspired)
#include "nodes/3D/Node3D.hpp"
#include "nodes/3D/VisualInstance3D.hpp"
#include "nodes/3D/GeometryInstance3D.hpp"
#include "nodes/3D/MeshInstance3D.hpp"
#include "nodes/3D/Label3D.hpp"

// 3D Cameras & Lights
#include "nodes/3D/Camera3D.hpp"
#include "nodes/3D/CameraController3D.hpp"
#include "nodes/3D/Light3D.hpp"
#include "nodes/3D/DirectionalLight3D.hpp"
#include "nodes/3D/OmniLight3D.hpp"
#include "nodes/3D/SpotLight3D.hpp"
#include "nodes/3D/WorldEnvironment.hpp"
#include "nodes/3D/Marker3D.hpp"
#include "nodes/3D/SpringArm3D.hpp"

// 3D Bullet Physics & Collision Bodies
#include "nodes/3D/physics/Shape3D.hpp"
#include "nodes/3D/physics/BoxShape3D.hpp"
#include "nodes/3D/physics/SphereShape3D.hpp"
#include "nodes/3D/physics/CapsuleShape3D.hpp"
#include "nodes/3D/physics/CylinderShape3D.hpp"
#include "nodes/3D/physics/PhysicsServer3D.hpp"
#include "nodes/3D/physics/CollisionShape3D.hpp"
#include "nodes/3D/physics/CollisionObject3D.hpp"
#include "nodes/3D/physics/StaticBody3D.hpp"
#include "nodes/3D/physics/RigidBody3D.hpp"
#include "nodes/3D/physics/CharacterBody3D.hpp"
#include "nodes/3D/physics/Area3D.hpp"
#include "nodes/3D/RayCast3D.hpp"

// 3D Hardware Vulkan Renderer & ECS Systems
#include "components/Components3D.hpp"
#include "systems/Systems3D.hpp"
#include "renderers/vulkan/VulkanContext.hpp"
#include "renderers/vulkan/RenderingDevice3D.hpp"
#include "renderers/Renderer3D.hpp"

// Master Application / Game Engine Runtime Loop
#include "core/Application.hpp"

