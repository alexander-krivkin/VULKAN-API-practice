#pragma once

#include <windows.h>

#include <array>
#include <string>
#include <chrono>


namespace ak
{
	enum class LoggerLevel
	{
		LOG_INFO = 0,
		LOG_WARNING = 1,
		LOG_ERROR = 2,
		LOG_MAX = 3
	};

	class LoggerSystem final
	{
	public:
		LoggerSystem() = delete;
		explicit LoggerSystem(const HWND& hWnd) : hWnd_(hWnd){};
		explicit LoggerSystem(const LoggerSystem& obj) = delete;
		~LoggerSystem() {};
		LoggerSystem& operator=(const LoggerSystem& obj) = delete;

		void resetTimer() { tStart_ = std::chrono::steady_clock::now(); }
		void postMessage(LoggerLevel loggerLevel, const std::wstring& message);

	private:
		std::chrono::steady_clock::time_point tStart_{};
		std::chrono::steady_clock::time_point tNow_{};
		HWND hWnd_{};
	};
}