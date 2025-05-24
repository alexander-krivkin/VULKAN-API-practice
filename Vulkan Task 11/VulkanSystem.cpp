#include "VulkanSystem.h"
VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE


namespace ak
{
	constexpr int maxFramesInFlight{ 2 };


	// ПОКА ТАК
	graphics::Polyline crosshair
	{
		{
			{-0.01f, -0.01f},
			{0.01f, -0.01f},
			{0.01f, 0.01f},
			{-0.01f, 0.01f},

			{0.0f, -0.01f},	{0.0f, -0.25f},
			{0.01f, -0.0f},	{0.25f, -0.0f},
			{0.0f, 0.01f},	{0.0f, 0.25f},
			{-0.01f, 0.0f},	{-0.25f, 0.0f}
		},

		{0.85f, 0.05f, 0.05f, 0.0f}
	};

	std::vector<graphics::Point> sceneVertices{};

	const std::vector<uint16_t> sceneVertexIndices
	{
		0, 1, 1, 2, 2, 3, 3, 0, 4, 5, 6, 7, 8, 9, 10, 11
	};




	std::vector<std::uint32_t> loadShaderFile(const std::string& filename)
	{
		std::ifstream file{ filename, std::ios::ate | std::ios::binary };
		if (!file.is_open())
		{
			throw wexception(L"loadShaderFile() : Невозможно открыть файл");
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
			pVulkanSystem->postLogMessage_(LogLevelFlag::eVulkanVerbose, message);
			return vk::True;
		}
		else if (messageSeverity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo)
		{
			pVulkanSystem->postLogMessage_(LogLevelFlag::eVulkanInfo, message);
			return vk::True;
		}
		else if (messageSeverity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning)
		{
			pVulkanSystem->postLogMessage_(LogLevelFlag::eVulkanWarning, message);
			return vk::False;
		}
		else if (messageSeverity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eError)
		{
			pVulkanSystem->postLogMessage_(LogLevelFlag::eVulkanError, message);
			return vk::False;
		}

		return vk::True;
	}
#endif // _DEBUG

	void VulkanSystem::postLogMessage_(LogLevelFlag logLevel, const char* message)
	{
#ifdef _DEBUG
		shpLoggingSystem_->postLogMessage(logLevel, message);
#else
		UNREFERENCED_PARAMETER(logLevel);
		UNREFERENCED_PARAMETER(message);
#endif // _DEBUG
	}

	void VulkanSystem::postLogMessage_(LogLevelFlag logLevel, const std::string& message)
	{
#ifdef _DEBUG
		shpLoggingSystem_->postLogMessage(logLevel, message);
#else
		UNREFERENCED_PARAMETER(logLevel);
		UNREFERENCED_PARAMETER(message);
#endif // _DEBUG
	}

	void VulkanSystem::postLogMessage_(LogLevelFlag logLevel, const std::wstring& message)
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
		createGraphicsPipelineLayout_();
		createRenderPass_();
		createGraphicsPipeline_();
		createFramebuffers_();
		createSyncObjects_();
		createCommandPools_();
		allocateCommandBuffers_();
		createVertexBuffer_();
		createIndexBuffer_();

