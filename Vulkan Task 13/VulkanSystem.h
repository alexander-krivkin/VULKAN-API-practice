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
#include <random>

#define VK_USE_PLATFORM_WIN32_KHR
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>

#include "shared.h"
#include "wexception.h"
#include "LoggingSystem.h"
#include "Geometry.h"


namespace ak
{
	enum class VulkanSystemFlag : uint32_t
	{
		eNotReady = 0,
		eReadyAndPaused = 1,
		eReadyAndStarted = 2,
		eMax = 3
	};



	enum class PhysicalDeviceCriteriaFlag : uint32_t
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
		VulkanSystem() {};
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
		void updateMouseCursorPosition(POINT cursorPosition);
		void updateKeyboardMouseFlags(uint32_t keyboardMouseFlags);
		void updateMouseWheelDelta(int16_t mouseWheelDelta);


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
		void createSwapchainImageViews_();
		void createDepthImage_();
		void createDepthImageView_();
		void createShaderModules_();
		void createDescriptorSetLayout_();
		void createGraphicsPipelineLayout_();
		void createRenderPass_();
		void createGraphicsPipelines_();
		void createFramebuffers_();
		void createSyncObjects_();
		void createCommandPools_();
		void allocateCommandBuffers_();
		void createDescriptorPool_();

		uint32_t findSuitableMemoryIndex_(uint32_t allowedTypesMask, vk::MemoryPropertyFlags requiredMemoryFlags);
		vk::Format findSupportedFormat_(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags featureFlags);
		std::pair<vk::UniqueBuffer, vk::UniqueDeviceMemory> createBufferMemoryPair_(vk::DeviceSize size, std::vector<uint32_t> queueFamilyIndices,
			vk::BufferUsageFlags usageFlags, vk::SharingMode sharingMode, vk::MemoryPropertyFlags requiredMemoryFlags);
		std::pair<vk::UniqueImage, vk::UniqueDeviceMemory> createImageMemoryPair_(vk::Format format, vk::Extent2D extent, vk::ImageTiling tiling,
			std::vector<uint32_t> queueFamilyIndices,
			vk::ImageUsageFlags usageFlags, vk::SharingMode sharingMode, vk::MemoryPropertyFlags requiredMemoryFlags);
		void copyBuffer_(uint32_t transferIdx, vk::Buffer& hSrcBuffer, vk::Buffer& hDstBuffer, vk::DeviceSize size);

		void initGrid_();
		void initCrosshair_();
		void addPrimitiveToScene_(TopologyTypeFlag topologyType, geometry::Primitive& primitive);

		void updateWindowTransformUniformBuffer_(ObjectFlag object);
		void updateWindowSizeTransform_();
		void updateWindowViewCameraRightAndUpTransform_(float rightFactor, float upFactor);
		void updateWindowViewCameraForwardTransform_(float forwardFactor);
		void updateWindowViewCameraRotateEyeTransform_(float rightFactor, float upFactor);
		void updateWindowViewCameraRotateTargetTransform_(float rightFactor, float upFactor);
		void updateCrosshairTransformUniformBuffer_();

		void createVertexBuffer_(ObjectFlag object, TopologyTypeFlag topologyType);
		void createIndexBuffer_(ObjectFlag object, TopologyTypeFlag topologyType);
		void createUniformBuffers_(ObjectFlag object);
		void allocateDescriptorSets_(ObjectFlag object);

		void recordObjectToCommandBuffer_(vk::CommandBuffer hCommandBuffer,
			TopologyTypeFlag topologyType, ObjectFlag object);
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
		vk::Format depthImageFormat_{};
		std::pair<vk::UniqueImage, vk::UniqueDeviceMemory> uhDepthImage_{};
		vk::UniqueImageView uhDepthImageView_{};
		std::array<vk::UniqueShaderModule, numShaders> uhShaderModules_{};
		vk::UniqueDescriptorSetLayout uhDescriptorSetLayout_{};
		vk::UniquePipelineLayout uhGraphicsPipelineLayout_{};
		vk::UniqueRenderPass uhRenderPass_{};
		std::array<vk::UniquePipeline, numTopologyTypes> uhGraphicsPipelines_{};
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
		uint32_t numSwapchainImages_{};
		std::array<std::string, numShaders> uhShaderModuleFilenames_{ "./Shaders/vert.spv", "./Shaders/frag.spv" };

