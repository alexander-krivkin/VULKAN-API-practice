#pragma once

#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <CommCtrl.h>

#include <iostream>
#include <string>
#include <memory>
#include <thread>
#include <chrono>

#include "wexception.h"
#include "LoggingSystem.h"
#include "VulkanSystem.h"


namespace ak
{
	enum class ElementFlag
	{
		eCtrlEdit = 0,
		eCtrlDebugEdit = 1
	};


	class Application final
	{
	public:
		Application() {};
		explicit Application(const Application& obj) = delete;
		~Application() {};
		Application& operator=(const Application& obj) = delete;

		int run();

	private:
		void postMessage_(LogLevelFlagBits logLevel, const std::wstring& message);
		void initWin32_();
#ifdef _DEBUG
		void initLoggingSystem_(const HWND& hWnd, int logFlags, std::string logFilename = "");
#endif // _DEBUG
		void initVulkanSystem_();
		void mainLoop_();

		static LRESULT CALLBACK parentProc_(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		LRESULT CALLBACK parentWndProc_(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		static LRESULT CALLBACK workspaceProc_(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		LRESULT CALLBACK workspaceWndProc_(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#ifdef _DEBUG
		static LRESULT CALLBACK debugProc_(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		LRESULT CALLBACK debugWndProc_(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#endif // _DEBUG

		WNDCLASSEX parentClass_{}, workspaceClass_{};
		HWND hParent_{}, hWorkspace_{}, hToolbar_{}, hState_{}, hCommandEdit_{};
#ifdef _DEBUG
		WNDCLASSEX debugClass_{};
		HWND hDebug_{}, hDebugEdit_{};
#endif // _DEBUG
		HBRUSH hWhiteBrush_{}, hDarkGrayBrush_{};
		HCURSOR hCursorArrow_{}, hCursorBeam_{}, hCursorSizeAll_{},
			hCursorSizeNWSE_{}, hCursorSizeNESW_{}, hCursorSizeWE_{}, hCursorSizeNS_{};
		HMENU hMenu_{}, hMenuMain_{}, hMenuEdit_{}, hMenuView_{}, hMenuStyles_{},
			hMenuDrawPrimitives_{}, hMenuDrawSurfaces_{}, hMenuDrawSolids_{},
			hMenuModifyPrimitives_{}, hMenuModifySurfaces_{}, hMenuModifySolids_{},
			hMenuAnnotation_{}, hMenuTools_{}, hMenuInfo_{};
		HFONT hFont_{}, hStateFont_{};

#ifdef _DEBUG
		const std::wstring parentName_{ L"ak Vulkan API practice (Debug mode)" };
		const std::wstring debugName_{ L"Debug" }, debugClassName_{ L"Debug" };
#else
		const std::wstring parentName_{ L"ak Vulkan API practice" };
#endif // _DEBUG
		const std::wstring parentClassName_{ L"ak Vulkan API practice" };
		const std::wstring workspaceName_{ L"Vulkan workspace" }, workspaceClassName_{ L"Vulkan workspace" };

		std::unique_ptr<VulkanSystem> upVulkanSystem_{};
#ifdef _DEBUG
		std::shared_ptr<LoggingSystem> shpLoggingSystem_{};
#endif // _DEBUG

		std::chrono::steady_clock::time_point tStartFPS_{};
		std::chrono::steady_clock::time_point tStart_{};
		std::chrono::steady_clock::time_point tNow_{};
		std::chrono::nanoseconds tElapsedFPS_{};
		std::chrono::nanoseconds tElapsed_{};

		struct
		{
			RECT parentRect_{ 50,50,1550,1000 }, parentClientRect_{};
			RECT workspaceRect_{}, workspaceClientRect_{};
			LONG workspaceHOffset_{}, workspaceTopOffset_{}, workspaceBottomOffset_{};
			POINT screenWorkspaceCenterPoint_{};

			LONG toolbarHeight_{ 50 }, stateWidth_{ 1000 }, stateHeight_{ 30 }, commandEditHeight_{ 100 };
			LONG stateMargin_{ 5 }, workspaceMargin_{ 0 }, commandEditMargin_{ 0 };

#ifdef _DEBUG
			RECT debugRect_{ 1575,50,2500,500 }, debugClientRect_{};
			LONG debugEditMargin_{ 0 };
			DWORD debugSleepDelay_{ 0 };
			int logFlags = LogLevelFlagBits::eAppWarning | LogLevelFlagBits::eAppError | LogLevelFlagBits::eVulkanWarning | LogLevelFlagBits::eVulkanError;
#endif // _DEBUG

			int targetFPS{ 60 };
			long long drawFrameMicrosecondsDuration{ 1'000'000 / targetFPS };
			std::wstring hStateText_{};
			bool cursorVisible_{ true };
			POINT screenCursorPoint_{}, workspaceCenteredCursorPoint_{};
			int cursorPosition_{};
		} state_{};
	};
}