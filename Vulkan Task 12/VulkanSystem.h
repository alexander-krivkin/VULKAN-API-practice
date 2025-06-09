#pragma once

#include <iostream>
#include <filesystem>
#include <fstream>
#include <array>
#include <map>
#include <optional>
#include <string>
#include <memory>
#include <functional>

#define VK_USE_PLATFORM_WIN32_KHR
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>

#include "wexception.h"
#include "LoggingSystem.h"
#include "Geometry.h"


namespace ak
{
	constexpr uint32_t numFramesInFlight{ 2 };
	constexpr uint32_t numShaders{ 2 };
	constexpr uint32_t numObjects{ 2 };

	enum class VulkanSystemFlag
	{
		eNotReady = 0,
		eReadyAndPaused = 1,
		eReadyAndStarted = 2,
		eMax = 3
	};

	enum class ShaderFlag
	{
		eVertex = 0,
		eFragment = 1,
		eMax = numShaders
	};

	enum class ObjectFlag
	{
		eGrid = 0,
		//eScene = 1,
		eCrosshair = 1,
		eMax = numObjects
	};

	enum class PhysicalDeviceCriteriaFlag
	{
		eGeometryShaderSupport = 0,
		eSwapchainEXTSupport = 1,
		eSurfaceRGBAAndNonlinearFormatSupport = 2,
		eSurfaceMailboxPresentModeSupport = 3,
		eGraphicsAndPresentQueueFamilySupport = 4,
		eMax = 5
	};


	std::vector<uint32_t> loadShaderFile(const std::string& filename);


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

		std::function<void()> draw{ std::bind(&VulkanSystem::drawPaused_, this) };
		void resize();
		void pause();
		void resume();
		void updateCrosshairPosition(POINT cursorPosition);

	private:
#ifdef _DEBUG
		static VKAPI_ATTR vk::Bool32 VKAPI_PTR debugCallback_(
			vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			vk::DebugUtilsMessageTypeFlagsEXT messageType,
			const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData);
#endif // _DEBUG
		void postLogMessage_(LogLevelFlagBits logLevel, const char* message);
		void postLogMessage_(LogLevelFlagBits logLevel, const std::string& message);
		void postLogMessage_(LogLevelFlagBits logLevel, const std::wstring& message);

		void createInstance_();
		void createSurface_();
		void pickPhysicalDevice_();
		void createLogicalDevice_();
		void createSwapchain_();
		void createImageViews_();
		void createShaderModules_();
		void createDescriptorSetLayout_();
		void createGraphicsPipelineLayout_();
		void createRenderPass_();
		void createGraphicsPipeline_();
		void createFramebuffers_();
		void createSyncObjects_();
		void createCommandPools_();
		void allocateCommandBuffers_();
		void createDescriptorPool_();

		uint32_t findSuitableMemoryIndex(uint32_t allowedTypesMask, vk::MemoryPropertyFlags requiredMemoryFlags);
		std::pair<vk::UniqueBuffer, vk::UniqueDeviceMemory> createBufferMemoryPair_(vk::DeviceSize size, vk::BufferUsageFlags usageFlags,
			vk::SharingMode sharingMode, std::vector<uint32_t> queueFamilyIndices, vk::MemoryPropertyFlags requiredMemoryFlags);
		void copyBuffer_(uint32_t transferIdx, vk::Buffer& hSrcBuffer, vk::Buffer& hDstBuffer, vk::DeviceSize size);

		void initGrid3D_();
		void initCrosshair3D_();
		void updateWindowUniformModelTranformations_();
		void updateCrosshairUniformModelTranformations_();
		void createVertexBuffer_(ObjectFlag objectFlag);
		void createIndexBuffer_(ObjectFlag objectFlag);
		void createUniformBuffers_(ObjectFlag objectFlag);
		void allocateDescriptorSets_(ObjectFlag objectFlag);

