#include "LoggerSystem.h"


namespace ak
{
	std::wstring stringToWstring(const char* str)
	{
		return std::wstring(str, str + strlen(str));
	}

	std::wstring stringToWstring(const std::string& str)
	{
		return std::wstring(str.c_str(), str.c_str() + str.length());
	}

	std::array<std::wstring, 7> LoggerLevelName{ L"APP INFO: ", L"APP WARNING: ", L"APP ERROR: ",
		L"VULKAN VERBOSE: ", L"VULKAN INFO: ", L"VULKAN WARNING: ", L"VULKAN ERROR: " };

	void LoggerSystem::postMessage(LoggerLevel loggerLevel, const std::wstring& message)
	{
		tNow_ = std::chrono::steady_clock::now();
		auto tElapsed = tNow_ - tStart_;

		std::vector<WCHAR> tmpMsg(GetWindowTextLength(hWnd_) + 1);
		GetWindowText(hWnd_, &tmpMsg[0], GetWindowTextLength(hWnd_) + 1);

		std::wstring msg = std::wstring(&tmpMsg[0]);
		msg += std::to_wstring(std::chrono::duration_cast<std::chrono::milliseconds>(tElapsed).count());
		msg += L" ms | " + LoggerLevelName[static_cast<size_t>(loggerLevel)] + message + L"\r\n";

		SetWindowText(hWnd_, msg.c_str());
		SendMessage(hWnd_, EM_SETSEL, 0, -1);
		SendMessage(hWnd_, EM_SETSEL, -1, -1);
		SendMessage(hWnd_, EM_SCROLLCARET, 0, 0);
		UpdateWindow(hWnd_);
	}
}