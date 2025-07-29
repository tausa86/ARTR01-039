// Include header files
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include "VK.h"

// Vulkan related header lines
#define VK_USE_PLATFORM_WIN32_KHR	// To include WIN32SDK specific defines
#include <vulkan\vulkan.h>	// First vulkan code line for all platforms 

// Vulkan related libraray
#pragma comment(lib, "vulkan-1")	// To include vulkan library

// Global declarations
#define TS_WIN_WIDTH 800
#define TS_WIN_HEIGHT 600

DWORD dwTsStyle;
WINDOWPLACEMENT wpTsPrev = { sizeof(WINDOWPLACEMENT) };

HWND ghTsWnd = NULL;		// Handle to root window
BOOL gbTsFullScreen = FALSE;	// Toggle Fullscreen mode
BOOL gbTsActiveWindow = FALSE;	// To check active window

FILE* gpTsFile = NULL;	// File IO

// Vulkan related global variables
// Instance extension related variables
//typedef uint32_t long;

uint32_t enableInstanceExtentionCount = 0;
//unsigned long enableInstanceExtentionCount = 0;
// VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME
const char* enabledInstanceExtensionNames_array[2];

// Global functions declarations
LRESULT CALLBACK TsWndProc(HWND, UINT, WPARAM, LPARAM);
const char* gpszTsAppName = "ARTR";

