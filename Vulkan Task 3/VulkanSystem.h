#pragma once

#include <iostream>
#include <string>
#include <memory>

#include <vulkan/vulkan.hpp>

#include "wexception.h"
#include "LoggerSystem.h"


namespace ak
{
	class VulkanSystem final
	{
	public:
		VulkanSystem() {};
		explicit VulkanSystem(const VulkanSystem& obj) = delete;
		~VulkanSystem() {};
		VulkanSystem& operator=(const VulkanSystem& obj) = delete;

		void registerLogger(const std::shared_ptr<LoggerSystem>& shpLoggerSystem);
		void createInstance();
		//void setupDebugMessenger();
		
	private:
#ifdef _DEBUG
		const std::vector<const char*> instanceLayerNames_{
			"VK_LAYER_KHRONOS_validation",
			"VK_LAYER_KHRONOS_synchronization2",
			"VK_LAYER_LUNARG_monitor" };
		const std::vector<const char*> instanceExtensionNames_{
			"VK_EXT_debug_utils",
			"VK_KHR_surface",
			"VK_KHR_win32_surface" };
#else
		const std::vector<const char*> instanceLayerNames_{};
		const std::vector<const char*> instanceExtensionNames_{
			"VK_KHR_surface",
			"VK_KHR_win32_surface" };
#endif // _DEBUG

#ifdef _DEBUG
		std::shared_ptr<LoggerSystem> shpLoggerSystem_{};
#endif // _DEBUG

		vk::ApplicationInfo sApplicationInfo_{};
		vk::InstanceCreateInfo sInstanceCreateInfo_{};
		vk::UniqueInstance uhInstance_{};
	};
}