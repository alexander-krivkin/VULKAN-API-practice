#include "LoggingSystem.h"


namespace ak
{
	std::map<LogLevelFlagBits, std::wstring> LoggerLevelName
	{
		{LogLevelFlagBits::eAppInfo, L"APP INFO: "},
		{LogLevelFlagBits::eAppWarning, L"APP WARNING: "},
		{LogLevelFlagBits::eAppError, L"APP ERROR: "},
		{LogLevelFlagBits::eVulkanVerbose, L"VULKAN VERBOSE: "},
		{LogLevelFlagBits::eVulkanInfo, L"VULKAN INFO: "},
		{LogLevelFlagBits::eVulkanWarning, L"VULKAN WARNING: "},
		{LogLevelFlagBits::eVulkanError, L"VULKAN ERROR: "}
	};

	std::wstring stringToWstring(const char* str)
	{
		return stringToWstring(std::string(str));
	}

	std::wstring stringToWstring(const std::string& str)
	{
		int len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, nullptr, 0);
		std::wstring wstr(len, L'\0');
		MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, &wstr[0], len);
		return wstr;
	}

	LoggingSystem::~LoggingSystem()
	{
		if (!logFilename_.empty())
		{
			logFile_.open(logFilename_);
		}

		std::vector<char> tmpMsg(GetWindowTextLengthA(hWnd_) + 1);
		GetWindowTextA(hWnd_, &tmpMsg[0], GetWindowTextLengthA(hWnd_) + 1);

		std::string msg = std::string(&tmpMsg[0]);

		logFile_ << msg.c_str();
	}

	void LoggingSystem::init(const HWND& hWnd, std::underlying_type_t<LogLevelFlagBits> flags, std::string logFilename)
	{
		hWnd_ = hWnd;
		flags_ = flags;
		logFilename_ = logFilename;
	}

	void LoggingSystem::resetTimer()
	{
		tStart_ = std::chrono::steady_clock::now();
	}

	void LoggingSystem::postLogMessage(LogLevelFlagBits logLevel, const char* message)
	{
		postLogMessage(logLevel, stringToWstring(message));
	}

	void LoggingSystem::postLogMessage(LogLevelFlagBits logLevel, const std::string& message)
	{
		postLogMessage(logLevel, stringToWstring(message));
	}

	void LoggingSystem::postLogMessage(LogLevelFlagBits logLevel, const std::wstring& message)
	{
		if (logLevel & flags_)
		{
			tNow_ = std::chrono::steady_clock::now();
			tElapsed_ = tNow_ - tStart_;

			std::vector<WCHAR> tmpMsg(GetWindowTextLength(hWnd_) + 1);
			GetWindowText(hWnd_, &tmpMsg[0], GetWindowTextLength(hWnd_) + 1);

			std::wstring msg = std::wstring(&tmpMsg[0]);
			msg += L"[" + std::to_wstring(std::chrono::duration_cast<std::chrono::milliseconds>(tElapsed_).count());
			msg += L" ms]   " + LoggerLevelName[logLevel] + L"\r\n";
			msg += message + L"\r\n";

			SetWindowText(hWnd_, msg.c_str());
			SendMessage(hWnd_, EM_SETSEL, 0, -1);
			SendMessage(hWnd_, EM_SETSEL, -1, -1);
			SendMessage(hWnd_, EM_SCROLLCARET, 0, 0);
			UpdateWindow(hWnd_);
		}
	}
}