		void recordGraphicsCommandBuffer_();
		void drawPaused_();
		void drawStarted_();


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
		vk::Queue hTransferQueue_{};
		vk::Queue hGraphicsQueue_{};
		vk::UniqueSwapchainKHR uhOldSwapchain_{};
		vk::UniqueSwapchainKHR uhSwapchain_{};
		std::vector<vk::Image> hSwapchainImages_{};
		std::vector<vk::UniqueImageView> uhSwapchainImageViews_{};
		std::array<vk::UniqueShaderModule, numShaders> uhShaderModules_{};
		vk::UniqueDescriptorSetLayout uhDescriptorSetLayout_{};
		vk::UniquePipelineLayout uhGraphicsPipelineLayout_{};
		vk::UniqueRenderPass uhRenderPass_{};
		vk::UniquePipeline uhGraphicsPipeline_{};
		std::vector<vk::UniqueFramebuffer> uhFramebuffers_{};
		std::vector<vk::UniqueFence> uhTransferFences_{};
		std::vector<vk::UniqueFence> uhInFlightFences_{};
		std::vector<vk::UniqueSemaphore> uhReadyForRenderingSemaphores_{};
		std::vector<vk::UniqueSemaphore> uhReadyForPresentingSemaphores_{};
		vk::UniqueCommandPool uhTransferCommandPool_{};
		vk::UniqueCommandPool uhGraphicsCommandPool_{};
		std::vector<vk::CommandBuffer> hTransferCommandBuffers_{};
		std::vector<vk::CommandBuffer> hGraphicsCommandBuffers_{};
		vk::UniqueDescriptorPool uhDescriptorPool_{};

		std::optional<uint32_t> transferQueueFamilyIdx_{};
		std::optional<uint32_t> graphicsAndPresentQueueFamilyIdx_{};
		vk::PhysicalDeviceMemoryProperties sPhysicalDeviceMemoryProperties_{};
		vk::PhysicalDeviceProperties sPhysicalDeviceProperties_{};
		vk::PhysicalDeviceFeatures sPhysicalDeviceFeatures_{};
		vk::SurfaceCapabilitiesKHR sPhysicalDeviceSurfaceCapabilities_{};
		std::array<std::string, numShaders> uhShaderModuleFilenames_{ "./Shaders/vert.spv", "./Shaders/frag.spv" };
		uint32_t transferIdx_{};
		uint32_t frameInFlightIdx_{};
		uint32_t swapchainImageIdx_{};

		struct
		{			
			VulkanSystemFlag readiness{ VulkanSystemFlag::eNotReady };
			vk::Format windowFormat{ vk::Format::eR8G8B8A8Srgb };
			vk::Extent2D windowExtent{};
			vk::Viewport windowViewport{};
			vk::Rect2D windowScissor{};
			vk::ClearColorValue windowBackgroundColor{ 1.00f, 1.00f, 1.00f, 1.0f };

			glm::vec2 windowModelScale{};
			glm::vec3 gridSpacing{ 25.0f, 25.0f, 25.0f };
			float gridDimension{ 50.0f };
			glm::vec4 gridColor{ 0.60f, 0.60f, 0.60f, 1.0f };
			glm::vec2 crosshairPosition{};
			float crosshairAimSize{ 100.0f };
			std::array<glm::vec4, 3> crosschairColors{ glm::vec4{ 0.60f, 0.00f, 0.00f, 1.0f },
			glm::vec4{ 0.00f, 0.60f, 0.00f, 1.0f }, glm::vec4{ 0.00f, 0.00f, 0.60f, 1.0f } };
		} state_{};

		std::array<std::vector<geometry::Point3D>, numObjects> vertices_{};
		std::array<std::vector<uint16_t>, numObjects> vertexIndices_{};
		std::array<std::array<geometry::UniformTransformation, numFramesInFlight>, numObjects> uniformTranformations_{};
		std::array<std::pair<vk::UniqueBuffer, vk::UniqueDeviceMemory>, numObjects> uhVertexStagingBuffers_{};
		std::array<std::pair<vk::UniqueBuffer, vk::UniqueDeviceMemory>, numObjects> uhVertexBuffers_{};
		std::array<std::pair<vk::UniqueBuffer, vk::UniqueDeviceMemory>, numObjects> uhIndexStagingBuffers_{};
		std::array<std::pair<vk::UniqueBuffer, vk::UniqueDeviceMemory>, numObjects> uhIndexBuffers_{};
		std::array<std::array<std::pair<vk::UniqueBuffer, vk::UniqueDeviceMemory>, numFramesInFlight>, numObjects> uhUniformBuffers_{};
		std::array<std::array<void*, numFramesInFlight>, numObjects> pMappedUniformBufferMemorySections_{};
		std::array<std::vector<vk::UniqueDescriptorSet>, numObjects> uhDescriptorSets_{};
	};
}