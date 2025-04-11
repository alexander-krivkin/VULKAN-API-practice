#include "LoggerSystem.h"


namespace ak
{
	std::array<std::wstring, 3> LoggerLevelName{ L"INFO: ", L"WARN: ", L"ERROR: " };

	void LoggerSystem::postMessage(LoggerLevel loggerLevel, const std::wstring& message)
	{
		tNow_ = std::chrono::steady_clock::now();
		auto tElapsed = tNow_ - tStart_;

		std::vector<WCHAR> tmpMsg(GetWindowTextLength(hWnd_) + 1);
		GetWindowText(hWnd_, &tmpMsg[0], GetWindowTextLength(hWnd_) + 1);

		std::wstring msg = std::to_wstring(std::chrono::duration_cast<std::chrono::milliseconds>(tElapsed).count());
		msg += L" ms | " + std::wstring(&tmpMsg[0]);
		msg += LoggerLevelName[static_cast<size_t>(loggerLevel)] + message + L"\r\n";

		SetWindowText(hWnd_, msg.c_str());
		//SendMessage(hWnd_, EM_SETSEL, 0, -1);
		//SendMessage(hWnd_, EM_SETSEL, -1, -1);
		//SendMessage(hWnd_, EM_SCROLLCARET, 0, 0);
		UpdateWindow(hWnd_);
	}
}