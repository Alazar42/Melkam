#pragma once

#include <entt/entt.hpp>
#include <utility>

// Lightweight handle representing an entity in the ECS world.
// Wraps an entt::entity identifier and provides a complete suite of
// helpers for component management, queries, views, signals, and context.
class Entity {
public:
  // Constructs a null / uninitialized Entity handle.
  Entity() = default;

  // Constructs an Entity wrapper around an existing EnTT entity identifier.
  Entity(entt::entity handle) : m_handle(handle) {}

  // ==========================================
  // Lifecycle Management
  // ==========================================

  // Creates a new entity in the shared ECS registry.
  static Entity create() { return Entity(s_registry.create()); }

  // Destroys this entity and all its components from the registry.
  void destroy() {
    if (m_handle != entt::null && s_registry.valid(m_handle)) {
      s_registry.destroy(m_handle);
      m_handle = entt::null;
    }
  }

  // Checks if this entity handle is valid and exists in the registry.
  bool isValid() const {
    return m_handle != entt::null && s_registry.valid(m_handle);
  }

  // Returns the version number encoded in this entity identifier.
  auto version() const { return entt::to_version(m_handle); }

  // Returns the integral entity ID without version bits.
  auto id() const { return entt::to_entity(m_handle); }

  // Checks if this entity has no components assigned to it.
  bool orphan() const { return s_registry.orphan(m_handle); }

  // ==========================================
  // Component Addition / Emplacement / Patching
  // ==========================================

  // Constructs and assigns a component of type T to this entity in-place.
  // Asserts if the component already exists on this entity.
  template <typename T, typename... Args>
  decltype(auto) addComponent(Args &&...args) {
    return s_registry.emplace<T>(m_handle, std::forward<Args>(args)...);
  }

  // Constructs and assigns a component of type T to this entity in-place (alias for addComponent).
  template <typename T, typename... Args>
  decltype(auto) emplace(Args &&...args) {
    return s_registry.emplace<T>(m_handle, std::forward<Args>(args)...);
  }

  // Assigns or replaces a component of type T on this entity.
  // If T doesn't exist, it is created; if T already exists, it is replaced.
  template <typename T, typename... Args>
  decltype(auto) emplaceOrReplace(Args &&...args) {
    return s_registry.emplace_or_replace<T>(m_handle,
                                           std::forward<Args>(args)...);
  }

  // Replaces an existing component of type T on this entity.
  // Assumes the component already exists.
  template <typename T, typename... Args>
  decltype(auto) replace(Args &&...args) {
    return s_registry.replace<T>(m_handle, std::forward<Args>(args)...);
  }

  // In-place modifies an existing component via a lambda and triggers update signals.
  // Example:
  //   entity.patch<Position>([](auto &pos) { pos.x += 10.0f; });
  template <typename T, typename Func>
  decltype(auto) patch(Func &&func) {
    return s_registry.patch<T>(m_handle, std::forward<Func>(func));
  }

  // Gets an existing component of type T, or constructs it in-place if not present.
  template <typename T, typename... Args>
  decltype(auto) getOrEmplace(Args &&...args) {
    return s_registry.get_or_emplace<T>(m_handle,
                                        std::forward<Args>(args)...);
  }

  // Gets an existing component of type T, or adds it in-place if not present (alias for getOrEmplace).
  template <typename T, typename... Args>
  decltype(auto) getOrAddComponent(Args &&...args) {
    return getOrEmplace<T>(std::forward<Args>(args)...);
  }

  // ==========================================
  // Component Retrieval (get / try_get)
  // ==========================================

  // Retrieves reference(s) to one or more components owned by this entity.
  // - Single component: returns Component&
  // - Multiple components: returns std::tuple<Components&...> (supports structured bindings)
  // Example:
  //   auto &pos = entity.get<Position>();
  //   auto [pos, vel] = entity.get<Position, Velocity>();
  template <typename... Components> decltype(auto) getComponent() {
    return s_registry.get<Components...>(m_handle);
  }

  // Retrieves reference(s) to one or more components owned by this entity (alias for getComponent).
  template <typename... Components> decltype(auto) get() {
    return s_registry.get<Components...>(m_handle);
  }

