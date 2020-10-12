# Functional Entity Component System (fecs)

This is a C++ library that provides an Entity-Component System with a functional interface.
That is, instead of using references to modify components, you *return values* that replace them.
This leads to more testable code, at the possible expense of some speed (in very basic "let's see what happens" benchmarks, the compiler's inlining becomes a saving grace!).

This library uses concepts to get error messages that are *marginally* less awful than normal.

## Getting Started

The first thing to do is to define your world.
You do this by making a version of `fecs::world` with some *stores*.
Stores are things that hold components.
They conform to a common interface that allows setting, updating, and doing other things.
Let's build one:

```cpp

using MyWorld = fecs::world<
  fecs::vector_store<int>,
  fecs::vector_store<float>
>;

```

Great, now I can store ints and floats.
Let's make some entities with components, shall we?

```cpp
MyWorld world;

for(int i = 0; i < 100; ++i) {
  const auto entity = world.newEntity();
  world.addComponent<int>(entity, i);
  if(i % 2 == 0) {
    world.addComponent<float>(entity, static_cast<float>(i));
  }
}
```

Neat!
Let's say we wanted to print each entity with both an int and a float in our world.
That becomes:

```cpp
fecs::mapEntities<int, float>(w, [=](int i, float f) -> void {
  std::cout << i << " " << f << "\n";
});
```

### Mapping Results

What if we want to increment each `int` component by one?
Simple: return the new value:

```cpp
fecs::mapEntities<int>(w, [=](int i) -> int { return i + 1; });
```

Behind the scenes we'll manage storage for you.
It's important to note that, by not doing *mutation*, we can have "fancy" stores that give us more functionality.
For example, we could have a storage for physics bodies that automatically updates a bounding volume hierarchy when a component is replaced, which is much harder to do if we're mutating values!

## Removing Components

What if we want to remove an entity?
We can do this!

```cpp
fecs::mapEntities<int>(w, [=](int i) -> std::optional<int> {
  if(i % 2 == 0) return std::nullopt;
  return i;
});
```

Pretty cool.

### Multiple-Update

What if we wanted to update two things at once?
Well, we can just return two things via a tuple!

```cpp
fecs::mapEntities<int>(w, [=](int i, float f) -> std::tuple<int, float> {
  return { i * 2, f * 3 };
});
```

### Conditional Update

What if we want to update a different component conditionally?
That is, we want to set EITHER the float or the int, and not both?

You could use a tuple, but that's slow.
Instead, use a *variant*.

```cpp
fecs::mapEntity<int>(w, [=](int i) -> std::variant<int, float> {
  if(i % 2 == 0) {
    return {static_cast<float>(i)};
  }
  return { i * 10 };
});
```

### Composability 

It's important to note that these are *fully composible*.
That is, stuff like this works fine:

```cpp
  fecs::mapEntities<int>(world, [=](int i) -> std::variant<std::optional<int>, float> {
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
```

## Next Steps

This is not a production-ready library yet, more an experiment.
In order for this to be cooler, I want:

1. Better support for "partial" stores: a store that does not allow deleting may be useful for an event system of some kind, for example
2. Optimization checks
3. Better error messages (concepts help a bit here but they're still... not super great, frankly)
4. A more robust test suite
5. The ability to `unsafeGet` an `std::variant`. 
   I can't make the templated types seem to work, but the basic idea is fairly simple: if you run something like:
      ```cpp
        fecs::mapEntities<std::variant<int, float>>(world, [](const std::variant<int, float>& variant) { /* stuff here */ });
      ```
  It should look for an `int` component, and if that's not found, a `float` component, and if neither is found, skip over the entity as it is not applicable to the mapping function.
6. An automatic concurrency generator.
   In principle, if you give me a set of mapping functions, I can determine *exactly* what they might read or modify based on their arguments and return type.
   If all of these functions can be called in any order (for example, "update animation components to the current frame", "update physics bodies to the current frame", and "update particle effects to the current frame"), we *should* be able to use some horrible C++ template magic to derive an "optimally concurrent" mapper, which knows what things it can run concurrently, knows what things it has to run sequentially, and automatically uses whatever thread-pooling library you give it to do these things concurrently.
   Note that even if it turns out to be some stupid O(2^n) algorithm to find an *optimal* path, even finding a *pretty good* path gives you *massive conurrency for very little effort*, which is increasingly useful in the era of core wars.
7. Some sort of "cache-local" store that stores *multiple components together* transparently.
   This would improve cache performance, but once again the templates are very tricky to get right (you'll wind up needing a vector of optional tuples of optional values, which... gross)
   Still, since cache is king, it's worth the awful effort.
8. Better docs!
   Doxygen *really* does not like c++20 `requires` statements, which is something I unfortunately don't know how to fix.

## Special Thanks

This library wholesale steals all of its ideas from the *absolutely excellent*, *brain-expanding* [apecs](https://github.com/jonascarpay/apecs) by [Jonas Carpay](https://github.com/jonascarpay).
Check it out, and give him massive kudos.
