#pragma once
#include <iostream>
#include <concepts>
#include "fecs/concepts.hpp"
#include <functional>
#include <tuple>
#include <variant>

namespace fecs {
  template<typename ...Stores>
  class world : private Stores... {
    EntityId nextId = 0;

  public:

    EntityId newEntity() {
      (Stores::resizeToFit(nextId), ...);
      return ++nextId;
    }

    EntityId maxId() const { return nextId; }

    using Stores::hasElement...;
    using Stores::getSafe...;
    using Stores::getUnsafe...;
    using Stores::addComponent...;
    using Stores::moveComponent...;
    using Stores::removeComponent...;

    template<typename ...Elements>
      requires (concepts::QueryContainer<world, Elements> && ...)
    inline bool hasAllElements(EntityId i) const {
      return (this->template hasElement<Elements>(i) &&  ...);
    }

    template<typename T, std::same_as<typename T::value_type> F>
      requires 
        concepts::AddContainer<world, F> &&
        concepts::RemoveContainer<world, F>
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
      if((w.template hasAllElements<Args...>(i))) {
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
      if((w.template hasAllElements<Args...>(i))) {
        f(w.template getUnsafe<Args>(i)...);
      }
    }
  }
}
