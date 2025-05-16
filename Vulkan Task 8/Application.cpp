#include "Application.h"


namespace ak
{
	constexpr int targetFPS{ 120 };
	constexpr long long drawFrameMicrosecondsDuration{ 1'000'000 / targetFPS };

	int Application::run()
	{
		try
		{
			initWin32_();
#ifdef _DEBUG
			initLoggingSystem_(hDebugEdit_);
#endif // _DEBUG
			initVulkanSystem_();
			mainLoop_();
		}
		catch (const std::exception& ex)
		{
			SetCursor(hCursorArrow_);
			ShowCursor(TRUE);
			MessageBox(hParent_, stringToWstring(ex.what()).c_str(), L"Исключение системное", MB_OK | MB_SYSTEMMODAL);
			return EXIT_FAILURE;
		}
		catch (const wexception& ex)
		{
			SetCursor(hCursorArrow_);
			ShowCursor(TRUE);
			MessageBox(hParent_, ex.what(), L"Исключение программное", MB_OK | MB_SYSTEMMODAL);
			return ex.code();
		}
		return EXIT_SUCCESS;
	}

	void Application::postMessage_(LogLevelFlag logLevel, const std::wstring& message)
	{
#ifdef _DEBUG
		shpLoggingSystem_->postLogMessage(logLevel, message);
#else
		UNREFERENCED_PARAMETER(logLevel);
		UNREFERENCED_PARAMETER(message);
#endif // _DEBUG
	}

