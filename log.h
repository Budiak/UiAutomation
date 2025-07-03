#pragma once
#include "spdlog/spdlog.h"

#define LOGGER_DEFAULT_LOGGER_NAME "UiAutologger"

#if defined(LOGGER_PLATFORM_WINDOWS)
#define LOGGER_BREAK __debugbreak();
#elif defined (LOGGER_PLATFORM_MAC)
#define LOGGER_BREAK __builtin_debugtrap();
#else
#define LOGGER_BREAK __builtin_trap();
#endif

#ifndef LOGGER_CONFIG_RELEASE
#define LOGGER_TRACE(...)	if (spdlog::get(LOGGER_DEFAULT_LOGGER_NAME) != nullptr) {spdlog::get(LOGGER_DEFAULT_LOGGER_NAME)->trace(__VA_ARGS__);}
#define LOGGER_DEBUG(...)	if (spdlog::get(LOGGER_DEFAULT_LOGGER_NAME) != nullptr) {spdlog::get(LOGGER_DEFAULT_LOGGER_NAME)->debug(__VA_ARGS__);}
#define LOGGER_INFO(...)		if (spdlog::get(LOGGER_DEFAULT_LOGGER_NAME) != nullptr) {spdlog::get(LOGGER_DEFAULT_LOGGER_NAME)->info(__VA_ARGS__);}
#define LOGGER_WARN(...)		if (spdlog::get(LOGGER_DEFAULT_LOGGER_NAME) != nullptr) {spdlog::get(LOGGER_DEFAULT_LOGGER_NAME)->warn(__VA_ARGS__);}
#define LOGGER_ERROR(...)	if (spdlog::get(LOGGER_DEFAULT_LOGGER_NAME) != nullptr) {spdlog::get(LOGGER_DEFAULT_LOGGER_NAME)->error(__VA_ARGS__);}
#define LOGGER_FATAL(...)	if (spdlog::get(LOGGER_DEFAULT_LOGGER_NAME) != nullptr) {spdlog::get(LOGGER_DEFAULT_LOGGER_NAME)->critical(__VA_ARGS__);}
#define LOGGER_ASSERT(x, msg) if ((x)) {} else { LOGGER_FATAL("ASSERT - {}\n\t{}\n\tin file: {}\n\ton line: {}", #x, msg, __FILE__, __LINE__); LOGGER_BREAK }
#else
// Disable logging for release builds
#define LOGGER_TRACE(...)	(void)0
#define LOGGER_DEBUG(...)	(void)0
#define LOGGER_INFO(...)		(void)0
#define LOGGER_WARN(...)		(void)0
#define LOGGER_ERROR(...)	(void)0
#define LOGGER_FATAL(...)	(void)0
#define LOGGER_ASSERT(x, msg) (void)0
#endif