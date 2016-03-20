//=====================================================
// Main_Win32.cpp
// by Andrew Socha
//=====================================================

#include "Engine/Core/EngineCore.hpp"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "TheApp.hpp"
#include "Engine\Console\ConsoleCommands.hpp"
#include "Engine\Threads\JobManager.hpp"
#include "Engine\Core\Assert.hpp"
#include "Engine\Core\SignpostMemoryManager.hpp"

TheApp* s_theApp = NULL;

///=====================================================
/// 
///=====================================================
LRESULT __stdcall GameMessageProcessingFunction(HWND windowHandle, UINT messageID, WPARAM wParam, LPARAM lParam){
	switch (messageID){
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE){
			PostQuitMessage(0);
		}
		break;
	case WM_KEYUP:
		break;
	default:
		break;
	}
	return DefWindowProc(windowHandle, messageID, wParam, lParam);
}

///=====================================================
/// 
///=====================================================
HWND CreateAppWindow(HINSTANCE thisAppInstance, int nShowCmd){
	LPCWSTR GAME_NAME = TEXT("Dagger");

	WNDCLASSEX WindowDescription;
	memset(&WindowDescription, 0, sizeof(WNDCLASSEX));
	WindowDescription.cbSize = sizeof(WNDCLASSEX);
	WindowDescription.hInstance = thisAppInstance;
	WindowDescription.lpszClassName = GAME_NAME;
	WindowDescription.lpfnWndProc = GameMessageProcessingFunction; //processing function for Windows to call
	WindowDescription.style = CS_HREDRAW | CS_OWNDC | CS_VREDRAW;

	RegisterClassEx(&WindowDescription);

	SetProcessDPIAware();
	HWND desktopWindow = GetDesktopWindow();
	RECT desktopWindowRect;
	GetClientRect(desktopWindow, &desktopWindowRect); //can use to resize window proportionally to screen resolution

	RECT windowRect;
	windowRect.left = 100;
	windowRect.right = 1700;
	windowRect.top = 50;
	windowRect.bottom = 950;

	DWORD windowStyleFlags = WS_OVERLAPPEDWINDOW;
	DWORD windowStyleFlagsEx = WS_EX_APPWINDOW;
	AdjustWindowRectEx(&windowRect, windowStyleFlags, false, windowStyleFlagsEx);

	int width = windowRect.right - windowRect.left;
	int height = windowRect.bottom - windowRect.top;

	HWND windowHandle = CreateWindow(GAME_NAME, GAME_NAME, windowStyleFlags, windowRect.left, windowRect.top, width, height, NULL, NULL, thisAppInstance, NULL);

	if (windowHandle){
		ShowWindow(windowHandle, nShowCmd);
		UpdateWindow(windowHandle);
	}
	return windowHandle;
}


///=====================================================
/// 
///=====================================================
int __stdcall WinMain(HINSTANCE thisAppInstance, HINSTANCE /*hPrevInstance*/, LPSTR lpCmdLine, int nShowCmd){
	FATAL_ASSERT(s_theMemoryManager != nullptr);

	JobManager* jobManager = new JobManager();
	RECOVERABLE_ASSERT(jobManager != nullptr);
	if (jobManager != nullptr){
		jobManager->Startup(6); //main thread + cricket audio thread + 6 = 8
	}

	CommandList commands = ParseCommands(lpCmdLine);
	bool forceQuit = ProcessCommands(commands);
	if (forceQuit){
		if (jobManager != nullptr){
			jobManager->Shutdown();
			delete jobManager;
		}

		return 0;
	}


	HWND myWindowHandle = CreateAppWindow(thisAppInstance, nShowCmd);

	s_theApp = new TheApp();
	s_theApp->Startup((void*)myWindowHandle);
	s_theApp->Run();
	s_theApp->Shutdown();

	delete s_theApp;


	if (jobManager != nullptr){
		jobManager->Shutdown();
		delete jobManager;
	}

	s_theMemoryManager->PrintMemoryLeaks();

	return 0;
}