	void Application::initWin32_()
	{
		hWhiteBrush_ = CreateSolidBrush(RGB(255, 255, 255));
		hDarkGrayBrush_ = CreateSolidBrush(RGB(35, 35, 35));

		hCursorArrow_ = LoadCursor(nullptr, IDC_ARROW);
		hCursorBeam_ = LoadCursor(nullptr, IDC_IBEAM);
		hCursorSizeAll_ = LoadCursor(nullptr, IDC_SIZEALL);
		hCursorSizeNWSE_ = LoadCursor(nullptr, IDC_SIZENWSE);
		hCursorSizeNESW_ = LoadCursor(nullptr, IDC_SIZENESW);
		hCursorSizeWE_ = LoadCursor(nullptr, IDC_SIZEWE);
		hCursorSizeNS_ = LoadCursor(nullptr, IDC_SIZENS);

		hMenu_ = CreateMenu();
		hMenuMain_ = CreateMenu();
		hMenuEdit_ = CreateMenu();
		hMenuView_ = CreateMenu();
		hMenuStyles_ = CreateMenu();
		hMenuDrawPrimitives_ = CreateMenu();
		hMenuDrawSurfaces_ = CreateMenu();
		hMenuDrawSolids_ = CreateMenu();
		hMenuModifyPrimitives_ = CreateMenu();
		hMenuModifySurfaces_ = CreateMenu();
		hMenuModifySolids_ = CreateMenu();
		hMenuAnnotation_ = CreateMenu();
		hMenuTools_ = CreateMenu();
		hMenuInfo_ = CreateMenu();

		AppendMenu(hMenu_, MF_STRING, reinterpret_cast<UINT_PTR>(hMenuMain_), L"Главное");
		AppendMenu(hMenu_, MF_STRING, reinterpret_cast<UINT_PTR>(hMenuEdit_), L"Правка");
		AppendMenu(hMenu_, MF_STRING, reinterpret_cast<UINT_PTR>(hMenuView_), L"Вид");
		AppendMenu(hMenu_, MF_STRING, reinterpret_cast<UINT_PTR>(hMenuStyles_), L"Стили");
		AppendMenu(hMenu_, MF_STRING, reinterpret_cast<UINT_PTR>(hMenuDrawPrimitives_), L"Примитивы");
		AppendMenu(hMenu_, MF_STRING, reinterpret_cast<UINT_PTR>(hMenuDrawSurfaces_), L"Поверхности");
		AppendMenu(hMenu_, MF_STRING, reinterpret_cast<UINT_PTR>(hMenuDrawSolids_), L"Твёрдые тела");
		AppendMenu(hMenu_, MF_STRING, reinterpret_cast<UINT_PTR>(hMenuModifyPrimitives_), L"Ред примитивов");
		AppendMenu(hMenu_, MF_STRING, reinterpret_cast<UINT_PTR>(hMenuModifySurfaces_), L"Ред поверхностей");
		AppendMenu(hMenu_, MF_STRING, reinterpret_cast<UINT_PTR>(hMenuModifySolids_), L"Ред твёрдых тел");
		AppendMenu(hMenu_, MF_STRING, reinterpret_cast<UINT_PTR>(hMenuAnnotation_), L"Аннотации");
		AppendMenu(hMenu_, MF_STRING, reinterpret_cast<UINT_PTR>(hMenuTools_), L"Средства");
		AppendMenu(hMenu_, MF_STRING, reinterpret_cast<UINT_PTR>(hMenuInfo_), L"Инфо");

		hFont_ = CreateFont(20, 0, 0, 0, FW_REGULAR, 0, 0, 0, DEFAULT_CHARSET,
			OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Consolas");
		hStateFont_ = CreateFont(22, 0, 0, 0, FW_REGULAR, 0, 0, 0, DEFAULT_CHARSET,
			OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Consolas");


		// 1. Родительское окно с меню
		parentClass_.cbSize = sizeof(WNDCLASSEX);
		parentClass_.hInstance = GetModuleHandle(nullptr);
		parentClass_.lpszClassName = parentClassName_.c_str();
		parentClass_.lpfnWndProc = Application::parentProc_;
		parentClass_.style = CS_OWNDC;
		parentClass_.hCursor = nullptr;
		parentClass_.hIcon = LoadIcon(nullptr, IDI_ASTERISK);
		parentClass_.hIconSm = nullptr;
		parentClass_.hbrBackground = hWhiteBrush_;
		parentClass_.lpszMenuName = nullptr;

		if (!RegisterClassEx(&parentClass_))
		{
			throw wexception(L"Application::initWin32_() : Невозможно зарегистрировать класс окна приложения");
		}

		parentClientRect_ = { 0, 0, parentRect_.right - parentRect_.left, parentRect_.bottom - parentRect_.top };
		AdjustWindowRect(&parentRect_, WS_OVERLAPPEDWINDOW, false);

		if (!(hParent_ = CreateWindowEx(
			WS_EX_LAYERED,
			parentClassName_.c_str(), parentName_.c_str(),
			WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
			parentRect_.left, parentRect_.top, parentRect_.right - parentRect_.left, parentRect_.bottom - parentRect_.top,
			nullptr, hMenu_, nullptr, this)))
		{
			throw wexception(L"Application::initWin32_() : Невозможно создать окно приложения");
		}

		SetLayeredWindowAttributes(hParent_, RGB(255, 255, 255), 255, LWA_ALPHA);
		SendMessage(hParent_, WM_SETFONT, reinterpret_cast<WPARAM>(hFont_), TRUE);
		ShowWindow(hParent_, SW_SHOWNORMAL);
		GetClientRect(hParent_, &parentClientRect_);
		SetCursor(hCursorArrow_);
		UpdateWindow(hParent_);


		// 2. Панель инструментов
		if (!(hToolbar_ = CreateWindowEx(
			WS_EX_LAYERED,
			TOOLBARCLASSNAME, L"Панель инструментов",
			WS_CHILD | WS_VISIBLE | TBSTYLE_WRAPABLE,
			0, 0, 0, toolbarHeight_,
			hParent_, nullptr, parentClass_.hInstance, nullptr)))
		{
			throw wexception(L"Application::initWin32_() : Невозможно создать панель инструментов");
		}

		SetLayeredWindowAttributes(hToolbar_, RGB(255, 255, 255), 255, LWA_ALPHA);
		SendMessage(hToolbar_, WM_SETFONT, reinterpret_cast<WPARAM>(hFont_), TRUE);
		ShowWindow(hToolbar_, SW_SHOWNORMAL);
		UpdateWindow(hToolbar_);


		// 3. Тектовое поле статуса
		if (!(hState_ = CreateWindowEx(
			WS_EX_LAYERED,
			WC_EDIT, L"",
			WS_CHILD | WS_VISIBLE | WS_DISABLED | ES_READONLY,
			stateMargin_ + workspaceMargin_, stateMargin_ + workspaceMargin_ + toolbarHeight_,
			stateWidth_, stateHeight_,
			hParent_, reinterpret_cast<HMENU>(ElementFlag::eCtrlEdit), nullptr, nullptr)))
		{
			throw wexception(L"Application::initWin32_() : Невозможно создать текстовое поле");
		}

		SetLayeredWindowAttributes(hState_, RGB(255, 255, 255), 196, LWA_ALPHA);
		SendMessage(hState_, WM_SETFONT, reinterpret_cast<WPARAM>(hStateFont_), TRUE);
		ShowWindow(hState_, SW_SHOWNORMAL);
		UpdateWindow(hState_);


		// 4. Тектовое поле нижнее
		if (!(hCommandEdit_ = CreateWindowEx(
			WS_EX_LAYERED,
			WC_EDIT, L"",
			WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
			commandEditMargin_, parentClientRect_.bottom - (commandEditHeight_ + commandEditMargin_),
			parentClientRect_.right - 2 * commandEditMargin_, commandEditHeight_,
			hParent_, reinterpret_cast<HMENU>(ElementFlag::eCtrlEdit), nullptr, nullptr)))
		{
			throw wexception(L"Application::initWin32_() : Невозможно создать текстовое поле");
		}

		SetLayeredWindowAttributes(hCommandEdit_, RGB(255, 255, 255), 255, LWA_ALPHA);
		SendMessage(hCommandEdit_, WM_SETFONT, reinterpret_cast<WPARAM>(hFont_), TRUE);
		ShowWindow(hCommandEdit_, SW_SHOWNORMAL);
		UpdateWindow(hCommandEdit_);


		// 5. Окно рабочего пространства
		workspaceClass_.cbSize = sizeof(WNDCLASSEX);
		workspaceClass_.hInstance = GetModuleHandle(nullptr);
		workspaceClass_.lpszClassName = workspaceClassName_.c_str();
		workspaceClass_.lpfnWndProc = Application::workspaceProc_;
		workspaceClass_.style = CS_HREDRAW | CS_VREDRAW;
		workspaceClass_.hCursor = nullptr;
		workspaceClass_.hIcon = nullptr;
		workspaceClass_.hIconSm = nullptr;
		workspaceClass_.hbrBackground = nullptr;
		workspaceClass_.lpszMenuName = nullptr;

		if (!RegisterClassEx(&workspaceClass_))
		{
			throw wexception(L"Application::initWin32_() : Невозможно зарегистрировать класс окна рабочего пространства");
		}

		if (!(hWorkspace_ = CreateWindowEx(
			WS_EX_LAYERED,
			workspaceClassName_.c_str(), workspaceName_.c_str(),
			WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
			workspaceMargin_, workspaceMargin_ + toolbarHeight_, parentClientRect_.right - 2 * workspaceMargin_,
			parentClientRect_.bottom - (2 * workspaceMargin_ + toolbarHeight_ + commandEditMargin_ + commandEditHeight_),
			hParent_, nullptr, nullptr, this)))
		{
			throw wexception(L"Application::initWin32_() : Невозможно создать окно рабочего пространства");
		}

		SetLayeredWindowAttributes(hWorkspace_, RGB(255, 255, 255), 255, LWA_ALPHA);
		SendMessage(hWorkspace_, WM_SETFONT, reinterpret_cast<WPARAM>(hFont_), TRUE);
		ShowWindow(hWorkspace_, SW_SHOWNORMAL);
		UpdateWindow(hWorkspace_);
		GetWindowRect(hWorkspace_, &workspaceRect_);
		GetClientRect(hWorkspace_, &workspaceClientRect_);

		screenWorkspaceCenterPoint_ = {
			(workspaceRect_.left + workspaceRect_.right) / 2,
			(workspaceRect_.top + workspaceRect_.bottom) / 2
		};


#ifdef _DEBUG
		// 6. Независимое окно отладки
		debugClass_.cbSize = sizeof(WNDCLASSEX);
		debugClass_.hInstance = GetModuleHandle(nullptr);
		debugClass_.lpszClassName = debugClassName_.c_str();
		debugClass_.lpfnWndProc = Application::debugProc_;
		debugClass_.style = CS_HREDRAW | CS_VREDRAW;
		debugClass_.hCursor = hCursorArrow_;
		debugClass_.hIcon = nullptr;
		debugClass_.hIconSm = nullptr;
		debugClass_.hbrBackground = hWhiteBrush_;
		debugClass_.lpszMenuName = nullptr;

		if (!RegisterClassEx(&debugClass_))
		{
			throw wexception(L"Application::initWin32_() : Невозможно зарегистрировать класс окна отладки");
		}

		debugClientRect_ = { 0, 0, debugRect_.right - debugRect_.left, debugRect_.bottom - debugRect_.top };
		AdjustWindowRect(&debugRect_, WS_OVERLAPPEDWINDOW, false);

		if (!(hDebug_ = CreateWindowEx(
			WS_EX_LAYERED,
			debugClassName_.c_str(), debugName_.c_str(),
			WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
			debugRect_.left, debugRect_.top, debugRect_.right - debugRect_.left, debugRect_.bottom - debugRect_.top,
			nullptr, nullptr, nullptr, this)))
		{
			throw wexception(L"Application::initWin32_() : Невозможно создать окно отладки");
		}

		SetLayeredWindowAttributes(hDebug_, RGB(255, 255, 255), 255, LWA_ALPHA);
		SendMessage(hDebug_, WM_SETFONT, reinterpret_cast<WPARAM>(hFont_), TRUE);
		ShowWindow(hDebug_, SW_SHOWNORMAL);
		UpdateWindow(hDebug_);
		GetClientRect(hDebug_, &debugClientRect_);


		// 7. Тектовое поле окна отладки
		if (!(hDebugEdit_ = CreateWindowEx(
			WS_EX_LAYERED,
			WC_EDIT, L"",
			WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
			debugEditMargin_, debugEditMargin_,
			debugClientRect_.right - 2 * debugEditMargin_, debugClientRect_.bottom - 2 * debugEditMargin_,
			hDebug_, reinterpret_cast<HMENU>(ElementFlag::eCtrlDebugEdit), nullptr, nullptr)))
		{
			throw wexception(L"Application::initWin32_() : Невозможно создать текстовое поле окна отладки");
		}

		SetLayeredWindowAttributes(hDebugEdit_, RGB(255, 255, 255), 255, LWA_ALPHA);
		SendMessage(hDebugEdit_, WM_SETFONT, reinterpret_cast<WPARAM>(hFont_), TRUE);
		ShowWindow(hDebugEdit_, SW_SHOWNORMAL);
		UpdateWindow(hDebugEdit_);
#endif // _DEBUG
	}

#ifdef _DEBUG
	void Application::initLoggingSystem_(const HWND& hWnd)
	{
		shpLoggingSystem_ = std::shared_ptr<LoggingSystem>(new LoggingSystem(hWnd));
		shpLoggingSystem_->resetTimer();
	}
#endif // _DEBUG

	void Application::initVulkanSystem_()
	{
		upVulkanSystem_ = std::unique_ptr<VulkanSystem>(new VulkanSystem);

#ifdef _DEBUG
		upVulkanSystem_->init(hWorkspace_, shpLoggingSystem_);
#else
		upVulkanSystem_->init(hWorkspace_);
#endif // _DEBUG
	}

	void Application::mainLoop_()
	{
		tStartFPS_ = std::chrono::steady_clock::now();

		MSG msg{};
		while (true)
		{
			tStart_ = std::chrono::steady_clock::now();

			PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE);
			if (msg.message == WM_QUIT)
			{
				break;
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			//RedrawWindow(hWorkspace_, nullptr, nullptr, RDW_INTERNALPAINT);
			upVulkanSystem_->draw();
			FPS_++;

			tNow_ = std::chrono::steady_clock::now();
			tElapsedFPS_ = tNow_ - tStartFPS_;

			if (std::chrono::duration_cast<std::chrono::milliseconds>(tElapsedFPS_).count() >= 1'000)
			{
				hStateText_ = L"FPS: " + std::to_wstring(FPS_);
				SetWindowText(hState_, hStateText_.c_str());
				UpdateWindow(hState_);
				tStartFPS_ = std::chrono::steady_clock::now();
				FPS_ = 0;
			}
			else
			{
				tNow_ = std::chrono::steady_clock::now();
				tElapsed_ = tNow_ - tStart_;

				std::this_thread::sleep_for(std::chrono::microseconds(drawFrameMicrosecondsDuration -
					std::chrono::duration_cast<std::chrono::microseconds>(tElapsed_).count()));
			}
		}

		upVulkanSystem_->waitIdle();
	}



	LRESULT CALLBACK ak::Application::parentProc_(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		Application* pApplication;
		if (uMsg == WM_NCCREATE)
		{
			pApplication = static_cast<Application*>(reinterpret_cast<CREATESTRUCTW*>(lParam)->lpCreateParams);
			SetLastError(0);
			if (!SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pApplication)))
			{
				if (GetLastError() != 0)
					return false;
			}
		}
		else
		{
			pApplication = reinterpret_cast<Application*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
		}

		if (pApplication)
		{
			pApplication->hParent_ = hWnd;
			return pApplication->parentWndProc_(hWnd, uMsg, wParam, lParam);
		}
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}


	LRESULT CALLBACK ak::Application::parentWndProc_(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
		case WM_MOVE:
		{
			GetWindowRect(hWnd, &parentRect_);
			GetWindowRect(hWorkspace_, &workspaceRect_);

			screenWorkspaceCenterPoint_ = {
				(workspaceRect_.left + workspaceRect_.right) / 2,
				(workspaceRect_.top + workspaceRect_.bottom) / 2
			};
		}
		return 0;
		case WM_SIZE:
		{
			GetWindowRect(hWnd, &parentRect_);
			GetClientRect(hWnd, &parentClientRect_);
			SetWindowPos(hToolbar_, nullptr,
				0, 0, parentClientRect_.right, toolbarHeight_,
				SWP_NOZORDER | SWP_NOOWNERZORDER);
			SetWindowPos(hCommandEdit_, nullptr,
				commandEditMargin_, parentClientRect_.bottom - (commandEditHeight_ + commandEditMargin_),
				parentClientRect_.right - 2 * commandEditMargin_, commandEditHeight_,
				SWP_NOZORDER | SWP_NOOWNERZORDER);
			SetWindowPos(hWorkspace_, nullptr,
				workspaceMargin_, workspaceMargin_ + toolbarHeight_, parentClientRect_.right - 2 * workspaceMargin_,
				parentClientRect_.bottom - (2 * workspaceMargin_ + toolbarHeight_ + commandEditMargin_ + commandEditHeight_),
				SWP_NOZORDER | SWP_NOOWNERZORDER);
			GetWindowRect(hWorkspace_, &workspaceRect_);
			GetClientRect(hWorkspace_, &workspaceClientRect_);

			screenWorkspaceCenterPoint_ = {
				(workspaceRect_.left + workspaceRect_.right) / 2,
				(workspaceRect_.top + workspaceRect_.bottom) / 2
			};

			/*if (wParam == SIZE_MINIMIZED) appVulkan_.pause();
			if (wParam == SIZE_RESTORED) appVulkan_.resume();*/

			//upVulkanSystem_->resize();
		}
		return 0;
		case WM_SETCURSOR:
		{
			if ((reinterpret_cast<HWND>(wParam) == hWorkspace_) && (cursorVisible_))
			{
				ShowCursor(FALSE);
				cursorVisible_ = false;
			}
			if ((reinterpret_cast<HWND>(wParam) != hWorkspace_) && (!cursorVisible_))
			{
				ShowCursor(TRUE);
				cursorVisible_ = true;
			}
			if ((reinterpret_cast<HWND>(wParam) != hWorkspace_) && (reinterpret_cast<HWND>(wParam) != hCommandEdit_))
			{
				GetCursorPos(&screenCursorPoint_);
				cursorPosition_ = static_cast<int>(DefWindowProc(hWnd, WM_NCHITTEST, NULL,
					MAKELPARAM(screenCursorPoint_.x, screenCursorPoint_.y)));

				switch (cursorPosition_)
				{
				case HTTOPLEFT: SetCursor(hCursorSizeNWSE_); break;
				case HTBOTTOMRIGHT: SetCursor(hCursorSizeNWSE_); break;
				case HTTOPRIGHT: SetCursor(hCursorSizeNESW_); break;
				case HTBOTTOMLEFT: SetCursor(hCursorSizeNESW_); break;
				case HTLEFT: SetCursor(hCursorSizeWE_); break;
				case HTRIGHT: SetCursor(hCursorSizeWE_); break;
				case HTTOP: SetCursor(hCursorSizeNS_); break;
				case HTBOTTOM: SetCursor(hCursorSizeNS_); break;
				default: SetCursor(hCursorArrow_);
				}
			}
		}
		return 0;
		case WM_DESTROY:
		{
#ifdef _DEBUG
			Sleep(debugSleepDelay_);
#endif // _DEBUG
			PostQuitMessage(EXIT_SUCCESS);
		}
		return 0;
		}

		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}


