#pragma once

#include <iostream>
#include <filesystem>
#include <fstream>
#include <array>
#include <map>
#include <optional>
#include <string>
#include <memory>

#define VK_USE_PLATFORM_WIN32_KHR
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>

#include "wexception.h"
#include "LoggingSystem.h"


namespace ak
{
	enum class ShaderFlag
	{
		eVertex = 0,
		eFragment = 1,
		eMax = 2
	};

	std::vector<std::uint32_t> loadShaderFile(const std::string& filename);

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

		void draw();
		void waitIdle();

	private:
#ifdef _DEBUG
		static VKAPI_ATTR vk::Bool32 VKAPI_PTR debugCallback_(
			vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			vk::DebugUtilsMessageTypeFlagsEXT messageType,
			const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData);
#endif // _DEBUG
		void postLogMessage_(LogLevelFlag logLevel, const char* message);
		void postLogMessage_(LogLevelFlag logLevel, const std::string& message);
		void postLogMessage_(LogLevelFlag logLevel, const std::wstring& message);

		void createInstance_();
		void createSurface_();
		void pickPhysicalDevice_();
		void createLogicalDevice_();
		void createSwapchain_();
		void createImageViews_();
		void createShaderModules_();
		void createGraphicsPipelineLayout_();
		void createRenderPass_();
		void createGraphicsPipeline_();
		void createFramebuffers_();
		void createCommandPool_();
		void allocateCommandBuffers_();
		void createSyncObjects_();
		void recordCommandBuffer_(const vk::CommandBuffer& hCommandBuffer, const vk::Framebuffer& hFrameBuffer);

#ifdef _DEBUG
		const std::vector<const char*> instanceLayerNames_{
			"VK_LAYER_KHRONOS_validation" };
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
		vk::Queue hQueue_{};
		vk::UniqueSwapchainKHR uhOldSwapchain_{};
		vk::UniqueSwapchainKHR uhSwapchain_{};
		std::vector<vk::Image> hSwapchainImages_{};
		std::vector<vk::UniqueImageView> uhSwapchainImageViews_{};
		std::vector<vk::UniqueShaderModule> uhShaderModules_{};
		vk::UniquePipelineLayout uhGraphicsPipelineLayout_{};
		vk::UniqueRenderPass uhRenderPass_{};
		vk::UniquePipeline uhGraphicsPipeline_{};
		std::vector<vk::UniqueFramebuffer> uhFramebuffers_{};
		vk::UniqueCommandPool uhCommandPool_{};
		std::vector<vk::CommandBuffer> hCommandBuffers_{};
		std::vector<vk::UniqueSemaphore> uhReadyForRenderingSemaphores_{};
		std::vector<vk::UniqueSemaphore> uhReadyForPresentingSemaphores_{};
		std::vector<vk::UniqueFence> uhInFlightFences_{};

		std::optional<uint32_t> graphicsAndPresentQueueFamilyIdx_{};
		vk::PhysicalDeviceProperties sPhysicalDeviceProperties_{};
		vk::PhysicalDeviceFeatures sPhysicalDeviceFeatures_{};
		vk::SurfaceCapabilitiesKHR sPhysicalDeviceSurfaceCapabilities_{};
		vk::Format swapchainImageFormat_{ vk::Format::eR8G8B8A8Srgb };
		vk::Extent2D swapchainImageExtent_{};
		vk::Viewport swapchainViewport_{};
		vk::Rect2D swapchainScissor_{};
		vk::ClearValue swapchainClearValue_{};		
		std::vector<std::string> uhShaderModuleFilenames_{ "./Shaders/vert.spv","./Shaders/frag.spv" };
		uint32_t frameInFlightIdx_{};
		uint32_t swapchainImageIdx_{};
	};
}