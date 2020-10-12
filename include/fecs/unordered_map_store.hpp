#pragma once

#include <fecs/concepts.hpp>
#include <unordered_map>
#include <optional>
#include <concepts>

namespace fecs {
  template<typename T>
  /**
   * A store that uses an unordered_map.
   * This saves memory in the case that this component is used infrequently.
   *
   * Note: std::unorderd_map has not-so-good performance.
   * In the future I might change this so it also takes a map type
   * as a template parameter, but that's way too complicated for right now.
   */
  class unordered_map_store {

    using ElementType = T;
    std::unordered_map<EntityId, ElementType> map;

    public:
      template<std::same_as<T> T2 = T>
      inline bool hasComponent(EntityId id) const;

      template<std::same_as<T> T2 = T>
      inline std::optional<ElementType> getSafe(EntityId id) const;

      template<std::same_as<T> T2 = T>
      inline ElementType getUnsafe(EntityId id) const;

      template<std::same_as<T> T2 = T>
      inline void addComponent(EntityId id, T2 comp);

      template<std::same_as<T> T2 = T>
      inline void moveComponent(EntityId id, T2&& c);

      template<std::same_as<T> T2 = T>
      inline void removeComponent(EntityId id);

      // Does not do resizing
      inline void resizeToFit(EntityId id) {}
  };

  template<typename T>
  template<std::same_as<T> T2>
  inline bool unordered_map_store<T>::hasComponent(EntityId id) const {
    return map.contains(id);
  }

  template<typename T>
  template<std::same_as<T> T2>
  inline std::optional<T> unordered_map_store<T>::getSafe(EntityId id) const {
    auto it = map.find(id);
    if(it != map.end()) {
      return *it;
    }
    return std::nullopt;
  }

  template<typename T>
  template<std::same_as<T> T2>
  inline T unordered_map_store<T>::getUnsafe(EntityId id) const {
    return map.at(id);
  }

  template<typename T>
  template<std::same_as<T> T2>
  inline void unordered_map_store<T>::addComponent(EntityId id, T2 comp) {
    map.emplace(id, comp);
  }

  template<typename T>
  template<std::same_as<T> T2>
  inline void unordered_map_store<T>::moveComponent(EntityId id, T2&& t2) {
    map.emplace(id, std::move(t2));
  }


  template<typename T>
  template<std::same_as<T> T2>
  inline void unordered_map_store<T>::removeComponent(EntityId id) {
    map.erase(id);
  }

}