	LRESULT CALLBACK ak::Application::workspaceProc_(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		Application* pApplication;
		if (uMsg == WM_NCCREATE)
		{
			pApplication = static_cast<Application*>(reinterpret_cast<CREATESTRUCTW*>(lParam)->lpCreateParams);
			SetLastError(0);
			if (!SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pApplication)))
			{
				if (GetLastError() != 0)
					return false;
			}
		}
		else
		{
			pApplication = reinterpret_cast<Application*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
		}

		if (pApplication)
		{
			pApplication->hWorkspace_ = hWnd;
			return pApplication->workspaceWndProc_(hWnd, uMsg, wParam, lParam);
		}
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}


	LRESULT CALLBACK ak::Application::workspaceWndProc_(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
		case WM_SETCURSOR:
		{
			GetCursorPos(&screenCursorPoint_);
			cursorPosition_ = static_cast<int>(DefWindowProc(hWnd, WM_NCHITTEST, NULL,
				MAKELPARAM(screenCursorPoint_.x, screenCursorPoint_.y)));

			workspaceCenteredCursorPoint_ = {
				screenCursorPoint_.x - screenWorkspaceCenterPoint_.x,
				screenCursorPoint_.y - screenWorkspaceCenterPoint_.y
			};
		}
		break;
		case WM_PAINT:
		{
			// upVulkanSystem_->draw();
		}
		break;
		}
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}


