del VK.exe
del VK.obj
cl.exe /c /EHsc /I "C:\VulkanSDK\Vulkan\Include" /I "C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v13.1\include" VK.cpp
rc.exe VK.rc
link.exe VK.obj VK.res /LIBPATH:C:\VulkanSDK\Vulkan\Lib /LIBPATH:"C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v13.1\lib\x64" user32.lib gdi32.lib /SUBSYSTEM:WINDOWS