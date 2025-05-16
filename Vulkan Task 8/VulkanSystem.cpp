#include "VulkanSystem.h"
VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE


namespace ak
{
	constexpr int maxFramesInFlight{ 2 };

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
		createCommandPool_();
		allocateCommandBuffers_();
		createSyncObjects_();
	}

	void VulkanSystem::draw()
	{
		auto result = uhDevice_->waitForFences(uhInFlightFences_[frameInFlightIdx_].get(), vk::True, UINT64_MAX);
		if (result != vk::Result::eSuccess)
		{
			throw wexception(L"VulkanSystem::draw() : Ошибка выполнения waitForFences");
		}
		uhDevice_->resetFences(uhInFlightFences_[frameInFlightIdx_].get());

		swapchainImageIdx_ = uhDevice_->acquireNextImageKHR(uhSwapchain_.get(),
			UINT64_MAX, uhReadyForRenderingSemaphores_[frameInFlightIdx_].get(), VK_NULL_HANDLE).value;

		hCommandBuffers_[frameInFlightIdx_].reset();
		recordCommandBuffer_(hCommandBuffers_[frameInFlightIdx_], uhFramebuffers_[swapchainImageIdx_].get());

		vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };

		auto sSubmitInfo = vk::SubmitInfo{}
			.setCommandBuffers(hCommandBuffers_[frameInFlightIdx_])
			.setWaitSemaphores(uhReadyForRenderingSemaphores_[frameInFlightIdx_].get())
			.setSignalSemaphores(uhReadyForPresentingSemaphores_[frameInFlightIdx_].get())
			.setPWaitDstStageMask(waitStages);

		hQueue_.submit(sSubmitInfo, uhInFlightFences_[frameInFlightIdx_].get());

		auto sPresentInfo = vk::PresentInfoKHR{}
			.setSwapchains(uhSwapchain_.get())
			.setImageIndices(swapchainImageIdx_)
			.setWaitSemaphores(uhReadyForPresentingSemaphores_[frameInFlightIdx_].get());

		result = hQueue_.presentKHR(sPresentInfo);
		if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR)
		{
			throw wexception(L"VulkanSystem::draw() : Ошибка выполнения presentKHR");
		}

		frameInFlightIdx_ = (frameInFlightIdx_ + 1) % maxFramesInFlight;
	}

	void VulkanSystem::waitIdle()
	{
		uhDevice_->waitIdle();
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
			.setApiVersion(vk::makeApiVersion(0, 1, 4, 0));

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
			throw wexception(L"VulkanSystem::createInstance_() : Невозможно создать экземпляр (Instance)");
		}
		vk::detail::defaultDispatchLoaderDynamic.init(uhInstance_.get());
		postLogMessage_(LogLevelFlag::eAppInfo, L"VulkanSystem::createInstance_() : Экземпляр (Instance) создан");

