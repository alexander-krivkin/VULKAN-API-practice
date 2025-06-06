#pragma once

#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <CommCtrl.h>

#include <iostream>
#include <string>
#include <memory>

#include "wexception.h"
#include "LoggerSystem.h"
#include "VulkanSystem.h"


namespace ak
{
#ifdef _DEBUG
	//VkResult createDebugUtilsMessengerEXT(
	//	VkInstance instance,
	//	const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
	//	const VkAllocationCallbacks* pAllocator,
	//	VkDebugUtilsMessengerEXT* pDebugMessenger);

	//void destroyDebugUtilsMessengerEXT(
	//	VkInstance instance,
	//	VkDebugUtilsMessengerEXT debugMessenger,
	//	const VkAllocationCallbacks* pAllocator);
#endif // _DEBUG


	class Application final
	{
	public:
		Application() {};
		explicit Application(const Application& obj) = delete;
		~Application() {};
		Application& operator=(const Application& obj) = delete;

		int run();

	private:
		void initWin32();
		void initLoggerSystem(const HWND& hWnd);
		void initVulkanSystem();
		void mainLoop() const;

		static LRESULT CALLBACK parentProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		LRESULT CALLBACK parentWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		static LRESULT CALLBACK workspaceProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		LRESULT CALLBACK workspaceWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#ifdef _DEBUG
		static LRESULT CALLBACK debugProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		LRESULT CALLBACK debugWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#endif // _DEBUG

		WNDCLASSEX parentClass_{}, workspaceClass_{};
		HWND hParent_{}, hWorkspace_{}, hToolbar_{}, hState_{}, hCommandEdit_{};
#ifdef _DEBUG
		WNDCLASSEX debugClass_{};
		HWND hDebug_{}, hDebugEdit_{};
		DWORD debugSleepDelay_{ 0 };
#endif // _DEBUG
		HBRUSH hWhiteBrush_{}, hDarkGrayBrush_{};
		HCURSOR hCursorArrow_{}, hCursorBeam_{}, hCursorSizeAll_{},
			hCursorSizeNWSE_{}, hCursorSizeNESW_{}, hCursorSizeWE_{}, hCursorSizeNS_{};
		HMENU hMenu_{}, hMenuMain_{}, hMenuEdit_{}, hMenuView_{}, hMenuStyles_{},
			hMenuDrawPrimitives_{}, hMenuDrawSurfaces_{}, hMenuDrawSolids_{},
			hMenuModifyPrimitives_{}, hMenuModifySurfaces_{}, hMenuModifySolids_{},
			hMenuAnnotation_{}, hMenuTools_{}, hMenuInfo_{};
		HFONT hFont_{}, hStateFont_{};

		enum class ELEM
		{
			CTRL_EDIT_ID,
			CTRL_DEBUG_EDIT_ID
		};

#ifdef _DEBUG
		const std::wstring parentName_{ L"ak Vulkan API practice (Debug mode)" };
		const std::wstring debugName_{ L"Debug" }, debugClassName_{ L"Debug" };
#else
		const std::wstring parentName_{ L"ak Vulkan API practice" };
#endif // _DEBUG
		const std::wstring parentClassName_{ L"ak Vulkan API practice" };
		const std::wstring workspaceName_{ L"Vulkan workspace" }, workspaceClassName_{ L"Vulkan workspace" };

		RECT parentRect_{ 600,150,1500,900 }, parentClientRect_{};
		RECT workspaceRect_{}, workspaceClientRect_{};
		LONG workspaceHOffset_{}, workspaceTopOffset_{}, workspaceBottomOffset_{};
		POINT screenWorkspaceCenterPoint_{};

		LONG toolbarHeight_{ 50 }, stateWidth_{ 700 }, stateHeight_{ 30 }, commandEditHeight_{ 150 };
		LONG stateMargin_{ 5 }, workspaceMargin_{ 0 }, commandEditMargin_{ 0 };

#ifdef _DEBUG
		RECT debugRect_{ 1550,150,2100,900 }, debugClientRect_{};
		LONG debugEditMargin_{ 0 };
#endif // _DEBUG

		bool cursorVisible_{ true };
		POINT screenCursorPoint_{}, workspaceCenteredCursorPoint_{};
		int cursorPosition_{};


#ifdef _DEBUG
		std::shared_ptr<LoggerSystem> shpLoggerSystem_{};
#endif // _DEBUG
		std::unique_ptr<VulkanSystem> upVulkanSystem_{};
	};
}