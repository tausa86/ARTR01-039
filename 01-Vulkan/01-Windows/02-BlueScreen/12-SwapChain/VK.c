// Include header files
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include "VK.h"

// Vulkan related header lines
#define VK_USE_PLATFORM_WIN32_KHR	// To include WIN32SDK specific defines
#include <vulkan\vulkan.h>	// First vulkan code line for all platforms 

// Vulkan related libraray
#pragma comment(lib, "vulkan-1.lib")	// To include vulkan library

// Global declarations
#define TS_WIN_WIDTH 800
#define TS_WIN_HEIGHT 600

DWORD dwTsStyle;
WINDOWPLACEMENT wpTsPrev;

HWND ghTsWnd = NULL;		// Handle to root window
BOOL gbTsFullScreen = FALSE;	// Toggle Fullscreen mode
BOOL gbTsActiveWindow = FALSE;	// To check active window

FILE* gpTsFile = NULL;	// File IO

// Vulkan related global variables
// Instance extension related variables
uint32_t enableInstanceExtentionCount = 0;
// VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME
const char* enabledInstanceExtensionNames_array[2];

// Vulkan instance
VkInstance vkTsInstance = VK_NULL_HANDLE;

// Vulkan Presentation Surface
VkSurfaceKHR vkTsSurfaceKHR = VK_NULL_HANDLE;

// Vulkan Physical Device Related Global Variables
VkPhysicalDevice vkTsPhysicalDevice_selected = VK_NULL_HANDLE;
uint32_t graphicsQueueFamilyIndex_selected = UINT32_MAX;	// Vulkan's max limit
VkPhysicalDeviceMemoryProperties vkPhysicalDeviceMemoryProperties;	// For staging buffer and non-staging buffers

// Remove local variables PrintVKInfo() required changes
uint32_t physicalDeviceCount = 0;
VkPhysicalDevice* vkPhysicalDevice_array = NULL;

// Instance device related variables
uint32_t enabledDeviceExtentionCount = 0;
// VK_KHR_SWAPCHAIN_EXTENSION_NAME
const char* enabledDeviceExtensionNames_array[1];

// [STEP-8] Vulkan Device
VkDevice vkDevice = VK_NULL_HANDLE;

// [STEP-9] Vulkan Queue
VkQueue vkQueue = VK_NULL_HANDLE;

// [STEP-10] Color format and color space
VkFormat vkFormat_color = VK_FORMAT_UNDEFINED;
VkColorSpaceKHR vkColorSpaceKHR = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

// [STEP-11] Presentation mode
VkPresentModeKHR vkPresentModeKHR = VK_PRESENT_MODE_FIFO_KHR;

// [STEP-12] Swapchain related variables
int winWidth = TS_WIN_WIDTH;
int winHeight = TS_WIN_HEIGHT;
VkSwapchainKHR vkSwapchainKHR = VK_NULL_HANDLE; 
VkExtent2D vkExtent2D_swapchain;

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
	MONITORINFO tsMi;
	tsMi.cbSize = sizeof(MONITORINFO);

	// Code
	wpTsPrev.length = sizeof(WINDOWPLACEMENT);

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
	// Local function decalration
	VkResult TsCreateVulkanInstance(void);
	VkResult TsGetSupportedSurface(void);
	VkResult TsGetPhysicalDevice(void);
	VkResult TsPrintVkInfo(void);	
	VkResult TsCreateVulkanDevice(void);
	void TsGetDeviceQueue(void);
	
	VkResult TsCreateSwapchain(VkBool32);

	// Local variable declaration
	VkResult vkTsResult;

	// Code
	vkTsResult = TsCreateVulkanInstance();
	if (VK_SUCCESS != vkTsResult)
	{
		fprintf(gpTsFile, "[ERROR] TsInitialize() -> TsCreateVulkanInstance() failed at %d\n", __LINE__);
		return(vkTsResult);
	}
	else
	{
		fprintf(gpTsFile, "[INFO] TsInitialize() -> TsCreateVulkanInstance() succeeded at %d \n", __LINE__);
	}

	// Create Vulkan Presentation Surface
	vkTsResult = TsGetSupportedSurface();
	if (VK_SUCCESS != vkTsResult)
	{
		fprintf(gpTsFile, "[ERROR] TsInitialize() -> TsGetSupportedSurface() failed with %d at %d\n",vkTsResult, __LINE__);
		return(vkTsResult);
	}
	else
	{
		fprintf(gpTsFile, "[INFO] TsInitialize() -> TsGetSupportedSurface() succeeded at %d \n", __LINE__);
	}

	// Get Required Physical Device and its Queue Family Index
	vkTsResult = TsGetPhysicalDevice();
	if (VK_SUCCESS != vkTsResult)
	{
		fprintf(gpTsFile, "[ERROR] TsInitialize() -> TsGetPhysicalDevice() failed with %d at %d\n", vkTsResult, __LINE__);
		return(vkTsResult);
	}
	else
	{
		fprintf(gpTsFile, "[INFO] TsInitialize() -> TsGetPhysicalDevice() succeeded at %d \n", __LINE__);
	}

	// Print Vulkan Info
	vkTsResult = TsPrintVkInfo();
	if (VK_SUCCESS != vkTsResult)
	{
		fprintf(gpTsFile, "[ERROR] TsInitialize() -> TsPrintVkInfo() failed with %d at %d\n", vkTsResult, __LINE__);
		return(vkTsResult);
	}
	else
	{
		fprintf(gpTsFile, "[INFO] TsInitialize() -> TsPrintVkInfo() succeeded at %d \n", __LINE__);
	}

	// Create Vulkan (Logical) device
	vkTsResult = TsCreateVulkanDevice();
	if (VK_SUCCESS != vkTsResult)
	{
		fprintf(gpTsFile, "[ERROR] TsInitialize() -> TsCreateVulkanDevice() failed with %d at %d\n", vkTsResult, __LINE__);
		return(vkTsResult);
	}
	else
	{
		fprintf(gpTsFile, "[INFO] TsInitialize() -> TsCreateVulkanDevice() succeeded at %d \n", __LINE__);
	}

	// [STEP-9] Get Device Queue
	TsGetDeviceQueue();	

	// Create Swapchain
	vkTsResult = TsCreateSwapchain(VK_FALSE);
	if (VK_SUCCESS != vkTsResult)
	{
		// Print actual returned VkResult value and hardcoded return value
		fprintf(gpTsFile, "[ERROR] TsInitialize() -> TsCreateSwapchain() failed with %d at %d\n", vkTsResult, __LINE__);
		vkTsResult = VK_ERROR_INITIALIZATION_FAILED;	// Sir will answer while TsResize() chya wedes sangtil ki ka Hardcoded return value?
		return(vkTsResult);
	}
	else
	{
		fprintf(gpTsFile, "[INFO] TsInitialize() -> TsCreateSwapchain() succeeded at %d \n", __LINE__);
	}

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

	// Destroy Swapchain
	if(vkSwapchainKHR)
	{
		vkDestroySwapchainKHR(vkDevice, vkSwapchainKHR, NULL);
		vkSwapchainKHR = VK_NULL_HANDLE;
		fprintf(gpTsFile, "[INFO] TsUninitialize() -> vkDestroySwapchainKHR() is successfully done\n");
	}

	// No need to Destroy/uninitialize Vulkan Device Queue

	// Destroy Vulkan Device
	if(vkDevice)
	{
		vkDeviceWaitIdle(vkDevice);	// First synchronization function
		fprintf(gpTsFile, "[INFO] TsUninitialize() -> vkDeviceWaitIdle() is done\n");

		vkDestroyDevice(vkDevice, NULL);
		vkDevice = VK_NULL_HANDLE;
		fprintf(gpTsFile, "[INFO] TsUninitialize() -> vkDestroyDevice() is successfully done\n");
	}

	// No need to destroy selected physical device

	// Destroy Presetation Surface 
	if (vkTsSurfaceKHR)
	{
		vkDestroySurfaceKHR(vkTsInstance, vkTsSurfaceKHR, NULL);	// Destroy function of vkSurfaceKHR is generic function and not paltform specific
		vkTsSurfaceKHR = VK_NULL_HANDLE;
		fprintf(gpTsFile, "[INFO] TsUninitialize() -> vkDestroySurfaceKHR() successfully done\n");
	}

	// Destroy vulkan instance
	if (vkTsInstance)
	{
		vkDestroyInstance(vkTsInstance, NULL);
		vkTsInstance = VK_NULL_HANDLE;
		fprintf(gpTsFile, "[INFO] TsUninitialize() -> vkDestroyInstance() successfully done\n");
	}

	if (NULL != gpTsFile)
	{
		fprintf(gpTsFile, "[INFO] TsUninitialize() -> Log file closed successfully and program closed successfully\n");
		fclose(gpTsFile);
	}

	gpTsFile = NULL;
}

