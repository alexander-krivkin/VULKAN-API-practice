#include "VulkanSystem.h"
VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE


namespace ak
{
	std::vector<uint32_t> loadShaderFile(const std::string& filename)
	{
		std::ifstream file{ filename, std::ios::ate | std::ios::binary };
		if (!file.is_open())
		{
			throw wexception(L"loadShaderFile() : Невозможно открыть файл");
		}

		auto ret = std::vector<uint32_t>{};
		auto fileSize = static_cast<size_t>(file.tellg());
		ret.resize(fileSize / sizeof(uint32_t));
		file.seekg(0);
		file.read(reinterpret_cast<char*>(ret.data()), fileSize);

		return ret;
	}

	VulkanSystem::VulkanSystem()
	{
	}

	VulkanSystem::~VulkanSystem()
	{
		uhDevice_->waitIdle();
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
			pVulkanSystem->postLogMessage_(LogLevelFlagBits::eVulkanVerbose, message);
			return vk::True;
		}
		else if (messageSeverity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo)
		{
			pVulkanSystem->postLogMessage_(LogLevelFlagBits::eVulkanInfo, message);
			return vk::True;
		}
		else if (messageSeverity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning)
		{
			pVulkanSystem->postLogMessage_(LogLevelFlagBits::eVulkanWarning, message);
			return vk::False;
		}
		else if (messageSeverity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eError)
		{
			pVulkanSystem->postLogMessage_(LogLevelFlagBits::eVulkanError, message);
			return vk::False;
		}

		return vk::True;
	}
#endif // _DEBUG

	void VulkanSystem::postLogMessage_(LogLevelFlagBits logLevel, const char* message)
	{
#ifdef _DEBUG
		shpLoggingSystem_->postLogMessage(logLevel, message);
#else
		UNREFERENCED_PARAMETER(logLevel);
		UNREFERENCED_PARAMETER(message);
#endif // _DEBUG
	}

	void VulkanSystem::postLogMessage_(LogLevelFlagBits logLevel, const std::string& message)
	{
#ifdef _DEBUG
		shpLoggingSystem_->postLogMessage(logLevel, message);
#else
		UNREFERENCED_PARAMETER(logLevel);
		UNREFERENCED_PARAMETER(message);
#endif // _DEBUG
	}

	void VulkanSystem::postLogMessage_(LogLevelFlagBits logLevel, const std::wstring& message)
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
		createInstance_();
		createSurface_();
		pickPhysicalDevice_();
		createLogicalDevice_();
		createSwapchain_();
		createImageViews_();
		createShaderModules_();
		createDescriptorSetLayout_();
		createGraphicsPipelineLayout_();
		createRenderPass_();
		createGraphicsPipeline_();
		createFramebuffers_();
		createSyncObjects_();
		createCommandPools_();
		allocateCommandBuffers_();
		createDescriptorPool_();

		initGrid3D_();
		initCrosshair3D_();
		createVertexBuffer_(ObjectFlag::eGrid);
		createVertexBuffer_(ObjectFlag::eCrosshair);
		createIndexBuffer_(ObjectFlag::eGrid);
		createIndexBuffer_(ObjectFlag::eCrosshair);
		createUniformBuffers_(ObjectFlag::eGrid);
		createUniformBuffers_(ObjectFlag::eCrosshair);
		allocateDescriptorSets_(ObjectFlag::eGrid);
		allocateDescriptorSets_(ObjectFlag::eCrosshair);

		updateWindowUniformModelTranformations_();

		state_.readiness = VulkanSystemFlag::eReadyAndStarted;
		draw = std::bind(&VulkanSystem::drawStarted_, this);
	}

	void VulkanSystem::resize()
	{
		if (state_.readiness != VulkanSystemFlag::eNotReady)
		{
			uhDevice_->waitIdle();

			createSwapchain_();
			updateWindowUniformModelTranformations_();
			createImageViews_();
			createFramebuffers_();
		}
	}

	void VulkanSystem::pause()
	{
		if (state_.readiness == VulkanSystemFlag::eReadyAndStarted)
		{
			state_.readiness = VulkanSystemFlag::eReadyAndPaused;
			draw = std::bind(&VulkanSystem::drawPaused_, this);
		}
	}

	void VulkanSystem::resume()
	{
		if (state_.readiness == VulkanSystemFlag::eReadyAndPaused)
		{
			state_.readiness = VulkanSystemFlag::eReadyAndStarted;
			draw = std::bind(&VulkanSystem::drawStarted_, this);
		}
	}

	void VulkanSystem::updateCrosshairPosition(POINT cursorPosition)
	{
		state_.crosshairPosition.x = static_cast<float>(cursorPosition.x);
		state_.crosshairPosition.y = static_cast<float>(cursorPosition.y);
	}



	void VulkanSystem::createInstance_()
	{
		static vk::detail::DynamicLoader dynamicLoader;
		auto vkGetInstanceProcAddr = dynamicLoader.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
		VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

		auto sApplicationInfo = vk::ApplicationInfo()
			.setPApplicationName("Vulkan API практика")
			.setApplicationVersion(vk::makeApiVersion(0, 0, 1, 0))
			.setPEngineName("ak engine")
			.setEngineVersion(vk::makeApiVersion(0, 0, 1, 0))
			.setApiVersion(vk::makeApiVersion(0, 1, 3, 0));

		auto sInstanceCreateInfo = vk::InstanceCreateInfo()
			.setPApplicationInfo(&sApplicationInfo)
			.setPEnabledLayerNames(instanceLayerNames_)
			.setPEnabledExtensionNames(instanceExtensionNames_);

#ifdef _DEBUG

		vk::DebugUtilsMessageSeverityFlagsEXT messageSeverityFlags{};

		if (shpLoggingSystem_->getFlags() & LogLevelFlagBits::eVulkanVerbose)
		{
			messageSeverityFlags |= vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose;
		}
		if (shpLoggingSystem_->getFlags() & LogLevelFlagBits::eVulkanInfo)
		{
			messageSeverityFlags |= vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo;
		}
		if (shpLoggingSystem_->getFlags() & LogLevelFlagBits::eVulkanWarning)
		{
			messageSeverityFlags |= vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning;
		}
		if (shpLoggingSystem_->getFlags() & LogLevelFlagBits::eVulkanError)
		{
			messageSeverityFlags |= vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
		}

		auto sDebugUtilsMessengerCreateInfo = vk::DebugUtilsMessengerCreateInfoEXT{}
			.setMessageSeverity(messageSeverityFlags)
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
			throw wexception(L"VulkanSystem::createInstance_() : Невозможно создать экземпляр (Instance)");
		}
		vk::detail::defaultDispatchLoaderDynamic.init(uhInstance_.get());
		postLogMessage_(LogLevelFlagBits::eAppInfo, L"VulkanSystem::createInstance_() : Экземпляр (Instance) создан");

