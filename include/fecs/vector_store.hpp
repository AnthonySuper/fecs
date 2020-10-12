#include <fecs/concepts.hpp>
#pragma once
#include <vector>
#include <optional>
#include <concepts>

namespace fecs {

  template<typename T>
  /**
   * A store backed by a vector of optional values.
   * This makes it very fast (iterating through it generally optimizes out to incrementing a pointer) but it uses a lot of memory.
   */
  class vector_store {

    using ElementType = T;
    std::vector<std::optional<ElementType>> elements;

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

    inline void resizeToFit(EntityId id) {
      elements.resize(std::max(elements.size(), id + 1));
    }

    auto operator<=>(const vector_store&) const = default;
  };

  template<typename T>
  template<std::same_as<T> T2>
  inline bool vector_store<T>::hasComponent(EntityId id) const {
    if(id >= elements.size()) {
      return false;
    }

    return elements[id].has_value();
  }

  template<typename T>
  template<std::same_as<T> T2>
  inline std::optional<T> vector_store<T>::getSafe(EntityId id) const {
    if(id >= elements.size()) {
      return std::nullopt;
    }
    return elements[id];
  }

  template<typename T>
  template<std::same_as<T> T2>
  inline T vector_store<T>::getUnsafe(EntityId id) const {
    return *(elements.at(id));
  }

  template<typename T>
  template<std::same_as<T> T2>
  inline void vector_store<T>::addComponent(EntityId id, T2 comp) {
    if(elements.size() <= id) {
      elements.resize(id + 1);
    }
    elements.at(id) = std::move(comp);
  }

  template<typename T>
  template<std::same_as<T> T2>
  inline void vector_store<T>::moveComponent(EntityId id, T2&& t2) {
    if(elements.size() <= id) {
      elements.resize(id + 1);
    }
    elements.emplace(elements.begin() + id, std::forward<T2&&>(t2));
  }


  template<typename T>
  template<std::same_as<T> T2>
  inline void vector_store<T>::removeComponent(EntityId id) {
    if(elements.size() <= id) {
      return;
    }
    elements.at(id) = std::nullopt;
  }

}