///////////////////////////////////////// Defination Vulkan Related Functions ////////////////////////////////////////////////////////
VkResult TsPrintVkInfo(void)
{
	// Local variable declarations
	VkResult vkTsResult = VK_SUCCESS;

	// Code
	fprintf(gpTsFile, "[INFO] ********** Print Vulkan Info **********\n");
	for(uint32_t i = 0; i < physicalDeviceCount; i++)
	{
		VkPhysicalDeviceProperties vkPhysicalDeviceProperties;
		memset((void*)&vkPhysicalDeviceProperties, 0, sizeof(VkPhysicalDeviceProperties));
		vkGetPhysicalDeviceProperties(vkPhysicalDevice_array[i], &vkPhysicalDeviceProperties);
		
		uint32_t majorVersion = VK_API_VERSION_MAJOR(vkPhysicalDeviceProperties.apiVersion);
		uint32_t minorVersion = VK_API_VERSION_MINOR(vkPhysicalDeviceProperties.apiVersion);
		uint32_t patchVersion = VK_API_VERSION_PATCH(vkPhysicalDeviceProperties.apiVersion);

		// API Version
		fprintf(gpTsFile, "[INFO] API Version = %d.%d.%d\n", majorVersion, minorVersion, patchVersion);

		// Device Name
		fprintf(gpTsFile, "[INFO] Device Name = %s\n", vkPhysicalDeviceProperties.deviceName);

		// Device Type
		switch(vkPhysicalDeviceProperties.deviceType)
		{
			case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
			fprintf(gpTsFile, "[INFO] Device Type = Integrated GPU (iGPU)\n");
			break;
			case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
			fprintf(gpTsFile, "[INFO] Device Type = Discrete GPU (dGPU)\n");
			break;
			case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
			fprintf(gpTsFile, "[INFO] Device Type = Virtual GPU (vGPU)\n");
			break;
			case VK_PHYSICAL_DEVICE_TYPE_CPU:
			fprintf(gpTsFile, "[INFO] Device Type = CPU\n");
			break;
			case VK_PHYSICAL_DEVICE_TYPE_OTHER:
			fprintf(gpTsFile, "[INFO] Device Type = OTHER\n");
			break;
			default:
			fprintf(gpTsFile, "[INFO] Device Type = UNKNOWN\n");
			break;
		}

		// Vendor ID
		fprintf(gpTsFile, "[INFO] Vendor ID = 0x%04x\n", vkPhysicalDeviceProperties.vendorID);

		// Device ID
		fprintf(gpTsFile, "[INFO] Device ID = 0x%04x\n", vkPhysicalDeviceProperties.deviceID);
	}

	// Free Global Physical Device Error
	if (vkPhysicalDevice_array)
	{
		free(vkPhysicalDevice_array);
		vkPhysicalDevice_array = NULL;
		fprintf(gpTsFile, "[INFO] TsPrintVKInfo() -> Succeeded to free global vkPhysicalDevice_array at %d\n", __LINE__);
	}

	return(vkTsResult);
}

