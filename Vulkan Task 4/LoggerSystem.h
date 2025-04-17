#pragma once

#include <windows.h>

#include <array>
#include <string>
#include <chrono>


namespace ak
{
	std::wstring stringToWstring(const char* str);
	std::wstring stringToWstring(const std::string& str);

	enum class LoggerLevel
	{
		APP_INFO = 0,
		APP_WARNING = 1,
		APP_ERROR = 2,
		VULKAN_VERBOSE = 3,
		VULKAN_INFO = 4,
		VULKAN_WARNING = 5,
		VULKAN_ERROR = 6,
		LOG_MAX = 7
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