// Include header files
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include "VK.h"

// Global declarations
#define TS_WIN_WIDTH 800
#define TS_WIN_HEIGHT 600

DWORD dwTsStyle;
WINDOWPLACEMENT wpTsPrev = { sizeof(WINDOWPLACEMENT) };

HWND ghTsWnd = NULL;		// Handle to root window
BOOL gbTsFullScreen = FALSE;	// Toggle Fullscreen mode
BOOL gbTsActiveWindow = FALSE;	// To check active window

FILE* gpTsFile = NULL;	// File IO

// Global functions declarations
LRESULT CALLBACK TsWndProc(HWND, UINT, WPARAM, LPARAM);

// Entry point function
int WINAPI WinMain(HINSTANCE hTsInstance, HINSTANCE hTsPrevInstance, LPSTR lpszTsCmdLine, int iTsCmdShow)
{
	// Local function declarations
	int TsInitialize(void);
	void TsDisplay(void);
	void TsUpdate(void);
	void TsUninitialize(void);

	// Local variable declarations
	WNDCLASSEX tsWndClass = { sizeof(WNDCLASSEX) };
	HWND hTsWnd = NULL;
	TCHAR szTsAppName[] = TEXT("VK");
	BOOL bTsDone = FALSE;
	MSG tsMsg = { 0 };

	// Code
	// Open the log file	
	gpTsFile = fopen("vklog.txt", "w");
	if (NULL == gpTsFile)
	{
		MessageBox(NULL, TEXT("fopen() failed to open the log file. Exiting...\n"), TEXT("ERROR"), MB_OK | MB_ICONERROR | MB_ICONSTOP | MB_TOPMOST);
		exit(EXIT_FAILURE);
	}
	else
	{
		fprintf(gpTsFile, "WinMain() -> Log file created successfully\n");
	}

	// Step:1 Initialize the WNDCLASSEX
	tsWndClass.cbSize = sizeof(WNDCLASSEX);
	tsWndClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	tsWndClass.cbClsExtra = 0;
	tsWndClass.cbWndExtra = 0;
	tsWndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	tsWndClass.hInstance = hTsInstance;
	tsWndClass.hIcon = LoadIcon(hTsInstance, MAKEINTRESOURCE(TSARTRICON));
	tsWndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	tsWndClass.lpfnWndProc = TsWndProc;
	tsWndClass.lpszClassName = szTsAppName;
	tsWndClass.lpszMenuName = NULL;
	tsWndClass.hIconSm = LoadIcon(hTsInstance, MAKEINTRESOURCE(TSARTRICON));

	// Step 2: Register window class
	RegisterClassEx(&tsWndClass);

	// Step 3: create main window in memory
	hTsWnd = CreateWindowEx(WS_EX_APPWINDOW,
		szTsAppName, TEXT("TsVulkan"),
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
		(GetSystemMetrics(SM_CXSCREEN) - TS_WIN_WIDTH) / 2,
		(GetSystemMetrics(SM_CYSCREEN) - TS_WIN_HEIGHT) / 2,
		TS_WIN_WIDTH,
		TS_WIN_HEIGHT,
		NULL,
		NULL,
		hTsInstance,
		NULL
	);

	if (NULL == hTsWnd)
	{
		MessageBox(NULL, TEXT("CreateWindow() failed"), TEXT("ERROR"), MB_OK | MB_ICONERROR);
		exit(EXIT_FAILURE);
	}

	// Update global window handle
	ghTsWnd = hTsWnd;

	int iTsResult = TsInitialize();
	if (0 == iTsResult)
	{
		fprintf(gpTsFile, "[INFO] WinMain() -> Initialize successful\n");
	}

	// Step 4: Show window to user
	ShowWindow(hTsWnd, iTsCmdShow);

	// Step 5: Update window focus
	SetForegroundWindow(hTsWnd);
	SetFocus(hTsWnd);

	// Step 6: Game loop
	while (FALSE == bTsDone)
	{
		ZeroMemory(&tsMsg, sizeof(MSG));
		if (PeekMessage(&tsMsg, NULL, 0, 0, PM_REMOVE))
		{
			if (WM_QUIT == tsMsg.message)
			{
				bTsDone = TRUE;
			}
			else
			{
				TranslateMessage(&tsMsg);
				DispatchMessage(&tsMsg);
			}
		}
		else
		{
			if (TRUE == gbTsActiveWindow)
			{
				TsDisplay();
				TsUpdate();
			}
		}
	}

	// Step 7: Uninitialize
	TsUninitialize();	

	// Unregister the class
	UnregisterClass(szTsAppName, hTsInstance);

	return((int)tsMsg.wParam);
}