#ifdef _DEBUG
		if (!(uhDebugUtilsMessenger_ = uhInstance_->createDebugUtilsMessengerEXTUnique(
			sDebugUtilsMessengerCreateInfo, nullptr)))
		{
			throw wexception(L"VulkanSystem::createInstance_() : Невозможно создать обработчик диагностических сообщений");
		}
		postLogMessage_(LogLevelFlagBits::eAppInfo, L"VulkanSystem::createInstance_() : Обработчик диагностических сообщений создан");
#endif // _DEBUG
	}

	void VulkanSystem::createSurface_()
	{
		auto sWin32SurfaceCreateInfo = vk::Win32SurfaceCreateInfoKHR{}
			.setHwnd(hWnd_)
			.setHinstance(GetModuleHandle(nullptr));

		if (!(uhSurface_ = uhInstance_->createWin32SurfaceKHRUnique(sWin32SurfaceCreateInfo)))
		{
			throw wexception(L"VulkanSystem::createSurface_() : Невозможно создать поверхность Win32 (Win32Surface)");
		}
		postLogMessage_(LogLevelFlagBits::eAppInfo, L"VulkanSystem::createSurface_() : Поверхность Win32 (Win32Surface) создана");
	}

	void VulkanSystem::pickPhysicalDevice_()
	{
		const auto physicalDevices = uhInstance_->enumeratePhysicalDevices();

		if (!physicalDevices.size())
		{
			throw wexception(L"VulkanSystem::pickPhysicalDevice_() : GPU с драйвером Vulkan API отсутствует");
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

			for (const auto& physicalDeviceExtention : physicalDeviceExtentions)
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
					vk::SurfaceFormatKHR(state_.windowFormat, vk::ColorSpaceKHR::eSrgbNonlinear))
					!= physicalDeviceSurfaceFormats.end());

			PhysicalDevicesCriteria[physicalDeviceIdx][static_cast<int>(PhysicalDeviceCriteriaFlag::eSurfaceMailboxPresentModeSupport)] =
				(std::find(physicalDeviceSurfacePresentModes.begin(), physicalDeviceSurfacePresentModes.end(), vk::PresentModeKHR::eMailbox)
					!= physicalDeviceSurfacePresentModes.end());


			uint32_t queueFamilyIdx{};

			for (auto& physicalDeviceQueueFamilyProperty : physicalDeviceQueueFamilyProperties)
			{
				if (!(physicalDeviceQueueFamilyProperty.queueFlags & vk::QueueFlagBits::eGraphics) &&
					(physicalDeviceQueueFamilyProperty.queueFlags & vk::QueueFlagBits::eTransfer))
				{
					transferQueueFamilyIdx_ = queueFamilyIdx;
					break;
				}
				queueFamilyIdx++;
			}

			queueFamilyIdx = 0;

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
			sPhysicalDeviceMemoryProperties_ = hPhysicalDevice_.getMemoryProperties();
			sPhysicalDeviceProperties_ = hPhysicalDevice_.getProperties();
			sPhysicalDeviceFeatures_ = hPhysicalDevice_.getFeatures();

			postLogMessage_(LogLevelFlagBits::eAppInfo, std::string("VulkanSystem::pickPhysicalDevice_() : GPU: ") +
				std::string(sPhysicalDeviceProperties_.deviceName.data()) + std::string("\r\n"));
			postLogMessage_(LogLevelFlagBits::eAppInfo, std::string("VulkanSystem::pickPhysicalDevice_() : Семейство очередей GPU: ") +
				std::to_string(graphicsAndPresentQueueFamilyIdx_.value()) + std::string("\r\n"));
		}
		else
		{
			throw wexception(L"VulkanSystem::pickPhysicalDevice_() : GPU с заданными критериями отсутствует");
		}
	}

	void VulkanSystem::createLogicalDevice_()
	{
		auto queuePriority = 1.0f;

		std::vector<vk::DeviceQueueCreateInfo> sDeviceQueueCreateInfos
		{
		vk::DeviceQueueCreateInfo{}
			.setQueueFamilyIndex(transferQueueFamilyIdx_.value())
			.setQueueCount(1)
			.setQueuePriorities(queuePriority),
		vk::DeviceQueueCreateInfo{}
			.setQueueFamilyIndex(graphicsAndPresentQueueFamilyIdx_.value())
			.setQueueCount(1)
			.setQueuePriorities(queuePriority)
		};

		auto sDeviceCreateInfo = vk::DeviceCreateInfo{}
			.setPEnabledLayerNames(instanceLayerNames_)
			.setPEnabledExtensionNames(deviceExtensionNames_)
			.setPEnabledFeatures(&sPhysicalDeviceFeatures_)
			.setQueueCreateInfos(sDeviceQueueCreateInfos);

		if (!(uhDevice_ = hPhysicalDevice_.createDeviceUnique(sDeviceCreateInfo)))
		{
			throw wexception(L"VulkanSystem::createLogicalDevice_() : Невозможно создать логическое устройство (Device)");
		}
		postLogMessage_(LogLevelFlagBits::eAppInfo, L"VulkanSystem::createLogicalDevice_() : Логическое устройство (Device) создано");

		if (!transferQueueFamilyIdx_.has_value())
		{
			transferQueueFamilyIdx_ = graphicsAndPresentQueueFamilyIdx_.value();
		}
		hTransferQueue_ = uhDevice_->getQueue(transferQueueFamilyIdx_.value(), 0);
		hGraphicsQueue_ = uhDevice_->getQueue(graphicsAndPresentQueueFamilyIdx_.value(), 0);
	}

	void VulkanSystem::createSwapchain_()
	{
		if (uhSwapchain_)
		{
			uhOldSwapchain_ = std::move(uhSwapchain_);
		}

		sPhysicalDeviceSurfaceCapabilities_ = hPhysicalDevice_.getSurfaceCapabilitiesKHR(uhSurface_.get());
		state_.windowExtent = sPhysicalDeviceSurfaceCapabilities_.currentExtent;

		state_.windowViewport = vk::Viewport{}
			.setX(0.0f)
			.setY(0.0f)
			.setWidth(static_cast<float>(state_.windowExtent.width))
			.setHeight(static_cast<float>(state_.windowExtent.height))
			.setMinDepth(0.0f)
			.setMaxDepth(1.0f);

		state_.windowScissor = vk::Rect2D{ { 0, 0 }, state_.windowExtent };

		auto sSwapchainCreateInfo = vk::SwapchainCreateInfoKHR{}
			.setSurface(uhSurface_.get())
			.setMinImageCount(sPhysicalDeviceSurfaceCapabilities_.minImageCount + 1)
			.setImageFormat(state_.windowFormat)
			.setImageColorSpace(vk::ColorSpaceKHR::eSrgbNonlinear)
			.setImageExtent(state_.windowExtent)
			.setImageArrayLayers(1)
			.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
			.setImageSharingMode(vk::SharingMode::eExclusive)
			.setQueueFamilyIndices(graphicsAndPresentQueueFamilyIdx_.value())
			.setPreTransform(sPhysicalDeviceSurfaceCapabilities_.currentTransform)
			.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
			.setPresentMode(vk::PresentModeKHR::eMailbox)
			.setClipped(vk::True)
			.setOldSwapchain(uhOldSwapchain_.get());

		if (!(uhSwapchain_ = uhDevice_->createSwapchainKHRUnique(sSwapchainCreateInfo)))
		{
			throw wexception(L"VulkanSystem::createSwapchain_() : Невозможно создать цепочку показа (Swapchain)");
		}
		postLogMessage_(LogLevelFlagBits::eAppInfo, L"VulkanSystem::createSwapchain_() : Цепочка показа (Swapchain) создана");

		hSwapchainImages_ = uhDevice_->getSwapchainImagesKHR(uhSwapchain_.get());
	}

	void VulkanSystem::createImageViews_()
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
				.setFormat(state_.windowFormat)
				.setSubresourceRange(sImageSubresourceRange);

			if (!(uhSwapchainImageView = uhDevice_->createImageViewUnique(sSwapchainImageViewCreateInfo)))
			{
				throw wexception(L"VulkanSystem::createImageViews_() : Невозможно создать вид изображения (ImageView)");
			}
			swapchainImageIdx++;
		}
		postLogMessage_(LogLevelFlagBits::eAppInfo, L"VulkanSystem::createImageViews_() : Виды изображений (ImageViews) созданы");
	}

	void VulkanSystem::createShaderModules_()
	{
		uint32_t shaderModuleIdx{};
		for (auto& uhShaderModule : uhShaderModules_)
		{
			auto buffer = loadShaderFile(uhShaderModuleFilenames_[shaderModuleIdx]);
			auto sShaderModuleCreateInfo = vk::ShaderModuleCreateInfo{}
			.setCode(buffer);

			if (!(uhShaderModule = uhDevice_->createShaderModuleUnique(sShaderModuleCreateInfo)))
			{
				throw wexception(L"VulkanSystem::createShaderModules_() : Невозможно создать шейдерный модуль (ShaderModule)");
			}
			shaderModuleIdx++;
		}
		postLogMessage_(LogLevelFlagBits::eAppInfo, L"VulkanSystem::createShaderModules_() : Шейдерные модули (ShaderModules) созданы");
	}

	void VulkanSystem::createDescriptorSetLayout_()
	{
		auto sDescriptorSetLayoutBinding = vk::DescriptorSetLayoutBinding{}
			.setBinding(0)
			.setDescriptorType(vk::DescriptorType::eUniformBuffer)
			.setDescriptorCount(1)
			.setStageFlags(vk::ShaderStageFlagBits::eVertex);

		auto sDescriptorSetLayoutCreateInfo = vk::DescriptorSetLayoutCreateInfo{}
		.setBindings(sDescriptorSetLayoutBinding);

		if (!(uhDescriptorSetLayout_ = uhDevice_->createDescriptorSetLayoutUnique(sDescriptorSetLayoutCreateInfo)))
		{
			throw wexception(L"VulkanSystem::createDescriptorSetLayout_() : Невозможно создать макет набора дескрипторов (DescriptorSetLayout)");
		}
		postLogMessage_(LogLevelFlagBits::eAppInfo, L"VulkanSystem::createDescriptorSetLayout_() : Макет набора дескрипторов (DescriptorSetLayout) создан");
	}

	void VulkanSystem::createGraphicsPipelineLayout_()
	{
		auto sGraphicsPipelineLayoutCreateInfo = vk::PipelineLayoutCreateInfo{}
		.setSetLayouts(uhDescriptorSetLayout_.get());

		if (!(uhGraphicsPipelineLayout_ = uhDevice_->createPipelineLayoutUnique(sGraphicsPipelineLayoutCreateInfo)))
		{
			throw wexception(L"VulkanSystem::createGraphicsPipelineLayout_() : Невозможно создать макет графического конвейера (GraphicsPipelineLayout)");
		}
		postLogMessage_(LogLevelFlagBits::eAppInfo, L"VulkanSystem::createGraphicsPipelineLayout_() : Макет графического конвейера (GraphicsPipelineLayout) создан");
	}

	void VulkanSystem::createRenderPass_()
	{
		auto sAttachmentDescription = vk::AttachmentDescription{}
			.setFormat(state_.windowFormat)
			.setSamples(vk::SampleCountFlagBits::e1)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eStore)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

		auto sAttachmentReference = vk::AttachmentReference{}
			.setAttachment(0)
			.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

		const auto sSubpassDescription = vk::SubpassDescription{}
			.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
			.setColorAttachments(sAttachmentReference);

		auto sSubpassDependency = vk::SubpassDependency{}
			.setSrcSubpass(VK_SUBPASS_EXTERNAL)
			.setDstSubpass(0)
			.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
			.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
			.setSrcAccessMask(vk::AccessFlagBits::eNone)
			.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);

		auto sRenderPassCreateInfo = vk::RenderPassCreateInfo{}
			.setAttachments(sAttachmentDescription)
			.setSubpasses(sSubpassDescription)
			.setDependencies(sSubpassDependency);

		if (!(uhRenderPass_ = uhDevice_->createRenderPassUnique(sRenderPassCreateInfo)))
		{
			throw wexception(L"VulkanSystem::createRenderPass_() : Невозможно создать проход рендеринга (RenderPass)");
		}
		postLogMessage_(LogLevelFlagBits::eAppInfo, L"VulkanSystem::createRenderPass_() : Проход рендеринга (RenderPass) создан");
	}

	void VulkanSystem::createGraphicsPipeline_()
	{
		// 1. Стадия - Шейдеры
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

		// 2. Стадия - Входные данные вершин
		auto sVertexInputBindingDescription = geometry::getBindDescription<geometry::Point3D>(0);
		auto sVertexInputAttributeDescriptions = geometry::getAttributeDescriptions<geometry::Point3D>(0);

		auto sGraphicsPipelineVertexInputStateCreateInfo = vk::PipelineVertexInputStateCreateInfo{}
			.setVertexBindingDescriptions(sVertexInputBindingDescription)
			.setVertexAttributeDescriptions(sVertexInputAttributeDescriptions);

		// 3. Стадия - Входная сборка
		auto sGraphicsPipelineInputAssemblyStateCreateInfo = vk::PipelineInputAssemblyStateCreateInfo{}
		.setTopology(vk::PrimitiveTopology::eLineList);

		// 4. Стадия - Тесселяция
		auto sGraphicsPipelineTessellationStateCreateInfo = vk::PipelineTessellationStateCreateInfo{};

		// 5. Стадия - Область вывода
		auto sGraphicsPipelineViewportStateCreateInfo = vk::PipelineViewportStateCreateInfo{}
			.setViewports(state_.windowViewport)
			.setScissors(state_.windowScissor);

		// 6. Стадия - Растеризация
		auto sGraphicsPipelineRasterizationStateCreateInfo = vk::PipelineRasterizationStateCreateInfo{}
			.setDepthClampEnable(vk::False)
			.setRasterizerDiscardEnable(vk::False)
			.setPolygonMode(vk::PolygonMode::eFill)
			.setLineWidth(1.0f);

		// 7. Стадия - Мультисэмплинг
		auto sGraphicsPipelineMultisampleStateCreateInfo = vk::PipelineMultisampleStateCreateInfo{};

		// 8. Стадия - Глубина и трафарет
		auto sGraphicsPipelineDepthStencilStateCreateInfo = vk::PipelineDepthStencilStateCreateInfo{};

		// 9. Стадия - Смешивание цветов
		auto sColorBlendAttachment = vk::PipelineColorBlendAttachmentState{}
			.setBlendEnable(vk::False)
			.setColorWriteMask(
				vk::ColorComponentFlagBits::eR |
				vk::ColorComponentFlagBits::eG |
				vk::ColorComponentFlagBits::eB |
				vk::ColorComponentFlagBits::eA);

		auto sGraphicsPipelineColorBlendStateCreateInfo = vk::PipelineColorBlendStateCreateInfo{}
		.setAttachments(sColorBlendAttachment);

		// 10. Динамическое состояние
		auto graphicsPipelineDynamicStates = std::vector<vk::DynamicState>{
			vk::DynamicState::eViewport,
			vk::DynamicState::eScissor };

		auto sGraphicsPipelineDynamicStateCreateInfo = vk::PipelineDynamicStateCreateInfo{}
		.setDynamicStates(graphicsPipelineDynamicStates);

		// 11. Графический конвейер
		auto sGraphicsPipelineCreateInfo = vk::GraphicsPipelineCreateInfo{}
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
			.setLayout(uhGraphicsPipelineLayout_.get())
			.setRenderPass(uhRenderPass_.get());

		if (!(uhGraphicsPipeline_ = uhDevice_->createGraphicsPipelineUnique(vk::PipelineCache{}, sGraphicsPipelineCreateInfo).value))
		{
			throw wexception(L"VulkanSystem::createGraphicsPipeline_() : Невозможно создать графический конвейер (GraphicsPipeline)");
		}
		postLogMessage_(LogLevelFlagBits::eAppInfo, L"VulkanSystem::createGraphicsPipeline_() : Графический конвейер (GraphicsPipeline) создан");
	}

	void VulkanSystem::createFramebuffers_()
	{
		uhFramebuffers_.resize(hSwapchainImages_.size());

		uint32_t framebufferIdx{};
		for (auto& uhFramebuffer : uhFramebuffers_)
		{
			auto sFramebufferCreateInfo = vk::FramebufferCreateInfo{}
				.setRenderPass(uhRenderPass_.get())
				.setAttachments(uhSwapchainImageViews_[framebufferIdx].get())
				.setWidth(state_.windowExtent.width)
				.setHeight(state_.windowExtent.height)
				.setLayers(1);

			if (!(uhFramebuffer = uhDevice_->createFramebufferUnique(sFramebufferCreateInfo)))
			{
				throw wexception(L"VulkanSystem::createFramebuffers_() : Невозможно создать буфер кадра (Framebuffer)");
			}
			framebufferIdx++;
		}
		postLogMessage_(LogLevelFlagBits::eAppInfo, L"VulkanSystem::createFramebuffers_() : Буферы кадра (Framebuffers) созданы");
	}

	void VulkanSystem::createSyncObjects_()
	{
		uhTransferFences_.resize(numFramesInFlight + 2);
		uhInFlightFences_.resize(numFramesInFlight);
		uhReadyForRenderingSemaphores_.resize(numFramesInFlight);
		uhReadyForPresentingSemaphores_.resize(numFramesInFlight);

		auto sSemaphoreCreateInfo = vk::SemaphoreCreateInfo{};

		auto sFenceCreateInfo = vk::FenceCreateInfo{}
		.setFlags(vk::FenceCreateFlagBits::eSignaled);

		for (uint32_t idx{}; idx < numFramesInFlight + 2; idx++)
		{
			if (!(uhTransferFences_[idx] = uhDevice_->createFenceUnique(sFenceCreateInfo)))
			{
				throw wexception(L"VulkanSystem::createSyncObjects_() : Невозможно создать забор (Fence)");
			}
		}

		for (uint32_t idx{}; idx < numFramesInFlight; idx++)
		{
			if (!(uhInFlightFences_[idx] = uhDevice_->createFenceUnique(sFenceCreateInfo)))
			{
				throw wexception(L"VulkanSystem::createSyncObjects_() : Невозможно создать забор (Fence)");
			}

			if (!(uhReadyForRenderingSemaphores_[idx] = uhDevice_->createSemaphoreUnique(sSemaphoreCreateInfo)))
			{
				throw wexception(L"VulkanSystem::createSyncObjects_() : Невозможно создать семафор (Semaphore)");
			}

			if (!(uhReadyForPresentingSemaphores_[idx] = uhDevice_->createSemaphoreUnique(sSemaphoreCreateInfo)))
			{
				throw wexception(L"VulkanSystem::createSyncObjects_() : Невозможно создать семафор (Semaphore)");
			}
		}
		postLogMessage_(LogLevelFlagBits::eAppInfo, L"VulkanSystem::createSyncObjects_() : Объекты синхронизации созданы");
	}

	void VulkanSystem::createCommandPools_()
	{
		auto sTransferCommandPoolCreateInfo = vk::CommandPoolCreateInfo{}
			.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
			.setQueueFamilyIndex(transferQueueFamilyIdx_.value());

		if (!(uhTransferCommandPool_ = uhDevice_->createCommandPoolUnique(sTransferCommandPoolCreateInfo)))
		{
			throw wexception(L"VulkanSystem::createCommandPools_() : Невозможно создать командный пул (CommandPool) для передачи");
		}
		postLogMessage_(LogLevelFlagBits::eAppInfo, L"VulkanSystem::createCommandPool_() : Командный пул (CommandPool) для передачи создан");

		auto sCommandPoolCreateInfo = vk::CommandPoolCreateInfo{}
			.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
			.setQueueFamilyIndex(graphicsAndPresentQueueFamilyIdx_.value());

		if (!(uhGraphicsCommandPool_ = uhDevice_->createCommandPoolUnique(sCommandPoolCreateInfo)))
		{
			throw wexception(L"VulkanSystem::createCommandPools_() : Невозможно создать командный пул (CommandPool) для графики");
		}
		postLogMessage_(LogLevelFlagBits::eAppInfo, L"VulkanSystem::createCommandPool_() : Командный пул (CommandPool) для графики создан");
	}

	void VulkanSystem::allocateCommandBuffers_()
	{
		auto sTransferCommandBufferAllocateInfo = vk::CommandBufferAllocateInfo{}
			.setCommandPool(*uhTransferCommandPool_)
			.setLevel(vk::CommandBufferLevel::ePrimary)
			.setCommandBufferCount(numFramesInFlight + 2);

		if (!(hTransferCommandBuffers_ = uhDevice_->allocateCommandBuffers(sTransferCommandBufferAllocateInfo)).size())
		{
			throw wexception(L"VulkanSystem::allocateCommandBuffers_() : Невозможно выделить командные буферы (CommandBuffers) для передачи");
		}
		postLogMessage_(LogLevelFlagBits::eAppInfo, L"VulkanSystem::allocateCommandBuffers_() : Командные буферы (CommandBuffers) для передачи выделены");

		auto sGraphicsCommandBufferAllocateInfo = vk::CommandBufferAllocateInfo{}
			.setCommandPool(*uhGraphicsCommandPool_)
			.setLevel(vk::CommandBufferLevel::ePrimary)
			.setCommandBufferCount(numFramesInFlight);

		if (!(hGraphicsCommandBuffers_ = uhDevice_->allocateCommandBuffers(sGraphicsCommandBufferAllocateInfo)).size())
		{
			throw wexception(L"VulkanSystem::allocateCommandBuffers_() : Невозможно выделить командные буферы (CommandBuffers) для графики");
		}
		postLogMessage_(LogLevelFlagBits::eAppInfo, L"VulkanSystem::allocateCommandBuffers_() : Командные буферы (CommandBuffers) для графики выделены");
	}

	void VulkanSystem::createDescriptorPool_()
	{
		auto sDescriptorPoolSize = vk::DescriptorPoolSize{}
			.setType(vk::DescriptorType::eUniformBuffer)
			.setDescriptorCount(static_cast<uint32_t>(2 * numFramesInFlight));

		auto sDescriptorPoolCreateInfo = vk::DescriptorPoolCreateInfo{}
			.setPoolSizes(sDescriptorPoolSize)
			.setMaxSets(static_cast<uint32_t>(2 * numFramesInFlight));

		if (!(uhDescriptorPool_ = uhDevice_->createDescriptorPoolUnique(sDescriptorPoolCreateInfo)))
		{
			throw wexception(L"VulkanSystem::createDescriptorPool_() : Невозможно выделить пул дескрипторов (DescriptorPool)");
		}
		postLogMessage_(LogLevelFlagBits::eAppInfo, L"VulkanSystem::createDescriptorPool_() : Пул дескрипторов (DescriptorPool) создан");
	}



	uint32_t VulkanSystem::findSuitableMemoryIndex(uint32_t allowedTypesMask,
		vk::MemoryPropertyFlags requiredMemoryFlags)
	{
		for (uint32_t memoryType{ 1 }, idx{ 0 }; idx < sPhysicalDeviceMemoryProperties_.memoryTypeCount;
			++idx, memoryType <<= 1)
		{
			if ((allowedTypesMask & memoryType) > 0 &&
				((sPhysicalDeviceMemoryProperties_.memoryTypes[idx].propertyFlags &
					requiredMemoryFlags) == requiredMemoryFlags))
			{
				return idx;
			}
		}
		throw wexception(L"Невозможно найти подходящую память GPU");
	}

	std::pair<vk::UniqueBuffer, vk::UniqueDeviceMemory> VulkanSystem::createBufferMemoryPair_(vk::DeviceSize size, vk::BufferUsageFlags usageFlags,
		vk::SharingMode sharingMode, std::vector<uint32_t> queueFamilyIndices, vk::MemoryPropertyFlags requiredMemoryFlags)
	{
		std::pair<vk::UniqueBuffer, vk::UniqueDeviceMemory> bufferMemoryPair{};

		auto sBufferCreateInfo = vk::BufferCreateInfo{}
			.setSize(size)
			.setUsage(usageFlags)
			.setSharingMode(sharingMode)
			.setQueueFamilyIndices(queueFamilyIndices);

		if (!(bufferMemoryPair.first = uhDevice_->createBufferUnique(sBufferCreateInfo)))
		{
			throw wexception(L"VulkanSystem::createBufferMemoryPair_() : Невозможно создать буфер (Buffer)");
		}

		auto bufferMemoryRequirements = uhDevice_->getBufferMemoryRequirements(bufferMemoryPair.first.get());
		auto memoryIdx = findSuitableMemoryIndex(bufferMemoryRequirements.memoryTypeBits, requiredMemoryFlags);

		auto sMemoryAllocateInfo = vk::MemoryAllocateInfo{}
			.setAllocationSize(bufferMemoryRequirements.size)
			.setMemoryTypeIndex(memoryIdx);

		if (!(bufferMemoryPair.second = uhDevice_->allocateMemoryUnique(sMemoryAllocateInfo)))
		{
			throw wexception(L"VulkanSystem::createBufferMemoryPair_ : Невозможно выделить память буфера (BufferMemory)");
		}

		uhDevice_->bindBufferMemory(bufferMemoryPair.first.get(), bufferMemoryPair.second.get(), 0u);
		postLogMessage_(LogLevelFlagBits::eAppInfo, L"VulkanSystem::createBufferMemoryPair_ : Пара (Буфер - Память буфера) (Buffer, BufferMemory) создана");

		return bufferMemoryPair;
	}

	void VulkanSystem::copyBuffer_(uint32_t transferIdx, vk::Buffer& hSrcBuffer, vk::Buffer& hDstBuffer, vk::DeviceSize size)
	{
		auto result = uhDevice_->waitForFences(uhTransferFences_[transferIdx].get(), vk::True, UINT64_MAX);
		if (result != vk::Result::eSuccess)
		{
			throw wexception(L"VulkanSystem::copyBuffer_() : Ошибка выполнения waitForFences");
		}

		auto sCommandBufferBeginInfo = vk::CommandBufferBeginInfo{}
		.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

		auto sBufferCopy = vk::BufferCopy{}
			.setSrcOffset(0)
			.setDstOffset(0)
			.setSize(size);

		hTransferCommandBuffers_[transferIdx].begin(sCommandBufferBeginInfo);
		hTransferCommandBuffers_[transferIdx].copyBuffer(hSrcBuffer, hDstBuffer, sBufferCopy);
		hTransferCommandBuffers_[transferIdx].end();

		auto sSubmitInfo = vk::SubmitInfo{}
		.setCommandBuffers(hTransferCommandBuffers_[transferIdx]);

		uhDevice_->resetFences(uhTransferFences_[transferIdx].get());
		hTransferQueue_.submit(sSubmitInfo, uhTransferFences_[transferIdx].get());
	}



	void VulkanSystem::initGrid3D_()
	{
		auto& vertices = vertices_[static_cast<size_t>(ObjectFlag::eGrid)];

		vertices.push_back({ { -state_.gridDimension * state_.gridSpacing.x, 0.0f, 0.0f }, state_.gridColor });
		vertices.push_back({ { state_.gridDimension * state_.gridSpacing.x, 0.0f, 0.0f }, state_.gridColor });

		vertices.push_back({ { 0.0f, -state_.gridDimension * state_.gridSpacing.y, 0.0f }, state_.gridColor });
		vertices.push_back({ { 0.0f, state_.gridDimension * state_.gridSpacing.y, 0.0f }, state_.gridColor });

		for (uint32_t idx{ 1 }; idx < static_cast<uint32_t>(state_.gridDimension); idx++)
		{
			vertices.push_back({ { -state_.gridDimension * state_.gridSpacing.x, -state_.gridSpacing.y * idx, 0.0f}, state_.gridColor });
			vertices.push_back({ { state_.gridDimension * state_.gridSpacing.x,	-state_.gridSpacing.y * idx, 0.0f}, state_.gridColor });

			vertices.push_back({ { -state_.gridDimension * state_.gridSpacing.x, state_.gridSpacing.y * idx, 0.0f}, state_.gridColor });
			vertices.push_back({ { state_.gridDimension * state_.gridSpacing.x,	state_.gridSpacing.y * idx, 0.0f}, state_.gridColor });

			vertices.push_back({ { -state_.gridSpacing.x * idx,	-state_.gridDimension * state_.gridSpacing.y, 0.0f}, state_.gridColor });
			vertices.push_back({ { -state_.gridSpacing.x * idx, state_.gridDimension * state_.gridSpacing.y, 0.0f}, state_.gridColor });

			vertices.push_back({ { state_.gridSpacing.x * idx, -state_.gridDimension * state_.gridSpacing.y, 0.0f}, state_.gridColor });
			vertices.push_back({ { state_.gridSpacing.x * idx, state_.gridDimension * state_.gridSpacing.y, 0.0f}, state_.gridColor });
		}

		auto& vertexIndices = vertexIndices_[static_cast<size_t>(ObjectFlag::eGrid)];

		for (uint32_t idx{}; idx < vertices.size(); idx++)
		{
			vertexIndices.push_back(idx);
		}
	}

	void VulkanSystem::initCrosshair3D_()
	{
		auto& vertices = vertices_[static_cast<size_t>(ObjectFlag::eCrosshair)];
		vertices.resize(6);

		vertices =
		{
			{ {-state_.crosshairAimSize, 0.00f, 0.00f}, state_.crosschairColors[0] },
			{ {state_.crosshairAimSize, 0.00f, 0.00f}, state_.crosschairColors[0] },
			{ {0.00f, -state_.crosshairAimSize, 0.00f}, state_.crosschairColors[1] },
			{ {0.00f, state_.crosshairAimSize, 0.00f}, state_.crosschairColors[1] },
			{ {0.00f, 0.00f, -state_.crosshairAimSize}, state_.crosschairColors[2] },
			{ {0.00f, 0.00f, state_.crosshairAimSize}, state_.crosschairColors[2] }
		};

		auto& vertexIndices = vertexIndices_[static_cast<size_t>(ObjectFlag::eCrosshair)];

		for (uint32_t idx{}; idx < vertices.size(); idx++)
		{
			vertexIndices.push_back(idx);
		}
	}

	void VulkanSystem::updateWindowUniformModelTranformations_()
	{
		uint32_t objectIdx{};
		for (auto& objectUniformTranformations : uniformTranformations_)
		{
			uint32_t uniformTranformationIdx{};
			for (auto& uniformTranformation : objectUniformTranformations)
			{
				state_.windowModelScale.x = 2 / state_.windowViewport.width;
				state_.windowModelScale.y = 2 / state_.windowViewport.height;

				uniformTranformation.model = glm::scale(glm::mat4(1.0f), glm::vec3(state_.windowModelScale.x, state_.windowModelScale.y, 1.0f));
				uniformTranformation.view = glm::mat4(1.0f);
				uniformTranformation.proj = glm::mat4(1.0f);

				auto& pMappedUniformBufferMemorySection = pMappedUniformBufferMemorySections_[objectIdx][uniformTranformationIdx];

				memcpy(pMappedUniformBufferMemorySection, &uniformTranformation, sizeof(geometry::UniformTransformation));
				uniformTranformationIdx++;
			}
			objectIdx++;
		}
	}

	void VulkanSystem::updateCrosshairUniformModelTranformations_()
	{
		auto& scale = state_.windowModelScale;
		auto translate = glm::vec2{ state_.crosshairPosition.x * scale.x, state_.crosshairPosition.y * scale.y };

		auto& uniformTranformation = uniformTranformations_[static_cast<size_t>(ObjectFlag::eCrosshair)][frameInFlightIdx_];
		auto& pMappedUniformBufferMemorySection = pMappedUniformBufferMemorySections_[static_cast<size_t>(ObjectFlag::eCrosshair)][frameInFlightIdx_];

		uniformTranformation.model = glm::translate(glm::mat4(1.0f), glm::vec3(translate.x, translate.y, 0.0f));
		uniformTranformation.model = glm::scale(uniformTranformation.model, glm::vec3(scale.x, scale.y, 1.0f));

		memcpy(pMappedUniformBufferMemorySection, &uniformTranformation, sizeof(geometry::UniformTransformation));
	}

	void VulkanSystem::createVertexBuffer_(ObjectFlag objectFlag)
	{
		auto& vertices = vertices_[static_cast<size_t>(objectFlag)];
		auto& uhVertexStagingBuffer = uhVertexStagingBuffers_[static_cast<size_t>(objectFlag)];
		auto& uhVertexBuffer = uhVertexBuffers_[static_cast<size_t>(objectFlag)];

		auto bufferSize = static_cast<vk::DeviceSize>(sizeof(geometry::Point3D) * vertices.size());

		uhVertexStagingBuffer = createBufferMemoryPair_(bufferSize, vk::BufferUsageFlagBits::eTransferSrc,
			vk::SharingMode::eConcurrent, { transferQueueFamilyIdx_.value(), graphicsAndPresentQueueFamilyIdx_.value() },
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

		auto mappedVertexBufferMemory = uhDevice_->mapMemory(uhVertexStagingBuffer.second.get(), 0, bufferSize);
		memcpy(mappedVertexBufferMemory, vertices.data(), bufferSize);
		uhDevice_->unmapMemory(uhVertexStagingBuffer.second.get());

		uhVertexBuffer = createBufferMemoryPair_(bufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
			vk::SharingMode::eExclusive, { graphicsAndPresentQueueFamilyIdx_.value() }, vk::MemoryPropertyFlagBits::eDeviceLocal);

		copyBuffer_(numFramesInFlight, uhVertexStagingBuffer.first.get(), uhVertexBuffer.first.get(), bufferSize);
	}

	void VulkanSystem::createIndexBuffer_(ObjectFlag objectFlag)
	{
		auto& vertexIndices = vertexIndices_[static_cast<size_t>(objectFlag)];
		auto& uhIndexStagingBuffer = uhIndexStagingBuffers_[static_cast<size_t>(objectFlag)];
		auto& uhIndexBuffer = uhIndexBuffers_[static_cast<size_t>(objectFlag)];

		auto bufferSize = static_cast<vk::DeviceSize>(sizeof(uint16_t) * vertexIndices.size());

		uhIndexStagingBuffer = createBufferMemoryPair_(bufferSize, vk::BufferUsageFlagBits::eTransferSrc,
			vk::SharingMode::eConcurrent, { transferQueueFamilyIdx_.value(), graphicsAndPresentQueueFamilyIdx_.value() },
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

		auto mappedVertexBufferMemory = uhDevice_->mapMemory(uhIndexStagingBuffer.second.get(), 0, bufferSize);
		memcpy(mappedVertexBufferMemory, vertexIndices.data(), bufferSize);
		uhDevice_->unmapMemory(uhIndexStagingBuffer.second.get());

		uhIndexBuffer = createBufferMemoryPair_(bufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
			vk::SharingMode::eExclusive, { graphicsAndPresentQueueFamilyIdx_.value() }, vk::MemoryPropertyFlagBits::eDeviceLocal);

		copyBuffer_(numFramesInFlight + 1, uhIndexStagingBuffer.first.get(), uhIndexBuffer.first.get(), bufferSize);
	}

	void VulkanSystem::createUniformBuffers_(ObjectFlag objectFlag)
	{
		auto& uhUniformBuffers = uhUniformBuffers_[static_cast<size_t>(objectFlag)];

		auto bufferSize = static_cast<vk::DeviceSize>(sizeof(geometry::UniformTransformation));

		uint32_t uniformBufferIdx{};
		for (auto& uhUniformBuffer : uhUniformBuffers)
		{
			auto& pMappedUniformBufferMemorySection = pMappedUniformBufferMemorySections_[static_cast<size_t>(objectFlag)][uniformBufferIdx];

			uhUniformBuffer = createBufferMemoryPair_(bufferSize, vk::BufferUsageFlagBits::eUniformBuffer,
				vk::SharingMode::eExclusive, { graphicsAndPresentQueueFamilyIdx_.value() },
				vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

			pMappedUniformBufferMemorySection = uhDevice_->mapMemory(uhUniformBuffer.second.get(), 0, bufferSize);

			uniformBufferIdx++;
		}
	}

	void VulkanSystem::allocateDescriptorSets_(ObjectFlag objectFlag)
	{
		auto& uhDescriptorSets = uhDescriptorSets_[static_cast<size_t>(objectFlag)];
		auto& uhUniformBuffers = uhUniformBuffers_[static_cast<size_t>(objectFlag)];

		std::vector<vk::DescriptorSetLayout> hDescriptorSetLayouts(numFramesInFlight, uhDescriptorSetLayout_.get());

		auto sDescriptorSetAllocateInfo = vk::DescriptorSetAllocateInfo{}
			.setDescriptorPool(uhDescriptorPool_.get())
			.setDescriptorSetCount(static_cast<uint32_t>(numFramesInFlight))
			.setSetLayouts(hDescriptorSetLayouts);

		if (!(uhDescriptorSets = uhDevice_->allocateDescriptorSetsUnique(sDescriptorSetAllocateInfo)).size())
		{
			throw wexception(L"VulkanSystem::allocateDescriptorSets_() : Невозможно выделить наборы дескрипторов (DescriptorSets)");
		}
		postLogMessage_(LogLevelFlagBits::eAppInfo, L"VulkanSystem::allocateDescriptorSets_() : Наборы дескрипторов (DescriptorSets) выделены");

		uint32_t descriptorSetIdx{};
		for (auto& uhDescriptorSet : uhDescriptorSets)
		{
			auto sDescriptorBufferInfo = vk::DescriptorBufferInfo{}
				.setBuffer(uhUniformBuffers[descriptorSetIdx].first.get())
				.setOffset(0)
				.setRange(static_cast<vk::DeviceSize>(sizeof(geometry::UniformTransformation)));

			auto sWriteDescriptorSet = vk::WriteDescriptorSet{}
				.setDstSet(uhDescriptorSet.get())
				.setDstBinding(0)
				.setDstArrayElement(0)
				.setDescriptorType(vk::DescriptorType::eUniformBuffer)
				.setDescriptorCount(1)
				.setBufferInfo(sDescriptorBufferInfo);

			uhDevice_->updateDescriptorSets(sWriteDescriptorSet, nullptr);

			descriptorSetIdx++;
		}
	}



	void VulkanSystem::recordGraphicsCommandBuffer_()
	{
		auto& hCommandBuffer = hGraphicsCommandBuffers_[frameInFlightIdx_];
		auto& hFrameBuffer = uhFramebuffers_[swapchainImageIdx_].get();

		vk::ClearValue clearValue;
		clearValue.setColor(state_.windowBackgroundColor);

		auto sRenderPassBeginInfo = vk::RenderPassBeginInfo{}
			.setRenderPass(uhRenderPass_.get())
			.setFramebuffer(hFrameBuffer)
			.setRenderArea(state_.windowScissor)
			.setClearValues(clearValue);

		hCommandBuffer.reset();
		hCommandBuffer.begin(vk::CommandBufferBeginInfo{});
		hCommandBuffer.beginRenderPass(sRenderPassBeginInfo, vk::SubpassContents::eInline);
		hCommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, uhGraphicsPipeline_.get());
		hCommandBuffer.setViewport(0, 1, &state_.windowViewport);
		hCommandBuffer.setScissor(0, 1, &state_.windowScissor);

		for (uint32_t objectIdx{}; objectIdx < numObjects; objectIdx++)
		{
			auto& uhVertexBuffer = uhVertexBuffers_[objectIdx];
			auto& uhIndexBuffer = uhIndexBuffers_[objectIdx];
			auto& vertexIndices = vertexIndices_[objectIdx];
			auto& hDescriptorSet = uhDescriptorSets_[objectIdx][frameInFlightIdx_].get();

			hCommandBuffer.bindVertexBuffers(0, uhVertexBuffer.first.get(), { 0 });
			hCommandBuffer.bindIndexBuffer(uhIndexBuffer.first.get(), 0, vk::IndexType::eUint16);
			hCommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, uhGraphicsPipelineLayout_.get(), 0, hDescriptorSet, nullptr);
			hCommandBuffer.drawIndexed(static_cast<uint32_t>(vertexIndices.size()), 1, 0, 0, 0);
		}

		hCommandBuffer.endRenderPass();
		hCommandBuffer.end();
	}

	void VulkanSystem::drawPaused_()
	{
	}

	void VulkanSystem::drawStarted_()
	{
		auto result = uhDevice_->waitForFences(uhInFlightFences_[frameInFlightIdx_].get(), vk::True, UINT64_MAX);
		if (result != vk::Result::eSuccess)
		{
			throw wexception(L"VulkanSystem::draw() : Ошибка выполнения waitForFences");
		}

		swapchainImageIdx_ = uhDevice_->acquireNextImageKHR(uhSwapchain_.get(),
			UINT64_MAX, uhReadyForRenderingSemaphores_[frameInFlightIdx_].get(), VK_NULL_HANDLE).value;

		updateCrosshairUniformModelTranformations_();
		recordGraphicsCommandBuffer_();

		vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };

		auto sSubmitInfo = vk::SubmitInfo{}
			.setCommandBuffers(hGraphicsCommandBuffers_[frameInFlightIdx_])
			.setWaitSemaphores(uhReadyForRenderingSemaphores_[frameInFlightIdx_].get())
			.setSignalSemaphores(uhReadyForPresentingSemaphores_[frameInFlightIdx_].get())
			.setPWaitDstStageMask(waitStages);

		uhDevice_->resetFences(uhInFlightFences_[frameInFlightIdx_].get());
		hGraphicsQueue_.submit(sSubmitInfo, uhInFlightFences_[frameInFlightIdx_].get());

		auto sPresentInfo = vk::PresentInfoKHR{}
			.setSwapchains(uhSwapchain_.get())
			.setImageIndices(swapchainImageIdx_)
			.setWaitSemaphores(uhReadyForPresentingSemaphores_[frameInFlightIdx_].get());

		result = hGraphicsQueue_.presentKHR(sPresentInfo);
		if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR)
		{
			throw wexception(L"VulkanSystem::draw() : Ошибка выполнения presentKHR");
		}

		frameInFlightIdx_ = (frameInFlightIdx_ + 1) % numFramesInFlight;
	}
}