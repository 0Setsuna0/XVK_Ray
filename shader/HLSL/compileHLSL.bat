D:/Graphic/VulkanSDK/Bin/dxc.exe -spirv -T vs_6_7 -E main simpleRZ.vert.hlsl -Fo simpleRZ.vert.spv
D:/Graphic/VulkanSDK/Bin/dxc.exe -spirv -T ps_6_7 -E main simpleRZ.frag.hlsl -Fo simpleRZ.frag.spv
D:/Graphic/VulkanSDK/Bin/dxc.exe -spirv -T lib_6_7 -E main rgen.hlsl -Fo rgen.spv -fspv-target-env=vulkan1.3
D:/Graphic/VulkanSDK/Bin/dxc.exe -spirv -T lib_6_7 -E main rmiss.hlsl -Fo rmiss.spv -fspv-target-env=vulkan1.3
D:/Graphic/VulkanSDK/Bin/dxc.exe -spirv -T lib_6_7 -E main rhit.hlsl -Fo rhit.spv -fspv-target-env=vulkan1.3
pause