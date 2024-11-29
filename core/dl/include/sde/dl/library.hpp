#pragma once

// C++ Standard Library
#include <iosfwd>

// SDE
#include "sde/expected.hpp"

namespace sde::dl
{

struct Error
{
  const char* details;
};

std::ostream& operator<<(std::ostream& os, const Error& error);

class Symbol
{
public:
  Symbol() = default;

  constexpr explicit Symbol(void* handle) : handle_{handle} {}

  constexpr void* handle() const { return handle_; }

  constexpr bool isNull() const { return handle_ == nullptr; }
  constexpr bool isValid() const { return !isNull(); }
  constexpr operator bool() const { return isValid(); }

  constexpr void reset() { handle_ = nullptr; }

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

  Function() = default;

  constexpr explicit Function(Symbol sym) : symbol_{sym} {}

  constexpr bool isNull() const { return symbol_.isNull(); }
  constexpr bool isValid() const { return symbol_.isValid(); }
  constexpr operator bool() const { return isValid(); }

  constexpr void reset() { symbol_.reset(); }

  Function& operator=(const Symbol& symbol)
  {
    symbol_ = symbol;
    return *this;
  }

private:
  Symbol symbol_;
};


class Library
{
public:
  ~Library();

  Library() = default;

  Library(Library&& other);
  Library& operator=(Library&& other);

  expected<Symbol, Error> get(const char* symbol) const;

  static expected<Library, Error> load(const char* library_path);

  void reset();
  void swap(Library& other);

  constexpr void* handle() const { return handle_; }
  constexpr bool isNull() const { return handle_ == nullptr; }
  constexpr bool isValid() const { return !isNull(); }
  constexpr operator bool() const { return isValid(); }

private:
  constexpr explicit Library(void* handle) : handle_{handle} {}

  Library(const Library& other) = delete;
  Library& operator=(const Library& other) = delete;

  void* handle_ = nullptr;
};

}  // namespace sde::dl