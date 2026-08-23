#pragma once

// =============================================================================
// MelkamEngine Master Header
// Godot-inspired modular C++ Game Engine with ECS Backend & Box2D Physics.
// =============================================================================

// MSL - Melkam Standard Library (Color, Vector2, Vector3, Rect2, String, Array)
#include "helper/msl.hpp"
#include "helper/Rect2.hpp"

// Input, Time & Core Node
#include "input.hpp"
#include "time.hpp"
#include "core/Node.hpp"
#include "core/Signal.hpp"
#include "window.hpp"
#include "audio/Audio.hpp"
#include "ECS.hpp"

// 2D Physics & Box2D Server
#include "physics/2D/Collision2D.hpp"
#include "physics/2D/PhysicsServer2D.hpp"

// Scene Management & Runtime Application
#include "core/SceneTree.hpp"
#include "core/Application.hpp"

// 2D Spatial Nodes & Visuals
#include "nodes/2D/Camera2D.hpp"
#include "nodes/2D/Node2D.hpp"
#include "nodes/2D/MeshInstance2D.hpp"
#include "nodes/2D/Shape2D.hpp"
#include "nodes/2D/Sprite2D.hpp"
#include "nodes/2D/SpriteFrames.hpp"
#include "nodes/2D/AnimatedSprite2D.hpp"
#include "nodes/2D/Transform2D.hpp"

// Audio Nodes (Godot-Inspired)
#include "nodes/audio/AudioStreamPlayer.hpp"
#include "nodes/audio/AudioStreamPlayer2D.hpp"

// 2D Physics Nodes (Godot-Inspired)
#include "nodes/2D/physics/CollisionShape2D.hpp"
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
#include "nodes/UI/Control.hpp"
#include "nodes/UI/Range.hpp"
#include "nodes/UI/Label.hpp"
#include "nodes/UI/CheckBox.hpp"
#include "nodes/UI/Button.hpp"
#include "nodes/UI/TextureButton.hpp"
#include "nodes/UI/OptionButton.hpp"
#include "nodes/UI/LineEdit.hpp"
#include "nodes/UI/ProgressBar.hpp"
#include "nodes/UI/Slider.hpp"
#include "nodes/UI/Panel.hpp"
#include "nodes/UI/TextureRect.hpp"
#include "nodes/UI/NinePatchRect.hpp"
#include "nodes/UI/ReferenceRect.hpp"
#include "nodes/UI/BoxContainer.hpp"
#include "nodes/UI/GridContainer.hpp"
#include "nodes/UI/CenterContainer.hpp"
#include "nodes/UI/ScrollContainer.hpp"
#include "nodes/UI/CanvasLayer.hpp"

// ECS 2D Components & Simulation Systems
#include "components/Components2D.hpp"
#include "systems/Systems2D.hpp"