VkResult TsGetPhysicalDevice(void)
{
	// Local variable declarations
	VkResult vkTsResult = VK_SUCCESS;		

	// Code
	vkTsResult = vkEnumeratePhysicalDevices(vkTsInstance, &physicalDeviceCount, NULL);
	if (VK_SUCCESS != vkTsResult)
	{
		fprintf(gpTsFile, "[ERROR] TsGetPhysicalDevice() -> First call to vkEnumeratePhysicalDevices() failed with %d at %d\n", vkTsResult, __LINE__);
		return(vkTsResult);
	}
	else if (0 == physicalDeviceCount)
	{
		fprintf(gpTsFile, "[ERROR] TsGetPhysicalDevice() -> First call to vkEnumeratePhysicalDevices() resulted in physicalDeviceCount to zero at %d\n", __LINE__);
		vkTsResult = VK_ERROR_INITIALIZATION_FAILED;
		return(vkTsResult);
	}
	else
	{
		fprintf(gpTsFile, "[INFO] TsGetPhysicalDevice() -> First call to vkEnumeratePhysicalDevices() succeeded at %d\n", __LINE__);
	}

	vkPhysicalDevice_array = (VkPhysicalDevice*)malloc(sizeof(VkPhysicalDevice) * physicalDeviceCount);
	// Add error checking

	// Step 4:
	vkTsResult = vkEnumeratePhysicalDevices(vkTsInstance, &physicalDeviceCount, vkPhysicalDevice_array);
	if (VK_SUCCESS != vkTsResult)
	{
		fprintf(gpTsFile, "[ERROR] TsGetPhysicalDevice() -> Second call to vkEnumeratePhysicalDevices() failed with %d at %d\n", vkTsResult, __LINE__);
		return(vkTsResult);
	}	
	else
	{
		fprintf(gpTsFile, "[INFO] TsGetPhysicalDevice() -> Second call to vkEnumeratePhysicalDevices() succeeded at %d\n", __LINE__);
	}

	// Step 5:
	VkBool32 bFound = VK_FALSE;
	for (uint32_t i = 0; i < physicalDeviceCount; i++)
	{
		uint32_t queueCount = UINT32_MAX;

		// If physical device present then it must support at least one queue
		vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice_array[i], &queueCount, NULL);
		// Step 5c:
		VkQueueFamilyProperties* vkQueueFamilyProperties_array = NULL;
		vkQueueFamilyProperties_array = (VkQueueFamilyProperties*)malloc(sizeof(VkQueueFamilyProperties) * queueCount);
		// Add error checking
		// Step 5d:
		vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice_array[i], &queueCount, vkQueueFamilyProperties_array);
		
		// Step 5e:
		VkBool32* isQueueSurfaceSupported_array = NULL;
		isQueueSurfaceSupported_array = (VkBool32*)malloc(sizeof(VkBool32) * queueCount);
		// Add error checking

		// Step 5f:
		for (uint32_t j = 0; j < queueCount; j++)
		{
			vkGetPhysicalDeviceSurfaceSupportKHR(vkPhysicalDevice_array[i], j, vkTsSurfaceKHR, &isQueueSurfaceSupported_array[j]);
			// 
		}
		for (uint32_t j = 0; j < queueCount; j++)
		{
			if (vkQueueFamilyProperties_array[j].queueFlags & VK_QUEUE_GRAPHICS_BIT)	// Compute cha kaam karel kiwan render cha kaam karel kiwan data copy karel
			{
				if (VK_TRUE == isQueueSurfaceSupported_array[j])
				{
					vkTsPhysicalDevice_selected = vkPhysicalDevice_array[i];		// Integrated GPU
					//vkTsPhysicalDevice_selected = vkPhysicalDevice_array[i+1];	// Discrete GPU settings
					graphicsQueueFamilyIndex_selected = j;
					//fprintf(gpTsFile, "[INFO] TsGetPhysicalDevice() vkTsPhysicalDevice_selected = %u, graphicsQueueFamilyIndex_selected = %u \n",vkTsPhysicalDevice_selected, graphicsQueueFamilyIndex_selected);
					bFound = VK_TRUE;
				}
			}
		}

		if (isQueueSurfaceSupported_array)
		{
			free(isQueueSurfaceSupported_array);
			isQueueSurfaceSupported_array = NULL;
			fprintf(gpTsFile, "[INFO] TsGetPhysicalDevice() -> Succeeded to free isQueueSurfaceSupported_array at %d\n", __LINE__);
		}

		if (vkQueueFamilyProperties_array)
		{
			free(vkQueueFamilyProperties_array);
			vkQueueFamilyProperties_array = NULL;
			fprintf(gpTsFile, "[INFO] TsGetPhysicalDevice() -> Succeeded to free vkQueueFamilyProperties_array at %d\n", __LINE__);
		}

		if (VK_TRUE == bFound)
			break;
	}

	if (VK_TRUE == bFound)
	{
		VkPhysicalDeviceProperties vkPhysicalDeviceProperties_selected;
		memset((void*)&vkPhysicalDeviceProperties_selected, 0, sizeof(VkPhysicalDeviceProperties));
		vkGetPhysicalDeviceProperties(vkTsPhysicalDevice_selected, &vkPhysicalDeviceProperties_selected);
		fprintf(gpTsFile, "[INFO] TsGetPhysicalDevice() -> vkGetPhysicalDeviceProperties() %s physical device with graphics enabled is selected at %d\n", vkPhysicalDeviceProperties_selected.deviceName, __LINE__);
		fprintf(gpTsFile, "[INFO] TsGetPhysicalDevice() -> is succeeded to select required physical device with graphics enabled at %d\n", __LINE__);
		
		if(vkPhysicalDevice_array)
		{
			// Do nothing
		}
		else
		{
			fprintf(gpTsFile, "[INFO] TsGetPhysicalDevice() -> is failed to obtain grahics enabled physical device at %d\n", __LINE__);
			free(vkPhysicalDevice_array);
			vkPhysicalDevice_array = NULL;
			fprintf(gpTsFile, "[INFO] TsGetPhysicalDevice() -> Succeeded to free vkPhysicalDevice_array at %d\n", __LINE__);
			vkTsResult = VK_ERROR_INITIALIZATION_FAILED;
			return(vkTsResult);
		}		
	}

	// Step 7:
	memset((void*)&vkPhysicalDeviceMemoryProperties, 0, sizeof(VkPhysicalDeviceMemoryProperties));

	// Step 8: // Qualcomm code comment this is the good place to do it
	vkGetPhysicalDeviceMemoryProperties(vkTsPhysicalDevice_selected, &vkPhysicalDeviceMemoryProperties);

	// Step 9:
	VkPhysicalDeviceFeatures vkPhysicalDeviceFeatures;
	memset((void*)&vkPhysicalDeviceFeatures, 0, sizeof(VkPhysicalDeviceFeatures));
	vkGetPhysicalDeviceFeatures(vkTsPhysicalDevice_selected, &vkPhysicalDeviceFeatures);

	// Step 10:
	if (vkPhysicalDeviceFeatures.tessellationShader)
	{
		fprintf(gpTsFile, "[INFO] TsGetPhysicalDevice() -> Selected physical device supports tessellation shader\n");
	}
	else
	{
		fprintf(gpTsFile, "[INFO] TsGetPhysicalDevice() -> Selected physical device not supported tessellation shader\n");
	}

	if (vkPhysicalDeviceFeatures.geometryShader)
	{
		fprintf(gpTsFile, "[INFO] TsGetPhysicalDevice() -> Selected physical device supports geometry shader\n");
	}
	else
	{
		fprintf(gpTsFile, "[INFO] TsGetPhysicalDevice() -> Selected physical device not supported geometry shader\n");
	}

	return(vkTsResult);
}

