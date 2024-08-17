#pragma once

// C++ Standard Library
#include <string>

// SDE
#include "sde/expected.hpp"

namespace sde::dl
{

struct Error
{
  std::string details;
};

class Symbol
{
public:
  bool isNull() const { return handle_ == nullptr; }
  bool isValid() const { return !isNull(); }

private:
  void* handle_ = nullptr;
};

class Library
{
public:
  expected<Symbol, Error> get(const char* symbol) const;
  static expected<Library, Error> load(const char* library_path);

private:
  void* handle_ = nullptr;
};

}  // namespace sde::dl