  // Const version: retrieves const reference(s) to component(s).
  template <typename... Components> decltype(auto) getComponent() const {
    return s_registry.get<Components...>(m_handle);
  }

  // Const version: retrieves const reference(s) to component(s) (alias for getComponent).
  template <typename... Components> decltype(auto) get() const {
    return s_registry.get<Components...>(m_handle);
  }

  // Safely retrieves pointer(s) to component(s), returning nullptr if absent.
  // - Single component: returns Component* (or nullptr)
  // - Multiple components: returns std::tuple<Components*...>
  // Example:
  //   if (auto *pos = entity.tryGet<Position>()) { pos->x += 1.0f; }
  template <typename... Components> decltype(auto) tryGetComponent() {
    return s_registry.try_get<Components...>(m_handle);
  }

  // Safely retrieves pointer(s) to component(s) (alias for tryGetComponent).
  template <typename... Components> decltype(auto) tryGet() {
    return s_registry.try_get<Components...>(m_handle);
  }

  // Const version: safely retrieves const pointer(s) to component(s).
  template <typename... Components> decltype(auto) tryGetComponent() const {
    return s_registry.try_get<Components...>(m_handle);
  }

  // Const version: safely retrieves const pointer(s) to component(s) (alias for tryGetComponent).
  template <typename... Components> decltype(auto) tryGet() const {
    return s_registry.try_get<Components...>(m_handle);
  }

  // ==========================================
  // Component Checks (all_of / any_of)
  // ==========================================

  // Returns true if this entity has ALL given components.
  template <typename... Components> bool hasComponent() const {
    return s_registry.all_of<Components...>(m_handle);
  }

  // Returns true if this entity has ALL given components (alias for hasComponent).
  template <typename... Components> bool has() const {
    return s_registry.all_of<Components...>(m_handle);
  }

  // Returns true if this entity has ALL given components (alias for hasComponent).
  template <typename... Components> bool hasAll() const {
    return s_registry.all_of<Components...>(m_handle);
  }

  // Returns true if this entity has AT LEAST ONE of the given components.
  template <typename... Components> bool hasAny() const {
    return s_registry.any_of<Components...>(m_handle);
  }

  // ==========================================
  // Component Removal (erase / remove)
  // ==========================================

  // Unconditionally erases component(s) from this entity.
  // Fast path; assumes the entity already owns the component(s).
  template <typename... Components> auto eraseComponent() {
    return s_registry.erase<Components...>(m_handle);
  }

  // Unconditionally erases component(s) from this entity (alias for eraseComponent).
  template <typename... Components> auto erase() {
    return s_registry.erase<Components...>(m_handle);
  }

  // Safely removes component(s) from this entity if present.
  // Returns the number of components successfully removed.
  template <typename... Components> auto removeComponent() {
    return s_registry.remove<Components...>(m_handle);
  }

  // Safely removes component(s) from this entity if present (alias for removeComponent).
  template <typename... Components> auto remove() {
    return s_registry.remove<Components...>(m_handle);
  }

  // ==========================================
  // Iteration Views & Groups (Systems / Loops)
  // ==========================================

  // Creates a view to iterate over entities matching the requested components.
  // Example:
  //   auto view = Entity::view<Position, Velocity>();
  //   for (auto [entity, pos, vel] : view.each()) { ... }
  template <typename... Components>
  static auto view() {
    return s_registry.view<Components...>();
  }

  // Creates a view to iterate over entities with components while excluding others.
  // Example:
  //   auto view = Entity::view<Position>(entt::exclude<Velocity>);
  template <typename... Components, typename... Exclude>
  static auto view(entt::exclude_t<Exclude...> exclude) {
    return s_registry.view<Components...>(exclude);
  }

  // Directly executes a function or lambda on all entities matching the components.
  // Example:
  //   Entity::each<Position, Velocity>([](auto entity, auto &pos, auto &vel) {
  //       pos.x += vel.dx;
  //   });
  template <typename... Components, typename Func>
  static void each(Func &&func) {
    s_registry.view<Components...>().each(std::forward<Func>(func));
  }