// Entry point function
int WINAPI WinMain(HINSTANCE hTsInstance, HINSTANCE hTsPrevInstance, LPSTR lpszTsCmdLine, int iTsCmdShow)
{
	// Local function declarations
	VkResult TsInitialize(void);
	void TsDisplay(void);
	void TsUpdate(void);
	void TsUninitialize(void);

	// Local variable declarations
	WNDCLASSEX tsWndClass = { sizeof(WNDCLASSEX) };
	HWND hTsWnd = NULL;
	TCHAR szTsAppName[255];
	BOOL bTsDone = FALSE;
	MSG tsMsg;

	VkResult vkTsResult = VK_SUCCESS;	// Initialize to success

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

	wsprintf(szTsAppName, TEXT("%s"), gpszTsAppName);	// Get global App Name in local App Name variable

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
		MessageBox(NULL, TEXT("CreateWindowEx() failed"), TEXT("ERROR"), MB_OK | MB_ICONERROR);
		exit(EXIT_FAILURE);
	}

	// Update global window handle
	ghTsWnd = hTsWnd;

	vkTsResult = TsInitialize();
	if (VK_SUCCESS != vkTsResult)
	{
		fprintf(gpTsFile, "[ERROR] WinMain() -> TsInitialize() failed. Exiting...\n");
		DestroyWindow(hTsWnd);
		hTsWnd = NULL;
	}
	else
	{
		fprintf(gpTsFile, "[INFO] Winmain() TsInitialize() succeeded\n");
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

VkResult TsInitialize(void)
{
	//void TsResize(int, int);
	// Local variable declaration
	VkResult vkTsResult = VK_SUCCESS;

	// Local function decalration
	VkResult TsFillInstanceExtensionNames(void);

	vkTsResult = TsFillInstanceExtensionNames();
	if (VK_SUCCESS != vkTsResult)
	{
		fprintf(gpTsFile, "[ERROR] TsInitialize() -> TsFillInstanceExtensionNames() failed. Exiting...\n");
		return(vkTsResult);
	}
	else
	{
		fprintf(gpTsFile, "[INFO] TsInitialize() TsFillInstanceExtensionNames() succeeded\n");
	}

	// Warm up call to Resize()
	//TsResize(TS_WIN_WIDTH, TS_WIN_HEIGHT);

	return(vkTsResult);
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

///////////////////////////////////////// Defination Vulkan Related Functions ////////////////////////////////////////////////////////
VkResult TsFillInstanceExtensionNames(void)
{
	// Local variable decalarations
	VkResult vkTsResult = VK_SUCCESS;

	// Step 1: Find how many instance extensions are supported by vulkan driver of this version & keep it in a local variables
	uint32_t instanceExtensionCount = 0;

	// param1 = kontya layerchi extension chi pahijet = NULL to get all extension names
	// param2 = to get number count of extensions
	// param3 = instance extension chya properties ghenya sathi = NULL as we do not know about count
	vkTsResult = vkEnumerateInstanceExtensionProperties(NULL, &instanceExtensionCount, NULL);
	if (VK_SUCCESS != vkTsResult)
	{
		fprintf(gpTsFile, "[ERROR]  TsFillInstanceExtensionNames() -> First call to vkEnumerateInstanceExtensionProperties() failad at %d. Exiting...\n", __LINE__);
		return(vkTsResult);
	}
	else
	{
		fprintf(gpTsFile, "[INFO] TsFillInstanceExtensionNames() -> First call to vkEnumerateInstanceExtensionProperties() succeeded at %d\n", __LINE__);
	}

	// Step 2: Allocate & fill VkExtenstionProperties array corresponding to above count
	VkExtensionProperties* vkTsExtensionProperties_array = NULL;
	vkTsExtensionProperties_array = (VkExtensionProperties*)malloc(sizeof(VkExtensionProperties) * instanceExtensionCount);
	if (NULL == vkTsExtensionProperties_array)
	{
		// Error check for malloc()
	}

	// param1 = kontya layerchi extension chi pahijet = NULL to get all extension names
	// param2 = to get number count of extensions
	// param3 = instance extension chya properties ghenya sathi = vkExtensionProperties_array to get all instance extensions
	vkTsResult = vkEnumerateInstanceExtensionProperties(NULL, &instanceExtensionCount, vkTsExtensionProperties_array);
	if (VK_SUCCESS != vkTsResult)
	{
		fprintf(gpTsFile, "[ERROR]  TsFillInstanceExtensionNames() -> Second call to vkEnumerateInstanceExtensionProperties() failad at %d. Exiting...\n", __LINE__);
		return(vkTsResult);
	}
	else
	{
		fprintf(gpTsFile, "[INFO] TsFillInstanceExtensionNames() -> Second call to vkEnumerateInstanceExtensionProperties() succeeded at %d\n", __LINE__);
	}

	// Step 3: Fill and display a local string array of extension names obtained from VkExtensionProperties{extensionName, version} structure array
	char** instanceExtensionNames_array = NULL;
	instanceExtensionNames_array = (char**)malloc(sizeof(char*) * instanceExtensionCount);
	if (NULL == instanceExtensionNames_array)
	{
		// Error check for malloc()
	}

	for (uint32_t i = 0; i < instanceExtensionCount; i++)
	{
		// Allocate memory assigned as required
		instanceExtensionNames_array[i] = (char*)malloc( sizeof(char) * (strlen(vkTsExtensionProperties_array[i].extensionName) + 1) );
		memcpy(instanceExtensionNames_array[i], vkTsExtensionProperties_array[i].extensionName, (strlen(vkTsExtensionProperties_array[i].extensionName) + 1));
		fprintf(gpTsFile, "[INFO] TsFillInstanceExtensionNames() -> Vulkan Extension Name = %s\n", instanceExtensionNames_array[i]);
	}

	// Step 4: As not required here onwards free vkExtensionProperties_array
	if (NULL != vkTsExtensionProperties_array)
	{
		free(vkTsExtensionProperties_array);
		vkTsExtensionProperties_array = NULL;
	}

	// Step 5: Find whether above extension names contain our required 2 extensions VK_KHR_SURFACE_EXTENSION_NAME AND VK_KHR_WIN32_SURFACE_EXTENSION_NAME
	// Accordingly, set 2 global variables requiredExtensionCount, enabledInstanceExtensionNames_array
	VkBool32 vulkanSurfaceExtensionFound = VK_FALSE;
	VkBool32 win32SurfaceExtentionFound = VK_FALSE;

	for (uint32_t i = 0; i < instanceExtensionCount; i++)
	{
		if (0 == strcmp(instanceExtensionNames_array[i], VK_KHR_SURFACE_EXTENSION_NAME))
		{
			vulkanSurfaceExtensionFound = VK_TRUE;
			enabledInstanceExtensionNames_array[enableInstanceExtentionCount++] = VK_KHR_SURFACE_EXTENSION_NAME;
		}

		if (0 == strcmp(instanceExtensionNames_array[i], VK_KHR_WIN32_SURFACE_EXTENSION_NAME))
		{
			win32SurfaceExtentionFound = VK_TRUE;
			enabledInstanceExtensionNames_array[enableInstanceExtentionCount++] = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
		}
	}

	// Step 6: As not needed henceforth free the local string array
	for (uint32_t i = 0; i < instanceExtensionCount; i++)
	{
		if (instanceExtensionNames_array[i])
		{
			free(instanceExtensionNames_array[i]);
			instanceExtensionNames_array[i] = NULL;
		}
	}

	if (instanceExtensionNames_array)
	{
		free(instanceExtensionNames_array);
		instanceExtensionNames_array = NULL;
	}

	// Step 7: Print whether our vulkan driver supports our required extension name or not
	if (VK_FALSE == vulkanSurfaceExtensionFound)
	{
		vkTsResult = VK_ERROR_INITIALIZATION_FAILED;	// Return hardcoded failure
		fprintf(gpTsFile, "[ERROR] TsFillInstanceExtensionNames() -> VK_KHR_SURFACE_EXTENSION_NAME not found at %d\n", __LINE__);
		return(vkTsResult);
	}
	else
	{
		fprintf(gpTsFile, "[INFO] TsFillInstanceExtensionNames() -> VK_KHR_SURFACE_EXTENSION_NAME found at %d\n", __LINE__);
	}

	if (VK_FALSE == win32SurfaceExtentionFound)
	{
		vkTsResult = VK_ERROR_INITIALIZATION_FAILED;	// Return hardcoded failure
		fprintf(gpTsFile, "[ERROR] TsFillInstanceExtensionNames() -> VK_KHR_WIN32_SURFACE_EXTENSION_NAME not found at %d\n", __LINE__);
		return(vkTsResult);
	}
	else
	{
		fprintf(gpTsFile, "[INFO] TsFillInstanceExtensionNames() -> VK_KHR_WIN32_SURFACE_EXTENSION_NAME found at %d\n", __LINE__);
	}

	// Step 8: Print only supported/enaabled extension name
	for (uint32_t i = 0; i < enableInstanceExtentionCount; i++)
	{
		fprintf(gpTsFile, "[INFO] TsFillInstanceExtensionNames() -> Enabled Vulkan Instance Extension Name = %s\n", enabledInstanceExtensionNames_array[i]);
	}

	//fprintf(gpTsFile, "[INFO] TsFillInstanceExtensionNames() -> ")

	return(vkTsResult);
}
