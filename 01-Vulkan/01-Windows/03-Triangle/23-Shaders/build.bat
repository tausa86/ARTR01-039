del VK.exe
del VK.obj
cl.exe /c /EHsc /I "D:\VulkanSDK\Vulkan\Include" VK.c
rc.exe VK.rc
link.exe VK.obj VK.res /LIBPATH:D:\VulkanSDK\Vulkan\Lib user32.lib gdi32.lib /SUBSYSTEM:WINDOWS