		state_ = VulkanSystemFlag::eReadyAndStarted;
		draw = std::bind(&VulkanSystem::drawStarted_, this);
	}

	void VulkanSystem::resize()
	{
		if (state_ != VulkanSystemFlag::eNotReady)
		{
			uhDevice_->waitIdle();

			createSwapchain_();
			createImageViews_();
			createFramebuffers_();
		}
	}

	void VulkanSystem::pause()
	{
		if (state_ == VulkanSystemFlag::eReadyAndStarted)
		{
			state_ = VulkanSystemFlag::eReadyAndPaused;
			draw = std::bind(&VulkanSystem::drawPaused_, this);
		}
	}

	void VulkanSystem::resume()
	{
		if (state_ == VulkanSystemFlag::eReadyAndPaused)
		{
			state_ = VulkanSystemFlag::eReadyAndStarted;
			draw = std::bind(&VulkanSystem::drawStarted_, this);
		}
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
		auto sDebugUtilsMessengerCreateInfo = vk::DebugUtilsMessengerCreateInfoEXT{}
			.setMessageSeverity(
				//vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
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
			throw wexception(L"VulkanSystem::createInstance_() : Невозможно создать экземпляр (Instance)");
		}
		vk::detail::defaultDispatchLoaderDynamic.init(uhInstance_.get());
		// postLogMessage_(LogLevelFlag::eAppInfo, L"VulkanSystem::createInstance_() : Экземпляр (Instance) создан");

#ifdef _DEBUG
		if (!(uhDebugUtilsMessenger_ = uhInstance_->createDebugUtilsMessengerEXTUnique(
			sDebugUtilsMessengerCreateInfo, nullptr)))
		{
			throw wexception(L"VulkanSystem::createInstance_() : Невозможно создать обработчик диагностических сообщений");
		}
		// postLogMessage_(LogLevelFlag::eAppInfo, L"VulkanSystem::createInstance_() : Обработчик диагностических сообщений создан");
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
		// postLogMessage_(LogLevelFlag::eAppInfo, L"VulkanSystem::createSurface_() : Поверхность Win32 (Win32Surface) создана");
	}

	void VulkanSystem::pickPhysicalDevice_()
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
					vk::SurfaceFormatKHR(swapchainImageFormat_, vk::ColorSpaceKHR::eSrgbNonlinear))
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

			// postLogMessage_(LogLevelFlag::eAppInfo, std::string("VulkanSystem::pickPhysicalDevice_() : GPU: ") +
			// 	std::string(sPhysicalDeviceProperties_.deviceName.data()) + std::string("\r\n"));
			// postLogMessage_(LogLevelFlag::eAppInfo, std::string("VulkanSystem::pickPhysicalDevice_() : Семейство очередей GPU: ") +
			// 	std::to_string(graphicsAndPresentQueueFamilyIdx_.value()) + std::string("\r\n"));
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
		// postLogMessage_(LogLevelFlag::eAppInfo, L"VulkanSystem::createLogicalDevice_() : Логическое устройство (Device) создано");

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
		swapchainImageExtent_ = sPhysicalDeviceSurfaceCapabilities_.currentExtent;

		swapchainViewport_ = vk::Viewport{}
			.setX(0.f)
			.setY(0.f)
			.setWidth(static_cast<float>(swapchainImageExtent_.width))
			.setHeight(static_cast<float>(swapchainImageExtent_.height))
			.setMinDepth(0.f)
			.setMaxDepth(1.f);

		swapchainScissor_ = vk::Rect2D{ { 0, 0 }, swapchainImageExtent_ };

		swapchainClearValue_.setColor((std::array<float, 4>{ { 0.95f, 0.95f, 0.95f, 1.0f } }));
		
		auto sSwapchainCreateInfo = vk::SwapchainCreateInfoKHR{}
			.setSurface(uhSurface_.get())
			.setMinImageCount(sPhysicalDeviceSurfaceCapabilities_.minImageCount + 1)
			.setImageFormat(swapchainImageFormat_)
			.setImageColorSpace(vk::ColorSpaceKHR::eSrgbNonlinear)
			.setImageExtent(swapchainImageExtent_)
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
		// postLogMessage_(LogLevelFlag::eAppInfo, L"VulkanSystem::createSwapchain_() : Цепочка показа (Swapchain) создана");

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
				.setFormat(swapchainImageFormat_)
				.setSubresourceRange(sImageSubresourceRange);

			if (!(uhSwapchainImageView = uhDevice_->createImageViewUnique(sSwapchainImageViewCreateInfo)))
			{
				throw wexception(L"VulkanSystem::createImageViews_() : Невозможно создать вид изображения (ImageView)");
			}
			swapchainImageIdx++;
		}
		// postLogMessage_(LogLevelFlag::eAppInfo, L"VulkanSystem::createImageViews_() : Виды изображений (ImageViews) созданы");
	}

	void VulkanSystem::createShaderModules_()
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
				throw wexception(L"VulkanSystem::createShaderModules_() : Невозможно создать шейдерный модуль (ShaderModule)");
			}
			shaderModuleIdx++;
		}
		// postLogMessage_(LogLevelFlag::eAppInfo, L"VulkanSystem::createShaderModules_() : Шейдерные модули (ShaderModules) созданы");
	}

	void VulkanSystem::createGraphicsPipelineLayout_()
	{
		auto sGraphicsPipelineLayoutCreateInfo = vk::PipelineLayoutCreateInfo{};

		if (!(uhGraphicsPipelineLayout_ = uhDevice_->createPipelineLayoutUnique(sGraphicsPipelineLayoutCreateInfo)))
		{
			throw wexception(L"VulkanSystem::createGraphicsPipelineLayout_() : Невозможно создать макет графического конвейера (GraphicsPipelineLayout)");
		}
		// postLogMessage_(LogLevelFlag::eAppInfo, L"VulkanSystem::createGraphicsPipelineLayout_() : Макет графического конвейера (GraphicsPipelineLayout) создан");
	}

	void VulkanSystem::createRenderPass_()
	{
		auto sAttachmentDescription = vk::AttachmentDescription{}
			.setFormat(swapchainImageFormat_)
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
		// postLogMessage_(LogLevelFlag::eAppInfo, L"VulkanSystem::createRenderPass_() : Проход рендеринга (RenderPass) создан");
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
		auto sVertexInputBindingDescription = graphics::getBindDescription<graphics::Point>(0);
		auto sVertexInputAttributeDescriptions = graphics::getAttributeDescriptions<graphics::Point>(0);

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
			.setViewports(swapchainViewport_)
			.setScissors(swapchainScissor_);

		// 6. Стадия - Растеризация
		auto sGraphicsPipelineRasterizationStateCreateInfo = vk::PipelineRasterizationStateCreateInfo{}
			.setDepthClampEnable(vk::False)
			.setRasterizerDiscardEnable(vk::False)
			.setPolygonMode(vk::PolygonMode::eFill)
			.setLineWidth(1.f);

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
		// postLogMessage_(LogLevelFlag::eAppInfo, L"VulkanSystem::createGraphicsPipeline_() : Графический конвейер (GraphicsPipeline) создан");
	}

	void VulkanSystem::createFramebuffers_()
	{
		uhFramebuffers_.resize(hSwapchainImages_.size());
		
		int framebufferIdx{};
		for (auto& uhFramebuffer : uhFramebuffers_)
		{
			auto sFramebufferCreateInfo = vk::FramebufferCreateInfo{}
				.setRenderPass(uhRenderPass_.get())
				.setAttachments(uhSwapchainImageViews_[framebufferIdx].get())
				.setWidth(swapchainImageExtent_.width)
				.setHeight(swapchainImageExtent_.height)
				.setLayers(1);
			
			if (!(uhFramebuffer = uhDevice_->createFramebufferUnique(sFramebufferCreateInfo)))
			{
				throw wexception(L"VulkanSystem::createFramebuffers_() : Невозможно создать буфер кадра (Framebuffer)");
			}
			framebufferIdx++;
		}
		// postLogMessage_(LogLevelFlag::eAppInfo, L"VulkanSystem::createFramebuffers_() : Буферы кадра (Framebuffers) созданы");
	}

	void VulkanSystem::createSyncObjects_()
	{
		uhReadyForRenderingSemaphores_.resize(maxFramesInFlight);
		uhReadyForPresentingSemaphores_.resize(maxFramesInFlight);
		uhInFlightFences_.resize(maxFramesInFlight);

		auto sSemaphoreCreateInfo = vk::SemaphoreCreateInfo{};

		auto sFenceCreateInfo = vk::FenceCreateInfo{}
		.setFlags(vk::FenceCreateFlagBits::eSignaled);

		for (size_t idx{}; idx < maxFramesInFlight; idx++)
		{
			if (!(uhReadyForRenderingSemaphores_[idx] = uhDevice_->createSemaphoreUnique(sSemaphoreCreateInfo)))
			{
				throw wexception(L"VulkanSystem::createSyncObjects_() : Невозможно создать семафор (Semaphore)");
			}

			if (!(uhReadyForPresentingSemaphores_[idx] = uhDevice_->createSemaphoreUnique(sSemaphoreCreateInfo)))
			{
				throw wexception(L"VulkanSystem::createSyncObjects_() : Невозможно создать семафор (Semaphore)");
			}

			if (!(uhInFlightFences_[idx] = uhDevice_->createFenceUnique(sFenceCreateInfo)))
			{
				throw wexception(L"VulkanSystem::createSyncObjects_() : Невозможно создать забор (Fence)");
			}
		}
		// postLogMessage_(LogLevelFlag::eAppInfo, L"VulkanSystem::createSyncObjects_() : Объекты синхронизации созданы");
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
		// postLogMessage_(LogLevelFlag::eAppInfo, L"VulkanSystem::createCommandPool_() : Командный пул (CommandPool) для передачи создан");

		auto sCommandPoolCreateInfo = vk::CommandPoolCreateInfo{}
			.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
			.setQueueFamilyIndex(graphicsAndPresentQueueFamilyIdx_.value());

		if (!(uhGraphicsCommandPool_ = uhDevice_->createCommandPoolUnique(sCommandPoolCreateInfo)))
		{
			throw wexception(L"VulkanSystem::createCommandPools_() : Невозможно создать командный пул (CommandPool) для графики");
		}
		// postLogMessage_(LogLevelFlag::eAppInfo, L"VulkanSystem::createCommandPool_() : Командный пул (CommandPool) для графики создан");
	}

	void VulkanSystem::allocateCommandBuffers_()
	{
		auto sTransferCommandBufferAllocateInfo = vk::CommandBufferAllocateInfo{}
			. setCommandPool(*uhTransferCommandPool_)
			.setLevel(vk::CommandBufferLevel::ePrimary)
			.setCommandBufferCount(maxFramesInFlight);

		if (!(hTransferCommandBuffers_ = uhDevice_->allocateCommandBuffers(sTransferCommandBufferAllocateInfo)).size())
		{
			throw wexception(L"VulkanSystem::allocateCommandBuffers_() : Невозможно выделить командные буферы (CommandBuffers) для передачи");
		}
		// postLogMessage_(LogLevelFlag::eAppInfo, L"VulkanSystem::allocateCommandBuffers_() : Командные буферы (CommandBuffers) для передачи выделены");

		auto sGraphicsCommandBufferAllocateInfo = vk::CommandBufferAllocateInfo{}
			. setCommandPool(*uhGraphicsCommandPool_)
			.setLevel(vk::CommandBufferLevel::ePrimary)
			.setCommandBufferCount(maxFramesInFlight);

		if (!(hGraphicsCommandBuffers_ = uhDevice_->allocateCommandBuffers(sGraphicsCommandBufferAllocateInfo)).size())
		{
			throw wexception(L"VulkanSystem::allocateCommandBuffers_() : Невозможно выделить командные буферы (CommandBuffers) для графики");
		}
		// postLogMessage_(LogLevelFlag::eAppInfo, L"VulkanSystem::allocateCommandBuffers_() : Командные буферы (CommandBuffers) для графики выделены");
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
		// postLogMessage_(LogLevelFlag::eAppInfo, L"VulkanSystem::createBufferMemoryPair_ : Буфер (Buffer) создан");

		auto bufferMemoryRequirements = uhDevice_->getBufferMemoryRequirements(bufferMemoryPair.first.get());
		auto memoryIdx = findSuitableMemoryIndex(bufferMemoryRequirements.memoryTypeBits, requiredMemoryFlags);
		
		auto sMemoryAllocateInfo = vk::MemoryAllocateInfo{}
			.setAllocationSize(bufferMemoryRequirements.size)
			.setMemoryTypeIndex(memoryIdx);

		if (!(bufferMemoryPair.second = uhDevice_->allocateMemoryUnique(sMemoryAllocateInfo)))
		{
			throw wexception(L"VulkanSystem::createBufferMemoryPair_ : Невозможно выделить память буфера (BufferMemory)");
		}
		// postLogMessage_(LogLevelFlag::eAppInfo, L"VulkanSystem::createBufferMemoryPair_ : Память буфера (BufferMemory) выделена");

		uhDevice_->bindBufferMemory(bufferMemoryPair.first.get(), bufferMemoryPair.second.get(), 0u);

		return bufferMemoryPair;
	}

	void VulkanSystem::copyBuffer_(vk::Buffer& hSrcBuffer, vk::Buffer& hDstBuffer, vk::DeviceSize size)
	{
		auto sCommandBufferBeginInfo = vk::CommandBufferBeginInfo{}
			.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

			auto sBufferCopy = vk::BufferCopy{}
				.setSrcOffset(0)
				.setDstOffset(0)
				.setSize(size);

		hTransferCommandBuffers_[0].begin(sCommandBufferBeginInfo);  // TODO [0] index
		hTransferCommandBuffers_[0].copyBuffer(hSrcBuffer, hDstBuffer, sBufferCopy);
		hTransferCommandBuffers_[0].end();

		auto sSubmitInfo = vk::SubmitInfo{}
			.setCommandBuffers(hTransferCommandBuffers_[0]);

		hTransferQueue_.submit(sSubmitInfo);
		hTransferQueue_.waitIdle();					// TO DO FENCE
	}

	void VulkanSystem::createVertexBuffer_()
	{
		// Для тестирования пока так
		sceneVertices.resize(crosshair.vertices.size());
		int idx{};
		for (auto& vertex : crosshair.vertices)
		{
			sceneVertices[idx].vertex = vertex;
			sceneVertices[idx].color = crosshair.color;
			idx++;
		}
		//

		auto bufferSize = static_cast<vk::DeviceSize>(sizeof(graphics::Point) * sceneVertices.size());

		auto uhStagingBuffer = createBufferMemoryPair_(bufferSize, vk::BufferUsageFlagBits::eTransferSrc,
			vk::SharingMode::eConcurrent, { transferQueueFamilyIdx_.value(), graphicsAndPresentQueueFamilyIdx_.value() },
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

		auto mappedVertexBufferMemory = uhDevice_->mapMemory(uhStagingBuffer.second.get(), 0, bufferSize);
		memcpy(mappedVertexBufferMemory, sceneVertices.data(),	bufferSize);
		uhDevice_->unmapMemory(uhStagingBuffer.second.get());

		uhVertexBuffer_ = createBufferMemoryPair_(bufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
			vk::SharingMode::eExclusive, { graphicsAndPresentQueueFamilyIdx_.value() }, vk::MemoryPropertyFlagBits::eDeviceLocal);

		copyBuffer_(uhStagingBuffer.first.get(), uhVertexBuffer_.first.get(), bufferSize);
	}

	void VulkanSystem::createIndexBuffer_()
	{
		auto bufferSize = static_cast<vk::DeviceSize>(sizeof(uint16_t) * sceneVertexIndices.size());

		auto uhStagingBuffer = createBufferMemoryPair_(bufferSize, vk::BufferUsageFlagBits::eTransferSrc,
			vk::SharingMode::eConcurrent, { transferQueueFamilyIdx_.value(), graphicsAndPresentQueueFamilyIdx_.value() },
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

		auto mappedVertexBufferMemory = uhDevice_->mapMemory(uhStagingBuffer.second.get(), 0, bufferSize);
		memcpy(mappedVertexBufferMemory, sceneVertexIndices.data(), bufferSize);
		uhDevice_->unmapMemory(uhStagingBuffer.second.get());

		uhIndexBuffer_ = createBufferMemoryPair_(bufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
			vk::SharingMode::eExclusive, { graphicsAndPresentQueueFamilyIdx_.value() }, vk::MemoryPropertyFlagBits::eDeviceLocal);

		copyBuffer_(uhStagingBuffer.first.get(), uhIndexBuffer_.first.get(), bufferSize);
	}

	void VulkanSystem::recordGraphicsCommandBuffer_(const vk::CommandBuffer& hCommandBuffer, const vk::Framebuffer& hFrameBuffer)
	{
		auto sRenderPassBeginInfo = vk::RenderPassBeginInfo{}
			.setRenderPass(uhRenderPass_.get())
			.setFramebuffer(hFrameBuffer)
			.setRenderArea(swapchainScissor_)
			.setClearValues(swapchainClearValue_);

		hCommandBuffer.begin(vk::CommandBufferBeginInfo{});
		hCommandBuffer.beginRenderPass(sRenderPassBeginInfo, vk::SubpassContents::eInline);
		hCommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, uhGraphicsPipeline_.get());
		hCommandBuffer.setViewport(0, 1, &swapchainViewport_);
		hCommandBuffer.setScissor(0, 1, &swapchainScissor_);
		hCommandBuffer.bindVertexBuffers(0, uhVertexBuffer_.first.get(), { 0 });
		hCommandBuffer.bindIndexBuffer(uhIndexBuffer_.first.get(), 0, vk::IndexType::eUint16);
		hCommandBuffer.drawIndexed(sceneVertexIndices.size(), 1, 0, 0, 0);
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

		hGraphicsCommandBuffers_[frameInFlightIdx_].reset();
		recordGraphicsCommandBuffer_(hGraphicsCommandBuffers_[frameInFlightIdx_], uhFramebuffers_[swapchainImageIdx_].get());

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

		frameInFlightIdx_ = (frameInFlightIdx_ + 1) % maxFramesInFlight;
	}	
}