VkResult TsGetSupportedSurface(void)
{
	// Local variable declaration
	VkResult vkTsResult = VK_SUCCESS;
	VkWin32SurfaceCreateInfoKHR vkWin32SurfaceCreateInfoKHR;

	// Code
	memset((void*)&vkWin32SurfaceCreateInfoKHR, 0, sizeof(VkWin32SurfaceCreateInfoKHR));
	vkWin32SurfaceCreateInfoKHR.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	vkWin32SurfaceCreateInfoKHR.pNext = NULL;
	vkWin32SurfaceCreateInfoKHR.flags = 0;
	vkWin32SurfaceCreateInfoKHR.hinstance = (HINSTANCE)GetWindowLongPtr(ghTsWnd, GWLP_HINSTANCE); // One of the way (HINSTANCE)GetModuleHandle(NULL);
	vkWin32SurfaceCreateInfoKHR.hwnd = ghTsWnd;

	vkTsResult = vkCreateWin32SurfaceKHR(vkTsInstance, &vkWin32SurfaceCreateInfoKHR, NULL, &vkTsSurfaceKHR);
	if (VK_SUCCESS != vkTsResult)
	{
		fprintf(gpTsFile, "[ERROR] TsGetSupportedSurface() -> vkCreateWin32SurfaceKHR() failed with %d at %d\n",vkTsResult, __LINE__);
		return(vkTsResult);
	}
	else
	{
		fprintf(gpTsFile, "[INFO] TsGetSupportedSurface() -> vkCreateWin32SurfaceKHR() succeeded at %d\n", __LINE__);
	}

	return(vkTsResult);
}

VkResult TsCreateVulkanInstance(void)
{
	// Local function declaration
	VkResult TsFillInstanceExtensionNames(void);

	// Local variable declaration
	VkResult vkTsResult = VK_SUCCESS;

	// Step 1: Initialize required extension name and count global variables
	vkTsResult = TsFillInstanceExtensionNames();
	if (VK_SUCCESS != vkTsResult)
	{
		fprintf(gpTsFile, "[ERROR] TsCreateVulkanInstance() -> TsFillInstanceExtensionNames() failed at %d\n", __LINE__);
		return(vkTsResult);
	}
	else
	{
		fprintf(gpTsFile, "[INFO] TsCreateVulkanInstance() -> TsFillInstanceExtensionNames() succeeded at %d\n", __LINE__);
	}

	// Step 2: Initialize struct VkApplicationInfo
	VkApplicationInfo vkApplicationInfo;
	memset((void*)&vkApplicationInfo, 0, sizeof(VkApplicationInfo));

	vkApplicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;	// Type safety and across version
	vkApplicationInfo.pNext = NULL;	// Linked list of a structure
	vkApplicationInfo.pApplicationName = gpszTsAppName;
	vkApplicationInfo.applicationVersion = 1;
	vkApplicationInfo.pEngineName = gpszTsAppName;
	vkApplicationInfo.engineVersion = 1;
	//vkApplicationInfo.apiVersion = VK_API_VERSION_1_4;
	vkApplicationInfo.apiVersion = VK_API_VERSION_1_3;

	// Step 3: Initialize struct VkInstanceCreateInfo
	VkInstanceCreateInfo vkInstanceCreateInfo;
	memset((void*)&vkInstanceCreateInfo, 0, sizeof(VkInstanceCreateInfo));

	vkInstanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	vkInstanceCreateInfo.pNext = NULL;
	vkInstanceCreateInfo.pApplicationInfo = &vkApplicationInfo;
	vkInstanceCreateInfo.enabledExtensionCount = enableInstanceExtentionCount;
	vkInstanceCreateInfo.ppEnabledExtensionNames = enabledInstanceExtensionNames_array;

	// Step 4: Call vkCreateInstance() to get VkInstance in a global variable
	vkTsResult = vkCreateInstance(&vkInstanceCreateInfo, NULL, &vkTsInstance);
	if (vkTsResult == VK_ERROR_INCOMPATIBLE_DRIVER)
	{
		fprintf(gpTsFile, "[ERROR] TsCreateInstanceInf() -> vkCreateInstance() failed due to incompatible driver (%d) at %d\n",vkTsResult, __LINE__);
		return(vkTsResult);
	}
	else if (vkTsResult == VK_ERROR_EXTENSION_NOT_PRESENT)
	{
		fprintf(gpTsFile, "[ERROR] TsCreateInstanceInf() -> vkCreateInstance() failed due to required extension not present (%d) at %d\n", vkTsResult, __LINE__);
		return(vkTsResult);
	}
	else if (vkTsResult != VK_SUCCESS)
	{
		fprintf(gpTsFile, "[ERROR] TsCreateInstanceInf() -> vkCreateInstance() failed due to unknown reason (%d) at %d\n", vkTsResult, __LINE__);
		return(vkTsResult);
	}
	else
	{
		fprintf(gpTsFile, "[INFO] TsCreateInstanceInf() -> vkCreateInstance() succeeded at %d\n", __LINE__);
	}

	return(vkTsResult);
}

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

