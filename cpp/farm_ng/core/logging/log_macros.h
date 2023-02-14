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

#include "farm_ng/core/enum/enum.h"
#include "farm_ng/core/logging/format.h"
#include "farm_ng/core/logging/logger.h"

#include <farm_pp/preprocessor/comma.hpp>
#include <farm_pp/preprocessor/empty.hpp>

#include <filesystem>

#define FARM_LEVEL_TRACE 0
#define FARM_LEVEL_DEBUG 1
#define FARM_LEVEL_INFO 2
#define FARM_LEVEL_WARN 3
#define FARM_LEVEL_ERROR 4
#define FARM_LEVEL_CRITICAL 5
#define FARM_LEVEL_OFF 6

#ifdef FARM_ONLY_INCLUDE_ME_IN_CPP_FILE
static_assert(
    false,
    "Shoot FARM_ONLY_INCLUDE_ME_IN_CPP_FILE is already define, but it should "
    "not! You must include log_macros.h only from a *.cpp file and you must "
    "define "
    "FARM_LOG_LEVEL right above the include.");
#endif
#define FARM_ONLY_INCLUDE_ME_IN_CPP_FILE 1

#ifndef FARM_LOG_LEVEL
static_assert(
    false,
    "You must include log_macros.h only from a *.cpp file and you must define "
    "FARM_LOG_LEVEL right above the include.");
#endif

#if defined FARM_LOG_LEVEL && FARM_LOG_LEVEL <= FARM_LEVEL_TRACE
#define FARM_TRACE(...)         \
  farm_ng::defaultLogger().log( \
      farm_ng::LogLevel::trace, \
      "TRACE",                  \
      __FILE__,                 \
      __LINE__,                 \
      __func__,                 \
      __VA_ARGS__)
#else
#define FARM_TRACE(...)
#endif

#if defined FARM_LOG_LEVEL && FARM_LOG_LEVEL <= FARM_LEVEL_DEBUG
#define FARM_DEBUG(...)         \
  farm_ng::defaultLogger().log( \
      farm_ng::LogLevel::debug, \
      "DEBUG",                  \
      __FILE__,                 \
      __LINE__,                 \
      __func__,                 \
      __VA_ARGS__)
#else
#define FARM_DEBUG(...)
#endif

#if defined FARM_LOG_LEVEL && FARM_LOG_LEVEL <= FARM_LEVEL_INFO
#define FARM_INFO(...)          \
  farm_ng::defaultLogger().log( \
      farm_ng::LogLevel::info,  \
      "INFO",                   \
      __FILE__,                 \
      __LINE__,                 \
      __func__,                 \
      __VA_ARGS__)
#else
#define FARM_INFO(...)
#endif

#if defined FARM_LOG_LEVEL && FARM_LOG_LEVEL <= FARM_LEVEL_WARN
#define FARM_WARN(...)            \
  farm_ng::defaultLogger().log(   \
      farm_ng::LogLevel::warning, \
      "WARN",                     \
      __FILE__,                   \
      __LINE__,                   \
      __func__,                   \
      __VA_ARGS__)
#else
#define FARM_WARN(...)
#endif

#if defined FARM_LOG_LEVEL && FARM_LOG_LEVEL <= FARM_LEVEL_ERROR
#define FARM_ERROR(...)         \
  farm_ng::defaultLogger().log( \
      farm_ng::LogLevel::error, \
      "ERROR",                  \
      __FILE__,                 \
      __LINE__,                 \
      __func__,                 \
      __VA_ARGS__)
#else
#define FARM_ERROR(...)
#endif

#if defined FARM_LOG_LEVEL && FARM_LOG_LEVEL <= FARM_LEVEL_CRITICAL
#define FARM_CRITICAL(...)         \
  farm_ng::defaultLogger().log(    \
      farm_ng::LogLevel::critical, \
      "CRITICAL",                  \
      __FILE__,                    \
      __LINE__,                    \
      __func__,                    \
      __VA_ARGS__)
#else
#define FARM_CRITICAL(...)
#endif
