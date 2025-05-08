#include "VulkanSystem.h"
VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE


namespace ak
{
	std::vector<std::uint32_t> loadShaderFile(const std::string& filename)
	{
		std::ifstream file{ filename, std::ios::ate | std::ios::binary };
		if (!file.is_open())
		{
			throw wexception(L"loadShaderFile() : ���������� ������� ����");
		}

		auto ret = std::vector< std::uint32_t >{};
		auto fileSize = static_cast<size_t>(file.tellg());
		ret.resize(fileSize / sizeof(std::uint32_t));
		file.seekg(0);
		file.read(reinterpret_cast<char*>(ret.data()), fileSize);

		return ret;
	}

	VulkanSystem::VulkanSystem()
	{
	}

	VulkanSystem::~VulkanSystem()
	{
	}


#ifdef _DEBUG
	VKAPI_ATTR vk::Bool32 VKAPI_PTR VulkanSystem::debugCallback_(
		vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		vk::DebugUtilsMessageTypeFlagsEXT messageType,
		const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
	{
		VulkanSystem* pVulkanSystem = reinterpret_cast<VulkanSystem*>(pUserData);

		std::string message{};
		message += std::to_string(pCallbackData->messageIdNumber);
		message += ": ";
		message += pCallbackData->pMessage;
		message += "\r\n";

		if (messageSeverity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose)
		{
			pVulkanSystem->postLogMessage(LogLevelFlag::eVulkanVerbose, message);
			return VK_TRUE;
		}
		else if (messageSeverity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo)
		{
			pVulkanSystem->postLogMessage(LogLevelFlag::eVulkanInfo, message);
			return VK_TRUE;
		}
		else if (messageSeverity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning)
		{
			pVulkanSystem->postLogMessage(LogLevelFlag::eVulkanWarning, message);
			return VK_FALSE;
		}
		else if (messageSeverity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eError)
		{
			pVulkanSystem->postLogMessage(LogLevelFlag::eVulkanError, message);
			return VK_FALSE;
		}

		return VK_TRUE;
	}
#endif // _DEBUG

	void VulkanSystem::postLogMessage(LogLevelFlag logLevel, const char* message)
	{
#ifdef _DEBUG
		shpLoggingSystem_->postLogMessage(logLevel, message);
#else
		UNREFERENCED_PARAMETER(logLevel);
		UNREFERENCED_PARAMETER(message);
#endif // _DEBUG
	}

	void VulkanSystem::postLogMessage(LogLevelFlag logLevel, const std::string& message)
	{
#ifdef _DEBUG
		shpLoggingSystem_->postLogMessage(logLevel, message);
#else
		UNREFERENCED_PARAMETER(logLevel);
		UNREFERENCED_PARAMETER(message);
#endif // _DEBUG
	}

	void VulkanSystem::postLogMessage(LogLevelFlag logLevel, const std::wstring& message)
	{
#ifdef _DEBUG
		shpLoggingSystem_->postLogMessage(logLevel, message);
#else
		UNREFERENCED_PARAMETER(logLevel);
		UNREFERENCED_PARAMETER(message);
#endif // _DEBUG
	}


#ifdef _DEBUG	
	void VulkanSystem::init(const HWND& hWnd, const std::shared_ptr<LoggingSystem>& shpLoggingSystem)
#else
	void VulkanSystem::init(const HWND& hWnd)
#endif // _DEBUG
	{
#ifdef _DEBUG	
		shpLoggingSystem_ = shpLoggingSystem;
#endif // _DEBUG

		hWnd_ = hWnd;
		createInstance();
		createSurface();
		pickPhysicalDevice();
		createLogicalDevice();
		createSwapchain();
		createImageViews();
		createShaderModules();
		createGraphicsPipelineLayout();
		createGraphicsPipeline();
	}

	void VulkanSystem::createInstance()
	{
		static vk::detail::DynamicLoader dynamicLoader;
		auto vkGetInstanceProcAddr = dynamicLoader.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
		VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

		auto sApplicationInfo = vk::ApplicationInfo()
			.setPApplicationName("Vulkan API ��������")
			.setApplicationVersion(vk::makeApiVersion(0, 0, 1, 0))
			.setPEngineName("ak engine")
			.setEngineVersion(vk::makeApiVersion(0, 0, 1, 0))
			.setApiVersion(vk::makeApiVersion(0, 1, 3, 0));

		auto sInstanceCreateInfo = vk::InstanceCreateInfo()
			.setPApplicationInfo(&sApplicationInfo)
			.setPEnabledLayerNames(instanceLayerNames_)
			.setPEnabledExtensionNames(instanceExtensionNames_);

#ifdef _DEBUG
		auto sDebugUtilsMessengerCreateInfo = vk::DebugUtilsMessengerCreateInfoEXT{}
			.setMessageSeverity(
				vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
				//vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
				vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
				vk::DebugUtilsMessageSeverityFlagBitsEXT::eError)
			.setMessageType(
				vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
				vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
				vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance)
			//vk::DebugUtilsMessageTypeFlagBitsEXT::eDeviceAddressBinding
			.setPfnUserCallback(&debugCallback_)
			.setPUserData(this);

		sInstanceCreateInfo
			.setPNext(&sDebugUtilsMessengerCreateInfo);
#endif // _DEBUG

		if (!(uhInstance_ = vk::createInstanceUnique(sInstanceCreateInfo)))
		{
			throw wexception(L"VulkanSystem::createInstance() : ���������� ������� ��������� (Instance)");
		}
		vk::detail::defaultDispatchLoaderDynamic.init(uhInstance_.get());
		postLogMessage(LogLevelFlag::eAppInfo, L"VulkanSystem::createInstance() : ��������� (Instance) ������");

#ifdef _DEBUG
		if (!(uhDebugUtilsMessenger_ = uhInstance_->createDebugUtilsMessengerEXTUnique(
			sDebugUtilsMessengerCreateInfo, nullptr)))
		{
			throw wexception(L"VulkanSystem::createInstance() : ���������� ������� ���������� ��������������� ���������");
		}
		postLogMessage(LogLevelFlag::eAppInfo, L"VulkanSystem::createInstance() : ���������� ��������������� ��������� ������");
#endif // _DEBUG
	}

	void VulkanSystem::createSurface()
	{
		auto sWin32SurfaceCreateInfo = vk::Win32SurfaceCreateInfoKHR{}
			.setHwnd(hWnd_)
			.setHinstance(GetModuleHandle(nullptr));

		if (!(uhSurface_ = uhInstance_->createWin32SurfaceKHRUnique(sWin32SurfaceCreateInfo)))
		{
			throw wexception(L"VulkanSystem::createSurface() : ���������� ������� ����������� Win32 (Win32Surface)");
		}
		postLogMessage(LogLevelFlag::eAppInfo, L"VulkanSystem::createSurface() : ����������� Win32 (Win32Surface) �������");
	}

	void VulkanSystem::pickPhysicalDevice()
	{
		enum class PhysicalDeviceCriteriaFlag
		{
			eGeometryShaderSupport = 0,
			eSwapchainEXTSupport = 1,
			eSurfaceRGBAAndNonlinearFormatSupport = 2,
			eSurfaceMailboxPresentModeSupport = 3,
			eGraphicsAndPresentQueueFamilySupport = 4,
			eMax = 5
		};

		const auto physicalDevices = uhInstance_->enumeratePhysicalDevices();

		if (!physicalDevices.size())
		{
			throw wexception(L"VulkanSystem::pickPhysicalDevice() : GPU � ��������� Vulkan API �����������");
		}

		std::vector<std::array<bool, static_cast<size_t>(PhysicalDeviceCriteriaFlag::eMax)>> PhysicalDevicesCriteria{};
		PhysicalDevicesCriteria.resize(physicalDevices.size());

		uint32_t physicalDeviceIdx{};
		for (auto& physicalDevice : physicalDevices)
		{
			auto physicalDeviceProperties = physicalDevice.getProperties();
			auto physicalDeviceFeatures = physicalDevice.getFeatures();
			auto physicalDeviceExtentions = physicalDevice.enumerateDeviceExtensionProperties();
			auto physicalDeviceSurfaceFormats = physicalDevice.getSurfaceFormatsKHR(uhSurface_.get());
			auto physicalDeviceSurfacePresentModes = physicalDevice.getSurfacePresentModesKHR(uhSurface_.get());
			auto physicalDeviceQueueFamilyProperties = physicalDevice.getQueueFamilyProperties();

			PhysicalDevicesCriteria[physicalDeviceIdx][static_cast<int>(PhysicalDeviceCriteriaFlag::eGeometryShaderSupport)] =
				bool(physicalDeviceFeatures.geometryShader);

			for(const auto& physicalDeviceExtention : physicalDeviceExtentions)
			{
				if (std::find_if(deviceExtensionNames_.begin(), deviceExtensionNames_.end(),
					[&](const char* ref)
					{
						return static_cast<bool>(!std::strcmp(ref, physicalDeviceExtention.extensionName.data()));
					})
					!= deviceExtensionNames_.end())
				{
					PhysicalDevicesCriteria[physicalDeviceIdx][static_cast<int>(PhysicalDeviceCriteriaFlag::eSwapchainEXTSupport)] = true;
					break;
				}					
			}

			PhysicalDevicesCriteria[physicalDeviceIdx][static_cast<int>(PhysicalDeviceCriteriaFlag::eSurfaceRGBAAndNonlinearFormatSupport)] =
				(std::find(physicalDeviceSurfaceFormats.begin(), physicalDeviceSurfaceFormats.end(),
					vk::SurfaceFormatKHR(swapchainImageFormat_, vk::ColorSpaceKHR::eSrgbNonlinear))
					!= physicalDeviceSurfaceFormats.end());

			PhysicalDevicesCriteria[physicalDeviceIdx][static_cast<int>(PhysicalDeviceCriteriaFlag::eSurfaceMailboxPresentModeSupport)] =
				(std::find(physicalDeviceSurfacePresentModes.begin(), physicalDeviceSurfacePresentModes.end(), vk::PresentModeKHR::eMailbox)
					!= physicalDeviceSurfacePresentModes.end());
			

			uint32_t queueFamilyIdx{};

			int physicalDeviceQueueFamilyIdx{};
			for (auto& physicalDeviceQueueFamilyProperty : physicalDeviceQueueFamilyProperties)
			{
				PhysicalDevicesCriteria[physicalDeviceIdx][static_cast<int>(PhysicalDeviceCriteriaFlag::eGraphicsAndPresentQueueFamilySupport)] =
					bool(physicalDeviceQueueFamilyProperty.queueFlags & vk::QueueFlagBits::eGraphics);

				PhysicalDevicesCriteria[physicalDeviceIdx][static_cast<int>(PhysicalDeviceCriteriaFlag::eGraphicsAndPresentQueueFamilySupport)] &=
					bool(physicalDevice.getWin32PresentationSupportKHR(queueFamilyIdx));

				PhysicalDevicesCriteria[physicalDeviceIdx][static_cast<int>(PhysicalDeviceCriteriaFlag::eGraphicsAndPresentQueueFamilySupport)] &=
					bool(physicalDevice.getSurfaceSupportKHR(queueFamilyIdx, uhSurface_.get()));

				if (PhysicalDevicesCriteria[physicalDeviceIdx][static_cast<int>(PhysicalDeviceCriteriaFlag::eGraphicsAndPresentQueueFamilySupport)])
				{
					break;
				}
				queueFamilyIdx++;
			}

			if (std::find(PhysicalDevicesCriteria[physicalDeviceIdx].begin(), PhysicalDevicesCriteria[physicalDeviceIdx].end(), false) ==
				PhysicalDevicesCriteria[physicalDeviceIdx].end())
			{
				hPhysicalDevice_ = physicalDevice;
				graphicsAndPresentQueueFamilyIdx_ = queueFamilyIdx;
				break;
			}
			physicalDeviceIdx++;
		}

		if (hPhysicalDevice_)
		{
			sPhysicalDeviceProperties_ = hPhysicalDevice_.getProperties();
			sPhysicalDeviceFeatures_ = hPhysicalDevice_.getFeatures();
			sPhysicalDeviceSurfaceCapabilities_ = hPhysicalDevice_.getSurfaceCapabilitiesKHR(uhSurface_.get());

			postLogMessage(LogLevelFlag::eAppInfo, std::string("VulkanSystem::pickPhysicalDevice() : GPU: ") +
				std::string(sPhysicalDeviceProperties_.deviceName.data()) + std::string("\r\n"));
			postLogMessage(LogLevelFlag::eAppInfo, std::string("VulkanSystem::pickPhysicalDevice() : ��������� �������� GPU: ") +
				std::to_string(graphicsAndPresentQueueFamilyIdx_.value()) + std::string("\r\n"));
		}
		else
		{
			throw wexception(L"VulkanSystem::pickPhysicalDevice() : GPU � ��������� ���������� �����������");
		}
	}

	void VulkanSystem::createLogicalDevice()
	{
		const auto queuePriority = 1.0f;
		std::vector<vk::DeviceQueueCreateInfo> sDeviceQueueCreateInfos(1);
		sDeviceQueueCreateInfos[0]
			.setQueueFamilyIndex(graphicsAndPresentQueueFamilyIdx_.value())
			.setQueueCount(1)
			.setQueuePriorities(queuePriority);

		auto sDeviceCreateInfo = vk::DeviceCreateInfo{}
			.setPEnabledLayerNames(instanceLayerNames_)
			.setPEnabledExtensionNames(deviceExtensionNames_)
			.setPEnabledFeatures(&sPhysicalDeviceFeatures_)
			.setQueueCreateInfos(sDeviceQueueCreateInfos);

		if (!(uhDevice_ = hPhysicalDevice_.createDeviceUnique(sDeviceCreateInfo)))
		{
			throw wexception(L"VulkanSystem::createLogicalDevice() : ���������� ������� ���������� ���������� (Device)");
		}
		postLogMessage(LogLevelFlag::eAppInfo, L"VulkanSystem::createLogicalDevice() : ���������� ���������� (Device) �������");

		queue_ = uhDevice_->getQueue(graphicsAndPresentQueueFamilyIdx_.value(), 0);
	}

	void VulkanSystem::createSwapchain()
	{
		swapchainImageExtent_ = sPhysicalDeviceSurfaceCapabilities_.currentExtent;

		auto sSwapchainCreateInfo = vk::SwapchainCreateInfoKHR{}
			.setSurface(uhSurface_.get())
			.setMinImageCount(sPhysicalDeviceSurfaceCapabilities_.minImageCount + 1)
			.setImageFormat(swapchainImageFormat_)
			.setImageColorSpace(vk::ColorSpaceKHR::eSrgbNonlinear)
			.setImageExtent(swapchainImageExtent_)
			.setImageArrayLayers(1)
			.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
			.setImageSharingMode(vk::SharingMode::eExclusive)
			.setPreTransform(sPhysicalDeviceSurfaceCapabilities_.currentTransform)
			.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
			.setPresentMode(vk::PresentModeKHR::eMailbox)
			.setClipped(vk::True)
			.setOldSwapchain(uhOldSwapchain_.get());

		if (!(uhSwapchain_ = uhDevice_->createSwapchainKHRUnique(sSwapchainCreateInfo)))
		{
			throw wexception(L"VulkanSystem::createSwapchain() : ���������� ������� ������� ������ (Swapchain)");
		}
		postLogMessage(LogLevelFlag::eAppInfo, L"VulkanSystem::createSwapchain() : ������� ������ (Swapchain) �������");

		hSwapchainImages_ = uhDevice_->getSwapchainImagesKHR(uhSwapchain_.get());
	}

	void VulkanSystem::createImageViews()
	{
		auto sImageSubresourceRange = vk::ImageSubresourceRange{}
			.setAspectMask(vk::ImageAspectFlagBits::eColor)
			.setBaseMipLevel(0)
			.setLevelCount(1)
			.setBaseArrayLayer(0)
			.setLayerCount(1);

		uhSwapchainImageViews_.resize(hSwapchainImages_.size());

		uint32_t swapchainImageIdx{};
		for (auto& uhSwapchainImageView : uhSwapchainImageViews_)
		{
			auto sSwapchainImageViewCreateInfo = vk::ImageViewCreateInfo{}
				.setImage(hSwapchainImages_[swapchainImageIdx])
				.setViewType(vk::ImageViewType::e2D)
				.setFormat(swapchainImageFormat_)
				.setSubresourceRange(sImageSubresourceRange);

			if (!(uhSwapchainImageView = uhDevice_->createImageViewUnique(sSwapchainImageViewCreateInfo)))
			{
				throw wexception(L"VulkanSystem::createImageViews() : ���������� ������� ��� ����������� (ImageView)");
			}
		}
		postLogMessage(LogLevelFlag::eAppInfo, L"VulkanSystem::createImageViews() : ���� ����������� (ImageViews) �������");
	}

	void VulkanSystem::createShaderModules()
	{
		uhShaderModules_.resize(static_cast<int>(ShaderFlag::eMax));

		int shaderModuleIdx{};
		for (auto& uhShaderModule : uhShaderModules_)
		{
			auto buffer = loadShaderFile(uhShaderModuleFilenames_[shaderModuleIdx]);
			auto sShaderModuleCreateInfo = vk::ShaderModuleCreateInfo{}
				.setCode(buffer);

			if (!(uhShaderModule = uhDevice_->createShaderModuleUnique(sShaderModuleCreateInfo)))
			{
				throw wexception(L"VulkanSystem::createShaderModules() : ���������� ������� ��������� ������ (ShaderModule)");
			}
			shaderModuleIdx++;
		}
		postLogMessage(LogLevelFlag::eAppInfo, L"VulkanSystem::createShaderModules() : ��������� ������ (ShaderModules) �������");
	}

	void VulkanSystem::createGraphicsPipelineLayout()
	{
		auto sGraphicsPipelineLayoutCreateInfo = vk::PipelineLayoutCreateInfo{};
			
		if (!(uhGraphicsPipelineLayout_ = uhDevice_->createPipelineLayoutUnique(sGraphicsPipelineLayoutCreateInfo)))
		{
			throw wexception(L"VulkanSystem::createGraphicsPipelineLayout() : ���������� ������� ����� ������������ ��������� (GraphicsPipelineLayout)");
		}
		postLogMessage(LogLevelFlag::eAppInfo, L"VulkanSystem::createGraphicsPipelineLayout() : ����� ������������ ��������� (GraphicsPipelineLayout) ������");
	}

	void VulkanSystem::createGraphicsPipeline()
	{
		// 1. ������ - �������
		auto sGraphicsPipelineShaderStageCreateInfos = std::vector<vk::PipelineShaderStageCreateInfo>
		{
			vk::PipelineShaderStageCreateInfo{}
			.setStage(vk::ShaderStageFlagBits::eVertex)
			.setModule(uhShaderModules_[static_cast<int>(ShaderFlag::eVertex)].get())
			.setPName("main"),
			vk::PipelineShaderStageCreateInfo{}
			.setStage(vk::ShaderStageFlagBits::eFragment)
			.setModule(uhShaderModules_[static_cast<int>(ShaderFlag::eFragment)].get())
			.setPName("main")
		};

		// 2. ������ - ������� ������ ������
		auto sGraphicsPipelineVertexInputStateCreateInfo = vk::PipelineVertexInputStateCreateInfo{};

		// 3. ������ - ������� ������
		auto sGraphicsPipelineInputAssemblyStateCreateInfo = vk::PipelineInputAssemblyStateCreateInfo{}
			.setTopology(vk::PrimitiveTopology::eTriangleList);

		// 4. ������ - ����������
		auto sGraphicsPipelineTessellationStateCreateInfo = vk::PipelineTessellationStateCreateInfo{};

		// 5. ������ - ������� ������
		auto sViewport = vk::Viewport{}
			.setX(0.f)
			.setY(0.f)
			.setWidth(static_cast<float>(swapchainImageExtent_.width))
			.setHeight(static_cast<float>(swapchainImageExtent_.height))
			.setMinDepth(0.f)
			.setMaxDepth(1.f);

		auto sScissor = vk::Rect2D{ { 0, 0 }, swapchainImageExtent_ };

		auto sGraphicsPipelineViewportStateCreateInfo = vk::PipelineViewportStateCreateInfo{}
			.setViewports(sViewport)
			.setScissors(sScissor);

		// 6. ������ - ������������
		auto sGraphicsPipelineRasterizationStateCreateInfo = vk::PipelineRasterizationStateCreateInfo{}
			.setDepthClampEnable(false)
			.setRasterizerDiscardEnable(false)
			.setPolygonMode(vk::PolygonMode::eFill)
			.setLineWidth(1.f);

		// 7. ������ - ��������������
		auto sGraphicsPipelineMultisampleStateCreateInfo = vk::PipelineMultisampleStateCreateInfo{};

		// 8. ������ - ������� � ��������
		auto sGraphicsPipelineDepthStencilStateCreateInfo = vk::PipelineDepthStencilStateCreateInfo{};

		// 9. ������ - ���������� ������
		auto sColorBlendAttachment = vk::PipelineColorBlendAttachmentState{}
			.setBlendEnable(false)
			.setColorWriteMask(
				vk::ColorComponentFlagBits::eR |
				vk::ColorComponentFlagBits::eG |
				vk::ColorComponentFlagBits::eB |
				vk::ColorComponentFlagBits::eA);

		auto sGraphicsPipelineColorBlendStateCreateInfo = vk::PipelineColorBlendStateCreateInfo{}
			.setAttachments(sColorBlendAttachment);

		// 10. ������������ ���������
		auto graphicsPipelineDynamicStates = std::vector<vk::DynamicState>{ vk::DynamicState::eViewport };

		auto sGraphicsPipelineDynamicStateCreateInfo = vk::PipelineDynamicStateCreateInfo()
			.setDynamicStates(graphicsPipelineDynamicStates);


		// 11. ����������� ��������
		auto sGraphicsPipelineCreateInfo = vk::GraphicsPipelineCreateInfo()
			.setStages(sGraphicsPipelineShaderStageCreateInfos)
			.setPVertexInputState(&sGraphicsPipelineVertexInputStateCreateInfo)
			.setPInputAssemblyState(&sGraphicsPipelineInputAssemblyStateCreateInfo)
			.setPTessellationState(&sGraphicsPipelineTessellationStateCreateInfo)
			.setPViewportState(&sGraphicsPipelineViewportStateCreateInfo)
			.setPRasterizationState(&sGraphicsPipelineRasterizationStateCreateInfo)
			.setPMultisampleState(&sGraphicsPipelineMultisampleStateCreateInfo)
			.setPDepthStencilState(&sGraphicsPipelineDepthStencilStateCreateInfo)
			.setPColorBlendState(&sGraphicsPipelineColorBlendStateCreateInfo)
			.setPDynamicState(&sGraphicsPipelineDynamicStateCreateInfo)
			.setLayout(uhGraphicsPipelineLayout_.get());

		if (!(uhGraphicsPipeline_ = uhDevice_->createGraphicsPipelineUnique(vk::PipelineCache{}, sGraphicsPipelineCreateInfo).value))
		{
			throw wexception(L"VulkanSystem::createGraphicsPipeline() : ���������� ������� ����������� �������� (GraphicsPipeline)");
		}
		postLogMessage(LogLevelFlag::eAppInfo, L"VulkanSystem::createGraphicsPipeline() : ����������� �������� (GraphicsPipeline) ������");
	}
}