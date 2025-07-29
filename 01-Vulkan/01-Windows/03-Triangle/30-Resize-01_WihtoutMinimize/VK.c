// Include header files
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

// [STEP-2]
#include "VK.h"

// Vulkan related header lines
// [STEP-2] To express WIN flags supported for Vulkan
#define VK_USE_PLATFORM_WIN32_KHR // To include WIN32SDK specific defines
#include <vulkan\vulkan.h> // First vulkan code line for all platforms

// Vulkan related libraray
#pragma comment(lib, "vulkan-1.lib") // To include vulkan library

// Global declarations
#define TS_WIN_WIDTH 800
#define TS_WIN_HEIGHT 600
#define ARRAY_SIZE(array) (sizeof(array) / sizeof(*array))

DWORD dwTsStyle;
WINDOWPLACEMENT wpTsPrev;

HWND ghTsWnd = NULL; // Handle to root window
BOOL gbTsFullScreen = FALSE; // Toggle Fullscreen mode
BOOL gbTsActiveWindow = FALSE; // To check active window

FILE* gpTsFile = NULL; // File IO

// Vulkan related global variables
// [STEP-2] Instance extension related variables 
uint32_t enableInstanceExtentionCount = 0;

// VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME, VK_EXT_DEBUG_REPORT_EXTENSION_NAME
const char* enabledInstanceExtensionNames_array[3];

// [STEP-3] Vulkan instance
VkInstance vkTsInstance = VK_NULL_HANDLE;

// Vulkan Presentation Surface
VkSurfaceKHR vkTsSurfaceKHR = VK_NULL_HANDLE;

// Vulkan Physical Device Related Global Variables
VkPhysicalDevice vkTsPhysicalDevice_selected = VK_NULL_HANDLE;
uint32_t graphicsQueueFamilyIndex_selected = UINT32_MAX; // Vulkan's max limit
VkPhysicalDeviceMemoryProperties vkPhysicalDeviceMemoryProperties; // For staging buffer and non-staging buffers

// [STEP-6] Remove local variables PrintVKInfo() required changes
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

// [STEP-13] Swapchain images and image views related
uint32_t swapchainImageCount = UINT32_MAX;
VkImage * swapchainImage_array = NULL;

VkImageView * swapchainImageView_array = NULL;

 

VkCommandPool vkCommandPool = VK_NULL_HANDLE;

VkCommandBuffer* vkCommandBuffer_array = NULL;

 

// [STEP 16] RenderPass

VkRenderPass vkRenderPass = VK_NULL_HANDLE;

 

// [STEP-17] Framebuffer

VkFramebuffer * vkFrameBuffer_array = NULL;

 

// [STEP-18] Fences And Semaphores

VkSemaphore vkSemaphore_backbuffer = VK_NULL_HANDLE;

VkSemaphore vkSemaphore_rendercomplete = VK_NULL_HANDLE;

VkFence * vkFence_array = NULL;

 

// [STEP-19]

VkClearColorValue vkClearColorValue;

 

// [STEP-20] Render

BOOL bInitialized = FALSE;  //

uint32_t currentImageIndex = UINT_MAX;

 

// [STEP-21] Validation

BOOL bValidation = TRUE;

uint32_t enabledValidationLayerCount = 0;

const char * enabledValidationLayerNames_array[1];  // For VK_LAYER_KHRONOS_validation

VkDebugReportCallbackEXT vkDebugReportCallbackEXT = VK_NULL_HANDLE;

PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT_fnptr = NULL;

 

// [STEP-22] Vertex buffer related variables

typedef struct

{

    VkBuffer vkBuffer;

    VkDeviceMemory vkDeviceMemory;

} VertexData;

 

// Position related variables

VertexData vertexData_position;

 

// [STEP-23] Shaders related variables

VkShaderModule vkShaderModule_vertex_shader = VK_NULL_HANDLE;

VkShaderModule vkShaderModule_fragment_shader = VK_NULL_HANDLE;

 

// [STEP-24] Descriptor Set Layout related variables

VkDescriptorSetLayout vkDescriptorSetLayout = VK_NULL_HANDLE;

 

// [STEP-25] Pipeline Layout related variables

VkPipelineLayout vkPipelineLayout = VK_NULL_HANDLE;

 

// [STEP-26] Pipeline related variables

VkViewport vkViewport;

VkRect2D vkRect2D_scissor;  // To enable Scissor test => to remove unwanted canvas after our geomwtry

VkPipeline vkPipeline = VK_NULL_HANDLE;

 

// Global functions declarations

LRESULT CALLBACK TsWndProc(HWND, UINT, WPARAM, LPARAM);

const char* gpszTsAppName = "ARTR";

 

// Entry point function

int WINAPI WinMain(HINSTANCE hTsInstance, HINSTANCE hTsPrevInstance, LPSTR lpszTsCmdLine, int iTsCmdShow)

{

               // Local function declarations

               VkResult TsInitialize(void);

               VkResult TsDisplay(void);

               void TsUpdate(void);

               void TsUninitialize(void);

 

               // Local variable declarations

               WNDCLASSEX tsWndClass;// = { sizeof(WNDCLASSEX) };

               HWND hTsWnd = NULL;

               TCHAR szTsAppName[] = TEXT("ARTR Window");

               BOOL bTsDone = FALSE;

               MSG tsMsg;

 

               VkResult vkTsResult = VK_SUCCESS; // Initialize to success

 

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

 

               wsprintf(szTsAppName, TEXT("%s"), gpszTsAppName); // Get global App Name in local App Name variable

 

               // Step:1 Initialize the WNDCLASSEX

               tsWndClass.cbSize = sizeof(WNDCLASSEX);

               tsWndClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;   // Different Class Style

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
                                        vkTsResult = TsDisplay();
                                        if(VK_FALSE != vkTsResult && VK_SUCCESS != vkTsResult)
                                        {
                                            fprintf(gpTsFile, "[ERROR] WinMain() -> Call to TsDisplay() Failed at %d \n", __LINE__);
                                            bTsDone = TRUE;
                                        }
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
               VkResult TsResize(int, int);

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
                              //return(0);
                              break; // As this retained mode graphics there is WM_PAINT to paint
               case WM_SIZE:
                              TsResize(LOWORD(lTsParam), HIWORD(lTsParam));
                              break;
               case WM_CLOSE:
                              TsUninitialize();
                              DestroyWindow(hTsWnd);
                              break;
               case WM_KEYDOWN:

                              switch (wTsParam)

                              {

                              case VK_ESCAPE:   // case 27

                                             DestroyWindow(hTsWnd);

                                             break;

                              case 0x46:    // case 'F'

                              case 0x66:    // case 'f'

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

    //wpTsPrev.length = sizeof(WINDOWPLACEMENT);

 

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

        SetWindowPos(ghTsWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);

 

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

               VkResult TsCreateImagesAndImageViews(void);

               VkResult TsCreateCommandPool(void);

               VkResult TsCreateCommandBuffers(void);

               VkResult TsCreateVertexBuffer(void); // [STEP-22]

               VkResult TsCreateShaders(void); // [STEP-23]

               VkResult TsCreateDescriptorSetLayout(void);  // [STEP-24]

               VkResult TsCreatePipelineLayout(void);   // [STEP-25]

               VkResult TsCreatePipeline(void); // [STEP-26]              

               VkResult TsCreateRenderPass(void);

               VkResult TsCreateFrameBuffers(void);

               VkResult TsCreateSemaphores(void);

               VkResult TsCreateFences(void);

               VkResult TsBuildCommandBuffers(void);

 

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

                              fprintf(gpTsFile, "[ERROR] TsInitialize() -> TsGetSupportedSurface() failed with %d at %d\n", vkTsResult, __LINE__);

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

                              vkTsResult = VK_ERROR_INITIALIZATION_FAILED; // Sir will answer while TsResize() chya wedes sangtil ki ka Hardcoded return value?

                              return(vkTsResult);

               }

               else

               {

                              fprintf(gpTsFile, "[INFO] TsInitialize() -> TsCreateSwapchain() succeeded at %d \n", __LINE__);

               }

 

               // [STEP-13] Create Vulkan ImagesAndImageViews

               vkTsResult = TsCreateImagesAndImageViews();

               if (VK_SUCCESS != vkTsResult)

               {

                              fprintf(gpTsFile, "[ERROR] TsInitialize() -> TsCreateImagesAndImageViews() failed with %d at %d\n", vkTsResult, __LINE__);

                              return(vkTsResult);

               }

               else

               {

                              fprintf(gpTsFile, "[INFO] TsInitialize() -> TsCreateImagesAndImageViews() succeeded gives swapchain image count: %d at %d \n", swapchainImageCount, __LINE__);

               }

 

               // Create comamnd pool

               vkTsResult = TsCreateCommandPool();

               if (VK_SUCCESS != vkTsResult)

               {

                              fprintf(gpTsFile, "[ERROR] TsInitialize() -> TsCreateCommandPool() failed with %d at %d\n", vkTsResult, __LINE__);

                              return(vkTsResult);

               }

               else

               {

                              fprintf(gpTsFile, "[INFO] TsInitialize() -> TsCreateCommandPool() succeeded at %d \n", __LINE__);

               }

 

               // Create command buffers

               vkTsResult = TsCreateCommandBuffers();

               if (VK_SUCCESS != vkTsResult)

               {

                              fprintf(gpTsFile, "[ERROR] TsInitialize() -> TsCreateCommandBuffers() failed with %d at %d\n", vkTsResult, __LINE__);

                              return(vkTsResult);

               }

               else

               {

                              fprintf(gpTsFile, "[INFO] TsInitialize() -> TsCreateCommandBuffers() succeeded at %d \n", __LINE__);

               }

 

               // [STEP-22] Create Vertex Buffer

               vkTsResult = TsCreateVertexBuffer();

               if (VK_SUCCESS != vkTsResult)

               {

                              fprintf(gpTsFile, "[ERROR] TsInitialize() -> TsCreateVertexBuffer() failed with %d at %d\n", vkTsResult, __LINE__);

                              return(vkTsResult);

               }

               else

               {

                              fprintf(gpTsFile, "[INFO] TsInitialize() -> TsCreateVertexBuffer() succeeded at %d \n", __LINE__);

               }

 

               // [STEP-23]

               vkTsResult = TsCreateShaders();

               if (VK_SUCCESS != vkTsResult)

               {

                              fprintf(gpTsFile, "[ERROR] TsInitialize() -> TsCreateShaders() failed with %d at %d\n", vkTsResult, __LINE__);

                              return(vkTsResult);

               }

               else

               {

                              fprintf(gpTsFile, "[INFO] TsInitialize() -> TsCreateShaders() succeeded at %d \n", __LINE__);

               }

 

                // [STEP-24]

                vkTsResult = TsCreateDescriptorSetLayout();

                if (VK_SUCCESS != vkTsResult)

                {

                               fprintf(gpTsFile, "[ERROR] TsInitialize() -> TsCreateDescriptorSetLayout() failed with %d at %d\n", vkTsResult, __LINE__);

                               return(vkTsResult);

                }

                else

                {

                               fprintf(gpTsFile, "[INFO] TsInitialize() -> TsCreateDescriptorSetLayout() succeeded at %d \n", __LINE__);

                }

 

                // [STEP-25]

                vkTsResult = TsCreatePipelineLayout();

                if (VK_SUCCESS != vkTsResult)

                {

                               fprintf(gpTsFile, "[ERROR] TsInitialize() -> TsCreatePipelineLayout() failed with %d at %d\n", vkTsResult, __LINE__);

                               return(vkTsResult);

                }

                else

                {

                               fprintf(gpTsFile, "[INFO] TsInitialize() -> TsCreatePipelineLayout() succeeded at %d \n", __LINE__);

                }

 

               // RenderPass

               vkTsResult = TsCreateRenderPass();

               if (VK_SUCCESS != vkTsResult)

               {

                              fprintf(gpTsFile, "[ERROR] TsInitialize() -> TsCreateRenderPass() failed with %d at %d\n", vkTsResult, __LINE__);

                              return(vkTsResult);

               }

               else

               {

                              fprintf(gpTsFile, "[INFO] TsInitialize() -> TsCreateRenderPass() succeeded at %d \n", __LINE__);

               }

 

               // [STEP-26] Create Pipeline

               vkTsResult = TsCreatePipeline();

               if (VK_SUCCESS != vkTsResult)

               {

                              fprintf(gpTsFile, "[ERROR] TsInitialize() -> TsCreatePipeline() failed with %d at %d\n", vkTsResult, __LINE__);

                              return(vkTsResult);

               }

               else

               {

                              fprintf(gpTsFile, "[INFO] TsInitialize() -> TsCreatePipeline() succeeded at %d \n", __LINE__);

               }

 

               // Create Frame Buffer

               vkTsResult = TsCreateFrameBuffers();

               if (VK_SUCCESS != vkTsResult)

               {

                              fprintf(gpTsFile, "[ERROR] TsInitialize() -> TsCreateFrameBuffers() failed with %d at %d\n", vkTsResult, __LINE__);

                              return(vkTsResult);

               }

               else

               {

                              fprintf(gpTsFile, "[INFO] TsInitialize() -> TsCreateFrameBuffers() succeeded at %d \n", __LINE__);

               }

 

               // Create Vulkan Semaphores

               vkTsResult = TsCreateSemaphores();

               if (VK_SUCCESS != vkTsResult)

               {

                              fprintf(gpTsFile, "[ERROR] TsInitialize() -> TsCreateSemaphores() failed with %d at %d\n", vkTsResult, __LINE__);

                              return(vkTsResult);

               }

               else

               {

                              fprintf(gpTsFile, "[INFO] TsInitialize() -> TsCreateSemaphores() succeeded at %d \n", __LINE__);

               }

 

               // Create Vulkan Fences

               vkTsResult = TsCreateFences();

               if (VK_SUCCESS != vkTsResult)

               {

                              fprintf(gpTsFile, "[ERROR] TsInitialize() -> TsCreateFences() failed with %d at %d\n", vkTsResult, __LINE__);

                              return(vkTsResult);

               }

               else

               {

                              fprintf(gpTsFile, "[INFO] TsInitialize() -> TsCreateFences() succeeded at %d \n", __LINE__);

               }

 

               // Initialize clear color values

               memset((void *)&vkClearColorValue, 0, sizeof(VkClearColorValue));

 

               // Analogus to glClearColor() & DirectX

               vkClearColorValue.float32[0] = 0.0f;             // R

               vkClearColorValue.float32[1] = 0.0f;             // G

               vkClearColorValue.float32[2] = 1.0f;             // B

               vkClearColorValue.float32[3] = 1.0f;             // A

 

               // [STEP-19] Build command buffers

               vkTsResult = TsBuildCommandBuffers();

               if (VK_SUCCESS != vkTsResult)

               {

                              fprintf(gpTsFile, "[ERROR] TsInitialize() -> TsBuildCommandBuffers() failed with %d at %d\n", vkTsResult, __LINE__);

                              return(vkTsResult);

               }
               else
               {
                              fprintf(gpTsFile, "[INFO] TsInitialize() -> TsBuildCommandBuffers() succeeded at %d \n", __LINE__);
               }

               // [STEP-20] Render
               // Initialization is completed
               bInitialized = TRUE; 

               fprintf(gpTsFile, "[INFO] TsInitialize() -> Initialization completed successfully at %d \n", __LINE__);

               return(vkTsResult);
}
 
