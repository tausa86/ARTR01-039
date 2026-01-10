del VK.exe
del VK.obj
rc.exe VK.rc
nvcc.exe -I C:\VulkanSDK\Vulkan\Include -L C:\VulkanSDK\Vulkan\Lib -o VK.exe user32.lib gdi32.lib VK.res VK.cu -diag-suppress 20012 -diag-suppress 20013 -diag-suppress 20014 -diag-suppress 20015
del VK.exp
del VK.lib