#include <iostream>
#include <string>
#include <variant>
#include "fecs/vector_store.hpp"
#include "fecs/concepts.hpp"
#include "fecs/world.hpp"

using Foo = fecs::world<fecs::vector_store<int>, fecs::vector_store<float>>;

int main() {
  Foo w;

  static_assert(fecs::concepts::MapResultContainer<int, Foo>);
  static_assert(fecs::concepts::MapResultContainer<std::optional<int>, Foo>);
  static_assert(
      fecs::concepts::MapResultContainer<std::tuple<int, float>, Foo>
  );
  static_assert(
      fecs::concepts::MapResultContainer<std::variant<int, float>, Foo>
  );
  // static_assert(fecs::concepts::MapResultContainer<std::tuple<int, std::string>, Foo>);
  auto printEach = [&]() {
    fecs::mapEntities<int, float>(w, [&](int i, float f) -> void {
        std::cout << i << " " << f << "\n";
    });
  };

  for(int i = 0; i < 20; ++i) {
    auto entity = w.newEntity();
    w.addComponent<int>(entity, i);
    w.addComponent<float>(entity, i);
  }


  fecs::mapEntities<int>(w, [=](int i) -> std::variant<std::optional<int>, float> {
      if(i % 2 == 0) { 
        return {i * 1.0f};
      }
      else if (i % 3 == 0) {
        return {std::optional<int>{i + 10}};
      }
      else {
        return { std::nullopt };
      }
  });

  printEach();

  return 0;
}