		std::array<std::array<std::vector<geometry::Vertex3D>, numTopologyTypes>, numObjects> vertices_{};
		std::array<std::array<std::vector<uint32_t>, numTopologyTypes>, numObjects> vertexIndices_{};
		std::array<std::array<std::pair<vk::UniqueBuffer, vk::UniqueDeviceMemory>, numTopologyTypes>, numObjects> uhVertexStagingBuffers_{};
		std::array<std::array<std::pair<vk::UniqueBuffer, vk::UniqueDeviceMemory>, numTopologyTypes>, numObjects> uhVertexBuffers_{};
		std::array<std::array<std::pair<vk::UniqueBuffer, vk::UniqueDeviceMemory>, numTopologyTypes>, numObjects> uhIndexStagingBuffers_{};
		std::array<std::array<std::pair<vk::UniqueBuffer, vk::UniqueDeviceMemory>, numTopologyTypes>, numObjects> uhIndexBuffers_{};

		std::array<std::array<geometry::Uniform, numFramesInFlight>, numObjects> uniformTranformations_{};
		std::array<std::array<std::pair<vk::UniqueBuffer, vk::UniqueDeviceMemory>, numFramesInFlight>, numObjects> uhUniformBuffers_{};
		std::array<std::array<void*, numFramesInFlight>, numObjects> pMappedUniformBufferMemorySections_{};
		std::array<std::vector<vk::UniqueDescriptorSet>, numObjects> uhDescriptorSets_{};

		geometry::CoordinateSystem<glm::vec3> worldCS_{};

		struct
		{
			VulkanSystemFlag readiness_{ VulkanSystemFlag::eNotReady };
			uint32_t transferIdx_{};
			uint32_t frameInFlightIdx_{};
			uint32_t swapchainImageIdx_{};

			vk::Format windowFormat_{ vk::Format::eR8G8B8A8Srgb };
			vk::Extent2D windowExtent_{};
			vk::Viewport windowViewport_{};
			vk::Rect2D windowScissor_{};
			vk::ClearColorValue windowBackgroundColor_{ 1.00f, 1.00f, 1.00f, 1.0f };

			float windowAspectRatio_{};
			glm::vec3 windowViewCameraEye_{ 0.0f, 0.0f, 2000.0f };
			glm::vec3 windowViewCameraTarget_{ 0.0f, 0.0f, 0.0f };
			glm::vec3 windowViewCameraUp_{ 0.0f, 1.0f, 0.0f };
			glm::vec3 windowViewCameraForward_{};
			glm::vec3 windowViewCameraRight_{};
			float windowViewPerspectiveFocusAngle_{ 55.0f };
			float windowViewPerspectiveNear_{ 0.10f }, windowPerspectiveViewFar_{ std::numeric_limits<float>::max() };

			uint32_t keyboardMouseFlags_{};
			int16_t mouseWheelDelta_{};





			///
			/// Что делать с этим блоком? Вывести в отдельный объект?
			/// 
			glm::vec3 gridSpacing_{50.0f, 50.0f, 50.0f };
			float gridDimension_{ 100.0f };
			std::array<glm::vec4, 3> gridAxisColors_{ glm::vec4{ 0.80f, 0.20f, 0.20f, 1.0f },
				glm::vec4{ 0.20f, 0.20f, 0.80f, 1.0f }, glm::vec4{ 0.20f, 0.20f, 0.20f, 1.0f } };
			glm::vec4 gridColor_{ 0.80f, 0.80f, 0.80f, 1.0f };
			glm::vec4 gridPointColor_{ 0.20f, 0.20f, 0.20f, 1.0f };

			float crosshairAimSize_{ 100.0f };
			std::array<glm::vec4, 3> crosschairColors_{ glm::vec4{ 0.80f, 0.00f, 0.00f, 1.0f },
				glm::vec4{ 0.00f, 0.00f, 0.80f, 1.0f }, glm::vec4{ 0.00f, 0.80f, 0.00f, 1.0f } };
			glm::vec4 crosschairPointColor_{ 0.00f, 0.00f, 0.00f, 1.0f };
			glm::vec2 crosshairPosition_{};

		} state_{};
	};
}