#ifdef _DEBUG
		if (!(uhDebugUtilsMessenger_ = uhInstance_->createDebugUtilsMessengerEXTUnique(
			sDebugUtilsMessengerCreateInfo, nullptr)))
		{
			throw wexception(L"VulkanSystem::createInstance_() : Невозможно создать обработчик диагностических сообщений");
		}
		postLogMessage_(LogLevelFlag::eAppInfo, L"VulkanSystem::createInstance_() : Обработчик диагностических сообщений создан");
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
		postLogMessage_(LogLevelFlag::eAppInfo, L"VulkanSystem::createSurface_() : Поверхность Win32 (Win32Surface) создана");
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

			postLogMessage_(LogLevelFlag::eAppInfo, std::string("VulkanSystem::pickPhysicalDevice_() : GPU: ") +
				std::string(sPhysicalDeviceProperties_.deviceName.data()) + std::string("\r\n"));
			postLogMessage_(LogLevelFlag::eAppInfo, std::string("VulkanSystem::pickPhysicalDevice_() : Семейство очередей GPU: ") +
				std::to_string(graphicsAndPresentQueueFamilyIdx_.value()) + std::string("\r\n"));
		}
		else
		{
			throw wexception(L"VulkanSystem::pickPhysicalDevice_() : GPU с заданными критериями отсутствует");
		}
	}

	void VulkanSystem::createLogicalDevice_()
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
			throw wexception(L"VulkanSystem::createLogicalDevice_() : Невозможно создать логическое устройство (Device)");
		}
		postLogMessage_(LogLevelFlag::eAppInfo, L"VulkanSystem::createLogicalDevice_() : Логическое устройство (Device) создано");

		hQueue_ = uhDevice_->getQueue(graphicsAndPresentQueueFamilyIdx_.value(), 0);
	}

	void VulkanSystem::createSwapchain_()
	{
		swapchainImageExtent_ = sPhysicalDeviceSurfaceCapabilities_.currentExtent;

		swapchainViewport_ = vk::Viewport{}
			.setX(0.f)
			.setY(0.f)
			.setWidth(static_cast<float>(swapchainImageExtent_.width))
			.setHeight(static_cast<float>(swapchainImageExtent_.height))
			.setMinDepth(0.f)
			.setMaxDepth(1.f);

		swapchainScissor_ = vk::Rect2D{ { 0, 0 }, swapchainImageExtent_ };

		swapchainClearValue_.setColor((std::array<float, 4>{ { 0.05f, 0.05f, 0.05f, 1.0f } }));
		
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
		postLogMessage_(LogLevelFlag::eAppInfo, L"VulkanSystem::createSwapchain_() : Цепочка показа (Swapchain) создана");

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
		postLogMessage_(LogLevelFlag::eAppInfo, L"VulkanSystem::createImageViews_() : Виды изображений (ImageViews) созданы");
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
		postLogMessage_(LogLevelFlag::eAppInfo, L"VulkanSystem::createShaderModules_() : Шейдерные модули (ShaderModules) созданы");
	}

	void VulkanSystem::createGraphicsPipelineLayout_()
	{
		auto sGraphicsPipelineLayoutCreateInfo = vk::PipelineLayoutCreateInfo{};

		if (!(uhGraphicsPipelineLayout_ = uhDevice_->createPipelineLayoutUnique(sGraphicsPipelineLayoutCreateInfo)))
		{
			throw wexception(L"VulkanSystem::createGraphicsPipelineLayout_() : Невозможно создать макет графического конвейера (GraphicsPipelineLayout)");
		}
		postLogMessage_(LogLevelFlag::eAppInfo, L"VulkanSystem::createGraphicsPipelineLayout_() : Макет графического конвейера (GraphicsPipelineLayout) создан");
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
		postLogMessage_(LogLevelFlag::eAppInfo, L"VulkanSystem::createRenderPass_() : Проход рендеринга (RenderPass) создан");
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
		auto sGraphicsPipelineVertexInputStateCreateInfo = vk::PipelineVertexInputStateCreateInfo{};

		// 3. Стадия - Входная сборка
		auto sGraphicsPipelineInputAssemblyStateCreateInfo = vk::PipelineInputAssemblyStateCreateInfo{}
		.setTopology(vk::PrimitiveTopology::eTriangleList);

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
		postLogMessage_(LogLevelFlag::eAppInfo, L"VulkanSystem::createGraphicsPipeline_() : Графический конвейер (GraphicsPipeline) создан");
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
		postLogMessage_(LogLevelFlag::eAppInfo, L"VulkanSystem::createFramebuffers_() : Буферы кадра (Framebuffers) созданы");
	}

	void VulkanSystem::createCommandPool_()
	{
		auto sCommandPoolCreateInfo = vk::CommandPoolCreateInfo{}
			.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
			.setQueueFamilyIndex(graphicsAndPresentQueueFamilyIdx_.value());

		if (!(uhCommandPool_ = uhDevice_->createCommandPoolUnique(sCommandPoolCreateInfo)))
		{
			throw wexception(L"VulkanSystem::createCommandPool_() : Невозможно создать командный пул (CommandPool)");
		}
		postLogMessage_(LogLevelFlag::eAppInfo, L"VulkanSystem::createCommandPool_() : Командный пул (CommandPool) создан");
	}

	void VulkanSystem::allocateCommandBuffers_()
	{
		auto sCommandBufferAllocateInfo = vk::CommandBufferAllocateInfo{}
			. setCommandPool(*uhCommandPool_)
			.setLevel(vk::CommandBufferLevel::ePrimary)
			.setCommandBufferCount(maxFramesInFlight);

		if (!(hCommandBuffers_ = uhDevice_->allocateCommandBuffers(sCommandBufferAllocateInfo)).size())
		{
			throw wexception(L"VulkanSystem::allocateCommandBuffers_() : Невозможно выделить командные буферы (CommandBuffers)");
		}
		postLogMessage_(LogLevelFlag::eAppInfo, L"VulkanSystem::allocateCommandBuffers_() : Командные буферы (CommandBuffers) выделены");
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

		postLogMessage_(LogLevelFlag::eAppInfo, L"VulkanSystem::createSyncObjects_() : Объекты синхронизации созданы");
	}

	void VulkanSystem::recordCommandBuffer_(const vk::CommandBuffer& hCommandBuffer, const vk::Framebuffer& hFrameBuffer)
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
		hCommandBuffer.draw(3, 1, 0, 0);
		hCommandBuffer.endRenderPass();
		hCommandBuffer.end();
	}
}