#pragma once

// SDE
#include "sde/expected.hpp"

namespace sde::dl
{

struct Error
{
  const char* details;
};


class Symbol
{
public:
  constexpr explicit Symbol(void* handle) : handle_{handle} {}

  constexpr bool isNull() const { return handle_ == nullptr; }
  constexpr bool isValid() const { return !isNull(); }
  constexpr void* handle() const { return handle_; }

private:
  void* handle_ = nullptr;
};


template <typename F> class Function;


template <typename R, typename... Args> class Function<R(Args...)>
{
public:
  template <typename... Ts> R operator()(Ts&&... args) const
  {
    using FPtr = R (*)(Args...);
    return reinterpret_cast<FPtr>(symbol_.handle())(std::forward<Ts>(args)...);
  }

  constexpr explicit Function(Symbol sym) : symbol_{sym} {}

private:
  Symbol symbol_;
};


class Library
{
public:
  ~Library();

  Library(Library&& other);

  expected<Symbol, Error> get(const char* symbol) const;

  static expected<Library, Error> load(const char* library_path);

  void swap(Library& other);

  void* handle() const { return handle_; }

private:
  Library(const Library& other) = delete;
  constexpr explicit Library(void* handle) : handle_{handle} {}

  void* handle_ = nullptr;
};

}  // namespace sde::dl