VkResult TsResize(int iTsWidth, int iTsHeight)
{
    // Function declaration
    VkResult TsCreateSwapchain(VkBool32 vSync);
    VkResult TsCreateImagesAndImageViews(void);
    VkResult TsCreateCommandBuffers(void);
    VkResult TsCreatePipelineLayout(void);   // [STEP-25]
    VkResult TsCreatePipeline(void); // [STEP-26]              
    VkResult TsCreateRenderPass(void);
    VkResult TsCreateFrameBuffers(void);
    VkResult TsBuildCommandBuffers(void);

    // Local variable declaration
    VkResult vkTsResult = VK_SUCCESS;

    // Code
    // Check the bInitialize varaible
    if(FALSE == bInitialized)
    {
        fprintf(gpTsFile, "[ERROR] TsResize() -> Initialization yet not completed or failed at %d \n", __LINE__);
        vkTsResult = VK_ERROR_INITIALIZATION_FAILED;
        return(vkTsResult);
    }

    // As recreation of swapchain is needed, we are going to repeat many steps of TsInitialize() again. Hence set bInitialized = FALSE again
    bInitialized = FALSE;

    if (0 >= iTsHeight)
    {
        iTsHeight = 1; // To avoid Divide by zero error
    }

    // Set global win width and win height variable
    winWidth = iTsWidth;
    winHeight = iTsHeight;

    // Wait for device to complete in-hand tasks
    if (vkDevice)
    {
        vkDeviceWaitIdle(vkDevice); // First synchronization function
        fprintf(gpTsFile, "[INFO] TsResize() -> vkDeviceWaitIdle() is done\n");

        // Check presence of swapchain
        if(VK_NULL_HANDLE == vkSwapchainKHR)
        {
            fprintf(gpTsFile, "[ERROR] TsResize() -> vkSwapchainKHR is already null cannot proceed at %d\n", __LINE__);
            vkTsResult = VK_ERROR_INITIALIZATION_FAILED;
            return(vkTsResult);
        }

        // Destroy frame buffer
        for(uint32_t i = 0; i < swapchainImageCount; i++)
        {
                        vkDestroyFramebuffer(vkDevice, vkFrameBuffer_array[i], NULL);
                        fprintf(gpTsFile, "[INFO] TsResize() -> vkDestroyFramebuffer() is successfully done for %d index\n", i);
        }

        if(vkFrameBuffer_array)
        {
                        free(vkFrameBuffer_array);
                        vkFrameBuffer_array = NULL;
                        fprintf(gpTsFile, "[INFO] TsResize() -> vkFrameBuffer_array released successfully\n");
        }

        for(uint32_t i = 0; i < swapchainImageCount; i++)

                              {

                                             vkFreeCommandBuffers(vkDevice, vkCommandPool, 1, &vkCommandBuffer_array[i]);

                                             fprintf(gpTsFile, "[INFO] TsResize() -> vkFreeCommandBuffers() is successfully done for iteration: %d\n", i);

                              }

 

                              if(vkCommandBuffer_array)

                              {

                                             free(vkCommandBuffer_array);

                                             vkCommandBuffer_array = NULL;

                                             fprintf(gpTsFile, "[INFO] TsResize() -> vkCommandBuffer_array is successfully freed\n");

                              }

        if(vkPipeline)
        {
            vkDestroyPipeline(vkDevice, vkPipeline, NULL);
            vkPipeline = VK_NULL_HANDLE;
            fprintf(gpTsFile, "[INFO] TsResize() -> vkDestroyPipeline() is successfully done\n");
        }

        if(vkPipelineLayout)

                              {

                                vkDestroyPipelineLayout(vkDevice, vkPipelineLayout, NULL);

                                vkPipelineLayout = VK_NULL_HANDLE;

                                fprintf(gpTsFile, "[INFO] TsResize() -> vkDestroyPipelineLayout() vkPipelineLayout is successfully released\n");

                              }

        // Destroy renderpass
        if(vkRenderPass)
        {
                        vkDestroyRenderPass(vkDevice, vkRenderPass, NULL);
                        vkRenderPass = VK_NULL_HANDLE;
                        fprintf(gpTsFile, "[INFO] TsResize() -> vkDestroyRenderPass() is successfully done\n");
        }    

        // destroy image views

                              for(uint32_t i = 0; i < swapchainImageCount; i++)

                              {

                                             vkDestroyImageView(vkDevice, swapchainImageView_array[i], NULL);

                                             fprintf(gpTsFile, "[INFO] TsResize() -> vkDestroyImageView() is successfully done for iteration: %d\n", i);

                              }

 

                              // Free swapchain image view array

                              if(swapchainImageView_array)

                              {

                                             free(swapchainImageView_array);

                                             swapchainImageView_array = NULL;

                                             fprintf(gpTsFile, "[INFO] TsResize() -> swapchainImageView_array is successfully destroyed\n");

                              }

 

                              /*// Free swapchain images

                              for(uint32_t i = 0; i < swapchainImageCount; i++)

                              {

                                             vkDestroyImage(vkDevice, swapchainImage_array[i], NULL);

                                             fprintf(gpTsFile, "[INFO] TsResize() -> vkDestroyImage() is successfully done for iteration: %d\n", i);

                              }*/

 

                              if(swapchainImage_array)

                              {

                                             free(swapchainImage_array);

                                             swapchainImage_array = NULL;

                                             fprintf(gpTsFile, "[INFO] TsResize() -> swapchainImage_array is successfully destroyed\n");

                              }

        // Destroy Swapchain

        if (vkSwapchainKHR)

        {

                        vkDestroySwapchainKHR(vkDevice, vkSwapchainKHR, NULL);

                        vkSwapchainKHR = VK_NULL_HANDLE;

                        fprintf(gpTsFile, "[INFO] TsResize() -> vkDestroySwapchainKHR() is successfully done\n");

        }

        // Recreate for resize
        // Create Swapchain
        vkTsResult = TsCreateSwapchain(VK_FALSE);

        if (VK_SUCCESS != vkTsResult)

        {

                        // Print actual returned VkResult value and hardcoded return value

                        fprintf(gpTsFile, "[ERROR] TsResize() -> TsCreateSwapchain() failed with %d at %d\n", vkTsResult, __LINE__);

                        vkTsResult = VK_ERROR_INITIALIZATION_FAILED; // Sir will answer while TsResize() chya wedes sangtil ki ka Hardcoded return value?

                        return(vkTsResult);

        }

        // [STEP-13] Create Vulkan ImagesAndImageViews

               vkTsResult = TsCreateImagesAndImageViews();

               if (VK_SUCCESS != vkTsResult)

               {

                              fprintf(gpTsFile, "[ERROR] TsResize() -> TsCreateImagesAndImageViews() failed with %d at %d\n", vkTsResult, __LINE__);

                              return(vkTsResult);

               }

               // RenderPass

               vkTsResult = TsCreateRenderPass();

               if (VK_SUCCESS != vkTsResult)

               {

                              fprintf(gpTsFile, "[ERROR] TsResize() -> TsCreateRenderPass() failed with %d at %d\n", vkTsResult, __LINE__);

                              return(vkTsResult);

               }

               vkTsResult = TsCreatePipelineLayout();

                if (VK_SUCCESS != vkTsResult)

                {

                               fprintf(gpTsFile, "[ERROR] TsResize() -> TsCreatePipelineLayout() failed with %d at %d\n", vkTsResult, __LINE__);

                               return(vkTsResult);

                }


// [STEP-26] Create Pipeline

               vkTsResult = TsCreatePipeline();

               if (VK_SUCCESS != vkTsResult)

               {

                              fprintf(gpTsFile, "[ERROR] TsResize() -> TsCreatePipeline() failed with %d at %d\n", vkTsResult, __LINE__);

                              return(vkTsResult);

               }

               

// Create Frame Buffer

               vkTsResult = TsCreateFrameBuffers();

               if (VK_SUCCESS != vkTsResult)

               {

                              fprintf(gpTsFile, "[ERROR] TsResize() -> TsCreateFrameBuffers() failed with %d at %d\n", vkTsResult, __LINE__);

                              return(vkTsResult);

               }


               // Create command buffers

               vkTsResult = TsCreateCommandBuffers();

               if (VK_SUCCESS != vkTsResult)

               {

                              fprintf(gpTsFile, "[ERROR] TsResize() -> TsCreateCommandBuffers() failed with %d at %d\n", vkTsResult, __LINE__);

                              return(vkTsResult);

               }

                // [STEP-19] Build command buffers

               vkTsResult = TsBuildCommandBuffers();

               if (VK_SUCCESS != vkTsResult)

               {

                              fprintf(gpTsFile, "[ERROR] TsResize() -> TsBuildCommandBuffers() failed with %d at %d\n", vkTsResult, __LINE__);

                              return(vkTsResult);

               }

    }    

    bInitialized = TRUE;

    return(vkTsResult);
}

