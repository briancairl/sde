/**
 * @copyright 2024-present Brian Cairl
 *
 * @file resource_cache.hpp
 */
#pragma once

// C++ Standard Library
#include <cstdint>
#include <iosfwd>
#include <unordered_map>

// SDE
#include "sde/crtp.hpp"
#include "sde/expected.hpp"
#include "sde/resource_handle.hpp"
#include "sde/type.hpp"

namespace sde
{

template <typename DerivedT> struct ResourceCacheTypes;

template <typename DerivedT> class ResourceCache : public crtp_base<ResourceCache<DerivedT>>
{
public:
  using type_info = ResourceCacheTypes<DerivedT>;
  using error_type = typename type_info::error_type;
  using handle_type = typename type_info::handle_type;
  using value_type = typename type_info::value_type;

  using CacheMap = std::unordered_map<handle_type, value_type, ResourceHandleHash>;

  template <typename... CreateArgTs> expected<handle_type, error_type> create(CreateArgTs&&... args)
  {
    auto h = handle_lower_bound_;
    ++h;
    if (auto ok_or_error = this->add(h, std::forward<CreateArgTs>(args)...); !ok_or_error.has_value())
    {
      return make_unexpected(ok_or_error.error());
    }
    return h;
  }

  template <typename... CreateArgTs> expected<void, error_type> add(const handle_type& handle, CreateArgTs&&... args)
  {
    auto value_or_error = this->derived().generate(std::forward<CreateArgTs>(args)...);
    if (value_or_error.has_value())
    {
      handle_lower_bound_ = std::max(handle_lower_bound_, handle);
      handle_to_value_cache_.emplace(handle, std::move(value_or_error).value());
      return {};
    }
    return make_unexpected(value_or_error.error());
  }

  const value_type* get_if(const handle_type& handle) const
  {
    if (auto itr = handle_to_value_cache_.find(handle); itr != std::end(handle_to_value_cache_))
    {
      return std::addressof(itr->second);
    }
    return nullptr;
  }

  const bool exists(handle_type handle) const { return handle_to_value_cache_.count(handle) != 0; }

  void remove(handle_type handle) { handle_to_value_cache_.erase(handle); }

  const auto& cache() const { return handle_to_value_cache_; }

  const auto begin() const { return std::begin(handle_to_value_cache_); }

  const auto end() const { return std::end(handle_to_value_cache_); }

private:
  /// Last used resource handle
  handle_type handle_lower_bound_ = handle_type::null();
  /// Map of {resource_handle, resource_value} objects
  CacheMap handle_to_value_cache_;
};

}  // namespace sde