VkResult TsFillDeviceExtensionNames(void)
{
	// Local variable decalarations
	VkResult vkTsResult = VK_SUCCESS;

	// Step 1: Find how many device extensions are supported by vulkan driver of this version & keep it in a local variables
	uint32_t deviceExtensionCount = 0;

	// param2 = kontya layerchi extension chi pahijet = NULL to get all extension names
	// param3 = to get number count of extensions
	// param4 = device extension chya properties ghenya sathi = NULL as we do not know about count
	vkTsResult = vkEnumerateDeviceExtensionProperties(vkTsPhysicalDevice_selected, NULL, &deviceExtensionCount, NULL);
	if (VK_SUCCESS != vkTsResult)
	{
		fprintf(gpTsFile, "[ERROR]  TsFillDeviceExtensionNames() -> First call to vkEnumerateDeviceExtensionProperties() failad at %d. Exiting...\n", __LINE__);
		return(vkTsResult);
	}
	else
	{
		fprintf(gpTsFile, "[INFO] TsFillDeviceExtensionNames() -> First call to vkEnumerateDeviceExtensionProperties() succeeded at %d\n", __LINE__);
		fprintf(gpTsFile, "[INFO] TsFillDeviceExtensionNames() -> Device Count = %u\n", deviceExtensionCount);
	}

	// Step 2: Allocate & fill VkExtenstionProperties array corresponding to above count
	VkExtensionProperties* vkTsExtensionProperties_array = NULL;
	vkTsExtensionProperties_array = (VkExtensionProperties*)malloc(sizeof(VkExtensionProperties) * deviceExtensionCount);
	if (NULL == vkTsExtensionProperties_array)
	{
		// Error check for malloc()
	}

	// param1 = Selected device
	// param2 = kontya layerchi extension chi pahijet = NULL to get all extension names
	// param3 = to get number count of extensions
	// param4 = device extension chya properties ghenya sathi = vkExtensionProperties_array to get all instance extensions
	vkTsResult = vkEnumerateDeviceExtensionProperties(vkTsPhysicalDevice_selected, NULL, &deviceExtensionCount, vkTsExtensionProperties_array);
	if (VK_SUCCESS != vkTsResult)
	{
		fprintf(gpTsFile, "[ERROR]  TsFillDeviceExtensionNames() -> Second call to vkEnumerateDeviceExtensionProperties() failad at %d. Exiting...\n", __LINE__);
		return(vkTsResult);
	}
	else
	{
		fprintf(gpTsFile, "[INFO] TsFillDeviceExtensionNames() -> Second call to vkEnumerateDeviceExtensionProperties() succeeded at %d\n", __LINE__);
	}

	// Step 3: Fill and display a local string array of extension names obtained from VkExtensionProperties{extensionName, version} structure array
	char** deviceExtensionNames_array = NULL;
	deviceExtensionNames_array = (char**)malloc(sizeof(char*) * deviceExtensionCount);
	if (NULL == deviceExtensionNames_array)
	{
		// Error check for malloc()
	}

	for (uint32_t i = 0; i < deviceExtensionCount; i++)
	{
		// Allocate memory assigned as required
		deviceExtensionNames_array[i] = (char*)malloc( sizeof(char) * (strlen(vkTsExtensionProperties_array[i].extensionName) + 1) );
		memcpy(deviceExtensionNames_array[i], vkTsExtensionProperties_array[i].extensionName, (strlen(vkTsExtensionProperties_array[i].extensionName) + 1));
		fprintf(gpTsFile, "[INFO] TsFillDeviceExtensionNames() -> Vulkan Device Extension Name = %s\n", deviceExtensionNames_array[i]);
	}

	// Step 4: As not required here onwards free vkExtensionProperties_array
	if (NULL != vkTsExtensionProperties_array)
	{
		free(vkTsExtensionProperties_array);
		vkTsExtensionProperties_array = NULL;
	}

	// Step 5: Find whether above extension names contain our required 1 extension VK_KHR_SWAPCHAIN_EXTENSION_NAME
	// Accordingly, set 2 global variables requiredExtensionCount, enabledDeviceExtensionNames_array
	VkBool32 vulkanSwapchainExtensionFound = VK_FALSE;

	for (uint32_t i = 0; i < deviceExtensionCount; i++)
	{
		if (0 == strcmp(deviceExtensionNames_array[i], VK_KHR_SWAPCHAIN_EXTENSION_NAME))
		{
			vulkanSwapchainExtensionFound = VK_TRUE;
			enabledDeviceExtensionNames_array[enabledDeviceExtentionCount++] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
		}
	}

	// Step 6: As not needed henceforth free the local string array
	for (uint32_t i = 0; i < deviceExtensionCount; i++)
	{
		if (deviceExtensionNames_array[i])
		{
			free(deviceExtensionNames_array[i]);
			deviceExtensionNames_array[i] = NULL;
		}
	}

	if (deviceExtensionNames_array)
	{
		free(deviceExtensionNames_array);
		deviceExtensionNames_array = NULL;
	}

	// Step 7: Print whether our vulkan driver supports our required extension name or not
	if (VK_FALSE == vulkanSwapchainExtensionFound)
	{
		vkTsResult = VK_ERROR_INITIALIZATION_FAILED;	// Return hardcoded failure
		fprintf(gpTsFile, "[ERROR] TsFillDeviceExtensionNames() -> VK_KHR_SWAPCHAIN_EXTENSION_NAME not found at %d\n", __LINE__);
		return(vkTsResult);
	}
	else
	{
		fprintf(gpTsFile, "[INFO] TsFillDeviceExtensionNames() -> VK_KHR_SWAPCHAIN_EXTENSION_NAME found at %d\n", __LINE__);
	}

	// Step 8: Print only supported/enaabled device extension name
	for (uint32_t i = 0; i < enabledDeviceExtentionCount; i++)
	{
		fprintf(gpTsFile, "[INFO] TsFillDeviceExtensionNames() -> Enabled Vulkan Device Extension Name = %s\n", enabledDeviceExtensionNames_array[i]);
	}

	//fprintf(gpTsFile, "[INFO] TsFillInstanceExtensionNames() -> ")

	return(vkTsResult);
}