#ifdef _DEBUG
	LRESULT CALLBACK ak::Application::debugProc_(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		Application* pApplication;
		if (uMsg == WM_NCCREATE)
		{
			pApplication = static_cast<Application*>(reinterpret_cast<CREATESTRUCTW*>(lParam)->lpCreateParams);
			SetLastError(0);
			if (!SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pApplication)))
			{
				if (GetLastError() != 0)
					return false;
			}
		}
		else
		{
			pApplication = reinterpret_cast<Application*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
		}

		if (pApplication)
		{
			pApplication->hDebug_ = hWnd;
			return pApplication->debugWndProc_(hWnd, uMsg, wParam, lParam);
		}
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}


	LRESULT CALLBACK ak::Application::debugWndProc_(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
		case WM_MOVE:
		{
			GetWindowRect(hWnd, &debugRect_);
			GetClientRect(hWnd, &debugClientRect_);
		}
		return 0;
		case WM_SIZE:
		{
			GetWindowRect(hWnd, &debugRect_);
			GetClientRect(hWnd, &debugClientRect_);

			SetWindowPos(hDebugEdit_, nullptr,
				debugEditMargin_, debugEditMargin_,
				debugClientRect_.right - 2 * debugEditMargin_, debugClientRect_.bottom - 2 * debugEditMargin_,
				SWP_NOZORDER | SWP_NOOWNERZORDER);
		}
		return 0;
		case WM_DESTROY:
		{
			PostQuitMessage(EXIT_SUCCESS);
		}
		return 0;
		}
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
#endif // _DEBUG
}