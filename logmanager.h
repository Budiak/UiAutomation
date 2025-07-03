#pragma once

namespace LOGGER::managers
{
	class LogManager
	{
	public:
		LogManager() = default;
		~LogManager() = default;

		void Initialize();
		void Shutdown();
	};
}