VkResult TsCreateVulkanDevice(void)
{
	// Function declaration
	VkResult TsFillDeviceExtensionNames(void);

	// Local variable declaration
	VkResult vkTsResult = VK_SUCCESS;

	// Code
	// Fill Device Extensions
	// Step 1: Initialize required extension name and count global variables
	vkTsResult = TsFillDeviceExtensionNames();
	if (VK_SUCCESS != vkTsResult)
	{
		fprintf(gpTsFile, "[ERROR] TsCreateVulkanDevice() -> TsFillDeviceExtensionNames() failed at %d\n", __LINE__);
		return(vkTsResult);
	}
	else
	{
		fprintf(gpTsFile, "[INFO] TsCreateVulkanDevice() -> TsFillDeviceExtensionNames() succeeded at %d\n", __LINE__);
	}

	// Newly Added Code
	float queuePriority[1];
	queuePriority[0] = 1.0f;
	VkDeviceQueueCreateInfo vkDeviceQueueCreateInfo;
	memset((void*)&vkDeviceQueueCreateInfo, 0, sizeof(VkDeviceQueueCreateInfo));
	vkDeviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	vkDeviceQueueCreateInfo.pNext = NULL;
	vkDeviceQueueCreateInfo.flags = 0;
	vkDeviceQueueCreateInfo.queueFamilyIndex = graphicsQueueFamilyIndex_selected;
	vkDeviceQueueCreateInfo.queueCount = 1;
	//vkDeviceQueueCreateInfo.pQueuePriorities = NULL;
	vkDeviceQueueCreateInfo.pQueuePriorities = queuePriority;

	// Intialize VkDeviceCreateInfo Structure
	VkDeviceCreateInfo vkDeviceCreateInfo;
	memset((void*)&vkDeviceCreateInfo, 0, sizeof(VkDeviceCreateInfo));

	vkDeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	vkDeviceCreateInfo.pNext = NULL;
	vkDeviceCreateInfo.flags = 0;
	vkDeviceCreateInfo.enabledExtensionCount = enabledDeviceExtentionCount;
	vkDeviceCreateInfo.ppEnabledExtensionNames = enabledDeviceExtensionNames_array;

	// Deprecated in latest version
	vkDeviceCreateInfo.enabledLayerCount = 0;	// Very important member and will be used as life-line
	vkDeviceCreateInfo.ppEnabledLayerNames = NULL;	// Very Important member and will be used as life-line

	vkDeviceCreateInfo.pEnabledFeatures = NULL;
	
	vkDeviceCreateInfo.queueCreateInfoCount = 1;
	vkDeviceCreateInfo.pQueueCreateInfos = &vkDeviceQueueCreateInfo;

	//                          Kontya Physical Device Sathi    warcha structure
	vkTsResult = vkCreateDevice(vkTsPhysicalDevice_selected, &vkDeviceCreateInfo, NULL, &vkDevice);
	if (VK_SUCCESS != vkTsResult)
	{
		fprintf(gpTsFile, "[ERROR] TsCreateVulkanDevice() -> vkCreateDevice() failed with %d at %d\n", vkTsResult, __LINE__);
		return(vkTsResult);
	}
	else
	{
		fprintf(gpTsFile, "[INFO] TsCreateVulkanDevice() -> vkCreateDevice() succeeded at %d\n", __LINE__);
	}

	return(vkTsResult);
}

void TsGetDeviceQueue(void)
{
	// Local variable decalration
	
	// Code
	// Eka device madhye multiple queue aste
	vkGetDeviceQueue(vkDevice, graphicsQueueFamilyIndex_selected, 0, &vkQueue);

	if(VK_NULL_HANDLE == vkQueue)
	{
		fprintf(gpTsFile, "[ERROR] TsGetDeviceQueue() -> vkGetDeviceQueue() returned NULL for vkQueue\n");
		return;
	}
	else
	{
		fprintf(gpTsFile, "[INFO] TsGetDeviceQueue() -> vkGetDeviceQueue() succeeded\n");
	}
}

VkResult TsGetPhysicalDeviceSurfaceFormatAndColorSpace(void)
{
	// Local variable decalarations
	VkResult vkTsResult = VK_SUCCESS;
	uint32_t formatCount = 0;

	// Code
	// Get the count of supported surface color formats
	vkTsResult = vkGetPhysicalDeviceSurfaceFormatsKHR(vkTsPhysicalDevice_selected, vkTsSurfaceKHR, &formatCount, NULL);
	if (VK_SUCCESS != vkTsResult)
	{
		fprintf(gpTsFile, "[ERROR] TsGetPhysicalDeviceSurfaceFormatAndColorSpace() -> First call to vkGetPhysicalDeviceSurfaceFormatsKHR() failed with %d at %d\n", vkTsResult, __LINE__);
		return(vkTsResult);
	}
	else if(0 == formatCount)
	{
		fprintf(gpTsFile, "[ERROR] TsGetPhysicalDeviceSurfaceFormatAndColorSpace() -> First call to vkGetPhysicalDeviceSurfaceFormatsKHR() resulted in formatCount to zero at %d\n", __LINE__);
		vkTsResult = VK_ERROR_INITIALIZATION_FAILED;
		return(vkTsResult);
	}
	else
	{
		fprintf(gpTsFile, "[INFO] TsGetPhysicalDeviceSurfaceFormatAndColorSpace() -> First call to vkGetPhysicalDeviceSurfaceFormatsKHR() succeeded at %d\n", __LINE__);
	}

	// declare and allocate VkSurfaceFormatKHR
	VkSurfaceFormatKHR* vkSurfaceFormatKHR_array = (VkSurfaceFormatKHR*)malloc(formatCount * sizeof(VkSurfaceFormatKHR));
	// Add error checking

	// Filling the array
	vkTsResult = vkGetPhysicalDeviceSurfaceFormatsKHR(vkTsPhysicalDevice_selected, vkTsSurfaceKHR, &formatCount, vkSurfaceFormatKHR_array);
	if (VK_SUCCESS != vkTsResult)
	{
		fprintf(gpTsFile, "[ERROR] TsGetPhysicalDeviceSurfaceFormatAndColorSpace() -> Second call to vkGetPhysicalDeviceSurfaceFormatsKHR() failed with %d at %d\n", vkTsResult, __LINE__);
		return(vkTsResult);
	}
	else
	{
		fprintf(gpTsFile, "[INFO] TsGetPhysicalDeviceSurfaceFormatAndColorSpace() -> Second call to vkGetPhysicalDeviceSurfaceFormatsKHR() succeeded at %d\n", __LINE__);
	}

	// Important step and this format will change for different formats such as iOS, MacOS
	// Decide the surface color format first
	if(1 == formatCount && vkSurfaceFormatKHR_array[0].format == VK_FORMAT_UNDEFINED)
	{
		vkFormat_color = VK_FORMAT_B8G8R8A8_UNORM;	// if not found then rudimentary driver is there & swapchain will fail
	}
	else
	{
		vkFormat_color = vkSurfaceFormatKHR_array[0].format;
	}

	// Decide the surface color space 
	vkColorSpaceKHR = vkSurfaceFormatKHR_array[0].colorSpace;

	// Free the array
	if(vkSurfaceFormatKHR_array)
	{
		free(vkSurfaceFormatKHR_array);
		fprintf(gpTsFile, "[INFO] TsGetPhysicalDeviceSurfaceFormatAndColorSpace() -> vkSurfaceFormatKHR_array freed successfully vkFormat_color: %d and vkColorSpaceKHR: %d\n", vkFormat_color, vkColorSpaceKHR);
		vkSurfaceFormatKHR_array = NULL;
	}

	return(vkTsResult);
}