  // Creates a high-performance owning or non-owning group for tight iteration loops.
  template <typename... Owned, typename... Get, typename... Exclude>
  static auto group(entt::get_t<Get...> get, entt::exclude_t<Exclude...> exclude = {}) {
    return s_registry.group<Owned...>(get, exclude);
  }

  template <typename... Owned>
  static auto group() {
    return s_registry.group<Owned...>();
  }

  // ==========================================
  // Cache-Friendly Component Sorting
  // ==========================================

  // Sorts the component storage according to a custom comparator.
  // Example:
  //   Entity::sort<Renderable>([](const auto &lhs, const auto &rhs) { return lhs.z < rhs.z; });
  template <typename Component, typename Compare>
  static void sort(Compare &&compare) {
    s_registry.sort<Component>(std::forward<Compare>(compare));
  }

  // Rearranges ComponentToReorder in memory to match the order of ComponentToFollow (minimizes cache misses).
  template <typename ComponentToReorder, typename ComponentToFollow>
  static void sort() {
    s_registry.sort<ComponentToReorder, ComponentToFollow>();
  }

  // ==========================================
  // Signals & Reactive Observers
  // ==========================================

  // Returns the construction signal sink for a given component type.
  // Use .connect<...>() to attach callbacks: void(entt::registry &, entt::entity)
  template <typename Component>
  static auto onConstruct() {
    return s_registry.on_construct<Component>();
  }

  // Returns the update signal sink for a given component type.
  template <typename Component>
  static auto onUpdate() {
    return s_registry.on_update<Component>();
  }

  // Returns the destruction signal sink for a given component type.
  template <typename Component>
  static auto onDestroy() {
    return s_registry.on_destroy<Component>();
  }

  // ==========================================
  // Context Variables (Global ECS Singletons)
  // ==========================================

  // Stores a global context variable / singleton in the ECS registry.
  // Useful for engine globals (e.g. Window handle, DeltaTime, InputState).
  template <typename T, typename... Args>
  static decltype(auto) setContext(Args &&...args) {
    return s_registry.ctx().emplace<T>(std::forward<Args>(args)...);
  }

  // Retrieves a mutable reference to a global context variable.
  template <typename T>
  static T &getContext() {
    return s_registry.ctx().get<T>();
  }

  // Safely retrieves a pointer to a global context variable (returns nullptr if absent).
  template <typename T>
  static T *tryGetContext() {
    return s_registry.ctx().find<T>();
  }

  // Checks if a global context variable of type T exists.
  template <typename T>
  static bool hasContext() {
    return s_registry.ctx().contains<T>();
  }

  // Removes a global context variable of type T from the registry.
  template <typename T>
  static void removeContext() {
    s_registry.ctx().erase<T>();
  }

  // ==========================================
  // Registry-wide Clearing & Info
  // ==========================================

  // Returns the total number of alive entities in the registry.
  static size_t count() {
    return s_registry.view<entt::entity>().size();
  }

  // Erases all instances of the given component(s) across all entities in the registry.
  template <typename... Components> static void clearComponent() {
    s_registry.clear<Components...>();
  }

  // Destroys all entities and erases all components from the entire ECS registry.
  static void clearAll() { s_registry.clear(); }

  // ==========================================
  // Accessors, Comparisons, and Conversions
  // ==========================================

  // Returns the underlying entt::entity identifier.
  entt::entity handle() const { return m_handle; }
  entt::entity getHandle() const { return m_handle; }

  // Implicit conversion operator to entt::entity.
  operator entt::entity() const { return m_handle; }

  // Conversion operator to integer identifier (uint32_t).
  operator uint32_t() const { return static_cast<uint32_t>(m_handle); }

  // Boolean conversion: true if this entity is valid and alive.
  explicit operator bool() const { return isValid(); }

  // Entity equality comparisons
  bool operator==(const Entity &other) const { return m_handle == other.m_handle; }
  bool operator!=(const Entity &other) const { return m_handle != other.m_handle; }
  bool operator==(entt::entity other) const { return m_handle == other; }
  bool operator!=(entt::entity other) const { return m_handle != other; }

  // Direct access to the shared registry
  static entt::registry &getRegistry() { return s_registry; }
  static const entt::registry &getConstRegistry() { return s_registry; }

private:
  inline static entt::registry s_registry;
  entt::entity m_handle{entt::null};
};