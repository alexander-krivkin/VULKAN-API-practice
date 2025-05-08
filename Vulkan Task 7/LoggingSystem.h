#pragma once

#include <windows.h>

#include <array>
#include <string>
#include <chrono>


namespace ak
{
	std::wstring stringToWstring(const char* str);
	std::wstring stringToWstring(const std::string& str);

	enum class LogLevelFlag
	{
		eAppInfo = 0,
		eAppWarning = 1,
		eAppError = 2,
		eVulkanVerbose = 3,
		eVulkanInfo = 4,
		eVulkanWarning = 5,
		eVulkanError = 6
	};

	class LoggingSystem final
	{
	public:
		LoggingSystem() = delete;
		explicit LoggingSystem(const HWND& hWnd) : hWnd_(hWnd) {};
		explicit LoggingSystem(const LoggingSystem& obj) = delete;
		~LoggingSystem() {};
		LoggingSystem& operator=(const LoggingSystem& obj) = delete;

		void resetTimer() { tStart_ = std::chrono::steady_clock::now(); }
		void postLogMessage(LogLevelFlag logLevel, const char* message);
		void postLogMessage(LogLevelFlag logLevel, const std::string& message);
		void postLogMessage(LogLevelFlag logLevel, const std::wstring& message);

	private:
		std::chrono::steady_clock::time_point tStart_{};
		std::chrono::steady_clock::time_point tNow_{};
		std::chrono::nanoseconds tElapsed_{};
		HWND hWnd_{};
	};
}