VkResult TsGetPhysicalDevicePresentMode(void)
{
	// Local variable declaration
	VkResult vkTsResult = VK_SUCCESS;
	uint32_t presentModesCount = 0;

	// Code
	vkTsResult = vkGetPhysicalDeviceSurfacePresentModesKHR(vkTsPhysicalDevice_selected, vkTsSurfaceKHR, &presentModesCount, NULL);

	if (VK_SUCCESS != vkTsResult)
	{
		fprintf(gpTsFile, "[ERROR] TsGetPhysicalDevicePresentMode() -> First call to vkGetPhysicalDeviceSurfacePresentModesKHR() failed with %d at %d\n", vkTsResult, __LINE__);
		return(vkTsResult);
	}
	else if(0 == presentModesCount)
	{
		fprintf(gpTsFile, "[ERROR] TsGetPhysicalDevicePresentMode() -> First call to vkGetPhysicalDeviceSurfacePresentModesKHR() resulted in presentModeCount to zero at %d\n", __LINE__);
		vkTsResult = VK_ERROR_INITIALIZATION_FAILED;
		return(vkTsResult);
	}
	else
	{
		fprintf(gpTsFile, "[INFO] TsGetPhysicalDevicePresentMode() -> First call to vkGetPhysicalDeviceSurfacePresentModesKHR() succeeded at %d\n", __LINE__);
	}

	// 
	VkPresentModeKHR* vkPresentModeKHR_array = (VkPresentModeKHR*)malloc(presentModesCount * sizeof(VkPresentModeKHR));
	// Add error code

	vkTsResult = vkGetPhysicalDeviceSurfacePresentModesKHR(vkTsPhysicalDevice_selected, vkTsSurfaceKHR, &presentModesCount, vkPresentModeKHR_array);

	if (VK_SUCCESS != vkTsResult)
	{
		fprintf(gpTsFile, "[ERROR] TsGetPhysicalDevicePresentMode() -> Second call to vkGetPhysicalDeviceSurfacePresentModesKHR() failed with %d at %d\n", vkTsResult, __LINE__);
		return(vkTsResult);
	}
	else
	{
		fprintf(gpTsFile, "[INFO] TsGetPhysicalDevicePresentMode() -> Second call to vkGetPhysicalDeviceSurfacePresentModesKHR() succeeded at %d\n", __LINE__);
	}

	// Decide the presentation mode
	for(uint32_t i = 0; i < presentModesCount; i++)
	{
		if(VK_PRESENT_MODE_MAILBOX_KHR == vkPresentModeKHR_array[i])
		{
			vkPresentModeKHR = VK_PRESENT_MODE_MAILBOX_KHR;
			break;
		}
	}

	if(VK_PRESENT_MODE_MAILBOX_KHR != vkPresentModeKHR)
	{
		vkPresentModeKHR = VK_PRESENT_MODE_FIFO_KHR;	// Important
	}

	// Free array
	if(vkPresentModeKHR_array)
	{
		free(vkPresentModeKHR_array);
		vkPresentModeKHR_array = NULL;
		fprintf(gpTsFile, "[INFO] TsGetPhysicalDevicePresentMode() -> vkPresentModeKHR_array freed successfully vkPresentModeKHR: %d \n", vkPresentModeKHR);
	}

	return(vkTsResult);
}

