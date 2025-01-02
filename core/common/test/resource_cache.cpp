// GTest
#include <gtest/gtest.h>

// SDE
#include "sde/expected.hpp"
#include "sde/resource.hpp"
#include "sde/resource_cache.hpp"
#include "sde/resource_handle.hpp"

using namespace sde;

struct InnerResource : Resource<InnerResource>
{
  float a;
  int b;

  auto field_list() { return FieldList(Field{"a", a}, _Stub{"b", b}); }
};

template <> struct sde::Hasher<InnerResource> : ResourceHasher
{};

struct SimpleResource : Resource<SimpleResource>
{
  float a;
  InnerResource c;

  auto field_list() { return FieldList(Field{"a", a}, Field{"c", c}); }
};

enum class SimpleResourceError
{
  SDE_RESOURCE_CACHE_ERROR_ENUMS,
  kFailure
};

struct SimpleResourceCache;

struct SimpleResourceHandle : ResourceHandle<SimpleResourceHandle>
{
  SimpleResourceHandle() = default;
  explicit SimpleResourceHandle(id_type id) : ResourceHandle<SimpleResourceHandle>{id} {}
};

template <> struct sde::ResourceCacheTraits<SimpleResourceCache>
{
  using error_type = SimpleResourceError;
  using handle_type = SimpleResourceHandle;
  using value_type = SimpleResource;
  using dependencies = no_dependencies;
};

struct SimpleResourceCache : ResourceCache<SimpleResourceCache>
{
  expected<SimpleResource, SimpleResourceError> generate(dependencies deps, float a, int b)
  {
    if (b > 10)
    {
      return make_unexpected(SimpleResourceError::kFailure);
    }
    return SimpleResource{.a = a, .c = {.a = a, .b = b}};
  }

  expected<SimpleResource, SimpleResourceError> generate(dependencies deps, float a, InnerResource c)
  {
    return SimpleResource{.a = a, .c = c};
  }
};

TEST(ResourceCache, DefaultCache)
{
  SimpleResourceCache cache;
  ASSERT_EQ(cache.size(), 0UL);
}

TEST(ResourceCache, Create)
{
  SimpleResourceCache cache;
  auto resource_or_error = cache.create(NoDependencies, 1.0, 9);
  ASSERT_TRUE(resource_or_error.has_value());
}

TEST(ResourceCache, CreateWithOtherResource)
{
  SimpleResourceCache cache;
  auto resource_or_error = cache.create(NoDependencies, 1.0, InnerResource{});
  ASSERT_TRUE(resource_or_error.has_value());

  for (const auto& [handle, element] : cache)
  {
    ASSERT_TRUE(cache.exists(handle));
    ASSERT_GT(element.version.value, 0UL);
  }
}