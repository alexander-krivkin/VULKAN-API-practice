#include "LoggingSystem.h"


namespace ak
{
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

	std::array<std::wstring, 7> LoggerLevelName{ L"APP INFO: ", L"APP WARNING: ", L"APP ERROR: ",
		L"VULKAN VERBOSE: ", L"VULKAN INFO: ", L"VULKAN WARNING: ", L"VULKAN ERROR: " };

	void LoggingSystem::postLogMessage(LogLevel logLevel, const char* message)
	{
		postLogMessage(logLevel, stringToWstring(message));
	}

	void LoggingSystem::postLogMessage(LogLevel logLevel, const std::string& message)
	{
		postLogMessage(logLevel, stringToWstring(message));
	}

	void LoggingSystem::postLogMessage(LogLevel logLevel, const std::wstring& message)
	{
		tNow_ = std::chrono::steady_clock::now();
		tElapsed_ = tNow_ - tStart_;

		std::vector<WCHAR> tmpMsg(GetWindowTextLength(hWnd_) + 1);
		GetWindowText(hWnd_, &tmpMsg[0], GetWindowTextLength(hWnd_) + 1);

		std::wstring msg = std::wstring(&tmpMsg[0]);
		msg += L"[" + std::to_wstring(std::chrono::duration_cast<std::chrono::milliseconds>(tElapsed_).count());
		msg += L" ms]   " + LoggerLevelName[static_cast<size_t>(logLevel)] + L"\r\n";
		msg += message + L"\r\n";

		SetWindowText(hWnd_, msg.c_str());
		SendMessage(hWnd_, EM_SETSEL, 0, -1);
		SendMessage(hWnd_, EM_SETSEL, -1, -1);
		SendMessage(hWnd_, EM_SCROLLCARET, 0, 0);
		UpdateWindow(hWnd_);
	}
}