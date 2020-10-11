#include <fecs/concepts.hpp>
#pragma once
#include <vector>
#include <optional>
#include <concepts>

namespace fecs {

  template<typename T>
  class vector_store {

    using ElementType = T;
    std::vector<std::optional<ElementType>> elements;

  public:
    template<std::same_as<T> T2>
    bool hasElement(EntityId id) const;

    template<std::same_as<T> T2>
    std::optional<ElementType> getSafe(EntityId id) const;

    template<std::same_as<T> T2>
    ElementType getUnsafe(EntityId id) const;

    template<std::same_as<T> T2>
    void addComponent(EntityId id, const T2& comp);

    template<std::same_as<T> T2>
    void moveComponent(EntityId id, T2&& c);

    template<std::same_as<T> T2>
    void removeComponent(EntityId id);
  };

  template<typename T>
  template<std::same_as<T> T2>
  bool vector_store<T>::hasElement(EntityId id) const {
    if(id >= elements.size()) {
      return false;
    }

    return elements[id].has_value();
  }

  template<typename T>
  template<std::same_as<T> T2>
  std::optional<T> vector_store<T>::getSafe(EntityId id) const {
    if(id >= elements.size()) {
      return std::nullopt;
    }
    return elements[id];
  }

  template<typename T>
  template<std::same_as<T> T2>
  T vector_store<T>::getUnsafe(EntityId id) const {
    return *(elements.at(id));
  }

  template<typename T>
  template<std::same_as<T> T2>
  void vector_store<T>::addComponent(EntityId id, const T2& comp) {
    if(elements.size() <= id) {
      elements.resize(id + 1);
    }
    elements.at(id) = comp;
  }

  template<typename T>
  template<std::same_as<T> T2>
  void vector_store<T>::moveComponent(EntityId id, T2&& t2) {
    if(elements.size() <= id) {
      elements.resize(id + 1);
    }
    elements.emplace(elements.begin() + id, std::forward<T2&&>(t2));
  }


  template<typename T>
  template<std::same_as<T> T2>
  void vector_store<T>::removeComponent(EntityId id) {
    if(elements.size() <= id) {
      return;
    }
    elements.at(id) = std::nullopt;
  }

  static_assert(concepts::QueryContainer<vector_store<int>, int>);
  static_assert(concepts::GetSafeContainer<vector_store<int>, int>);
  static_assert(concepts::GetUnsafeContainer<vector_store<int>, int>);
  static_assert(concepts::MoveContainer<vector_store<int>, int>);
}