// Callback function
LRESULT CALLBACK TsWndProc(HWND hTsWnd, UINT uiTsMsg, WPARAM wTsParam, LPARAM lTsParam)
{
	// Local function declarations
	void TsToggleFullScreen(void);
	void TsUninitialize(void);
	void TsResize(int, int);

	// Code
	switch (uiTsMsg)
	{
	case WM_CREATE:
		memset(&wpTsPrev, 0, sizeof(WINDOWPLACEMENT));
		wpTsPrev.length = sizeof(WINDOWPLACEMENT);
		break;
	case WM_SETFOCUS:
		gbTsActiveWindow = TRUE;
		break;
	case WM_KILLFOCUS:
		gbTsActiveWindow = FALSE;
		break;
	case WM_ERASEBKGND:
		return(0);
	case WM_SIZE:
		TsResize(LOWORD(wTsParam), HIWORD(wTsParam));
		break;
	case WM_CLOSE:
		TsUninitialize();
		DestroyWindow(hTsWnd);
		break;
	case WM_KEYDOWN:
		switch (wTsParam)
		{
		case VK_ESCAPE:
			DestroyWindow(hTsWnd);
			break;
		case 0x46:
		case 0x66:
			TsToggleFullScreen();
			break;
		default:
			break;
		}
		break;
	case WM_DESTROY:
		TsUninitialize();
		PostQuitMessage(0);
		break;
	default:
		break;
	}

	return(DefWindowProc(hTsWnd, uiTsMsg, wTsParam, lTsParam));
}

void TsToggleFullScreen(void)
{
	// Local variable declarations
	MONITORINFO tsMi = { sizeof(MONITORINFO) };

	// Code
	if (FALSE == gbTsFullScreen)
	{
		dwTsStyle = GetWindowLong(ghTsWnd, GWL_STYLE);

		if (dwTsStyle & WS_OVERLAPPEDWINDOW)
		{
			if (GetWindowPlacement(ghTsWnd, &wpTsPrev) && GetMonitorInfo(MonitorFromWindow(ghTsWnd, MONITORINFOF_PRIMARY), &tsMi))
			{
				SetWindowLong(ghTsWnd, GWL_STYLE, (dwTsStyle & (~WS_OVERLAPPEDWINDOW)));

				SetWindowPos(ghTsWnd, HWND_TOP, tsMi.rcMonitor.left, tsMi.rcMonitor.top,
					(tsMi.rcMonitor.right - tsMi.rcMonitor.left),
					(tsMi.rcMonitor.bottom - tsMi.rcMonitor.top),
					SWP_NOZORDER | SWP_FRAMECHANGED);

			}
			ShowCursor(FALSE);
			gbTsFullScreen = TRUE;
		}
	}
	else   // Window is already in fullscreen mode
	{
		SetWindowLong(ghTsWnd, GWL_STYLE, (dwTsStyle | WS_OVERLAPPEDWINDOW));
		SetWindowPlacement(ghTsWnd, &wpTsPrev);
		SetWindowPos(ghTsWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);

		ShowCursor(TRUE);
		gbTsFullScreen = FALSE;
	}
}

int TsInitialize(void)
{
	// Local function decalration
	void TsResize(int, int);

	// Warm up call to Resize()
	//TsResize(TS_WIN_WIDTH, TS_WIN_HEIGHT);

	return(0);
}

void TsResize(int iTsWidth, int iTsHeight)
{
	// Code
	if (0 == iTsHeight)
	{
		iTsHeight = 1;	// To avoid Divide by zero error
	}
}

void TsDisplay(void)
{
	// Code
}

void TsUpdate(void)
{
	// Code
}

void TsUninitialize(void)
{
	// Local function decalarations
	void TsToggleFullScreen(void);

	// Code
	if (TRUE == gbTsFullScreen)
	{
		TsToggleFullScreen();
		gbTsFullScreen = FALSE;
	}

	// Destroy window
	if (ghTsWnd)
	{
		DestroyWindow(ghTsWnd);
		ghTsWnd = NULL;
	}

	if (NULL != gpTsFile)
	{
		fprintf(gpTsFile, "[INFO] TsUninitialize() -> Log file closed successfully and program closed successfully\n");
		fclose(gpTsFile);
	}

	gpTsFile = NULL;
}