VkResult TsDisplay(void)
{
    // Local variable decalration
    VkResult vkTsResult = VK_SUCCESS;

    // Code
    // if control comes here before intialization gets completed return(FALSE);
    if(FALSE == bInitialized)
    {
        fprintf(gpTsFile, "[INFO] TsDisplay() -> Initialization yet not completed at %d \n", __LINE__);
        return((VkResult)VK_FALSE);
    } 

    // Acquire index of next swapchain image
    // If this function do not return the next swapchain within TimeOut(NS) then this function will return VK_NOT_READY       
    vkTsResult = vkAcquireNextImageKHR(vkDevice,        // Our acquired device

                                        vkSwapchainKHR, // Swapchain

                                        UINT64_MAX, // TimeOut(NS) wait for swapchain to provide the next image

                                        // Backbuffer semaphore is not waiting for Swapchain while it is waiting for

                                        // another Queue to release the image held by another Queue required by the swapchain

                                        // Semaphores are used for Inter Queue Synchronization Operations means

                                        vkSemaphore_backbuffer,

                                        // Fence is used for inter host synchronization operations similar to above Semaphore

                                        VK_NULL_HANDLE,

                                        &currentImageIndex

                                    );

   

    if(VK_SUCCESS != vkTsResult)

    {

        fprintf(gpTsFile, "[INFO] TsDisplay() -> vkAcquireNextImageKHR() failed with %d at %d \n", vkTsResult, __LINE__);

        return(vkTsResult);

    }

 

    // Use Fence to allow host to wait for completion of execution previous command buffer

    vkTsResult = vkWaitForFences(vkDevice,  // Our acquired device

                            1,              // Kiti fences sathi thambaycha karan aapan eka wedi ekcah kartoy mhanoon 1

                            &vkFence_array[currentImageIndex],  // Konta Fence

                            VK_TRUE,    // Jer tumhi array dila asel ter I will wait for all Fences to get Signaled i.e. mi thambel sagdya Fences cha kaam sampe peryant

                            UINT64_MAX  // TimeOut(NS) kiti wed thambaych

                            );

    if(VK_SUCCESS != vkTsResult)

    {

        fprintf(gpTsFile, "[INFO] TsDisplay() -> vkWaitForFences() failed with %d at %d \n", vkTsResult, __LINE__);

        return(vkTsResult);

    }

 

    // Make Ready fences for execution of next command buffer

    vkTsResult = vkResetFences(vkDevice,

                            1,   // Kiti fences all reset karaych

                            &vkFence_array[currentImageIndex]   // Konta Fence

                            );

    if(VK_SUCCESS != vkTsResult)

    {

        fprintf(gpTsFile, "[INFO] TsDisplay() -> vkResetFences() failed with %d at %d \n", vkTsResult, __LINE__);

        return(vkTsResult);

    }

 

    // One of the member of VkSubmitInfo Structure require array of pipeline stage,

    // we have only one of completion of color attachment output,

    // still we need one member array

    const VkPipelineStageFlags waitDestinationStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

 

    // Declare, memset() and initialize VkSubmitInfo structure

    VkSubmitInfo vkSubmitInfo;

    memset((void *)&vkSubmitInfo, 0, sizeof(VkSubmitInfo));

 

    vkSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    vkSubmitInfo.pNext = NULL;

 

    vkSubmitInfo.pWaitDstStageMask = &waitDestinationStageMask;

 

    // Kaam karnya aadhi thambaych aahe

    vkSubmitInfo.waitSemaphoreCount = 1;

    vkSubmitInfo.pWaitSemaphores = &vkSemaphore_backbuffer;

 

    // Kaay kaam karaych

    vkSubmitInfo.commandBufferCount = 1;

    vkSubmitInfo.pCommandBuffers = &vkCommandBuffer_array[currentImageIndex];

 

    // Konala signal karaych

    vkSubmitInfo.signalSemaphoreCount = 1;

    vkSubmitInfo.pSignalSemaphores = &vkSemaphore_rendercomplete;

 

    // Now submit above work to queue

    vkTsResult = vkQueueSubmit(vkQueue, // Konti queue

        1, // How many submit info structure you wish

        &vkSubmitInfo,  // Which submit info structure/s

        vkFence_array[currentImageIndex]    // Fences

    );

 

    if(VK_SUCCESS != vkTsResult)

    {

        fprintf(gpTsFile, "[INFO] TsDisplay() -> vkQueueSubmit() failed with %d at %d \n", vkTsResult, __LINE__);

        return(vkTsResult);

    }

 

    // We are going to present rendered image after declaring and initialization VkPresentInfoKHR structure

    VkPresentInfoKHR vkPresentInfoKHR;

    memset((void *)&vkPresentInfoKHR, 0, sizeof(VkPresentInfoKHR));

 

    vkPresentInfoKHR.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    vkPresentInfoKHR.pNext = NULL;

   

    vkPresentInfoKHR.swapchainCount = 1;    // Swapchain count but we have 1

    vkPresentInfoKHR.pSwapchains = &vkSwapchainKHR;  // Swapchain Array but we have 1

    vkPresentInfoKHR.pImageIndices = &currentImageIndex;     // Current Image Index Array Count

    vkPresentInfoKHR.waitSemaphoreCount = 1;

    vkPresentInfoKHR.pWaitSemaphores = &vkSemaphore_rendercomplete; // Rendering samplay

   

    // Present the queue

    vkTsResult = vkQueuePresentKHR(vkQueue,

        &vkPresentInfoKHR

    );   

 

    if(VK_SUCCESS != vkTsResult)

    {

        fprintf(gpTsFile, "[INFO] TsDisplay() -> vkQueuePresentKHR() failed with %d at %d \n", vkTsResult, __LINE__);

        return(vkTsResult);

    }

 

    // Use case 1 to disable this call vkDeviceWaitIdle(vkDevice);

    vkDeviceWaitIdle(vkDevice);

 

    return(vkTsResult);

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

 

               // No need to Destroy/uninitialize Vulkan Device Queue

 

               // Destroy Vulkan Device

               if (vkDevice)

               {

                              vkDeviceWaitIdle(vkDevice); // First synchronization function

                              fprintf(gpTsFile, "[INFO] TsUninitialize() -> vkDeviceWaitIdle() is done\n");

 

                              // Destroy Fence

                              for(uint32_t i = 0; i < swapchainImageCount; i++)

                              {

                                             vkDestroyFence(vkDevice, vkFence_array[i], NULL);                                       

                                             fprintf(gpTsFile, "[INFO] TsUninitialize() -> vkDestroyFence() is successfully done for %d index\n", i);

                              }

 

                              if(vkFence_array)

                              {

                                             free(vkFence_array);

                                             vkFence_array = NULL;

                                             fprintf(gpTsFile, "[INFO] TsUninitialize() -> vkFence_array released successfully\n");

                              }

 

                              // Destroy Semaphore

                              // Use case 3 commenting semaphore/fence object, validation will provide error message that you have forget to destroy vulkan object

                              if(vkSemaphore_rendercomplete)

                              {

                                             vkDestroySemaphore(vkDevice, vkSemaphore_rendercomplete, NULL);

                                             vkSemaphore_rendercomplete = VK_NULL_HANDLE;

                                             fprintf(gpTsFile, "[INFO] TsUninitialize() -> vkSemaphore_rendercomplete released successfully\n");

                              }

 

                              if(vkSemaphore_backbuffer)

                              {

                                             vkDestroySemaphore(vkDevice, vkSemaphore_backbuffer, NULL);

                                             vkSemaphore_backbuffer = VK_NULL_HANDLE;

                                             fprintf(gpTsFile, "[INFO] TsUninitialize() -> vkSemaphore_backbuffer released successfully\n");

                              }

 

                              // Destroy Framebuffer

                              for(uint32_t i = 0; i < swapchainImageCount; i++)

                              {

                                             vkDestroyFramebuffer(vkDevice, vkFrameBuffer_array[i], NULL);

                                             fprintf(gpTsFile, "[INFO] TsUninitialize() -> vkDestroyFramebuffer() is successfully done for %d index\n", i);

                              }

                              if(vkFrameBuffer_array)

                              {

                                             free(vkFrameBuffer_array);

                                             vkFrameBuffer_array = NULL;

                                             fprintf(gpTsFile, "[INFO] TsUninitialize() -> vkFrameBuffer_array released successfully\n");

                              } 

                              

                              // [STEP-25]

                              if(vkPipelineLayout)

                              {

                                vkDestroyPipelineLayout(vkDevice, vkPipelineLayout, NULL);

                                vkPipelineLayout = VK_NULL_HANDLE;

                                fprintf(gpTsFile, "[INFO] TsUninitialize() -> vkDestroyPipelineLayout() vkPipelineLayout is successfully released\n");

                              }

 

                              // [STEP-24]

                              if(vkDescriptorSetLayout)

                              {

                                vkDestroyDescriptorSetLayout(vkDevice, vkDescriptorSetLayout, NULL);

                                vkDescriptorSetLayout = VK_NULL_HANDLE;

                                fprintf(gpTsFile, "[INFO] TsUninitialize() -> vkDestroyDescriptorSetLayout() vkDescriptorSetLayout is successfully released\n");

                              }

 

                              // [STEP-23]

                              if(vkShaderModule_fragment_shader)

                              {

                                vkDestroyShaderModule(vkDevice, vkShaderModule_fragment_shader, NULL);

                                vkShaderModule_fragment_shader = VK_NULL_HANDLE;

                                fprintf(gpTsFile, "[INFO] TsUninitialize() -> vkDestroyShaderModule() vkShaderModule_fragment_shader is successfully done\n");

                              }

 

                              if(vkShaderModule_vertex_shader)

                              {

                                vkDestroyShaderModule(vkDevice, vkShaderModule_vertex_shader, NULL);

                                vkShaderModule_vertex_shader = VK_NULL_HANDLE;

                                fprintf(gpTsFile, "[INFO] TsUninitialize() -> vkDestroyShaderModule() vkShaderModule_vertex_shader is successfully done\n");

                              }

 

                              // [STEP-22]

                              if(vertexData_position.vkDeviceMemory)

                              {

                                vkFreeMemory(vkDevice, vertexData_position.vkDeviceMemory, NULL);

                                vertexData_position.vkDeviceMemory = VK_NULL_HANDLE;

                                fprintf(gpTsFile, "[INFO] TsUninitialize() -> vkFreeMemory() is successfully done\n");

                              }

 

                              if(vertexData_position.vkBuffer)

                              {

                                vkDestroyBuffer(vkDevice, vertexData_position.vkBuffer, NULL);

                                vertexData_position.vkBuffer = VK_NULL_HANDLE;

                                fprintf(gpTsFile, "[INFO] TsUninitialize() -> vkDestroyBuffer() is successfully done\n");

                              }

 

                              // [STEP-26]

                              if(vkPipeline)

                              {

                                vkDestroyPipeline(vkDevice, vkPipeline, NULL);

                                vkPipeline = VK_NULL_HANDLE;

                                fprintf(gpTsFile, "[INFO] TsUninitialize() -> vkDestroyPipeline() is successfully done\n");

                              }

 

                              if(vkRenderPass)

                              {

                                             vkDestroyRenderPass(vkDevice, vkRenderPass, NULL);

                                             vkRenderPass = VK_NULL_HANDLE;

                                             fprintf(gpTsFile, "[INFO] TsUninitialize() -> vkDestroyRenderPass() is successfully done\n");

                              }

 

                              for(uint32_t i = 0; i < swapchainImageCount; i++)

                              {

                                             vkFreeCommandBuffers(vkDevice, vkCommandPool, 1, &vkCommandBuffer_array[i]);

                                             fprintf(gpTsFile, "[INFO] TsUninitialize() -> vkFreeCommandBuffers() is successfully done for iteration: %d\n", i);

                              }

 

                              if(vkCommandBuffer_array)

                              {

                                             free(vkCommandBuffer_array);

                                             vkCommandBuffer_array = NULL;

                                             fprintf(gpTsFile, "[INFO] TsUninitialize() -> vkCommandBuffer_array is successfully freed\n");

                              }

 

                              if(vkCommandPool)

                              {

                                             vkDestroyCommandPool(vkDevice, vkCommandPool, NULL);

                                             vkCommandPool = VK_NULL_HANDLE;

                                             fprintf(gpTsFile, "[INFO] TsUninitialize() -> vkDestroyCommandPool() is successfully done\n");

                              }

 

                              // destroy image views

                              for(uint32_t i = 0; i < swapchainImageCount; i++)

                              {

                                             vkDestroyImageView(vkDevice, swapchainImageView_array[i], NULL);

                                             fprintf(gpTsFile, "[INFO] TsUninitialize() -> vkDestroyImageView() is successfully done for iteration: %d\n", i);

                              }

 

                              // Free swapchain image view array

                              if(swapchainImageView_array)

                              {

                                             free(swapchainImageView_array);

                                             swapchainImageView_array = NULL;

                                             fprintf(gpTsFile, "[INFO] TsUninitialize() -> swapchainImageView_array is successfully destroyed\n");

                              }

 

                              /*// Free swapchain images

                              for(uint32_t i = 0; i < swapchainImageCount; i++)

                              {

                                             vkDestroyImage(vkDevice, swapchainImage_array[i], NULL);

                                             fprintf(gpTsFile, "[INFO] TsUninitialize() -> vkDestroyImage() is successfully done for iteration: %d\n", i);

                              }*/

 

                              if(swapchainImage_array)

                              {

                                             free(swapchainImage_array);

                                             swapchainImage_array = NULL;

                                             fprintf(gpTsFile, "[INFO] TsUninitialize() -> swapchainImage_array is successfully destroyed\n");

                              }                            

 

                              // Destroy Swapchain

                              if (vkSwapchainKHR)

                              {

                                             vkDestroySwapchainKHR(vkDevice, vkSwapchainKHR, NULL);

                                             vkSwapchainKHR = VK_NULL_HANDLE;

                                             fprintf(gpTsFile, "[INFO] TsUninitialize() -> vkDestroySwapchainKHR() is successfully done\n");

                              }

 

                              vkDestroyDevice(vkDevice, NULL);

                              vkDevice = VK_NULL_HANDLE;

                              fprintf(gpTsFile, "[INFO] TsUninitialize() -> vkDestroyDevice() is successfully done\n");

               }

 

               // No need to destroy selected physical device

 

               // Destroy Presetation Surface

               if (vkTsSurfaceKHR)

               {

                              vkDestroySurfaceKHR(vkTsInstance, vkTsSurfaceKHR, NULL); // Destroy function of vkSurfaceKHR is generic function and not paltform specific

                              vkTsSurfaceKHR = VK_NULL_HANDLE;

                              fprintf(gpTsFile, "[INFO] TsUninitialize() -> vkDestroySurfaceKHR() successfully done\n");

               }

 

               if(vkDebugReportCallbackEXT && vkDestroyDebugReportCallbackEXT_fnptr)

               {

                vkDestroyDebugReportCallbackEXT_fnptr(vkTsInstance, vkDebugReportCallbackEXT, NULL);

                vkDebugReportCallbackEXT = VK_NULL_HANDLE;

                vkDestroyDebugReportCallbackEXT_fnptr = NULL;

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

               for (uint32_t i = 0; i < physicalDeviceCount; i++)

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

                              switch (vkPhysicalDeviceProperties.deviceType)

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

 // [STEP-5]
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
                                             if (vkQueueFamilyProperties_array[j].queueFlags & VK_QUEUE_GRAPHICS_BIT) // Compute cha kaam karel kiwan render cha kaam karel kiwan data copy karel
                                             {
                                                            if (VK_TRUE == isQueueSurfaceSupported_array[j])
                                                            {
                                                                           vkTsPhysicalDevice_selected = vkPhysicalDevice_array[i]; // Integrated GPU
                                                                           //vkTsPhysicalDevice_selected = vkPhysicalDevice_array[i+1]; // Discrete GPU settings
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

                              if (vkPhysicalDevice_array)
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

// [STEP-4] Platform specific WIN 
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
                              fprintf(gpTsFile, "[ERROR] TsGetSupportedSurface() -> vkCreateWin32SurfaceKHR() failed with %d at %d\n", vkTsResult, __LINE__);
                              return(vkTsResult);
               }
               else
               {
                              fprintf(gpTsFile, "[INFO] TsGetSupportedSurface() -> vkCreateWin32SurfaceKHR() succeeded at %d\n", __LINE__);
               }

               return(vkTsResult);
}

 
// [STEP-3]
VkResult TsCreateVulkanInstance(void)

{

               // Local function declaration

               VkResult TsFillInstanceExtensionNames(void);

               VkResult TsFillValidationLayerNames(void);

               VkResult TsCreateValidationCallbackFunction(void);

 

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

 

               // Fill validation names

               if(TRUE == bValidation)

               {

                vkTsResult = TsFillValidationLayerNames();

                if (VK_SUCCESS != vkTsResult)

                {

                                fprintf(gpTsFile, "[ERROR] TsCreateVulkanInstance() -> TsFillValidationLayerNames() failed at %d\n", __LINE__);

                                return(vkTsResult);

                }

                else

                {

                                fprintf(gpTsFile, "[INFO] TsCreateVulkanInstance() -> TsFillValidationLayerNames() succeeded at %d\n", __LINE__);

                }

               }              

 

               // Step 2: Initialize struct VkApplicationInfo

               VkApplicationInfo vkApplicationInfo;

               memset((void*)&vkApplicationInfo, 0, sizeof(VkApplicationInfo));

 

               vkApplicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO; // Type safety and across version

               vkApplicationInfo.pNext = NULL; // Linked list of a structure

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

 

               if(TRUE == bValidation)

               {

                vkInstanceCreateInfo.enabledLayerCount = enabledValidationLayerCount;

                vkInstanceCreateInfo.ppEnabledLayerNames = enabledValidationLayerNames_array;

               }

               else

               {

                vkInstanceCreateInfo.enabledLayerCount = 0;

                vkInstanceCreateInfo.ppEnabledLayerNames = NULL;

               }

 

               // Step 4: Call vkCreateInstance() to get VkInstance in a global variable

               vkTsResult = vkCreateInstance(&vkInstanceCreateInfo, NULL, &vkTsInstance);

               if (vkTsResult == VK_ERROR_INCOMPATIBLE_DRIVER)

               {

                              fprintf(gpTsFile, "[ERROR] TsCreateInstanceInf() -> vkCreateInstance() failed due to incompatible driver (%d) at %d\n", vkTsResult, __LINE__);

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

 

               // Initialize for validation callbacks

               if(TRUE == bValidation)

               {

                vkTsResult = TsCreateValidationCallbackFunction();

                if (VK_SUCCESS != vkTsResult)

                {

                                fprintf(gpTsFile, "[ERROR] TsCreateVulkanInstance() -> TsCreateValidationCallbackFunction() failed at %d\n", __LINE__);

                                return(vkTsResult);

                }

                else

                {

                                fprintf(gpTsFile, "[INFO] TsCreateVulkanInstance() -> TsCreateValidationCallbackFunction() succeeded at %d\n", __LINE__);

                }

               }

 

               return(vkTsResult);

}

 // [STEP-2]

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

                              instanceExtensionNames_array[i] = (char*)malloc(sizeof(char) * (strlen(vkTsExtensionProperties_array[i].extensionName) + 1));

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

               VkBool32 debugReportExtensionFound = VK_FALSE;

 

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

 

                              if (0 == strcmp(instanceExtensionNames_array[i], VK_EXT_DEBUG_REPORT_EXTENSION_NAME))

                              {

                                        debugReportExtensionFound = VK_TRUE;

                                        if(TRUE == bValidation)

                                        {

                                            enabledInstanceExtensionNames_array[enableInstanceExtentionCount++] = VK_EXT_DEBUG_REPORT_EXTENSION_NAME;

                                        }

                                        else

                                        {

                                            // Array will not have entry of VK_EXT_DEBUG_REPOR_EXTENSION_NAME   

                                        }

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

                              vkTsResult = VK_ERROR_INITIALIZATION_FAILED; // Return hardcoded failure

                              fprintf(gpTsFile, "[ERROR] TsFillInstanceExtensionNames() -> VK_KHR_SURFACE_EXTENSION_NAME not found at %d\n", __LINE__);

                              return(vkTsResult);

               }

               else

               {

                              fprintf(gpTsFile, "[INFO] TsFillInstanceExtensionNames() -> VK_KHR_SURFACE_EXTENSION_NAME found at %d\n", __LINE__);

               }

 

               if (VK_FALSE == win32SurfaceExtentionFound)

               {

                              vkTsResult = VK_ERROR_INITIALIZATION_FAILED; // Return hardcoded failure

                              fprintf(gpTsFile, "[ERROR] TsFillInstanceExtensionNames() -> VK_KHR_WIN32_SURFACE_EXTENSION_NAME not found at %d\n", __LINE__);

                              return(vkTsResult);

               }

               else

               {

                              fprintf(gpTsFile, "[INFO] TsFillInstanceExtensionNames() -> VK_KHR_WIN32_SURFACE_EXTENSION_NAME found at %d\n", __LINE__);

               }

 

               if (VK_FALSE == debugReportExtensionFound)

               {

                    if(TRUE == bValidation)

                    {

                              vkTsResult = VK_ERROR_INITIALIZATION_FAILED; // Return hardcoded failure

                              fprintf(gpTsFile, "[ERROR] TsFillInstanceExtensionNames() -> Validation is on but required VK_EXT_DEBUG_REPORT_EXTENSION_NAME is not supported at %d\n", __LINE__);

                              return(vkTsResult);

                    }

                    else

                    {

                        fprintf(gpTsFile, "[ERROR] TsFillInstanceExtensionNames() -> Validation is off and required VK_EXT_DEBUG_REPORT_EXTENSION_NAME is not supported at %d\n", __LINE__);

                    }

               }

               else

               {

                    if(TRUE == bValidation)

                    {

                        fprintf(gpTsFile, "[INFO] TsFillInstanceExtensionNames() -> Validation is ON and required VK_EXT_DEBUG_REPORT_EXTENSION_NAME is supported at %d\n", __LINE__);

                    }

                    else

                    {                       

                        fprintf(gpTsFile, "[ERROR] TsFillInstanceExtensionNames() -> Validation is off but required VK_EXT_DEBUG_REPORT_EXTENSION_NAME is supported at %d\n", __LINE__);                       

                    }

               }

 

               // Step 8: Print only supported/enaabled extension name

               for (uint32_t i = 0; i < enableInstanceExtentionCount; i++)

               {

                              fprintf(gpTsFile, "[INFO] TsFillInstanceExtensionNames() -> Enabled Vulkan Instance Extension Name = %s\n", enabledInstanceExtensionNames_array[i]);

               }

 

               //fprintf(gpTsFile, "[INFO] TsFillInstanceExtensionNames() -> ")

 

               return(vkTsResult);

}

 

VkResult TsFillValidationLayerNames(void)

{

    // Local variable declarations

    VkResult vkTsResult = VK_SUCCESS;

    uint32_t validationLayerCount = 0;

 

    // Code   

    vkTsResult = vkEnumerateInstanceLayerProperties(&validationLayerCount, NULL);      

    if (VK_SUCCESS != vkTsResult)

    {

        fprintf(gpTsFile, "[ERROR]  TsFillValidationLayerNames() -> First call to vkEnumerateInstanceLayerProperties() failad at %d. Exiting...\n", __LINE__);

        return(vkTsResult);

    }

    else

    {

        fprintf(gpTsFile, "[INFO] TsFillValidationLayerNames() -> First call to vkEnumerateInstanceLayerProperties() succeeded with validationLayerCount %d at %d\n",validationLayerCount, __LINE__);

    }   

 

    VkLayerProperties * vkLayerProperties_array = NULL;

    vkLayerProperties_array = (VkLayerProperties *)malloc(sizeof(VkLayerProperties) * validationLayerCount);

    if(NULL == vkLayerProperties_array)

    {

        fprintf(gpTsFile, "[ERROR]  TsFillValidationLayerNames() -> Failad to allocate memory for vkLayerProperties_array at %d. Exiting...\n", __LINE__);

        vkTsResult = VK_ERROR_INITIALIZATION_FAILED;

        return(vkTsResult);

    }

 

    vkTsResult = vkEnumerateInstanceLayerProperties(&validationLayerCount, vkLayerProperties_array);

    if (VK_SUCCESS != vkTsResult)

    {

        fprintf(gpTsFile, "[ERROR]  TsFillValidationLayerNames() -> Second call to vkEnumerateInstanceLayerProperties() failad at %d. Exiting...\n", __LINE__);

        return(vkTsResult);

    }

    else

    {

        fprintf(gpTsFile, "[INFO] TsFillValidationLayerNames() -> Second call to vkEnumerateInstanceLayerProperties() succeeded at %d\n", __LINE__);

    }

   

 

    char ** validationLayerNames_array = NULL;

    validationLayerNames_array = (char **)malloc(sizeof(char *) * validationLayerCount);

    if(NULL == validationLayerNames_array)

    {

        fprintf(gpTsFile, "[ERROR]  TsFillValidationLayerNames() -> Failad to allocate memory for validationLayerNames_array at %d. Exiting...\n", __LINE__);

        vkTsResult = VK_ERROR_INITIALIZATION_FAILED;

        return(vkTsResult);

    }

 

    for(uint32_t i = 0; i < validationLayerCount; i++)

    {

        validationLayerNames_array[i] = (char *)malloc(sizeof(char) * (strlen(vkLayerProperties_array[i].layerName) + 1));

        if(NULL == validationLayerNames_array[i])

        {

            fprintf(gpTsFile, "[ERROR]  TsFillValidationLayerNames() -> Failad to allocate memory for validationLayerNames_array[%d] at %d. Exiting...\n", i, __LINE__);

            vkTsResult = VK_ERROR_INITIALIZATION_FAILED;

            return(vkTsResult);

        }    

 

        //memset((void *)validationLayerNames_array[i], 0, (strlen(vkLayerProperties_array[i].layerName)+1));

        memcpy(validationLayerNames_array[i], vkLayerProperties_array[i].layerName, (strlen(vkLayerProperties_array[i].layerName)+1));

        fprintf(gpTsFile, "[INFO] TsFillValidationLayerNames() -> Vulkan Instance Layer Name = %s\n", validationLayerNames_array[i]);       

    }

 

    if(vkLayerProperties_array)

    {

        free(vkLayerProperties_array);

        vkLayerProperties_array = NULL;

    }   

 

    VkBool32 validationLayerFound = VK_FALSE;

    for(uint32_t i = 0; i < validationLayerCount; i++)

    {

        if(0 == strcmp(validationLayerNames_array[i], "VK_LAYER_KHRONOS_validation"))

        {

            validationLayerFound = VK_TRUE;

            enabledValidationLayerNames_array[enabledValidationLayerCount++] = "VK_LAYER_KHRONOS_validation";

        }       

    }

 

    for(uint32_t i = 0; i < validationLayerCount; i++)

    {

        if(validationLayerNames_array[i])

        {

            free(validationLayerNames_array[i]);

            validationLayerNames_array[i] = NULL;

        }

    }

 

    if(validationLayerNames_array)

    {

        free(validationLayerNames_array);

        validationLayerNames_array = NULL;

    }

 

    if(VK_FALSE == validationLayerFound)

    {

        vkTsResult = VK_ERROR_INITIALIZATION_FAILED;

        fprintf(gpTsFile, "[ERROR] TsFillValidationLayerNames() -> VK_LAYER_KHRONOS_validation is not supported at %d\n", __LINE__);

        return(vkTsResult);

    }

    else

    {

        fprintf(gpTsFile, "[INFO] TsFillValidationLayerNames() -> VK_LAYER_KHRONOS_validation is supported at %d\n", __LINE__);

    }

 

    for(uint32_t i = 0; i < enabledValidationLayerCount; i++)

    {

        fprintf(gpTsFile, "[INFO] TsFillValidationLayerNames() -> Enabled Vulkan Validation Layer Name %s\n", enabledValidationLayerNames_array[i]);

    }

 

    return(vkTsResult);

}

 

VkResult TsCreateValidationCallbackFunction(void)

{

    // Local function declaration

    // VKAPI_ATTR => Calling convention for gcc, Clang must be used from C++11 and above

    // VkBool32 => Return Type

    // VKAPI_CALL => Calling convention for Windows

    VKAPI_ATTR VkBool32 VKAPI_CALL debugReportCallback(VkDebugReportFlagsEXT, VkDebugReportObjectTypeEXT, uint64_t, size_t, int32_t, const char *, const char *, void *);

 

    // Local variable declarations

    VkResult vkTsResult = VK_SUCCESS;

    PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT_fnptr = NULL;

 

    // Code

    // Get required function pointers

    vkCreateDebugReportCallbackEXT_fnptr = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(vkTsInstance, "vkCreateDebugReportCallbackEXT");

    if(NULL == vkCreateDebugReportCallbackEXT_fnptr)

    {

        vkTsResult = VK_ERROR_INITIALIZATION_FAILED;

        fprintf(gpTsFile, "[ERROR] TsCreateValidationCallbackFunction() -> vkGetInstanceProcAddr() failed to get function pointer for vkCreateDebugReportCallbackEXT() with %d at %d\n", vkTsResult, __LINE__);

        return(vkTsResult);

    }

    else

    {

        fprintf(gpTsFile, "[INFO] TsCreateValidationCallbackFunction() -> vkGetInstanceProcAddr() succeeded to get function pointer for vkCreateDebugReportCallbackEXT() at %d\n", __LINE__);

    }

 

    vkDestroyDebugReportCallbackEXT_fnptr = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(vkTsInstance, "vkDestroyDebugReportCallbackEXT");

    if(NULL == vkDestroyDebugReportCallbackEXT_fnptr)

    {

        vkTsResult = VK_ERROR_INITIALIZATION_FAILED;

        fprintf(gpTsFile, "[ERROR] TsCreateValidationCallbackFunction() -> vkGetInstanceProcAddr() failed to get function pointer for vkDestroyDebugReportCallbackEXT() with %d at %d\n", vkTsResult, __LINE__);

        return(vkTsResult);

    }

    else

    {

        fprintf(gpTsFile, "[INFO] TsCreateValidationCallbackFunction() -> vkGetInstanceProcAddr() succeeded to get function pointer for vkDestroyDebugReportCallbackEXT() at %d\n", __LINE__);

    }

 

    // Declare and initialize structure to get the vulkan debug report callback object

    VkDebugReportCallbackCreateInfoEXT vkDebugReportCallbackCreateInfoEXT;

    memset((void *)&vkDebugReportCallbackCreateInfoEXT, 0, sizeof(VkDebugReportCallbackCreateInfoEXT));

 

    vkDebugReportCallbackCreateInfoEXT.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;

    vkDebugReportCallbackCreateInfoEXT.pNext = NULL;

    vkDebugReportCallbackCreateInfoEXT.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;

 

    vkDebugReportCallbackCreateInfoEXT.pfnCallback = debugReportCallback;

    vkDebugReportCallbackCreateInfoEXT.pUserData = NULL;    // Parameters to callback function

 

    vkTsResult = vkCreateDebugReportCallbackEXT_fnptr(vkTsInstance, &vkDebugReportCallbackCreateInfoEXT, NULL, &vkDebugReportCallbackEXT);

    if(VK_FALSE == vkTsResult)

    {       

        fprintf(gpTsFile, "[ERROR] TsCreateValidationCallbackFunction() -> vkCreateDebugReportCallbackEXT_fnptr() failed with %d at %d\n", vkTsResult, __LINE__);

        return(vkTsResult);

    }

    else

    {

        fprintf(gpTsFile, "[INFO] TsCreateValidationCallbackFunction() -> vkCreateDebugReportCallbackEXT_fnptr() succeeded at %d\n", __LINE__);

    }

 

    return(vkTsResult);

}

 
// [STEP-7]
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

                              deviceExtensionNames_array[i] = (char*)malloc(sizeof(char) * (strlen(vkTsExtensionProperties_array[i].extensionName) + 1));

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

                              vkTsResult = VK_ERROR_INITIALIZATION_FAILED; // Return hardcoded failure

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

 
// [STEP-8]
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

               vkDeviceCreateInfo.enabledLayerCount = 0; // Very important member and will be used as life-line

               vkDeviceCreateInfo.ppEnabledLayerNames = NULL; // Very Important member and will be used as life-line

 

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

 
// [STEP-9]
void TsGetDeviceQueue(void)
{

               // Local variable decalration

 

               // Code

               // Eka device madhye multiple queue aste

               vkGetDeviceQueue(vkDevice, graphicsQueueFamilyIndex_selected, 0, &vkQueue);

 

               if (VK_NULL_HANDLE == vkQueue)

               {

                              fprintf(gpTsFile, "[ERROR] TsGetDeviceQueue() -> vkGetDeviceQueue() returned NULL for vkQueue\n");

                              return;

               }

               else

               {

                              fprintf(gpTsFile, "[INFO] TsGetDeviceQueue() -> vkGetDeviceQueue() succeeded\n");

               }

}

 
// [STEP-10]
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

               else if (0 == formatCount)

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

               if (1 == formatCount && vkSurfaceFormatKHR_array[0].format == VK_FORMAT_UNDEFINED)

               {

                              vkFormat_color = VK_FORMAT_B8G8R8A8_UNORM; // if not found then rudimentary driver is there & swapchain will fail

               }

               else

               {

                              vkFormat_color = vkSurfaceFormatKHR_array[0].format;

               }

 

               // Decide the surface color space

               vkColorSpaceKHR = vkSurfaceFormatKHR_array[0].colorSpace;

 

               // Free the array

               if (vkSurfaceFormatKHR_array)

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

               else if (0 == presentModesCount)

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

               for (uint32_t i = 0; i < presentModesCount; i++)

               {

                              if (VK_PRESENT_MODE_MAILBOX_KHR == vkPresentModeKHR_array[i])

                              {

                                             vkPresentModeKHR = VK_PRESENT_MODE_MAILBOX_KHR;

                                             break;

                              }

               }

 

               if (VK_PRESENT_MODE_MAILBOX_KHR != vkPresentModeKHR)

               {

                              vkPresentModeKHR = VK_PRESENT_MODE_FIFO_KHR; // Important

               }

 

               // Free array

               if (vkPresentModeKHR_array)

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

               memset((void*)&vkSurfaceCapabilitiesKHR, 0, sizeof(VkSurfaceCapabilitiesKHR));

 

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

 

               if (vkSurfaceCapabilitiesKHR.maxImageCount > 0 && vkSurfaceCapabilitiesKHR.maxImageCount < testingNumberOfSwapchainImages)

               {

                              desiredNumberOfSwapchainImages = vkSurfaceCapabilitiesKHR.maxImageCount;

               }

               else

               {

                              desiredNumberOfSwapchainImages = vkSurfaceCapabilitiesKHR.minImageCount;

               }

 

               // Step 4 Choose size of swapchain image

               memset((void*)&vkExtent2D_swapchain, 0, sizeof(VkExtent2D));

               if (UINT32_MAX != vkSurfaceCapabilitiesKHR.currentExtent.width)

               {

                              vkExtent2D_swapchain.width = vkSurfaceCapabilitiesKHR.currentExtent.width;

                              vkExtent2D_swapchain.height = vkSurfaceCapabilitiesKHR.currentExtent.height;

                              fprintf(gpTsFile, "[INFO] TsCreateSwapchain() -> Swapchain Image width x height = %d x %d maxImageCount = %d and minImageCount = %d at %d\n", vkExtent2D_swapchain.width, vkExtent2D_swapchain.height, vkSurfaceCapabilitiesKHR.maxImageCount, vkSurfaceCapabilitiesKHR.minImageCount, __LINE__);

               }

               else
               {
                              // If surface size is already defined then swapchain image size must match with it
                              VkExtent2D vkExtent2D;
                              memset((void*)&vkExtent2D, 0, sizeof(VkExtent2D));
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

               VkSurfaceTransformFlagBitsKHR vkSurfaceTransformfFlagBitsKHR; // Enum

               if (vkSurfaceCapabilitiesKHR.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)

               {

                              vkSurfaceTransformfFlagBitsKHR = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR; // No transform

               }

               else

               {

                              vkSurfaceTransformfFlagBitsKHR = vkSurfaceCapabilitiesKHR.currentTransform; // If manually transformed

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

               vkSwapchainCreateInfoKHR.imageArrayLayers = 1; // In mobile(or ARM base laptop as well)/GameConsoles layered rendering is very famous as we are not using it

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

 

// [STEP-13]

VkResult TsCreateImagesAndImageViews(void)

{

               // Local variable declaration

               VkResult vkTsResult = VK_SUCCESS;

 

               // Code

               // Get swapchain image count

               vkTsResult = vkGetSwapchainImagesKHR(vkDevice, vkSwapchainKHR, &swapchainImageCount, NULL);

               if (VK_SUCCESS != vkTsResult)

               {

                              fprintf(gpTsFile, "[ERROR] TsCreateImagesAndImageViews() -> vkGetSwapchainImagesKHR() first call failed with %d at %d\n", vkTsResult, __LINE__);

                              return(vkTsResult);

               }

               else if(0 == swapchainImageCount)

               {

                              fprintf(gpTsFile, "[ERROR] TsCreateImagesAndImageViews() -> vkGetSwapchainImagesKHR() first call failed due to swapchainImageCount zero %d at %d\n", vkTsResult, __LINE__);

                              vkTsResult = VK_ERROR_INITIALIZATION_FAILED;

                              return(vkTsResult);

               }

               else

               {

                              fprintf(gpTsFile, "[INFO] TsCreateImagesAndImageViews() -> vkGetSwapchainImagesKHR() first call with swapchain desired image count: %d succeeded at %d \n", swapchainImageCount,  __LINE__);

               }

 

               // Step 2 allocate the swapchain image array'

               swapchainImage_array = (VkImage*)malloc(sizeof(VkImage) * swapchainImageCount);

               // Add error chacking for malloc()

 

               // Step 3 Fill this array by swapchain images

               vkTsResult = vkGetSwapchainImagesKHR(vkDevice, vkSwapchainKHR, &swapchainImageCount, swapchainImage_array);

               if (VK_SUCCESS != vkTsResult)

               {

                              fprintf(gpTsFile, "[ERROR] TsCreateImagesAndImageViews() -> vkGetSwapchainImagesKHR() second call failed with %d at %d\n", vkTsResult, __LINE__);

                              return(vkTsResult);

               }

               else

               {

                              fprintf(gpTsFile, "[INFO] TsCreateImagesAndImageViews() -> vkGetSwapchainImagesKHR() second call succeeded at %d \n",  __LINE__);

               }

 

               // Step 4 Allocate array of swapchain image views

               swapchainImageView_array = (VkImageView*)malloc(sizeof(VkImageView) * swapchainImageCount);

               // Add error checkin for malloc()

 

               // Step 5 fill structure initialize VkImageCreateInfo structure

               VkImageViewCreateInfo vkImageViewCreateInfo;

               memset((void*)&vkImageViewCreateInfo, 0, sizeof(VkImageViewCreateInfo));

 

               vkImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;

               vkImageViewCreateInfo.flags = 0;

               vkImageViewCreateInfo.pNext = NULL;

 

               vkImageViewCreateInfo.format = vkFormat_color; //VK_FORMAT_COLOR;

               vkImageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_R;               // VkComponentMapping structure x,y,z la r,g,b lights la mapping kartana swizzle use kartoh

               vkImageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_G;

               vkImageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_B;

               vkImageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_A;

 

               vkImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

               vkImageViewCreateInfo.subresourceRange.baseMipLevel = 0;  // 0th level paasoon shuru ker (0th index)

               vkImageViewCreateInfo.subresourceRange.levelCount = 1;                        // Mahit nahi mala atleast 1 asayla hawe

               vkImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;              // For layered rendering sathi 0th layer paasoon shuru ker

               vkImageViewCreateInfo.subresourceRange.layerCount = 1;                        // Mahit nahi atleast 1 asyla hawe

 

               vkImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;               // VkImageViewType enum cha member aahe

              

               // Step 6 fill image view array using above structure

               for(uint32_t i = 0; i < swapchainImageCount; i++)

               {

                              vkImageViewCreateInfo.image = swapchainImage_array[i];

                              vkTsResult = vkCreateImageView(vkDevice, &vkImageViewCreateInfo, NULL, &swapchainImageView_array[i]);

                              if (VK_SUCCESS != vkTsResult)

                              {

                                             fprintf(gpTsFile, "[ERROR] TsCreateImagesAndImageViews() -> vkCreateImageView() for iteration:%d failed with error (%d) at %d\n", i, vkTsResult, __LINE__);

                                             return(vkTsResult);

                              }

                              else

                              {

                                             fprintf(gpTsFile, "[INFO] TsCreateImagesAndImageViews() -> vkCreateImageView() for iteration:%d succeeded at %d \n", i,  __LINE__);

                              }

               }

 

               return(vkTsResult);

}

 

VkResult TsCreateCommandPool(void)

{

               // Local variable declaration

               VkResult vkTsResult = VK_SUCCESS;

 

               // Code

               // Create

               VkCommandPoolCreateInfo vkCommandPoolCreateInfo;

               memset((void*)&vkCommandPoolCreateInfo, 0, sizeof(VkCommandPoolCreateInfo));

 

               // Initialize

               vkCommandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;

               vkCommandPoolCreateInfo.pNext = NULL;

 

               // Create command pools which are capable of resetted and restarted.

               // These command buffers are long lived

               vkCommandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

 

               vkCommandPoolCreateInfo.queueFamilyIndex = graphicsQueueFamilyIndex_selected;

 

               vkTsResult = vkCreateCommandPool(vkDevice, &vkCommandPoolCreateInfo, NULL, &vkCommandPool);

               if (VK_SUCCESS != vkTsResult)

               {

                              fprintf(gpTsFile, "[ERROR] TsCreateCommandPool() -> vkCreateCommandPool() second call failed with %d at %d\n", vkTsResult, __LINE__);

                              return(vkTsResult);

               }

               else

               {

                              fprintf(gpTsFile, "[INFO] TsCreateCommandPool() -> vkCreateCommandPool() second call succeeded at %d \n",  __LINE__);

               }

 

               return(vkTsResult);

}

 

VkResult TsCreateCommandBuffers(void)

{

               // Local variable declaration

               VkResult vkTsResult = VK_SUCCESS;

 

               // Code

               // Initialize

               VkCommandBufferAllocateInfo vkCommandBufferAllocateInfo;

               memset((void*)&vkCommandBufferAllocateInfo, 0, sizeof(VkCommandBufferAllocateInfo));

 

               vkCommandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;

               vkCommandBufferAllocateInfo.pNext = NULL;

               //vkCommandBufferAllocateInfo.flags = 0;

 

               vkCommandBufferAllocateInfo.commandPool = vkCommandPool;

 

               // Primary and Secondary command buffer.

               // Primary is used to submit command to queue

               // Secondary command buffers can be called within Primary command buffer

               vkCommandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;            

               vkCommandBufferAllocateInfo.commandBufferCount = 1;

 

               //

               vkCommandBuffer_array = (VkCommandBuffer*)malloc(sizeof(VkCommandBuffer) * swapchainImageCount);

               // Add error checking for malloc()

 

               // Step 3 allocate command buffers

               for(uint32_t i = 0; i < swapchainImageCount; i++)

               {

                              vkTsResult = vkAllocateCommandBuffers(vkDevice, &vkCommandBufferAllocateInfo, &vkCommandBuffer_array[i]);

                              if (VK_SUCCESS != vkTsResult)

                              {

                                             fprintf(gpTsFile, "[ERROR] TsCreateCommandBuffers() -> vkAllocateCommandBuffers() for iteration:%d failed with error (%d) at %d\n", i, vkTsResult, __LINE__);

                                             return(vkTsResult);

                              }

                              else

                              {

                                             fprintf(gpTsFile, "[INFO] TsCreateCommandBuffers() -> vkAllocateCommandBuffers() for iteration:%d succeeded at %d \n", i,  __LINE__);

                              }

               }

 

               return(vkTsResult);

}

 

// [STEP-22] create vertex buffer

VkResult TsCreateVertexBuffer(void)

{

    // Local variable decalration

    VkResult vkTsResult = VK_SUCCESS;

 

    // Code

    // Step 3 Declare a structure

    float triangle_position[] = {

        0.0f, 1.0f, 0.0f,

        -1.0f, -1.0f, 0.0f,

        1.0f, -1.0f, 0.0f

    };

 

    // Step 4

    memset((void*)&vertexData_position, 0, sizeof(VertexData));

   

    // Step 5

    VkBufferCreateInfo vkBufferCreateInfo;

    memset((void*)&vkBufferCreateInfo, 0, sizeof(VkBufferCreateInfo));

 

    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;

    vkBufferCreateInfo.pNext = NULL;

    vkBufferCreateInfo.flags = 0;   // Valid flags used in scattered/sparse buffer

 

    vkBufferCreateInfo.size = sizeof(triangle_position);

    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

 

    // sharingMode = 0 due to memset() which means Exclusive

    // In vulkan memory is not done in Bytes while it is done in Regions

    // and i.e. minimum it is 4096 & it is deliberatly made as Vulkan demands small number of large size allocations

    // & use them repeatetively for different resources

    // sharingMode = 1 then other 2 members need to fill with queue family index and queue family array

 

    // Step 6

    vkTsResult = vkCreateBuffer(vkDevice, &vkBufferCreateInfo, NULL, &vertexData_position.vkBuffer);

    if (VK_SUCCESS != vkTsResult)

    {

        fprintf(gpTsFile, "[ERROR] TsCreateVertexBuffer() -> vkCreateBuffer() failed at %d\n", __LINE__);

        return(vkTsResult);

    }

    else

    {

        fprintf(gpTsFile, "[INFO] TsCreateVertexBuffer() -> vkCreateBuffer() succeeded at %d \n", __LINE__);

    }

 

    // Step 7

    VkMemoryRequirements vkMemoryRequirements;

    memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));

    vkGetBufferMemoryRequirements(vkDevice, vertexData_position.vkBuffer, &vkMemoryRequirements);

    // No error checking

 

    // Step 8

    VkMemoryAllocateInfo vkMemoryAllocateInfo;

    memset((void*)&vkMemoryAllocateInfo, 0, sizeof(VkMemoryAllocateInfo));

 

    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

    vkMemoryAllocateInfo.pNext = NULL;

 

    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;   // This size is converted to Region wise allocation as needed for device memory

    vkMemoryAllocateInfo.memoryTypeIndex = 0;   // Initial value before entering the loop

    // Step a

    for(uint32_t i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)

    {

        // Step b

        if(1 == (vkMemoryRequirements.memoryTypeBits & 1))

        {

            // Step c

            if(vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)

            {

                // Step d

                vkMemoryAllocateInfo.memoryTypeIndex = i;

                break;

            }

        }

 

        // Step e

        vkMemoryRequirements.memoryTypeBits >>= 1;

    }

 

    // Step 9

    vkTsResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &vertexData_position.vkDeviceMemory);

    if (VK_SUCCESS != vkTsResult)

    {

        fprintf(gpTsFile, "[ERROR] TsCreateVertexBuffer() -> vkAllocateMemory() failed at %d\n", __LINE__);

        return(vkTsResult);

    }

    else

    {

        fprintf(gpTsFile, "[INFO] TsCreateVertexBuffer() -> vkAllocateMemory() succeeded at %d \n", __LINE__);

    }

 

    // Step 10 it binds vulkan device buffer object handle with vulkan device memory object handle

    vkTsResult = vkBindBufferMemory(vkDevice, vertexData_position.vkBuffer, vertexData_position.vkDeviceMemory, 0);

    if (VK_SUCCESS != vkTsResult)

    {

        fprintf(gpTsFile, "[ERROR] TsCreateVertexBuffer() -> vkBindBufferMemory() failed at %d\n", __LINE__);

        return(vkTsResult);

    }

    else

    {

        fprintf(gpTsFile, "[INFO] TsCreateVertexBuffer() -> vkBindBufferMemory() succeeded at %d \n", __LINE__);

    }

 

    // Step 11

    void * data = NULL;

    //                                                                                              

    vkTsResult = vkMapMemory(

        vkDevice,

        vertexData_position.vkDeviceMemory, //

        0, // start

        vkMemoryAllocateInfo.allocationSize, // sizeof

        0, // Reserved

        &data

    );

    if (VK_SUCCESS != vkTsResult)

    {

        fprintf(gpTsFile, "[ERROR] TsCreateVertexBuffer() -> vkMapMemory() failed at %d\n", __LINE__);

        return(vkTsResult);

    }

    else

    {

        fprintf(gpTsFile, "[INFO] TsCreateVertexBuffer() -> vkMapMemory() succeeded at %d \n", __LINE__);

    }

 

    // Step 12

    memcpy(data, triangle_position, sizeof(triangle_position));

 

    // Step 13

    vkUnmapMemory(vkDevice, vertexData_position.vkDeviceMemory);

 

    return(vkTsResult);

}

 

// [STEP-23]

VkResult TsCreateShaders(void)

{

    // Local variable declaration

    VkResult vkTsResult = VK_SUCCESS;

 

    // Code

    // For vertex shader

    const char* szFileName = "Shader.vert.spv";

    FILE * fp = NULL;

    size_t size;

 

    fp = fopen(szFileName, "rb");

    if(NULL == fp)

    {

        fprintf(gpTsFile, "[ERROR] TsCreateShaders() -> Failed to open %s at %d\n", szFileName, __LINE__);

        vkTsResult = VK_ERROR_INITIALIZATION_FAILED;

        return(vkTsResult);

    }

    else

    {

        fprintf(gpTsFile, "[INFO] TsCreateShaders() -> Succeeded to open %s at %d\n", szFileName, __LINE__);

    }

 

    fseek(fp, 0L, SEEK_END);    // Seek till end of file

    size = ftell(fp);   // File pointer jithe aahe ti jaga mala sang i.e. byte size of file

    if(0 == size)

    {

        fprintf(gpTsFile, "[ERROR] TsCreateShaders() -> The SPIRV shader file size 0 %s at %d\n", szFileName, __LINE__);

        vkTsResult = VK_ERROR_INITIALIZATION_FAILED;

        return(vkTsResult);

    }

 

    fseek(fp, 0L, SEEK_SET);    // Reset at start of the file, SEEK_CURR = Current Position in file.   

 

    char * shaderData = (char *)malloc(sizeof(char) * size);

    size_t retVal = fread(shaderData, size, 1, fp);

    if(1 != retVal)

    {

        fprintf(gpTsFile, "[ERROR] TsCreateShaders() -> Failed to read the SPIRV shader file %s at %d\n", szFileName, __LINE__);

        vkTsResult = VK_ERROR_INITIALIZATION_FAILED;

        return(vkTsResult);

    }

    else

    {

        fprintf(gpTsFile, "[INFO] TsCreateShaders() -> Succeeded to read %s at %d\n", szFileName, __LINE__);

    }

 

    fclose(fp);

 

    VkShaderModuleCreateInfo vkShaderModuleCreateInfo;

    memset((void *)&vkShaderModuleCreateInfo, 0, sizeof(VkShaderModuleCreateInfo));

 

    vkShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

    vkShaderModuleCreateInfo.pNext = NULL;

    vkShaderModuleCreateInfo.flags = 0; // Reserved for future use

 

    vkShaderModuleCreateInfo.codeSize = size;

    vkShaderModuleCreateInfo.pCode = (uint32_t *)shaderData;

 

    vkTsResult = vkCreateShaderModule(vkDevice, &vkShaderModuleCreateInfo, NULL, &vkShaderModule_vertex_shader);

    if(NULL == fp)

    {

        fprintf(gpTsFile, "[ERROR] TsCreateShaders() -> vkCreateShaderModule() Failed at %d\n", __LINE__);

        return(vkTsResult);

    }

    else

    {

        fprintf(gpTsFile, "[INFO] TsCreateShaders() -> vkCreateShaderModule() Succeeded at %d\n", __LINE__);

    }

 

    if(shaderData)

    {

        free(shaderData);

        shaderData = NULL;

    }

 

    fprintf(gpTsFile, "[INFO] TsCreateShaders() -> Vertex Shader Module is successfully created!!!\n");

 

    // For fragment shader

    szFileName = "Shader.frag.spv";

    fp = NULL;

    size = 0;

 

    fp = fopen(szFileName, "rb");

    if(NULL == fp)

    {

        fprintf(gpTsFile, "[ERROR] TsCreateShaders() -> Failed to open %s at %d\n", szFileName, __LINE__);

        vkTsResult = VK_ERROR_INITIALIZATION_FAILED;

        return(vkTsResult);

    }

    else

    {

        fprintf(gpTsFile, "[INFO] TsCreateShaders() -> Succeeded to open %s at %d\n", szFileName, __LINE__);

    }

 

    fseek(fp, 0L, SEEK_END);    // Seek till end of file

    size = ftell(fp);   // File pointer jithe aahe ti jaga mala sang i.e. byte size of file

    if(0 == size)

    {

        fprintf(gpTsFile, "[ERROR] TsCreateShaders() -> The SPIRV shader file size 0 %s at %d\n", szFileName, __LINE__);

        vkTsResult = VK_ERROR_INITIALIZATION_FAILED;

        return(vkTsResult);

    }

 

    fseek(fp, 0L, SEEK_SET);    // Reset at start of the file, SEEK_CURR = Current Position in file.   

 

    shaderData = (char *)malloc(sizeof(char) * size);

    retVal = fread(shaderData, size, 1, fp);

    if(1 != retVal)

    {

        fprintf(gpTsFile, "[ERROR] TsCreateShaders() -> Failed to read the SPIRV shader file %s at %d\n", szFileName, __LINE__);

        vkTsResult = VK_ERROR_INITIALIZATION_FAILED;

        return(vkTsResult);

    }

    else

    {

        fprintf(gpTsFile, "[INFO] TsCreateShaders() -> Succeeded to read %s at %d\n", szFileName, __LINE__);

    }

 

    fclose(fp);

 

    // Reset struct variable

    memset((void *)&vkShaderModuleCreateInfo, 0, sizeof(VkShaderModuleCreateInfo));

 

    vkShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

    vkShaderModuleCreateInfo.pNext = NULL;

    vkShaderModuleCreateInfo.flags = 0; // Reserved for future use

 

    vkShaderModuleCreateInfo.codeSize = size;

    vkShaderModuleCreateInfo.pCode = (uint32_t *)shaderData;

 

    vkTsResult = vkCreateShaderModule(vkDevice, &vkShaderModuleCreateInfo, NULL, &vkShaderModule_fragment_shader);

    if(NULL == fp)

    {

        fprintf(gpTsFile, "[ERROR] TsCreateShaders() -> vkCreateShaderModule() Failed in fragement shader at %d\n", __LINE__);

        return(vkTsResult);

    }

    else

    {

        fprintf(gpTsFile, "[INFO] TsCreateShaders() -> vkCreateShaderModule() Succeeded in fragment shader at %d\n", __LINE__);

    }

 

    if(shaderData)

    {

        free(shaderData);

        shaderData = NULL;

    }

 

    fprintf(gpTsFile, "[INFO] TsCreateShaders() -> Fragment Shader Module is successfully created!!!\n");

 

    return(vkTsResult);

}

 

// [STEP-24]

VkResult TsCreateDescriptorSetLayout(void)

{

    // Local variable declaration

    VkResult vkTsResult = VK_SUCCESS;

 

    // Code

    VkDescriptorSetLayoutCreateInfo vkDescriptorSetLayoutCreateInfo;

    memset((void *)&vkDescriptorSetLayoutCreateInfo, 0, sizeof(VkDescriptorSetLayoutCreateInfo));

 

    vkDescriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;

    vkDescriptorSetLayoutCreateInfo.pNext = NULL;

    vkDescriptorSetLayoutCreateInfo.flags = 0;  // Reserved for future use

 

    vkDescriptorSetLayoutCreateInfo.bindingCount = 0;   // Descriptor set sadhyala nahiye

    vkDescriptorSetLayoutCreateInfo.pBindings = NULL;   // Pointing to array of VkDescriptorSetLayout structure

 

    vkTsResult = vkCreateDescriptorSetLayout(vkDevice, &vkDescriptorSetLayoutCreateInfo, NULL, &vkDescriptorSetLayout);

    if(VK_SUCCESS != vkTsResult)

    {

        fprintf(gpTsFile, "[ERROR] TsCreateDescriptorSetLayout() -> vkCreateDescriptorSetLayout() Failed at %d\n", __LINE__);

        return(vkTsResult);

    }

    else

    {

        fprintf(gpTsFile, "[INFO] TsCreateDescriptorSetLayout() -> vkCreateDescriptorSetLayout() Succeeded at %d\n", __LINE__);

    }

 

    return(vkTsResult);

}

 

// [STEP-25]

VkResult TsCreatePipelineLayout(void)

{

    // Local variable declaration

    VkResult vkTsResult = VK_SUCCESS;

 

    // Code

    VkPipelineLayoutCreateInfo vkPipelineLayoutCreateInfo;

    memset((void *)&vkPipelineLayoutCreateInfo, 0, sizeof(VkPipelineLayoutCreateInfo));

 

    vkPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

    vkPipelineLayoutCreateInfo.pNext = NULL;

    vkPipelineLayoutCreateInfo.flags = 0;   // Reserved for future use

 

    vkPipelineLayoutCreateInfo.setLayoutCount = 1;  // Even empty we need atleast 1 here

    vkPipelineLayoutCreateInfo.pSetLayouts = &vkDescriptorSetLayout;

 

    vkPipelineLayoutCreateInfo.pushConstantRangeCount = 0;

    vkPipelineLayoutCreateInfo.pPushConstantRanges = NULL;

 

    vkTsResult = vkCreatePipelineLayout(vkDevice, &vkPipelineLayoutCreateInfo, NULL, &vkPipelineLayout);

    if(VK_SUCCESS != vkTsResult)

    {

        fprintf(gpTsFile, "[ERROR] TsCreatePipelineLayout() -> vkCreatePipelineLayout() Failed at %d\n", __LINE__);

        return(vkTsResult);

    }

    else

    {

        fprintf(gpTsFile, "[INFO] TsCreatePipelineLayout() -> vkCreatePipelineLayoutg() Succeeded at %d\n", __LINE__);

    }

 

    return(vkTsResult);

}

 

VkResult TsCreateRenderPass(void)

{

               // Local variable declaration

               VkResult vkTsResult = VK_SUCCESS;

 

               // Code

               // Step 1 Declare and initialize VkAttachmentDescription structure

               VkAttachmentDescription vkAttachmentDescription_array[1];

               memset((void *)vkAttachmentDescription_array, 0, sizeof(VkAttachmentDescription) * ARRAY_SIZE(vkAttachmentDescription_array));

 

               vkAttachmentDescription_array[0].flags = 0;

               vkAttachmentDescription_array[0].format = vkFormat_color;

               vkAttachmentDescription_array[0].samples = VK_SAMPLE_COUNT_1_BIT; // Aamchi image multi sampling wali nahiye

               // While going inside what has to be loaded

               // aat alya aalya hi layer clear karu shaktoh

               // Clear color ne clear karnaar aahot

               vkAttachmentDescription_array[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;

               vkAttachmentDescription_array[0].storeOp =  VK_ATTACHMENT_STORE_OP_STORE; // While going outside what needs to be done.

               vkAttachmentDescription_array[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; // It consist of both depth & stencil

               vkAttachmentDescription_array[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

               vkAttachmentDescription_array[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // Image Barrier, ATTACHMENT's Image chya datashi aat yetaanaa kaay karych

               vkAttachmentDescription_array[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // It deals with data arrangement. Similar to input source data arrangement.

 

               // Step 2 declare and initialize VkAttachmentRefe

               VkAttachmentReference vkAttachmentReference;

               memset((void*)&vkAttachmentReference, 0, sizeof(VkAttachmentReference));

               vkAttachmentReference.attachment = 0; // verchya array madhye 0th element/index refering

               vkAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // Layout kasa thevaychay, kasha sathi wapar. Asse thev layout ki mi tyala color sathi use karusahktoh

 

               // Step 3 declare & initialize VkSubpassDescription

               // Subpass is similar to main thread

               VkSubpassDescription vkSubpassDescription;

               //memset((void*)&vkSubpassDescription, 0 sizeof(VkSubpassDescription));

               memset((void*)&vkSubpassDescription, 0, sizeof(VkSubpassDescription));

               vkSubpassDescription.flags = 0;

               vkSubpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

               vkSubpassDescription.inputAttachmentCount = 0;

               vkSubpassDescription.pInputAttachments = NULL;

               vkSubpassDescription.colorAttachmentCount = ARRAY_SIZE(vkAttachmentDescription_array);

               vkSubpassDescription.pColorAttachments = &vkAttachmentReference;

               vkSubpassDescription.pResolveAttachments = NULL;

               vkSubpassDescription.pDepthStencilAttachment = NULL;

               vkSubpassDescription.preserveAttachmentCount = 0;

               vkSubpassDescription.pPreserveAttachments = NULL;

 

               // Step 4 declare & initialize VkRenderPassCreateInfo structure

               VkRenderPassCreateInfo vkRenderPassCreateInfo;

               memset((void*)&vkRenderPassCreateInfo, 0, sizeof(VkRenderPassCreateInfo));

               vkRenderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

               vkRenderPassCreateInfo.pNext = NULL;

               vkRenderPassCreateInfo.flags = 0;

 

               vkRenderPassCreateInfo.attachmentCount = ARRAY_SIZE(vkAttachmentDescription_array);

               vkRenderPassCreateInfo.pAttachments = vkAttachmentDescription_array;     // No specific attachments but its cumulative of all types of attachments

               vkRenderPassCreateInfo.subpassCount = 1;

               vkRenderPassCreateInfo.pSubpasses = &vkSubpassDescription;

               vkRenderPassCreateInfo.dependencyCount = 0;

               vkRenderPassCreateInfo.pDependencies = NULL;

 

               // Step 5 call vkCreateRenderPass() API to create actual render pass

               vkTsResult = vkCreateRenderPass(vkDevice, &vkRenderPassCreateInfo, NULL, &vkRenderPass);

               if (VK_SUCCESS != vkTsResult)

               {

                              fprintf(gpTsFile, "[ERROR] TsCreateRenderPass() -> vkCreateRenderPass() failed with %d at %d\n", vkTsResult, __LINE__);

                              return(vkTsResult);

               }

               else

               {

                              fprintf(gpTsFile, "[INFO] TsCreateRenderPass() -> vkCreateRenderPass() succeeded at %d \n",  __LINE__);

               }

 

               return(vkTsResult);

}

 

// [STEP-26] Create Pipeline

VkResult TsCreatePipeline(void)

{

    // Local variable declaration

    VkResult vkTsResult = VK_SUCCESS;

 

    // Code

    // Pipeline State Objects PSO

    // 1. Vertex input state

    VkVertexInputBindingDescription vkVertexInputBindingDescription_array[1];   // 1 means position if 2 then position, color etc.

    memset((void *)vkVertexInputBindingDescription_array, 0, sizeof(VkVertexInputBindingDescription) * ARRAY_SIZE(vkVertexInputBindingDescription_array));

 

    // Majha pahila buffer aahe toh 0th index la put ker i.e. in GL_ARRAY_BUFFER in OpenGL

    vkVertexInputBindingDescription_array[0].binding = 0;   //

    vkVertexInputBindingDescription_array[0].stride = sizeof(float) * 3;    // take step in 3 or 12 bytes

    vkVertexInputBindingDescription_array[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;   // This is vertices rate i.e. kimmat

 

    // Shader madhle attributes

    VkVertexInputAttributeDescription vkVertexInputAttributeDescription_array[1];  // 1 means position if 2 then position, color etc.

    memset((void *)vkVertexInputAttributeDescription_array, 0, sizeof(VkVertexInputAttributeDescription) * ARRAY_SIZE(vkVertexInputAttributeDescription_array));

 

    vkVertexInputAttributeDescription_array[0].binding = 0;     // vkVertexInputBindingDescription_array[0].binding = 0 <-> vkVertexInputAttributeDescription_array[0].binding = 0; but this will change in interleave

    vkVertexInputAttributeDescription_array[0].location = 0;    // layout(location = 0) in vec4 vPosition; // in Shader.vert

    vkVertexInputAttributeDescription_array[0].format = VK_FORMAT_R32G32B32_SFLOAT;     // Majha vertex X Y Z aahe te Signed Float 32-bit R32 = X, G32 = Y, G32 = Z

    vkVertexInputAttributeDescription_array[0].offset = 0;      // In interleaved how to jump in attribute layout. For now it is 0

 

    VkPipelineVertexInputStateCreateInfo vkPipelineVertexInputStateCreateInfo;

    memset((void *)&vkPipelineVertexInputStateCreateInfo, 0, sizeof(VkPipelineVertexInputStateCreateInfo));

 

    vkPipelineVertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    vkPipelineVertexInputStateCreateInfo.pNext = NULL;

    vkPipelineVertexInputStateCreateInfo.flags = 0;     // Reserved for future use

 

    vkPipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = ARRAY_SIZE(vkVertexInputBindingDescription_array);

    vkPipelineVertexInputStateCreateInfo.pVertexBindingDescriptions = vkVertexInputBindingDescription_array;

    vkPipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = ARRAY_SIZE(vkVertexInputAttributeDescription_array);

    vkPipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions = vkVertexInputAttributeDescription_array;

 

    // 2. Input Assembly State

    VkPipelineInputAssemblyStateCreateInfo vkPipelineInputAssemblyStateCreateInfo;

    memset((void*)&vkPipelineInputAssemblyStateCreateInfo, 0, sizeof(VkPipelineInputAssemblyStateCreateInfo));

 

    vkPipelineInputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;

    vkPipelineInputAssemblyStateCreateInfo.pNext = NULL;

    vkPipelineInputAssemblyStateCreateInfo.flags = 0;   // Reserved

 

    vkPipelineInputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    //vkPipelineInputAssemblyStateCreateInfo.primitiveRestartEnable = ; // This is used for Geometry Shader sathich lagtoh and Indexed drawing sathi lagtoh

 

    // 3. Rasterizer State

    VkPipelineRasterizationStateCreateInfo vkPipelineRasterizationStateCreateInfo;

    memset((void*)&vkPipelineRasterizationStateCreateInfo, 0, sizeof(VkPipelineRasterizationStateCreateInfo));

 

    vkPipelineRasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;

    vkPipelineRasterizationStateCreateInfo.pNext = NULL;

    vkPipelineRasterizationStateCreateInfo.flags = 0;   // Reserved

 

    vkPipelineRasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;  // GL_POLYGON

    vkPipelineRasterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;    // While drawing line and point both front and back culling is required as it do not have face

    vkPipelineRasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE; // Triangle winding order

    vkPipelineRasterizationStateCreateInfo.lineWidth = 1.0f;    // Must be minimum 1

 

    // 4. Color Blend State

    VkPipelineColorBlendAttachmentState vkPipelineColorBlendAttachmentState_array[1];

    memset((void*)vkPipelineColorBlendAttachmentState_array, 0, sizeof(VkPipelineColorBlendAttachmentState) * ARRAY_SIZE(vkPipelineColorBlendAttachmentState_array));

 

    vkPipelineColorBlendAttachmentState_array[0].colorWriteMask = 0xf;  //

    vkPipelineColorBlendAttachmentState_array[0].blendEnable = VK_FALSE;

 

    VkPipelineColorBlendStateCreateInfo vkPipelineColorBlendStateCreateInfo;

    memset((void*)&vkPipelineColorBlendStateCreateInfo, 0, sizeof(VkPipelineColorBlendStateCreateInfo));

 

    vkPipelineColorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;

    vkPipelineColorBlendStateCreateInfo.pNext = NULL;

    vkPipelineColorBlendStateCreateInfo.flags = 0;  // Change as per color blend

 

    vkPipelineColorBlendStateCreateInfo.attachmentCount = ARRAY_SIZE(vkPipelineColorBlendAttachmentState_array);

    vkPipelineColorBlendStateCreateInfo.pAttachments = vkPipelineColorBlendAttachmentState_array;

 

    // 5. Viewport or scissor state

    VkPipelineViewportStateCreateInfo vkPipelineViewportStateCreateInfo;

    memset((void*)&vkPipelineViewportStateCreateInfo, 0, sizeof(VkPipelineViewportStateCreateInfo));

 

    vkPipelineViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;

    vkPipelineViewportStateCreateInfo.pNext = NULL;

    vkPipelineViewportStateCreateInfo.flags = 0;

 

    vkPipelineViewportStateCreateInfo.viewportCount = 1;    // That means we can specify multiple viewports

 

    // Create Viewport

    memset((void*)&vkViewport, 0, sizeof(VkViewport));

    vkViewport.x = 0;

    vkViewport.y = 0;

    vkViewport.width = (float)vkExtent2D_swapchain.width;

    vkViewport.height = (float)vkExtent2D_swapchain.height;

    vkViewport.minDepth = 0.0f;

    vkViewport.maxDepth = 1.0f;

 

    vkPipelineViewportStateCreateInfo.pViewports = &vkViewport;

 

    vkPipelineViewportStateCreateInfo.scissorCount = 1;     // Must be same as viewportCount

 

    memset((void*)&vkRect2D_scissor, 0, sizeof(VkRect2D));

    vkRect2D_scissor.offset.x = 0;

    vkRect2D_scissor.offset.y = 0;

    vkRect2D_scissor.extent.width = vkExtent2D_swapchain.width;

    vkRect2D_scissor.extent.height = vkExtent2D_swapchain.height;

 

    vkPipelineViewportStateCreateInfo.pScissors = &vkRect2D_scissor;

 

    // 6. Depth Stencil state

    // As we do not have depth yet, we can omit this state

 

    // 7. Dynamic State

    // We do not have any dynamic state, skip

 

    // 8. Multi Sample State needed for Fragment Sahder

    VkPipelineMultisampleStateCreateInfo vkPipelineMultisampleStateCreateInfo;

    memset((void*)&vkPipelineMultisampleStateCreateInfo, 0, sizeof(VkPipelineMultisampleStateCreateInfo));

 

    vkPipelineMultisampleStateCreateInfo.sType =  VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;

    vkPipelineMultisampleStateCreateInfo.pNext = NULL;

    vkPipelineMultisampleStateCreateInfo.flags = 0; // Reserved

 

    vkPipelineMultisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;   // Validation error yete mhanoon yala value dyawa lagtoh

 

    // 9. SHADER STATE must be an array

    VkPipelineShaderStageCreateInfo vkPipelineShaderStageCreateInfo_array[2];   // Vertex and Fragment shaders

    memset((void*)vkPipelineShaderStageCreateInfo_array, 0, sizeof(VkPipelineShaderStageCreateInfo) * ARRAY_SIZE(vkPipelineShaderStageCreateInfo_array));

 

    // Vertex shader

    vkPipelineShaderStageCreateInfo_array[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;

    vkPipelineShaderStageCreateInfo_array[0].pNext = NULL;  // jya jya structures na extensions aahet tya tya structure la pNext dena must aahe

    vkPipelineShaderStageCreateInfo_array[0].flags = 0;

 

    vkPipelineShaderStageCreateInfo_array[0].stage = VK_SHADER_STAGE_VERTEX_BIT;

    vkPipelineShaderStageCreateInfo_array[0].module = vkShaderModule_vertex_shader;

    vkPipelineShaderStageCreateInfo_array[0].pName = "main";    // Name of entry point function of vertex shader

    vkPipelineShaderStageCreateInfo_array[0].pSpecializationInfo = NULL;    // LLVM (Low Level Virtual Machine) precompiled binding of constants in SPIR-V

 

    // Fragment shader

    vkPipelineShaderStageCreateInfo_array[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;

    vkPipelineShaderStageCreateInfo_array[1].pNext = NULL;  // jya jya structures na extensions aahet tya tya structure la pNext dena must aahe

    vkPipelineShaderStageCreateInfo_array[1].flags = 0;

 

    vkPipelineShaderStageCreateInfo_array[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;

    vkPipelineShaderStageCreateInfo_array[1].module = vkShaderModule_fragment_shader;

    vkPipelineShaderStageCreateInfo_array[1].pName = "main";    // Name of entry point function of vertex shader

    vkPipelineShaderStageCreateInfo_array[1].pSpecializationInfo = NULL;    // LLVM (Low Level Virtual Machine) precompiled binding of constants in SPIR-V

 

    // 10. Tessallation State

    // We do not have Tessillation shaders, so we can skip this state

 

    // As piplines are created from pipeline cache, now we will create pipeline cache object

    VkPipelineCacheCreateInfo vkPipelineCacheCreateInfo;

    memset((void*)&vkPipelineCacheCreateInfo, 0, sizeof(VkPipelineCacheCreateInfo));

 

    vkPipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

    vkPipelineCacheCreateInfo.pNext = NULL;

    vkPipelineCacheCreateInfo.flags = 0;

 

    VkPipelineCache vkPipelineCache = VK_NULL_HANDLE;

 

    vkTsResult = vkCreatePipelineCache(vkDevice, &vkPipelineCacheCreateInfo, NULL, &vkPipelineCache);

    if(VK_SUCCESS != vkTsResult)

    {

        fprintf(gpTsFile, "[ERROR] TsCreatePipeline() -> vkCreatePipelineCache() Failed at %d\n", __LINE__);

        return(vkTsResult);

    }

    else

    {

        fprintf(gpTsFile, "[INFO] TsCreatePipeline() -> vkCreatePipelineCache() Succeeded at %d\n", __LINE__);

    }

 

    // Create the actual graphics pipeline

    VkGraphicsPipelineCreateInfo vkGraphicsPipelineCreateInfo;

    memset((void*)&vkGraphicsPipelineCreateInfo, 0, sizeof(VkGraphicsPipelineCreateInfo));

 

    vkGraphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

    vkGraphicsPipelineCreateInfo.pNext = NULL;

    vkGraphicsPipelineCreateInfo.flags = 0;

 

    vkGraphicsPipelineCreateInfo.pVertexInputState = &vkPipelineVertexInputStateCreateInfo;

    vkGraphicsPipelineCreateInfo.pInputAssemblyState = &vkPipelineInputAssemblyStateCreateInfo;

    vkGraphicsPipelineCreateInfo.pRasterizationState = &vkPipelineRasterizationStateCreateInfo;

    vkGraphicsPipelineCreateInfo.pColorBlendState = &vkPipelineColorBlendStateCreateInfo;

    vkGraphicsPipelineCreateInfo.pViewportState = &vkPipelineViewportStateCreateInfo;

    vkGraphicsPipelineCreateInfo.pDepthStencilState = NULL;

    vkGraphicsPipelineCreateInfo.pDynamicState = NULL;

    vkGraphicsPipelineCreateInfo.pMultisampleState = &vkPipelineMultisampleStateCreateInfo;

    vkGraphicsPipelineCreateInfo.stageCount = ARRAY_SIZE(vkPipelineShaderStageCreateInfo_array);

    vkGraphicsPipelineCreateInfo.pStages = vkPipelineShaderStageCreateInfo_array;

    vkGraphicsPipelineCreateInfo.pTessellationState = NULL;

   

    vkGraphicsPipelineCreateInfo.layout = vkPipelineLayout;

    vkGraphicsPipelineCreateInfo.renderPass = vkRenderPass;

    vkGraphicsPipelineCreateInfo.subpass = 0;   // As we have only 1 renderpass

    

    vkGraphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;

    vkGraphicsPipelineCreateInfo.basePipelineIndex = 0;

 

    // Now create the pipeline

    vkTsResult = vkCreateGraphicsPipelines(vkDevice, vkPipelineCache,

        1,  // only have 1 pipeline

        &vkGraphicsPipelineCreateInfo,

        NULL,   // Allocator default

        &vkPipeline

    );

 

    if(VK_SUCCESS != vkTsResult)

    {

        fprintf(gpTsFile, "[ERROR] TsCreatePipeline() -> vkCreateGraphicsPipelines() Failed at %d\n", __LINE__);

        // We are done with pipeline cache

        if(vkPipelineCache)

        {

            vkDestroyPipelineCache(vkDevice, vkPipelineCache, NULL);

            vkPipelineCache = VK_NULL_HANDLE;

            fprintf(gpTsFile, "[INFO] TsCreatePipeline() -> vkDestroyPipelineCache() Succeeded at %d\n", __LINE__);

        }

        return(vkTsResult);

    }

    else

    {

        fprintf(gpTsFile, "[INFO] TsCreatePipeline() -> vkCreateGraphicsPipelines() Succeeded at %d\n", __LINE__);

    }

 

    // We are done with pipeline cache

    if(vkPipelineCache)

    {

        vkDestroyPipelineCache(vkDevice, vkPipelineCache, NULL);

        vkPipelineCache = VK_NULL_HANDLE;

        fprintf(gpTsFile, "[INFO] TsCreatePipeline() -> vkDestroyPipelineCache() Succeeded at %d\n", __LINE__);

    }

 

    return(vkTsResult);

}

 

VkResult TsCreateFrameBuffers(void)

{

               // Local variable declaration

               VkResult vkTsResult = VK_SUCCESS;

 

               // Code

               // Step 1 Declara an array of VkImageView

               VkImageView vkImageView_attachment_array[1];

               memset((void*)vkImageView_attachment_array, 0, sizeof(VkImageView) * ARRAY_SIZE(vkImageView_attachment_array));

              

               // Step 2 create

               VkFramebufferCreateInfo vkFramebufferCreateInfo;

               memset((void*)&vkFramebufferCreateInfo, 0, sizeof(VkFramebufferCreateInfo));

               vkFramebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;

               vkFramebufferCreateInfo.pNext = NULL;

               vkFramebufferCreateInfo.flags = 0;

 

               vkFramebufferCreateInfo.renderPass = vkRenderPass;

               vkFramebufferCreateInfo.attachmentCount = ARRAY_SIZE(vkImageView_attachment_array);

               vkFramebufferCreateInfo.pAttachments = vkImageView_attachment_array;

 

               vkFramebufferCreateInfo.width = vkExtent2D_swapchain.width;

               vkFramebufferCreateInfo.height = vkExtent2D_swapchain.height;

               // Use case 2 for validation vkFramebufferCreateInfo.layers = 1; // Composite layers in Vulkan allowed are 256 guranteed width = 4096 and height = 4096 i.e. 4K images. Desktop quality rendered supports 16K images & 2048 layers

               vkFramebufferCreateInfo.layers = 1;

 

               // Step 3 allocate memory to Frame Buffers

               vkFrameBuffer_array = (VkFramebuffer *)malloc(sizeof(VkFramebuffer) * swapchainImageCount);

               // Add error check of malloc()

 

               // Step 4 loop for swap

               for(uint32_t i = 0; i < swapchainImageCount; i++)

               {

                              vkImageView_attachment_array[0] = swapchainImageView_array[i];

                              vkTsResult = vkCreateFramebuffer(vkDevice, &vkFramebufferCreateInfo, NULL, &vkFrameBuffer_array[i]);

                              if (VK_SUCCESS != vkTsResult)

                              {

                                             fprintf(gpTsFile, "[ERROR] TsCreateFrameBuffers() -> vkCreateFramebuffer() failed for %d index with %d at %d\n", i, vkTsResult, __LINE__);

                                             return(vkTsResult);

                              }

                              else

                              {

                                             fprintf(gpTsFile, "[INFO] TsCreateFrameBuffers() -> vkCreateFramebuffer() succeeded  for %d index at %d \n", i, __LINE__);

                              }

               }

 

               return(vkTsResult);

}

 

// Create Vulkan semaphores

// By default Bianry Semaphore is created for Time Line Semaphore you need to assign Semaphore Type to pNext

VkResult TsCreateSemaphores(void)

{

               // Local variable declaration

               VkResult vkTsResult = VK_SUCCESS;

 

               // Code

               //

               VkSemaphoreCreateInfo vkSemaphoreCreateInfo;

               memset((void *)&vkSemaphoreCreateInfo, 0, sizeof(VkSemaphoreCreateInfo));

 

               vkSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

               vkSemaphoreCreateInfo.pNext = NULL;

               vkSemaphoreCreateInfo.flags = 0; // Must be zeroed as reserved

 

               // Back Buffer Semaphore

               vkTsResult = vkCreateSemaphore(vkDevice, &vkSemaphoreCreateInfo, NULL, &vkSemaphore_backbuffer);

               if (VK_SUCCESS != vkTsResult)

               {

                              fprintf(gpTsFile, "[ERROR] TsCreateSemaphores() -> vkCreateSemaphore() failed for vkSemaphore_backbuffer with %d at %d\n", vkTsResult, __LINE__);

                              return(vkTsResult);

               }

               else

               {

                              fprintf(gpTsFile, "[INFO] TsCreateSemaphores() -> vkCreateSemaphore() succeeded for vkSemaphore_backbuffer at %d \n", __LINE__);

               }

 

               // Render Complete Semaphore

               vkTsResult = vkCreateSemaphore(vkDevice, &vkSemaphoreCreateInfo, NULL, &vkSemaphore_rendercomplete);

               if (VK_SUCCESS != vkTsResult)

               {

                              fprintf(gpTsFile, "[ERROR] TsCreateSemaphores() -> vkCreateSemaphore() failed for vkSemaphore_rendercomplete with %d at %d\n", vkTsResult, __LINE__);

                              return(vkTsResult);

               }

               else

               {

                              fprintf(gpTsFile, "[INFO] TsCreateSemaphores() -> vkCreateSemaphore() succeeded for vkSemaphore_rendercomplete at %d \n", __LINE__);

               }

 

               return(vkTsResult);

}

 

// Create Vulkan Fences

VkResult TsCreateFences(void)

{

               // Local variable declaration

               VkResult vkTsResult = VK_SUCCESS;

 

               // Code

               // Declare and initialize VkFenceCreateInfo

               VkFenceCreateInfo vkFenceCreateInfo;

               memset((void *)&vkFenceCreateInfo, 0, sizeof(VkFenceCreateInfo));

 

               vkFenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

               vkFenceCreateInfo.pNext = NULL;

               vkFenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // Green signal i.e. ready to acquire semaphore

 

               // Allocate

               vkFence_array = (VkFence *)malloc(sizeof(VkFence) * swapchainImageCount);

               // Add error check

 

               //

               for(uint32_t i = 0; i < swapchainImageCount; i++)

               {

                              vkTsResult = vkCreateFence(vkDevice, &vkFenceCreateInfo, NULL, &vkFence_array[i]);

                              if (VK_SUCCESS != vkTsResult)

                              {

                                             fprintf(gpTsFile, "[ERROR] TsCreateFences() -> vkCreateFence() failed for %d index with %d at %d\n", i, vkTsResult, __LINE__);

                                             return(vkTsResult);

                              }

                              else

                              {

                                             fprintf(gpTsFile, "[INFO] TsCreateFences() -> vkCreateFence() succeeded for %d index at %d \n", i, __LINE__);

                              }

               }

 

               return(vkTsResult);

}

 

// Build command buffers

VkResult TsBuildCommandBuffers(void)

{

    // Local variable declaration

    VkResult vkTsResult = VK_SUCCESS;

 

    // Code

    // Loop per swapchain images

    for(uint32_t i = 0; i < swapchainImageCount; i++)

    {

        // Reset command buffers

        vkTsResult = vkResetCommandBuffer(vkCommandBuffer_array[i], 0);

        if (VK_SUCCESS != vkTsResult)

        {

            fprintf(gpTsFile, "[ERROR] TsBuildCommandBuffers() -> vkResetCommandBuffer() failed for %d index with %d at %d\n", i, vkTsResult, __LINE__);

            return(vkTsResult);

        }

        else

        {

            fprintf(gpTsFile, "[INFO] TsBuildCommandBuffers() -> vkResetCommandBuffer() succeeded for %d index at %d \n", i, __LINE__);

        }

 

        VkCommandBufferBeginInfo vkCommandBufferBeginInfo;

        memset((void *)&vkCommandBufferBeginInfo, 0, sizeof(VkCommandBufferBeginInfo));

       

        vkCommandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        vkCommandBufferBeginInfo.pNext = NULL;

        vkCommandBufferBeginInfo.flags = 0;         // 0 indicates use only primary command buffer and we are not going to use this command buffer simetaneously between multiple threads

 

        vkTsResult = vkBeginCommandBuffer(vkCommandBuffer_array[i], &vkCommandBufferBeginInfo);

        if (VK_SUCCESS != vkTsResult)

        {

            fprintf(gpTsFile, "[ERROR] TsBuildCommandBuffers() -> vkBeginCommandBuffer() failed for %d index with %d at %d\n", i, vkTsResult, __LINE__);

            return(vkTsResult);

        }

        else

        {

            fprintf(gpTsFile, "[INFO] TsBuildCommandBuffers() -> vkBeginCommandBuffer() succeeded for %d index at %d \n", i, __LINE__);

        }

 

        // Set clear value

        VkClearValue vkClearValue_array[1];

        memset((void *)vkClearValue_array, 0, sizeof(VkClearValue) * ARRAY_SIZE(vkClearValue_array));

 

        vkClearValue_array[0].color = vkClearColorValue;

       

        // Render Pass Begin Info struct

        VkRenderPassBeginInfo vkRenderPassBeginInfo;

        memset((void *)&vkRenderPassBeginInfo, 0, sizeof(VkRenderPassBeginInfo));

 

        vkRenderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;

        vkRenderPassBeginInfo.pNext = NULL;

        vkRenderPassBeginInfo.renderPass = vkRenderPass;

 

        // Analogus to glViewport() or d3dViewport()

        vkRenderPassBeginInfo.renderArea.offset.x = 0;

        vkRenderPassBeginInfo.renderArea.offset.y = 0;

        vkRenderPassBeginInfo.renderArea.extent.width = vkExtent2D_swapchain.width;

        vkRenderPassBeginInfo.renderArea.extent.height = vkExtent2D_swapchain.height;

 

        vkRenderPassBeginInfo.clearValueCount = ARRAY_SIZE(vkClearValue_array);

        vkRenderPassBeginInfo.pClearValues = vkClearValue_array;

 

        vkRenderPassBeginInfo.framebuffer = vkFrameBuffer_array[i];

 

        // Begin the render pass

        // INLINE => contents of this render pass are in-line with content of subpass & part of primary command buffer

        vkCmdBeginRenderPass(vkCommandBuffer_array[i], &vkRenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

 

        // Bind with the pipeline

        vkCmdBindPipeline(vkCommandBuffer_array[i], VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipeline);

 

        // Bind with the vertex buffer

        VkDeviceSize vkDeviceSize_offset_array[1];

        memset((void*)vkDeviceSize_offset_array, 0, sizeof(VkDeviceSize) * ARRAY_SIZE(vkDeviceSize_offset_array));

 

        vkCmdBindVertexBuffers(vkCommandBuffer_array[i], 0, 1, &vertexData_position.vkBuffer, vkDeviceSize_offset_array);

 

        // Here we should call Vulkan Drawing Functions

        vkCmdDraw(vkCommandBuffer_array[i],

            3,  // Kiti vertices draw karaychet

            1, // Kiti instances aahet 1

            0, // Drawing koothoon start karaych

            0  // From 1st instance

        );

 

        // End render pass

        vkCmdEndRenderPass(vkCommandBuffer_array[i]);

 

        // End Command Buffer recording

        vkTsResult = vkEndCommandBuffer(vkCommandBuffer_array[i]);

        if (VK_SUCCESS != vkTsResult)

        {

            fprintf(gpTsFile, "[ERROR] TsBuildCommandBuffers() -> vkEndCommandBuffer() failed for %d index with %d at %d\n", i, vkTsResult, __LINE__);

            return(vkTsResult);

        }

        else

        {

            fprintf(gpTsFile, "[INFO] TsBuildCommandBuffers() -> vkEndCommandBuffer() succeeded for %d index at %d \n", i, __LINE__);

        }

    }

 

    return(vkTsResult);

}

 

// VKAPI_ATTR => Calling convention for gcc, Clang must be used from C++11 and above

// VkBool32 => Return Type

// VKAPI_CALL => Calling convention for Windows

VKAPI_ATTR VkBool32 VKAPI_CALL debugReportCallback(

    VkDebugReportFlagsEXT vkDebugReportFlagsEXT,            // Logger settings for different types of message

    VkDebugReportObjectTypeEXT vkDebugReportObjectTypeEXT,  // Object type of calling this callback function

    uint64_t object,                // Actual object

    size_t location,                // Memory Location of error message

    int32_t messageCode,            // Message ID

    const char * pLayerPrefix,      // Kontya layer ne error dili

    const char * pMessage,          // Actual error message details

    void * pUserData                //

)

{

    // Code

    fprintf(gpTsFile, "[TS_VALIDATION]: debugReportCallback() -> %s (%d) = %s\n", pLayerPrefix, messageCode, pMessage);

    return(VK_FALSE);

}
