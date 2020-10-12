#include "fecs/vector_store.hpp"
#include "catch.hpp"
#include "fecs/concepts.hpp"

using namespace fecs;

static_assert(concepts::QueryContainer<vector_store<int>, int>);
static_assert(concepts::GetSafeContainer<vector_store<int>, int>);
static_assert(concepts::GetUnsafeContainer<vector_store<int>, int>);
static_assert(concepts::MoveContainer<vector_store<int>, int>);

TEST_CASE("operations on empty stores") {
  vector_store<int> storage;

  SECTION("hasComponent") {
    REQUIRE(storage.hasComponent(0) == false);
  }

  SECTION("getSafe") {
    REQUIRE(storage.getSafe(0) == std::nullopt);
  }
}

TEST_CASE("operations on singular stores") {
  vector_store<int> storage;
  storage.addComponent(0, 10);

  SECTION("hasComponent") {
    REQUIRE(storage.hasComponent(0));
  }

  SECTION("getSafe") {
    REQUIRE(storage.getSafe(0) == 10);
  }

  SECTION("getUnsafe") {
    REQUIRE(storage.getUnsafe(0) == 10);
  }

  SECTION("removeComponent") {
    storage.removeComponent(0);
    REQUIRE(storage.hasComponent(0) == false);
  }
}
