#include "logmanager.h"
#include "log.h"

#include "spdlog/sinks/stdout_color_sinks.h"

#include <memory>
#include <spdlog/sinks/basic_file_sink.h>

namespace LOGGER::managers
{
	void LogManager::Initialize()
	{
		auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		consoleSink->set_pattern("%^[%Y-%m-%d %H:%M:%S.%e] %v%$");
		consoleSink->set_level(spdlog::level::warn);

		auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/multisink3.txt", true);
		file_sink->set_pattern("[%H:%M:%S %z] [%l] %v");
		file_sink->set_level(spdlog::level::trace);

		std::vector<spdlog::sink_ptr> sinks{ consoleSink, file_sink };
		auto logger = std::make_shared<spdlog::logger>(LOGGER_DEFAULT_LOGGER_NAME, sinks.begin(), sinks.end());
		spdlog::register_logger(logger);
	}

	void LogManager::Shutdown()
	{
		spdlog::shutdown();
	}
}