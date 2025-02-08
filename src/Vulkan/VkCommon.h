#pragma once

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
void DebugTest();

#define RUNTIME_ERROR(Message) throw std::runtime_error(Message)

#define VULKAN_RUNTIME_CHECK(Func, Message)\
	if(Func != VK_SUCCESS)\
	{\
		throw std::runtime_error(std::string("Fail to") + std::string(Message));\
	}\

#define LOG(Message) std::cout << Message << std::endl;

namespace xvk
{
	template<class T>
	inline void GetEnumerateVector(VkResult(EnumerateFunc) (uint32_t*, T*), std::vector<T>& outvector)
	{
		uint32_t count = 0;
		EnumerateFunc(&count, nullptr);
		outvector.resize(count);
		EnumerateFunc(&count, outvector.data());
	}

	template<class T>
	inline std::vector<T> GetEnumerateVector(VkResult(EnumerateFunc)(uint32_t*, T*))
	{
		std::vector<T> props;
		GetEnumerateVector(EnumerateFunc, props);
		return props;
	}

	template<class THandle, class T>
	inline void GetEnumerateVector(THandle handle, void(*EnumerateFunc)(THandle, uint32_t*, T*), std::vector<T>& outvector)
	{
		uint32_t count = 0;
		EnumerateFunc(handle, &count, nullptr);
		outvector.resize(count);
		EnumerateFunc(handle, &count, outvector.data());
	}
}