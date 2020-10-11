#include <concepts>
#include <optional>
#pragma once 

namespace fecs {
  using EntityId = std::size_t;

  namespace concepts {
    template<typename Container, typename Component>
    concept QueryContainer = requires(const Container& c, EntityId id) {
      { c.template hasElement<Component>(id) } -> std::convertible_to<bool>;
    };

    template<typename Container, typename Component>
    concept GetSafeContainer = requires(const Container& c, EntityId id) {
      { c.template getSafe<Component>(id) } -> std::convertible_to<std::optional<Component>>;
    };

    template<typename Container, typename Component>
    concept GetUnsafeContainer = requires(const Container& c, EntityId id) {
      { c.template getUnsafe<Component>(id) } -> std::convertible_to<Component>;
    };

    template<typename Container, typename Component>
    concept RemoveContainer = requires(Container& c, EntityId id) {
      { c.template removeComponent<Component>(id) };
    };

    template<typename Container, typename Component>
    concept AddContainer = requires(Container& c, EntityId id, const Component& comp) {
      { c.template addComponent<Component>(id, comp) };
    };

    template<typename Container, typename Component>
    concept MoveContainer =
      requires(Container &c, EntityId id, Component&& comp) {
        { c.moveComponent(id, std::forward<Component&&>(comp)) };
      };

    template<typename Result, typename Container>
    concept MapResultContainer =
      requires(Container& c, EntityId id, Result result) {
        { c.template setMapResult<Result>(id, result) };
      };

    template<typename Container, typename Function, typename ...Args>
    concept ContainerMapFunction = 
      (GetUnsafeContainer<Container, Args> && ...) &&
      MapResultContainer<std::invoke_result_t<Function, Args...>, Container> &&
      requires(Function f, Args... args) {
      { f(args...) } -> MapResultContainer<Container>;
    };
      
    template<typename Container, typename Function, typename ...Args>
    concept ContainerVoidMapFunction =
      (GetUnsafeContainer<Container, Args> && ...) &&
      requires(Function f, Args... args) {
        { f(args...) } -> std::same_as<void>;
      };
  }
}
