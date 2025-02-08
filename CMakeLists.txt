cmake_minimum_required (VERSION 3.8)

project ("ReSTIR_PT")

if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

set(CMAKE_CXX_STANDARD 17)

set(SHADER_DIR "${CMAKE_SOURCE_DIR}/shader/")
set(ASSET_DIR "${CMAKE_SOURCE_DIR}/asset/")
add_definitions(-DSHADER_DIR="${SHADER_DIR}")
add_definitions(-DASSET_DIR="${ASSET_DIR}")

find_package(Vulkan REQUIRED)

IF (NOT Vulkan_FOUND)
    message(FATAL_ERROR "Could not find Vulkan library!")
ELSE()
    message(STATUS ${Vulkan_LIBRARY})
ENDIF()

find_program(Vulkan_GLSLANG_VALIDATOR 
	NAMES glslangValidator
	HINTS ENV VULKAN_SDK 
	PATH_SUFFIXES bin)
	
if (NOT Vulkan_GLSLANG_VALIDATOR)
    message(FATAL_ERROR "glslangValidator not found!")
endif()

add_subdirectory(src)
add_subdirectory(shader)
add_subdirectory(asset)
add_subdirectory(3rd)