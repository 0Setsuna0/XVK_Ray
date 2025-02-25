file(GLOB_RECURSE src_files ${CMAKE_SOURCE_DIR}/src/*.cpp ${CMAKE_SOURCE_DIR}/src/*.h)
file(GLOB src_files_vulkan ${CMAKE_SOURCE_DIR}/src/Vulkan/*.cpp ${CMAKE_SOURCE_DIR}/src/Vulkan/*.h)
file(GLOB src_files_vulkan_raytracing ${CMAKE_SOURCE_DIR}/src/Vulkan/VKRayTracing/*.cpp ${CMAKE_SOURCE_DIR}/src/Vulkan/VKRayTracing/*.h)
file(GLOB src_files_utility ${CMAKE_SOURCE_DIR}/src/Utility/*.cpp ${CMAKE_SOURCE_DIR}/src/Utility/*.h)
file(GLOB src_files_asset ${CMAKE_SOURCE_DIR}/src/Asset/*.cpp ${CMAKE_SOURCE_DIR}/src/Asset/*.h)


source_group("Main" FILES ${src_files})
source_group("Vulkan" FILES ${src_files_vulkan})
source_group("VulkanRT" FILES ${src_files_vulkan_raytracing})
source_group("Utility" FILES ${src_files_utility})
source_group("Asset" FILES ${src_files_asset})

add_executable(ReSTIR_PT 
  ${src_files} 
  ${src_files_vulkan} 
  ${src_files_vulkan_raytracing} 
  ${src_files_utility}
  ${src_files_asset})

target_link_libraries(ReSTIR_PT PUBLIC Vulkan::Vulkan)
target_link_libraries(ReSTIR_PT PUBLIC glfw)

target_include_directories(ReSTIR_PT PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_include_directories(ReSTIR_PT PUBLIC ${CMAKE_SOURCE_DIR}/3rd)
target_include_directories(ReSTIR_PT PUBLIC ${Vulkan_INCLUDE_DIRS})

