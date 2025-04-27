D:/Graphic/VulkanSDK/Bin/dxc.exe -spirv -T lib_6_7 -E main ReSTIRGIInitialSample.rgen.hlsl -Fo rgen_initialSample.spv -fspv-target-env=vulkan1.3
D:/Graphic/VulkanSDK/Bin/dxc.exe -spirv -T lib_6_7 -E main ReSTIRGITemporalReuse.rgen.hlsl -Fo rgen_temporalReuse.spv -fspv-target-env=vulkan1.3
D:/Graphic/VulkanSDK/Bin/dxc.exe -spirv -T lib_6_7 -E main ReSTIRGISpatialReuse.rgen.hlsl -Fo rgen_spatialReuse.spv -fspv-target-env=vulkan1.3
D:/Graphic/VulkanSDK/Bin/dxc.exe -spirv -T lib_6_7 -E main Ray.rmiss.hlsl -Fo rmiss.spv -fspv-target-env=vulkan1.3
D:/Graphic/VulkanSDK/Bin/dxc.exe -spirv -T lib_6_7 -E main Ray.rmiss_shadow.hlsl -Fo rmiss_shadow.spv -fspv-target-env=vulkan1.3
D:/Graphic/VulkanSDK/Bin/dxc.exe -spirv -T lib_6_7 -E main Ray.rchit.hlsl -Fo rchit.spv -fspv-target-env=vulkan1.3
D:/Graphic/VulkanSDK/Bin/dxc.exe -spirv -T lib_6_7 -E main Ray.rahit.hlsl -Fo rahit.spv -fspv-target-env=vulkan1.3
pause