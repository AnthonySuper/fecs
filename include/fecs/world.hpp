#pragma once
#include <iostream>
#include <concepts>
#include "fecs/concepts.hpp"
#include <functional>
#include <tuple>
#include <variant>
#include <stdexcept>

namespace fecs {
  template<typename ...Stores>
  /**
   * A world in fecs contains multiple components, and entities.
   *
   * By default all fecs stores have *value semantics*, meaning you can copy this around like normal.
   * This is extremely useful if you want to implement save-states or some other such thing!
   */
  class world : private Stores... {
    EntityId lastId = 0;

  public:

    /**
     * Get a new entity you can do stuff with.
     * Note: Entities *can not be deleted* so you'll "quasi-leak" memory as time goes on.
     * TODO: Some sort of entity recycling? Is that doable without destroying performance?
     */
    inline EntityId newEntity() {
      (Stores::resizeToFit(lastId), ...);
      return lastId++;
    }

    /**
     * Get the maximum entity id.
     * Useful for iteration.
     *
     * TODO: make this something you can iterate via ranges?
     */
    inline EntityId maxId() const { return lastId + 1; }
    auto operator<=>(const world&) const = default;

    using Stores::hasComponent...;
    using Stores::getSafe...;
    using Stores::getUnsafe...;
    using Stores::addComponent...;
    using Stores::moveComponent...;
    using Stores::removeComponent...;

    /**
     * Section: Specializations.
     * These let you have a much nicer mapping interface.
     */

    template<typename T, typename F = typename T::value_type>
      requires 
        concepts::GetSafeContainer<world, F> &&
        std::same_as<T, std::optional<F>>
    /**
     * You can get an optional for *any* component as we can always return nullopt
     */
    inline bool hasComponent(EntityId id) const {
      return true;
    }

    template<typename T, typename F = typename T::value_type>
      requires
        concepts::GetSafeContainer<world, F> &&
        std::same_as<T, std::optional<F>>
    inline std::optional<F> getUnsafe(EntityId id) const {
      return this->template getSafe<F>(id);
    }

    template<typename ...Elements>
      requires (concepts::QueryContainer<world, Elements> && ...)
    /**
     * Query function that ensures we have *all* the elements in a list.
     */
    inline bool hasAllComponents(EntityId i) const {
      return (this->template hasComponent<Elements>(i) &&  ...);
    }

    template<typename T, std::same_as<typename T::value_type> F>
      requires 
        concepts::AddContainer<world, F> &&
        concepts::RemoveContainer<world, F>
    /**
     * If the result of the map is an optional value, the following semantics are followed:
     *
     * 1. If the optional is empty, *delete* from the underlying component store
     * 2. If the optional has a value, *add or replace* to the underlying component store.
     */
    inline void setMapResult(EntityId id, const std::optional<F>& opt) {
      if(opt) {
        this->template addComponent<F>(id, *opt);
      }
      else {
        this->template removeComponent<F>(id);
      }
    }

    /**
     * If we can add type T as a component to an Entity,
     * we can use it as the result of a mapper function.
     */
    template<typename T, std::same_as<T> F> 
      requires concepts::AddContainer<world, T>
    inline void setMapResult(EntityId id, const F& r) {
      this->template addComponent<T>(id, r);
    }

    /**
     * If each element of a tuple is a valid map result,
     * we can use the entire tuple as a map result by applying all elements.
     *
     * Note: if you use a std::tuple<T, T> where both Ts are components, the *last one* wins.
     * Also, why are you doing that? We'll have to assign things twice and that's slow!
     */
    template<typename T, typename ...F>
      requires
        (concepts::MapResultContainer<F, world> && ...) &&
        std::same_as<T, std::tuple<F...>>
    inline void setMapResult(EntityId id, const std::tuple<F...>& opt) {
      setMapResultTupleDetail(
          id,
          opt,
          std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<std::tuple<F...>>>>{}
      );
    }

    template<typename T, typename ...F>
      requires
        (concepts::MapResultContainer<F, world> && ...) &&
        std::same_as<T, std::variant<F...>>
    inline void setMapResult(EntityId id, const std::variant<F...>& variant) {
      (
       this->template setMapResultVariantDetail<
          std::variant<F...>,
          F
      >(id, variant),
      ...
      );
    }

  private:

    template<typename Tuple, std::size_t ...Indexes>
    inline void setMapResultTupleDetail(
        EntityId id,
        const Tuple& tuple,
        std::index_sequence<Indexes...>
    ) {
      (this->template setMapResult<
        std::remove_reference_t<std::tuple_element_t<Indexes, Tuple>>
      >(id, std::get<Indexes>(tuple)), ...);
    }

    template<typename Variant, typename Type>
    inline void setMapResultVariantDetail(
        EntityId id,
        const Variant& variant
    ) {
      if(const auto val = std::get_if<Type>(&variant)) {
        this->template setMapResult<Type>(
            id,
            *val
        );
      }
    }
  };

  /**
   * Apply a mapper function over each applicable entity.
   * For each Entity that has all components in Args, call the function,
   * then assign the result type of the entity.
   *
   * By using special result types you can achieve different things.
   * For example, if you want to *remove* a component of type T,
   * you can return an empty std::optional<T>.
   * You can also use std::tuple to assign multiple components at once!
   */
  template<
    typename ...Args,
    typename Function,
    typename World
  > requires 
      concepts::ContainerMapFunction<World, Function, Args...>
  inline void mapEntities(World& w, Function f) {
    for(EntityId i = 0; i < w.maxId(); ++i) {
      if((w.template hasAllComponents<Args...>(i))) {
        w.template setMapResult<decltype(f(w.template getUnsafe<Args>(i)...))>
          (i, f(w.template getUnsafe<Args>(i)...));
      }
    }
  }

  /**
   * Same as the above overload of mapEntities, but used for functions
   * with a void result type.
   */
  template<
    typename ...Args,
    typename Function,
    typename World
  > requires concepts::ContainerVoidMapFunction<World, Function, Args...>
  inline void mapEntities(const World& w, Function f) {
    for(EntityId i = 0; i < w.maxId(); ++i) {
      if((w.template hasAllComponents<Args...>(i))) {
        f(w.template getUnsafe<Args>(i)...);
      }
    }
  }
}
