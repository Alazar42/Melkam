#pragma once

// =============================================================================
// MelkamEngine Master Header
// Godot-inspired modular C++ Game Engine with ECS Backend & Box2D Physics.
// =============================================================================

// MSL - Melkam Standard Library (Color, Vector2, Vector3, String, Array)
#include "helper/msl.hpp"

// Input, Time & Core Node
#include "input.hpp"
#include "time.hpp"
#include "core/Node.hpp"
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
#include "nodes/2D/Transform2D.hpp"

// 2D Physics Nodes (Godot-Inspired)
#include "nodes/2D/physics/CollisionShape2D.hpp"
#include "nodes/2D/physics/CollisionObject2D.hpp"
#include "nodes/2D/physics/StaticBody2D.hpp"
#include "nodes/2D/physics/CharacterBody2D.hpp"
#include "nodes/2D/physics/Area2D.hpp"
#include "nodes/2D/physics/RigidBody2D.hpp"

// 2D Graphics & Textures
#include "renderers/Renderer2D.hpp"
#include "renderers/Texture2D.hpp"

// ECS 2D Components & Simulation Systems
#include "components/Components2D.hpp"
#include "systems/Systems2D.hpp"