// Create Swapchain
// vSync => Vertical Synchronization -> Presentation 
// 0 : 
// 1 : Latency chalel & teri synchorinization pahije
VkResult TsCreateSwapchain(VkBool32 vSync)	
{
	// Function proptotype declarations
	VkResult TsGetPhysicalDeviceSurfaceFormatAndColorSpace(void);
	VkResult TsGetPhysicalDevicePresentMode(void);

	// Local variable declaration
	VkResult vkTsResult = VK_SUCCESS;

	// Code
	// [STEP-10] Color Format And Color Space
	// Step 1
	vkTsResult = TsGetPhysicalDeviceSurfaceFormatAndColorSpace();
	if (VK_SUCCESS != vkTsResult)
	{
		fprintf(gpTsFile, "[ERROR] TsCreateSwapchain() -> TsGetPhysicalDeviceSurfaceFormatAndColorSpace() failed with %d at %d\n", vkTsResult, __LINE__);
		return(vkTsResult);
	}
	else
	{
		fprintf(gpTsFile, "[INFO] TsCreateSwapchain() -> TsGetPhysicalDeviceSurfaceFormatAndColorSpace() succeeded at %d \n", __LINE__);
	}

	// Step 2 Get Physical Device Surface Capabilities
	VkSurfaceCapabilitiesKHR vkSurfaceCapabilitiesKHR;
	memset((void *)&vkSurfaceCapabilitiesKHR, 0, sizeof(VkSurfaceCapabilitiesKHR));

	vkTsResult = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkTsPhysicalDevice_selected, vkTsSurfaceKHR, &vkSurfaceCapabilitiesKHR);
	if (VK_SUCCESS != vkTsResult)
	{
		fprintf(gpTsFile, "[ERROR] TsCreateSwapchain() -> vkGetPhysicalDeviceSurfaceCapabilitiesKHR() failed with %d at %d\n", vkTsResult, __LINE__);
		return(vkTsResult);
	}
	else
	{
		fprintf(gpTsFile, "[INFO] TsCreateSwapchain() -> vkGetPhysicalDeviceSurfaceCapabilitiesKHR() succeeded at %d \n", __LINE__);
	}

	// Step 3 Find out desired number of swapchain image count
	uint32_t testingNumberOfSwapchainImages = vkSurfaceCapabilitiesKHR.minImageCount + 1;
	uint32_t desiredNumberOfSwapchainImages = 0;

	if(vkSurfaceCapabilitiesKHR.maxImageCount > 0 && vkSurfaceCapabilitiesKHR.maxImageCount < testingNumberOfSwapchainImages)
	{
		desiredNumberOfSwapchainImages = vkSurfaceCapabilitiesKHR.maxImageCount;
	}
	else
	{
		desiredNumberOfSwapchainImages = vkSurfaceCapabilitiesKHR.minImageCount;
	}

	// Step 4 Choose size of swapchain image
	memset((void *)&vkExtent2D_swapchain, 0, sizeof(VkExtent2D));
	if(UINT32_MAX != vkSurfaceCapabilitiesKHR.currentExtent.width)
	{
		vkExtent2D_swapchain.width = vkSurfaceCapabilitiesKHR.currentExtent.width;
		vkExtent2D_swapchain.height = vkSurfaceCapabilitiesKHR.currentExtent.height;
		fprintf(gpTsFile, "[INFO] TsCreateSwapchain() -> Swapchain Image width x height = %d x %d maxImageCount = %d and minImageCount = %d at %d\n", vkExtent2D_swapchain.width, vkExtent2D_swapchain.height, vkSurfaceCapabilitiesKHR.maxImageCount, vkSurfaceCapabilitiesKHR.minImageCount, __LINE__);
	}
	else
	{
		// If surface size is already defined then swapchain image size must match with it
		VkExtent2D vkExtent2D;
		memset((void *)&vkExtent2D, 0, sizeof(VkExtent2D));
		vkExtent2D.width = (uint32_t)winWidth;
		vkExtent2D.height = (uint32_t)winHeight;

		// 
		vkExtent2D_swapchain.width = max(vkSurfaceCapabilitiesKHR.minImageExtent.width, min(vkSurfaceCapabilitiesKHR.maxImageExtent.width, vkExtent2D.width));
		vkExtent2D_swapchain.height = max(vkSurfaceCapabilitiesKHR.minImageExtent.height, min(vkSurfaceCapabilitiesKHR.maxImageExtent.height, vkExtent2D.height));
		fprintf(gpTsFile, "[INFO] TsCreateSwapchain() -> Swapchain Image width x height = %d x %d at %d\n", vkExtent2D_swapchain.width, vkExtent2D_swapchain.height, __LINE__);
	}

	// Step 5 Set swapchain image usage flag
	VkImageUsageFlags vkImageUsageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

	// Step 6 Whether to consider Pre-Transform/Fliping or not
	VkSurfaceTransformFlagBitsKHR vkSurfaceTransformfFlagBitsKHR;	// Enum
	if(vkSurfaceCapabilitiesKHR.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
	{
		vkSurfaceTransformfFlagBitsKHR = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;	// No transform
	}
	else
	{
		vkSurfaceTransformfFlagBitsKHR = vkSurfaceCapabilitiesKHR.currentTransform;	// If manually transformed
	}

	// [STEP-11] Get Surface Present Mode
	// Step 7
	vkTsResult = TsGetPhysicalDevicePresentMode();
	if (VK_SUCCESS != vkTsResult)
	{
		fprintf(gpTsFile, "[ERROR] TsCreateSwapchain() -> TsGetPhysicalDevicePresentMode() failed with %d at %d\n", vkTsResult, __LINE__);
		return(vkTsResult);
	}
	else
	{
		fprintf(gpTsFile, "[INFO] TsCreateSwapchain() -> TsGetPhysicalDevicePresentMode() succeeded at %d \n", __LINE__);
	}

	// Step 8 Initialize VkSwapchainCreateInfo Structure
	VkSwapchainCreateInfoKHR vkSwapchainCreateInfoKHR;
	memset((void*)&vkSwapchainCreateInfoKHR, 0, sizeof(VkSwapchainCreateInfoKHR));
	vkSwapchainCreateInfoKHR.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	vkSwapchainCreateInfoKHR.pNext = NULL;
	vkSwapchainCreateInfoKHR.flags = 0;

	vkSwapchainCreateInfoKHR.surface = vkTsSurfaceKHR;
	vkSwapchainCreateInfoKHR.minImageCount = desiredNumberOfSwapchainImages;
	vkSwapchainCreateInfoKHR.imageFormat = vkFormat_color;
	vkSwapchainCreateInfoKHR.imageColorSpace = vkColorSpaceKHR;
	vkSwapchainCreateInfoKHR.imageExtent.width = vkExtent2D_swapchain.width;
	vkSwapchainCreateInfoKHR.imageExtent.height = vkExtent2D_swapchain.height;
	vkSwapchainCreateInfoKHR.imageUsage = vkImageUsageFlags;
	vkSwapchainCreateInfoKHR.preTransform = vkSurfaceTransformfFlagBitsKHR;
	vkSwapchainCreateInfoKHR.imageArrayLayers = 1;	// In mobile(or ARM base laptop as well)/GameConsoles layered rendering is very famous as we are not using it
	vkSwapchainCreateInfoKHR.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; // if swapchain images are need to be shared across multiple queues _SHARED is used
	vkSwapchainCreateInfoKHR.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // MEANS Alhpa = 1.0 
	vkSwapchainCreateInfoKHR.presentMode = vkPresentModeKHR;
	vkSwapchainCreateInfoKHR.clipped = VK_TRUE; // Similar to WS_SIBLING_CLIP
	
	vkTsResult = vkCreateSwapchainKHR(vkDevice, &vkSwapchainCreateInfoKHR, NULL, &vkSwapchainKHR);
	if (VK_SUCCESS != vkTsResult)
	{
		fprintf(gpTsFile, "[ERROR] TsCreateSwapchain() -> vkCreateSwapchainKHR() failed with %d at %d\n", vkTsResult, __LINE__);
		return(vkTsResult);
	}
	else
	{
		fprintf(gpTsFile, "[INFO] TsCreateSwapchain() -> vkCreateSwapchainKHR() succeeded at %d \n", __LINE__);
	}

	return(vkTsResult);
}
