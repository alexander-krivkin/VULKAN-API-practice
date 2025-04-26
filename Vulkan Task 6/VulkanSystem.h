#pragma once

#include <iostream>
#include <array>
#include <map>
#include <optional>
#include <string>
#include <memory>
#include <algorithm>
#include <functional>

#define VK_USE_PLATFORM_WIN32_KHR
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>

#include "wexception.h"
#include "LoggingSystem.h"


namespace ak
{
	class VulkanSystem final
	{
	public:
		VulkanSystem();
		explicit VulkanSystem(const VulkanSystem& obj) = delete;
		~VulkanSystem();
		VulkanSystem& operator=(const VulkanSystem& obj) = delete;

#ifdef _DEBUG
		void init(const HWND& hWnd, const std::shared_ptr<LoggingSystem>& shpLoggingSystem);
#else
		void init(const HWND& hWnd);
#endif // _DEBUG

	private:
#ifdef _DEBUG
		static VKAPI_ATTR vk::Bool32 VKAPI_PTR debugCallback_(
			vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			vk::DebugUtilsMessageTypeFlagsEXT messageType,
			const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData);
#endif // _DEBUG
		void postLogMessage(LogLevel logLevel, const char* message);
		void postLogMessage(LogLevel logLevel, const std::string& message);
		void postLogMessage(LogLevel logLevel, const std::wstring& message);

		void createInstance();
		void createSurface();
		void pickPhysicalDevice();
		void createLogicalDevice();
		void createSwapchain();
		void createImageViews();

#ifdef _DEBUG
		const std::vector<const char*> instanceLayerNames_{
			"VK_LAYER_KHRONOS_validation"};
		const std::vector<const char*> instanceExtensionNames_{
			VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
			VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
			VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME,
			VK_KHR_SURFACE_EXTENSION_NAME,
			VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
			VK_KHR_SURFACE_PROTECTED_CAPABILITIES_EXTENSION_NAME,
			VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME };
#else
		const std::vector<const char*> instanceLayerNames_{};
		const std::vector<const char*> instanceExtensionNames_{
			VK_KHR_SURFACE_EXTENSION_NAME,
			VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
			VK_KHR_SURFACE_PROTECTED_CAPABILITIES_EXTENSION_NAME };
#endif // _DEBUG
		const std::vector<const char*> deviceExtensionNames_{
			VK_KHR_SWAPCHAIN_EXTENSION_NAME };

		HWND hWnd_{};
		std::shared_ptr<LoggingSystem> shpLoggingSystem_{};

		vk::UniqueInstance uhInstance_{};
		vk::UniqueDebugUtilsMessengerEXT uhDebugUtilsMessenger_{};
		vk::UniqueSurfaceKHR uhSurface_{};
		vk::PhysicalDevice hPhysicalDevice_{};
		vk::UniqueDevice uhDevice_{};
		vk::UniqueSwapchainKHR uhOldSwapchain_{};
		vk::UniqueSwapchainKHR uhSwapchain_{};
		std::vector <vk::Image> hSwapchainImages_{};
		std::vector<vk::UniqueImageView> uhSwapchainImageViews_{};

		std::optional<uint32_t> graphicsAndPresentQueueFamilyIdx_{};
		vk::PhysicalDeviceProperties sPhysicalDeviceProperties_{};
		vk::PhysicalDeviceFeatures sPhysicalDeviceFeatures_{};
		vk::SurfaceCapabilitiesKHR sPhysicalDeviceSurfaceCapabilities_{};
		vk::Queue queue_{};
		vk::Format swapchainImageFormat_{ vk::Format::eR8G8B8A8Srgb };
		vk::Extent2D swapchainImageExtent_{};
	};
}