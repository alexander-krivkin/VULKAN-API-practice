#pragma once

#include <windows.h>

#include <array>
#include <map>
#include <string>
#include <fstream>
#include <chrono>


namespace ak
{
	enum class LogLevelFlagBits
	{
		eAppInfo = 0x00000001,
		eAppWarning = 0x00000002,
		eAppError = 0x00000004,
		eVulkanVerbose = 0x00000008,
		eVulkanInfo = 0x00000010,
		eVulkanWarning = 0x00000020,
		eVulkanError = 0x00000040
	};


	inline constexpr std::underlying_type_t<LogLevelFlagBits>
		operator|(LogLevelFlagBits lhs, LogLevelFlagBits rhs)
	{
		return (static_cast<std::underlying_type_t<LogLevelFlagBits>>(lhs) |
			static_cast<std::underlying_type_t<LogLevelFlagBits>>(rhs));
	}

	inline constexpr std::underlying_type_t<LogLevelFlagBits>
		operator|(int lhs, LogLevelFlagBits rhs)
	{
		return (lhs | static_cast<std::underlying_type_t<LogLevelFlagBits>>(rhs));
	}

	inline constexpr std::underlying_type_t<LogLevelFlagBits>
		operator|(LogLevelFlagBits lhs, int rhs)
	{
		return (static_cast<std::underlying_type_t<LogLevelFlagBits>>(lhs) | rhs);
	}

	inline constexpr std::underlying_type_t<LogLevelFlagBits>
		operator&(LogLevelFlagBits lhs, LogLevelFlagBits rhs)
	{
		return (static_cast<std::underlying_type_t<LogLevelFlagBits>>(lhs) &
			static_cast<std::underlying_type_t<LogLevelFlagBits>>(rhs));
	}

	inline constexpr std::underlying_type_t<LogLevelFlagBits>
		operator&(int lhs, LogLevelFlagBits rhs)
	{
		return (lhs & static_cast<std::underlying_type_t<LogLevelFlagBits>>(rhs));
	}

	inline constexpr std::underlying_type_t<LogLevelFlagBits>
		operator&(LogLevelFlagBits lhs, int rhs)
	{
		return (static_cast<std::underlying_type_t<LogLevelFlagBits>>(lhs) & rhs);
	}


	std::wstring stringToWstring(const char* str);
	std::wstring stringToWstring(const std::string& str);


	class LoggingSystem final
	{
	public:
		LoggingSystem() {}
		explicit LoggingSystem(const LoggingSystem& obj) = delete;
		~LoggingSystem();
		LoggingSystem& operator=(const LoggingSystem& obj) = delete;

		void init(const HWND& hWnd, std::underlying_type_t<LogLevelFlagBits> flags, std::string logFilename = "");
		std::underlying_type_t<LogLevelFlagBits> getFlags() const { return flags_; }
		void resetTimer();
		void postLogMessage(LogLevelFlagBits logLevel, const char* message);
		void postLogMessage(LogLevelFlagBits logLevel, const std::string& message);
		void postLogMessage(LogLevelFlagBits logLevel, const std::wstring& message);

	private:
		HWND hWnd_{};
		std::underlying_type_t<LogLevelFlagBits> flags_{};
		std::string logFilename_{};

		std::ofstream logFile_{};
		std::chrono::steady_clock::time_point tStart_{};
		std::chrono::steady_clock::time_point tNow_{};
		std::chrono::nanoseconds tElapsed_{};
	};
}