#include "fecs/world.hpp"
#include "fecs/vector_store.hpp"
#include "fecs/concepts.hpp"
#include "catch.hpp"

using namespace fecs;

using TestWorld =
  world<vector_store<int>, vector_store<float>>;

static_assert(
    concepts::QueryContainer<TestWorld, int>
);
static_assert(
    concepts::GetSafeContainer<TestWorld, int>
);
static_assert(
    concepts::GetUnsafeContainer<TestWorld, int>
);
static_assert(
    concepts::MoveContainer<TestWorld, int>
);
static_assert(
    concepts::MapResultContainer<int, TestWorld>
);
static_assert(
    concepts::MapResultContainer<std::optional<int>, TestWorld>
);
static_assert(
    concepts::MapResultContainer<std::tuple<int, float>, TestWorld>
);
static_assert(
    concepts::MapResultContainer<std::variant<int, float>, TestWorld>
);

static std::tuple<EntityId, TestWorld> makeWorld() {
  TestWorld w;
  auto entity = w.newEntity();
  w.addComponent(entity, 10);
  w.addComponent(entity, 10.0f);

  return { entity, w };
}

TEST_CASE("with an empty world") {
  TestWorld w;

  REQUIRE(w == TestWorld{w});
  REQUIRE(w.hasComponent<int>(0) == false);
  REQUIRE(w.hasComponent<float>(0) == false);
  REQUIRE(w.hasAllComponents<int, float>(0) == false);
}

TEST_CASE("with some elements") {
  auto [entity, w] = makeWorld();

  REQUIRE(w.hasComponent<int>(entity));
  REQUIRE(w.hasComponent<float>(entity));
  REQUIRE(w.hasAllComponents<int, float>(entity));
}

TEST_CASE("mapping with new values") {
  auto [entity, w] = makeWorld();

  mapEntities<int>(w, [](int i) -> int { return i + 1; });

  REQUIRE(w.hasComponent<int>(entity));
  REQUIRE(w.getSafe<int>(entity) == 11);
}

TEST_CASE("mapping with a null optional") {
  auto [entity, w] = makeWorld();

  mapEntities<int>(w, [](int i) -> std::optional<int> { return std::nullopt; });

  REQUIRE(w.hasComponent<int>(entity) == false);
  REQUIRE(w.getSafe<int>(entity) == std::nullopt);
}

TEST_CASE("mapping while requesting an optional") {
  TestWorld w;

  const auto e = w.newEntity();
  REQUIRE(w.hasComponent<std::optional<int>>(e));
  REQUIRE(w.hasComponent<int>(e) == false);
}
