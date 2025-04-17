#pragma once

#include <iostream>
#include <string>
#include <memory>
#include <functional>

#define VK_USE_PLATFORM_WIN32_KHR
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
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

#ifdef _DEBUG
		void registerLogger(const std::shared_ptr<LoggerSystem>& shpLoggerSystem);
#endif // _DEBUG
		void createInstance();

	private:
#ifdef _DEBUG
		const std::vector<const char*> instanceLayerNames_{
			"VK_LAYER_KHRONOS_validation" };
		const std::vector<const char*> instanceExtensionNames_{
			VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
			VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
			VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME,
			VK_KHR_SURFACE_EXTENSION_NAME,
			VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
			VK_KHR_SURFACE_PROTECTED_CAPABILITIES_EXTENSION_NAME };
#else
		const std::vector<const char*> instanceLayerNames_{};
		const std::vector<const char*> instanceExtensionNames_{
			VK_KHR_SURFACE_EXTENSION_NAME,
			VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
			VK_KHR_SURFACE_PROTECTED_CAPABILITIES_EXTENSION_NAME };
#endif // _DEBUG

		void postMessage(LoggerLevel loggerLevel, const std::wstring& message);

#ifdef _DEBUG
		static VKAPI_ATTR vk::Bool32 VKAPI_PTR debugCallback_(
			vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			vk::DebugUtilsMessageTypeFlagsEXT messageType,
			const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData)
		{
			VulkanSystem* pVulkanSystem = reinterpret_cast<VulkanSystem*>(pUserData);

			std::wstring message{};
			message += stringToWstring(std::to_string(pCallbackData->messageIdNumber));
			message += L": ";
			message += stringToWstring(pCallbackData->pMessage);
			message += L"\r\n";

			if (messageSeverity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose)
			{
				pVulkanSystem->postMessage(LoggerLevel::VULKAN_VERBOSE, message);
				return VK_TRUE;
			}
			else if (messageSeverity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo)
			{
				pVulkanSystem->postMessage(LoggerLevel::VULKAN_INFO, message);
				return VK_TRUE;
			}
			else if (messageSeverity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning)
			{
				pVulkanSystem->postMessage(LoggerLevel::VULKAN_WARNING, message);
				return VK_FALSE;
			}
			else if (messageSeverity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eError)
			{
				pVulkanSystem->postMessage(LoggerLevel::VULKAN_ERROR, message);
				return VK_FALSE;
			}

			return VK_TRUE;
		}
#endif // _DEBUG

		std::shared_ptr<LoggerSystem> shpLoggerSystem_{};

		vk::ApplicationInfo sApplicationInfo_{};
		vk::InstanceCreateInfo sInstanceCreateInfo_{};
		vk::DebugUtilsMessengerCreateInfoEXT sDebugUtilsMessengerCreateInfo_{};

		vk::UniqueInstance uhInstance_{};
		vk::UniqueDebugUtilsMessengerEXT uhDebugUtilsMessenger_{};
	};
}