//    Copyright 2022, farm-ng inc.
//
//    Licensed under the Apache License, Version 2.0 (the "License");
//    you may not use this file except in compliance with the License.
//    You may obtain a copy of the License at
//
//        http://www.apache.org/licenses/LICENSE-2.0
//
//    Unless required by applicable law or agreed to in writing, software
//    distributed under the License is distributed on an "AS IS" BASIS,
//    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//    See the License for the specific language governing permissions and
//    limitations under the License.

#pragma once

#include "farm_ng/core/logging/logger.h"

#include <tl/expected.hpp>

#include <iostream>
#include <optional>
#include <string>
#include <vector>

namespace farm_ng {

struct Error {
  std::vector<ErrorDetail> details;
};

std::ostream& operator<<(std::ostream& os, Error const& error);

struct Success {};

template <class TT, class TE = Error>
using Expected = tl::expected<TT, TE>;

namespace details {

template <class TT, class TE>
struct UnwrapImpl<tl::expected<TT, TE>> {
  static auto impl(
      tl::expected<TT, TE>& wrapper,
      char const* wrapper_cstr,
      ::farm_ng::ErrorDetail detail) -> decltype(*wrapper) {
    if (!bool(wrapper)) {
      FARM_IMPL_LOG_PRINTLN(
          "[FARM_UNWRAP failed in {}:{}]", detail.file, detail.line);
      FARM_IMPL_LOG_PRINTLN(
          "expected type `{}` does not contain a valid value", wrapper_cstr);
      if (!detail.msg.empty()) {
        ::fmt::print(stderr, "{}", detail.msg);
      }
      ::fmt::print(stderr, "{}", wrapper.error());
      FARM_IMPL_ABORT();
    }
    return *wrapper;
  }
};
}  // namespace details
}  // namespace farm_ng

#define FARM_ERROR_DETAIL(...)                                          \
  ::farm_ng::ErrorDetail {                                              \
    .file = __FILE__, .line = __LINE__, .msg = FARM_FORMAT(__VA_ARGS__) \
  }

#define FARM_ERROR_REPORT(cstr, ...)                      \
  ::farm_ng::Error {                                      \
    .details = { FARM_ERROR_DETAIL(cstr, ##__VA_ARGS__) } \
  }

#define FARM_UNEXPECTED(cstr, ...) \
  ::tl::make_unexpected(FARM_ERROR_REPORT(cstr, ##__VA_ARGS__))

/// Assigns `*expression` to `var` of `Type`, but returns error if there is
/// one.
#define FARM_TRY(Type, var, expression)               \
  auto maybe##var = (expression);                     \
  if (!maybe##var) {                                  \
    return ::tl::make_unexpected(maybe##var.error()); \
  }                                                   \
  Type var = ::std::move(*maybe##var);

#define FARM_TRY_ASSERT(condition, ...)                                  \
  if (!(condition)) {                                                    \
    return FARM_UNEXPECTED(                                              \
        "bool({}) not true.\n{}", #condition, FARM_FORMAT(__VA_ARGS__)); \
  }                                                                      \
  do {                                                                   \
  } while (false)

namespace farm_ng {
template <class TT, class TE = Error>
std::optional<TT> fromExpected(Expected<TT, TE> expected) {
  return expected ? std::optional<TT>(std::move(*expected)) : std::nullopt;
}

template <class TT>
Expected<TT> fromOptional(std::optional<TT> optional) {
  return optional ? Expected<TT>(std::move(*optional))
                  : FARM_UNEXPECTED("std::nullopt");
}

namespace detail {

template <class TT>
struct AssertTrue<Expected<TT>> {
  static void impl(
      Expected<TT> const& condition,
      std::string const& condition_str,
      std::string const& file,
      int line,
      std::string const& func,
      std::string const& str) {
    if (!condition) {
      farm_ng::defaultLogger().log(
          farm_ng::LogLevel::critical,
          FARM_FORMAT(
              "PANIC: ASSERT failed\n Expected ({}) has error: {}",
              condition_str,
              condition.error()),
          file,
          line,
          func,
          str);
      FARM_IMPL_ABORT();
    }
  }
};

}  // namespace detail
}  // namespace farm_ng
