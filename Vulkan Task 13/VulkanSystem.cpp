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
#endif // _DEBUG
#ifndef _DEBUG
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
		createSwapchainImageViews_();
		createDepthImage_();
		createDepthImageView_();
		createShaderModules_();
		createDescriptorSetLayout_();
		createGraphicsPipelineLayout_();
		createRenderPass_();
		createGraphicsPipelines_();
		createFramebuffers_();
		createSyncObjects_();
		createCommandPools_();
		allocateCommandBuffers_();
		createDescriptorPool_();

		initGrid_();
		initCrosshair_();

		//geometry::Tetrahedron tetrahedron1(
		//	{ 0.0f, 0.0f, 0.0f },
		//	{ glm::vec3{ 300.0f, -150.0f, -200.0f }, glm::vec3{ -300.0f, -150.0f, -200.0f },
		//		glm::vec3{ 0.0f, 300.0f, -200.0f }, glm::vec3{ 0.0f, 0.0f, 200.0f } },
		//	worldCS_.getRotatedX(glm::radians(0.0f)).getRotatedY(glm::radians(0.0f)),
		//	{ glm::vec4{ 0.80f, 0.10f, 0.80f, 1.0f },
		//	glm::vec4{ 0.10f, 0.10f, 0.10f, 1.0f },
		//	glm::vec4{ 0.30f, 0.90f, 0.10f, 1.0f } });

		//geometry::Circle2D circle2D1(
		//	{ 600.0f, 150.0f, 100.0f }, 200.0f,
		//	worldCS_.getRotatedX(glm::radians(15.0f)).getRotatedY(glm::radians(15.0f)), numFragments);

		std::array<std::unique_ptr<geometry::Sphere>, 21> upSpheres;

		for (auto& upSphere : upSpheres)
		{
			std::random_device rd;
			std::mt19937 gen(rd());
			std::uniform_int_distribution<> distr(0, 5000);

			float x = FLOAT(distr(gen) % 5000) - 2500.0f;
			float y = FLOAT(distr(gen) % 5000) - 2500.0f;
			float z = FLOAT(distr(gen) % 5000) - 2500.0f;
			float radius = FLOAT(distr(gen) % 500) + 100.0f;
			float r = FLOAT(distr(gen) % 100) / 100.0f;
			float g = FLOAT(distr(gen) % 100) / 100.0f;
			float b = FLOAT(distr(gen) % 100) / 100.0f;

			upSphere = std::make_unique<geometry::Sphere>
				(geometry::Sphere(
					glm::vec3{ x, y, z }, radius,
				worldCS_, numFragments,
				{ glm::vec4{ 0.60f, 0.20f, 0.20f, 1.0f },
				glm::vec4{ 0.80f, 0.80f, 0.20f, 1.0f },
				glm::vec4{ r, g, b, 1.0f } }));

			addPrimitiveToScene_(TopologyTypeFlag::eTriangles, *upSphere.get());
			addPrimitiveToScene_(TopologyTypeFlag::eLines, *upSphere.get());
			addPrimitiveToScene_(TopologyTypeFlag::ePoints, *upSphere.get());
		}

		//addPrimitiveToScene_(TopologyTypeFlag::eTriangles, tetrahedron1);
		//addPrimitiveToScene_(TopologyTypeFlag::eLines, tetrahedron1);
		//addPrimitiveToScene_(TopologyTypeFlag::ePoints, tetrahedron1);

		//addPrimitiveToScene_(TopologyTypeFlag::eLines, circle2D1);
		//addPrimitiveToScene_(TopologyTypeFlag::ePoints, circle2D1);



		for (uint32_t objectIdx{}; objectIdx < numObjects; objectIdx++)
		{
			for (uint32_t topologyTypeIdx{}; topologyTypeIdx < numObjects; topologyTypeIdx++)
			{
				createVertexBuffer_(static_cast<ObjectFlag>(objectIdx), static_cast<TopologyTypeFlag>(topologyTypeIdx));
				createIndexBuffer_(static_cast<ObjectFlag>(objectIdx), static_cast<TopologyTypeFlag>(topologyTypeIdx));
			}

			createUniformBuffers_(static_cast<ObjectFlag>(objectIdx));
			allocateDescriptorSets_(static_cast<ObjectFlag>(objectIdx));
		}

		updateWindowSizeTransform_();
		updateWindowTransformUniformBuffer_(ObjectFlag::eGrid);
		updateWindowTransformUniformBuffer_(ObjectFlag::eScene);
		updateWindowTransformUniformBuffer_(ObjectFlag::eCrosshair);

		state_.readiness_ = VulkanSystemFlag::eReadyAndStarted;
		draw = std::bind(&VulkanSystem::drawStarted_, this);
	}

	void VulkanSystem::resize()
	{
		if (state_.readiness_ != VulkanSystemFlag::eNotReady)
		{
			uhDevice_->waitIdle();

			createSwapchain_();
			createSwapchainImageViews_();
			createDepthImage_();
			createDepthImageView_();
			createFramebuffers_();

			updateWindowSizeTransform_();
			updateWindowTransformUniformBuffer_(ObjectFlag::eGrid);
			updateWindowTransformUniformBuffer_(ObjectFlag::eScene);
			updateWindowTransformUniformBuffer_(ObjectFlag::eCrosshair);
		}
	}

	void VulkanSystem::pause()
	{
		if (state_.readiness_ == VulkanSystemFlag::eReadyAndStarted)
		{
			state_.readiness_ = VulkanSystemFlag::eReadyAndPaused;
			draw = std::bind(&VulkanSystem::drawPaused_, this);
		}
	}

	void VulkanSystem::resume()
	{
		if (state_.readiness_ == VulkanSystemFlag::eReadyAndPaused)
		{
			state_.readiness_ = VulkanSystemFlag::eReadyAndStarted;
			draw = std::bind(&VulkanSystem::drawStarted_, this);
		}
	}

	void VulkanSystem::updateMouseCursorPosition(POINT cursorPosition)
	{
		float rightFactor = state_.crosshairPosition_.x - FLOAT(cursorPosition.x);
		float upFactor = state_.crosshairPosition_.y - FLOAT(cursorPosition.y);
		state_.crosshairPosition_.x = FLOAT(cursorPosition.x);
		state_.crosshairPosition_.y = FLOAT(cursorPosition.y);

		if (state_.keyboardMouseFlags_ & (KeyboardMouseFlagBits::eKeyLeftCtrl ^ KeyboardMouseFlagBits::eMouseLeft))
		{
			updateWindowViewCameraRotateEyeTransform_(rightFactor, upFactor);
			updateWindowTransformUniformBuffer_(ObjectFlag::eGrid);
			updateWindowTransformUniformBuffer_(ObjectFlag::eScene);
			return;
		}

		if (state_.keyboardMouseFlags_ & (KeyboardMouseFlagBits::eKeyLeftCtrl ^ KeyboardMouseFlagBits::eMouseMiddle))
		{
			updateWindowViewCameraRotateTargetTransform_(rightFactor, upFactor);
			updateWindowTransformUniformBuffer_(ObjectFlag::eGrid);
			updateWindowTransformUniformBuffer_(ObjectFlag::eScene);
			return;
		}

		if (state_.keyboardMouseFlags_ & KeyboardMouseFlagBits::eMouseMiddle)
		{
			updateWindowViewCameraRightAndUpTransform_(rightFactor, upFactor);
			updateWindowTransformUniformBuffer_(ObjectFlag::eGrid);
			updateWindowTransformUniformBuffer_(ObjectFlag::eScene);
			return;
		}
	}

	void VulkanSystem::updateKeyboardMouseFlags(uint32_t keyboardMouseFlags)
	{
		state_.keyboardMouseFlags_ = keyboardMouseFlags;
	}

	void VulkanSystem::updateMouseWheelDelta(int16_t mouseWheelDelta)
	{
		state_.mouseWheelDelta_ = mouseWheelDelta;
		updateWindowViewCameraForwardTransform_(0.02f * FLOAT(pow(state_.mouseWheelDelta_, 3)));
		updateWindowTransformUniformBuffer_(ObjectFlag::eGrid);
		updateWindowTransformUniformBuffer_(ObjectFlag::eScene);
	}


	void VulkanSystem::createInstance_()
	{
		static vk::detail::DynamicLoader dynamicLoader;
		auto vkGetInstanceProcAddr = dynamicLoader.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
		VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

		auto& sApplicationInfo = vk::ApplicationInfo()
			.setPApplicationName("Vulkan API практика")
			.setApplicationVersion(vk::makeApiVersion(0, 0, 1, 0))
			.setPEngineName("ak engine")
			.setEngineVersion(vk::makeApiVersion(0, 0, 1, 0))
			.setApiVersion(vk::makeApiVersion(0, 1, 3, 0));

		auto& sInstanceCreateInfo = vk::InstanceCreateInfo()
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

		auto& sDebugUtilsMessengerCreateInfo = vk::DebugUtilsMessengerCreateInfoEXT{}
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
		auto& sWin32SurfaceCreateInfo = vk::Win32SurfaceCreateInfoKHR{}
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

		std::vector<std::array<bool, UINT32(PhysicalDeviceCriteriaFlag::eMax)>> PhysicalDevicesCriteria{};
		PhysicalDevicesCriteria.resize(physicalDevices.size());

		uint32_t physicalDeviceIdx{};
		for (const auto& physicalDevice : physicalDevices)
		{
			const auto physicalDeviceProperties = physicalDevice.getProperties();
			const auto physicalDeviceFeatures = physicalDevice.getFeatures();
			const auto physicalDeviceExtentions = physicalDevice.enumerateDeviceExtensionProperties();
			const auto physicalDeviceSurfaceFormats = physicalDevice.getSurfaceFormatsKHR(uhSurface_.get());
			const auto physicalDeviceSurfacePresentModes = physicalDevice.getSurfacePresentModesKHR(uhSurface_.get());
			const auto physicalDeviceQueueFamilyProperties = physicalDevice.getQueueFamilyProperties();

			PhysicalDevicesCriteria[physicalDeviceIdx][UINT32(PhysicalDeviceCriteriaFlag::eGeometryShaderSupport)] =
				BOOL(physicalDeviceFeatures.geometryShader);

			for (const auto& physicalDeviceExtention : physicalDeviceExtentions)
			{
				if (std::find_if(deviceExtensionNames_.begin(), deviceExtensionNames_.end(),
					[&](const char* ref)
					{
						return BOOL(!std::strcmp(ref, physicalDeviceExtention.extensionName.data()));
					})
					!= deviceExtensionNames_.end())
				{
					PhysicalDevicesCriteria[physicalDeviceIdx][UINT32(PhysicalDeviceCriteriaFlag::eSwapchainEXTSupport)] = true;
					break;
				}
			}

			PhysicalDevicesCriteria[physicalDeviceIdx][UINT32(PhysicalDeviceCriteriaFlag::eSurfaceRGBAAndNonlinearFormatSupport)] =
				(std::find(physicalDeviceSurfaceFormats.begin(), physicalDeviceSurfaceFormats.end(),
					vk::SurfaceFormatKHR(state_.windowFormat_, vk::ColorSpaceKHR::eSrgbNonlinear))
					!= physicalDeviceSurfaceFormats.end());

			PhysicalDevicesCriteria[physicalDeviceIdx][UINT32(PhysicalDeviceCriteriaFlag::eSurfaceMailboxPresentModeSupport)] =
				(std::find(physicalDeviceSurfacePresentModes.begin(), physicalDeviceSurfacePresentModes.end(), vk::PresentModeKHR::eMailbox)
					!= physicalDeviceSurfacePresentModes.end());


			uint32_t queueFamilyIdx{};

			for (const auto& physicalDeviceQueueFamilyProperty : physicalDeviceQueueFamilyProperties)
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

			uint32_t physicalDeviceQueueFamilyIdx{};
			for (const auto& physicalDeviceQueueFamilyProperty : physicalDeviceQueueFamilyProperties)
			{
				PhysicalDevicesCriteria[physicalDeviceIdx][UINT32(PhysicalDeviceCriteriaFlag::eGraphicsAndPresentQueueFamilySupport)] =
					bool(physicalDeviceQueueFamilyProperty.queueFlags & vk::QueueFlagBits::eGraphics);

				PhysicalDevicesCriteria[physicalDeviceIdx][UINT32(PhysicalDeviceCriteriaFlag::eGraphicsAndPresentQueueFamilySupport)] &=
					bool(physicalDevice.getWin32PresentationSupportKHR(queueFamilyIdx));

				PhysicalDevicesCriteria[physicalDeviceIdx][UINT32(PhysicalDeviceCriteriaFlag::eGraphicsAndPresentQueueFamilySupport)] &=
					bool(physicalDevice.getSurfaceSupportKHR(queueFamilyIdx, uhSurface_.get()));

				if (PhysicalDevicesCriteria[physicalDeviceIdx][UINT32(PhysicalDeviceCriteriaFlag::eGraphicsAndPresentQueueFamilySupport)])
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

		auto& sDeviceCreateInfo = vk::DeviceCreateInfo{}
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
		numSwapchainImages_ = sPhysicalDeviceSurfaceCapabilities_.minImageCount + 1;
		state_.windowExtent_ = sPhysicalDeviceSurfaceCapabilities_.currentExtent;

		state_.windowViewport_ = vk::Viewport{}
			.setX(0.0f)
			.setY(0.0f)
			.setWidth(FLOAT(state_.windowExtent_.width))
			.setHeight(FLOAT(state_.windowExtent_.height))
			.setMinDepth(0.0f)
			.setMaxDepth(1.0f);

		state_.windowScissor_ = vk::Rect2D{ { 0, 0 }, state_.windowExtent_ };

		auto& sSwapchainCreateInfo = vk::SwapchainCreateInfoKHR{}
			.setSurface(uhSurface_.get())
			.setMinImageCount(numSwapchainImages_)
			.setImageFormat(state_.windowFormat_)
			.setImageColorSpace(vk::ColorSpaceKHR::eSrgbNonlinear)
			.setImageExtent(state_.windowExtent_)
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

	void VulkanSystem::createSwapchainImageViews_()
	{
		auto& sImageSubresourceRange = vk::ImageSubresourceRange{}
			.setAspectMask(vk::ImageAspectFlagBits::eColor)
			.setBaseMipLevel(0)
			.setLevelCount(1)
			.setBaseArrayLayer(0)
			.setLayerCount(1);

		uhSwapchainImageViews_.resize(numSwapchainImages_);

		uint32_t swapchainImageIdx{};
		for (auto& uhSwapchainImageView : uhSwapchainImageViews_)
		{
			auto& sSwapchainImageViewCreateInfo = vk::ImageViewCreateInfo{}
				.setImage(hSwapchainImages_[swapchainImageIdx])
				.setViewType(vk::ImageViewType::e2D)
				.setFormat(state_.windowFormat_)
				.setSubresourceRange(sImageSubresourceRange);

			if (!(uhSwapchainImageView = uhDevice_->createImageViewUnique(sSwapchainImageViewCreateInfo)))
			{
				throw wexception(
					L"VulkanSystem::createSwapchainImageViews_() : Невозможно создать вид изображения цепочки показа (Swapchain ImageView)");
			}
			swapchainImageIdx++;
		}
		postLogMessage_(LogLevelFlagBits::eAppInfo, 
			L"VulkanSystem::createSwapchainImageViews_() : Виды изображений цепочки показа (Swapchain ImageViews) созданы");
	}

	void VulkanSystem::createDepthImage_()
	{
		depthImageFormat_ = findSupportedFormat_(
			{ vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint },
			vk::ImageTiling::eOptimal,
			vk::FormatFeatureFlagBits::eDepthStencilAttachment);
				
		uhDepthImage_ = createImageMemoryPair_(depthImageFormat_, state_.windowExtent_, vk::ImageTiling::eOptimal,
			{ graphicsAndPresentQueueFamilyIdx_.value() },
			vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::SharingMode::eExclusive, vk::MemoryPropertyFlagBits::eDeviceLocal);
	}

	void VulkanSystem::createDepthImageView_()
	{
		auto& sImageSubresourceRange = vk::ImageSubresourceRange{}
			.setAspectMask(vk::ImageAspectFlagBits::eDepth)
			.setBaseMipLevel(0)
			.setLevelCount(1)
			.setBaseArrayLayer(0)
			.setLayerCount(1);

		auto& sDepthImageViewCreateInfo = vk::ImageViewCreateInfo{}
			.setImage(uhDepthImage_.first.get())
			.setViewType(vk::ImageViewType::e2D)
			.setFormat(depthImageFormat_)
			.setSubresourceRange(sImageSubresourceRange);

		if (!(uhDepthImageView_ = uhDevice_->createImageViewUnique(sDepthImageViewCreateInfo)))
		{
			throw wexception(
				L"VulkanSystem::createDepthImageView_() : Невозможно создать вид изображения глубины (Depth ImageView)");
		}
		postLogMessage_(LogLevelFlagBits::eAppInfo,
			L"VulkanSystem::createDepthImageView_() : Вид изображения глубины (Depth ImageView) создан");
	}

	void VulkanSystem::createShaderModules_()
	{
		uint32_t shaderModuleIdx{};
		for (auto& uhShaderModule : uhShaderModules_)
		{
			auto buffer = loadShaderFile(uhShaderModuleFilenames_[shaderModuleIdx]);
			auto& sShaderModuleCreateInfo = vk::ShaderModuleCreateInfo{}
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
		auto& sDescriptorSetLayoutBinding = vk::DescriptorSetLayoutBinding{}
			.setBinding(0)
			.setDescriptorType(vk::DescriptorType::eUniformBuffer)
			.setDescriptorCount(1)
			.setStageFlags(vk::ShaderStageFlagBits::eVertex);

		auto& sDescriptorSetLayoutCreateInfo = vk::DescriptorSetLayoutCreateInfo{}
		.setBindings(sDescriptorSetLayoutBinding);

		if (!(uhDescriptorSetLayout_ = uhDevice_->createDescriptorSetLayoutUnique(sDescriptorSetLayoutCreateInfo)))
		{
			throw wexception(L"VulkanSystem::createDescriptorSetLayout_() : Невозможно создать макет набора дескрипторов (DescriptorSetLayout)");
		}
		postLogMessage_(LogLevelFlagBits::eAppInfo, L"VulkanSystem::createDescriptorSetLayout_() : Макет набора дескрипторов (DescriptorSetLayout) создан");
	}

	void VulkanSystem::createGraphicsPipelineLayout_()
	{
		auto& sGraphicsPipelineLayoutCreateInfo = vk::PipelineLayoutCreateInfo{}
		.setSetLayouts(uhDescriptorSetLayout_.get());

		if (!(uhGraphicsPipelineLayout_ = uhDevice_->createPipelineLayoutUnique(sGraphicsPipelineLayoutCreateInfo)))
		{
			throw wexception(L"VulkanSystem::createGraphicsPipelineLayout_() : Невозможно создать макет графического конвейера (GraphicsPipelineLayout)");
		}
		postLogMessage_(LogLevelFlagBits::eAppInfo, L"VulkanSystem::createGraphicsPipelineLayout_() : Макет графического конвейера (GraphicsPipelineLayout) создан");
	}

	void VulkanSystem::createRenderPass_()
	{
		auto& sColorAttachmentDescription = vk::AttachmentDescription{}
			.setFormat(state_.windowFormat_)
			.setSamples(vk::SampleCountFlagBits::e1)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eStore)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

		auto& sDepthAttachmentDescription = vk::AttachmentDescription{}
			.setFormat(depthImageFormat_)
			.setSamples(vk::SampleCountFlagBits::e1)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

		auto& sColorAttachmentReference = vk::AttachmentReference{}
			.setAttachment(0)
			.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

		auto& sDepthAttachmentReference = vk::AttachmentReference{}
			.setAttachment(1)
			.setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

		auto& sSubpassDescription = vk::SubpassDescription{}
			.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
			.setColorAttachments(sColorAttachmentReference)
			.setPDepthStencilAttachment(&sDepthAttachmentReference);

		auto& sSubpassDependency = vk::SubpassDependency{}
			.setSrcSubpass(VK_SUBPASS_EXTERNAL)
			.setDstSubpass(0)
			.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eLateFragmentTests)
			.setSrcAccessMask(vk::AccessFlagBits::eDepthStencilAttachmentWrite)
			.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests)
			.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite);

		std::array<vk::AttachmentDescription, 2> attachments
		{ sColorAttachmentDescription, sDepthAttachmentDescription };

		auto& sRenderPassCreateInfo = vk::RenderPassCreateInfo{}
			.setAttachments(attachments)
			.setSubpasses(sSubpassDescription)
			.setDependencies(sSubpassDependency);

		if (!(uhRenderPass_ = uhDevice_->createRenderPassUnique(sRenderPassCreateInfo)))
		{
			throw wexception(L"VulkanSystem::createRenderPass_() : Невозможно создать проход рендеринга (RenderPass)");
		}
		postLogMessage_(LogLevelFlagBits::eAppInfo, L"VulkanSystem::createRenderPass_() : Проход рендеринга (RenderPass) создан");
	}

	void VulkanSystem::createGraphicsPipelines_()
	{
		// 1. Стадия - Шейдеры
		auto sGraphicsPipelineShaderStageCreateInfos = std::vector<vk::PipelineShaderStageCreateInfo>
		{
			vk::PipelineShaderStageCreateInfo{}
				.setStage(vk::ShaderStageFlagBits::eVertex)
				.setModule(uhShaderModules_[UINT32(ShaderFlag::eVertex)].get())
				.setPName("main"),
			vk::PipelineShaderStageCreateInfo{}
				.setStage(vk::ShaderStageFlagBits::eFragment)
				.setModule(uhShaderModules_[UINT32(ShaderFlag::eFragment)].get())
				.setPName("main")
		};

		// 2. Стадия - Входные данные вершин
		auto sVertexInputBindingDescription = geometry::getBindDescription<geometry::Vertex3D>(0);
		auto sVertexInputAttributeDescriptions = geometry::getAttributeDescriptions<geometry::Vertex3D>(0);

		auto& sGraphicsPipelineVertexInputStateCreateInfo = vk::PipelineVertexInputStateCreateInfo{}
			.setVertexBindingDescriptions(sVertexInputBindingDescription)
			.setVertexAttributeDescriptions(sVertexInputAttributeDescriptions);

		// 4. Стадия - Тесселяция
		auto sGraphicsPipelineTessellationStateCreateInfo = vk::PipelineTessellationStateCreateInfo{};

		// 5. Стадия - Область вывода
		auto& sGraphicsPipelineViewportStateCreateInfo = vk::PipelineViewportStateCreateInfo{}
			.setViewports(state_.windowViewport_)
			.setScissors(state_.windowScissor_);

		// 7. Стадия - Мультисэмплинг
		auto sGraphicsPipelineMultisampleStateCreateInfo = vk::PipelineMultisampleStateCreateInfo{};

		// 8. Стадия - Глубина и трафарет
		auto sGraphicsPipelineDepthStencilStateCreateInfo = vk::PipelineDepthStencilStateCreateInfo{}
			.setDepthTestEnable(vk::True)
			.setDepthWriteEnable(vk::True)
			.setDepthCompareOp(vk::CompareOp::eLess)
			.setDepthBoundsTestEnable(vk::False)
			.setStencilTestEnable(vk::False);

		// 9. Стадия - Смешивание цветов
		auto& sColorBlendAttachment = vk::PipelineColorBlendAttachmentState{}
			.setBlendEnable(vk::False)
			.setColorWriteMask(
				vk::ColorComponentFlagBits::eR |
				vk::ColorComponentFlagBits::eG |
				vk::ColorComponentFlagBits::eB |
				vk::ColorComponentFlagBits::eA);

		auto& sGraphicsPipelineColorBlendStateCreateInfo = vk::PipelineColorBlendStateCreateInfo{}
		.setAttachments(sColorBlendAttachment);

		// 10. Динамическое состояние
		auto graphicsPipelineDynamicStates = std::vector<vk::DynamicState>{
			vk::DynamicState::eViewport,
			vk::DynamicState::eScissor };

		auto& sGraphicsPipelineDynamicStateCreateInfo = vk::PipelineDynamicStateCreateInfo{}
		.setDynamicStates(graphicsPipelineDynamicStates);


		vk::PipelineInputAssemblyStateCreateInfo sGraphicsPipelineInputAssemblyStateCreateInfo{};
		vk::PipelineRasterizationStateCreateInfo sGraphicsPipelineRasterizationStateCreateInfo{};

		uint32_t pipelineIdx{};
		for (auto& uhGraphicsPipeline : uhGraphicsPipelines_)
		{
			switch (pipelineIdx)
			{
			case UINT32(TopologyTypeFlag::ePoints):
				// 3. Стадия - Входная сборка
				sGraphicsPipelineInputAssemblyStateCreateInfo = vk::PipelineInputAssemblyStateCreateInfo{}
				.setTopology(vk::PrimitiveTopology::ePointList);

				// 6. Стадия - Растеризация
				sGraphicsPipelineRasterizationStateCreateInfo = vk::PipelineRasterizationStateCreateInfo{}
					.setDepthClampEnable(vk::False)
					.setRasterizerDiscardEnable(vk::False)
					.setPolygonMode(vk::PolygonMode::ePoint)
					.setLineWidth(1.0f);
				break;

			case UINT32(TopologyTypeFlag::eLines):
				// 3. Стадия - Входная сборка
				sGraphicsPipelineInputAssemblyStateCreateInfo = vk::PipelineInputAssemblyStateCreateInfo{}
				.setTopology(vk::PrimitiveTopology::eLineList);

				// 6. Стадия - Растеризация
				sGraphicsPipelineRasterizationStateCreateInfo = vk::PipelineRasterizationStateCreateInfo{}
					.setDepthClampEnable(vk::False)
					.setRasterizerDiscardEnable(vk::False)
					.setPolygonMode(vk::PolygonMode::eLine)
					.setLineWidth(1.0f);
				break;

			case UINT32(TopologyTypeFlag::eTriangles):
				// 3. Стадия - Входная сборка
				sGraphicsPipelineInputAssemblyStateCreateInfo = vk::PipelineInputAssemblyStateCreateInfo{}
				.setTopology(vk::PrimitiveTopology::eTriangleList);

				// 6. Стадия - Растеризация
				sGraphicsPipelineRasterizationStateCreateInfo = vk::PipelineRasterizationStateCreateInfo{}
					.setDepthClampEnable(vk::False)
					.setRasterizerDiscardEnable(vk::False)
					.setPolygonMode(vk::PolygonMode::eFill)
					.setLineWidth(1.0f);
				break;
			}

			// 11. Графический конвейер
			auto& sGraphicsPipelineCreateInfo = vk::GraphicsPipelineCreateInfo{}
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

			if (!(uhGraphicsPipeline = uhDevice_->createGraphicsPipelineUnique(vk::PipelineCache{}, sGraphicsPipelineCreateInfo).value))
			{
				throw wexception(L"VulkanSystem::createGraphicsPipelines_() : Невозможно создать графический конвейер (GraphicsPipeline)");
			}
			pipelineIdx++;
		}

		postLogMessage_(LogLevelFlagBits::eAppInfo, L"VulkanSystem::createGraphicsPipelines_() : Графические конвейеры (GraphicsPipeline) созданы");
	}

	void VulkanSystem::createFramebuffers_()
	{
		uhFramebuffers_.resize(numSwapchainImages_);

		uint32_t framebufferIdx{};
		for (auto& uhFramebuffer : uhFramebuffers_)
		{
			std::array<vk::ImageView, 2> attachments
			{ uhSwapchainImageViews_[framebufferIdx].get(), uhDepthImageView_.get() };

			auto& sFramebufferCreateInfo = vk::FramebufferCreateInfo{}
				.setRenderPass(uhRenderPass_.get())
				.setAttachments(attachments)
				.setWidth(state_.windowExtent_.width)
				.setHeight(state_.windowExtent_.height)
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

		auto& sFenceCreateInfo = vk::FenceCreateInfo{}
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
		auto& sTransferCommandPoolCreateInfo = vk::CommandPoolCreateInfo{}
			.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
			.setQueueFamilyIndex(transferQueueFamilyIdx_.value());

		if (!(uhTransferCommandPool_ = uhDevice_->createCommandPoolUnique(sTransferCommandPoolCreateInfo)))
		{
			throw wexception(L"VulkanSystem::createCommandPools_() : Невозможно создать командный пул (CommandPool) для передачи");
		}
		postLogMessage_(LogLevelFlagBits::eAppInfo, L"VulkanSystem::createCommandPool_() : Командный пул (CommandPool) для передачи создан");

		auto& sCommandPoolCreateInfo = vk::CommandPoolCreateInfo{}
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
		auto& sTransferCommandBufferAllocateInfo = vk::CommandBufferAllocateInfo{}
			.setCommandPool(*uhTransferCommandPool_)
			.setLevel(vk::CommandBufferLevel::ePrimary)
			.setCommandBufferCount(numFramesInFlight + 2);

		if (!(hTransferCommandBuffers_ = uhDevice_->allocateCommandBuffers(sTransferCommandBufferAllocateInfo)).size())
		{
			throw wexception(L"VulkanSystem::allocateCommandBuffers_() : Невозможно выделить командные буферы (CommandBuffers) для передачи");
		}
		postLogMessage_(LogLevelFlagBits::eAppInfo, L"VulkanSystem::allocateCommandBuffers_() : Командные буферы (CommandBuffers) для передачи выделены");

		auto& sGraphicsCommandBufferAllocateInfo = vk::CommandBufferAllocateInfo{}
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
		auto& sDescriptorPoolSize = vk::DescriptorPoolSize{}
			.setType(vk::DescriptorType::eUniformBuffer)
			.setDescriptorCount(UINT32(numObjects) * numFramesInFlight);

		auto& sDescriptorPoolCreateInfo = vk::DescriptorPoolCreateInfo{}
			.setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet)
			.setPoolSizes(sDescriptorPoolSize)
			.setMaxSets(UINT32(numObjects) * numFramesInFlight);

		if (!(uhDescriptorPool_ = uhDevice_->createDescriptorPoolUnique(sDescriptorPoolCreateInfo)))
		{
			throw wexception(L"VulkanSystem::createDescriptorPool_() : Невозможно выделить пул дескрипторов (DescriptorPool)");
		}
		postLogMessage_(LogLevelFlagBits::eAppInfo, L"VulkanSystem::createDescriptorPool_() : Пул дескрипторов (DescriptorPool) создан");
	}



	uint32_t VulkanSystem::findSuitableMemoryIndex_(uint32_t allowedTypesMask,
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
		throw wexception(L"VulkanSystem::findSuitableMemoryIndex_() : Невозможно найти подходящую память GPU");
	}

	vk::Format VulkanSystem::findSupportedFormat_(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling,
		vk::FormatFeatureFlags featureFlags)
	{
		for (auto& format : candidates)
		{
			auto formatProperties = hPhysicalDevice_.getFormatProperties(format);

			if (tiling == vk::ImageTiling::eLinear && (formatProperties.linearTilingFeatures & featureFlags) == featureFlags)
			{
				return format;
			}
			else if (tiling == vk::ImageTiling::eOptimal && (formatProperties.optimalTilingFeatures & featureFlags) == featureFlags)
			{
				return format;
			}
		}
		throw wexception(L"VulkanSystem::findSupportedFormat_() : Невозможно найти подходящий формат");
	}

	std::pair<vk::UniqueBuffer, vk::UniqueDeviceMemory> VulkanSystem::createBufferMemoryPair_(vk::DeviceSize size,
		std::vector<uint32_t> queueFamilyIndices, vk::BufferUsageFlags usageFlags,
		vk::SharingMode sharingMode, vk::MemoryPropertyFlags requiredMemoryFlags)
	{
		std::pair<vk::UniqueBuffer, vk::UniqueDeviceMemory> bufferMemoryPair{};

		auto& sBufferCreateInfo = vk::BufferCreateInfo{}
			.setSize(size + 1)
			.setUsage(usageFlags)
			.setSharingMode(sharingMode)
			.setQueueFamilyIndices(queueFamilyIndices);

		if (!(bufferMemoryPair.first = uhDevice_->createBufferUnique(sBufferCreateInfo)))
		{
			throw wexception(L"VulkanSystem::createBufferMemoryPair_() : Невозможно создать буфер (Buffer)");
		}

		auto bufferMemoryRequirements = uhDevice_->getBufferMemoryRequirements(bufferMemoryPair.first.get());
		auto memoryIdx = findSuitableMemoryIndex_(bufferMemoryRequirements.memoryTypeBits, requiredMemoryFlags);

		auto& sMemoryAllocateInfo = vk::MemoryAllocateInfo{}
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

	std::pair<vk::UniqueImage, vk::UniqueDeviceMemory> VulkanSystem::createImageMemoryPair_(vk::Format format,
		vk::Extent2D extent, vk::ImageTiling tiling, std::vector<uint32_t> queueFamilyIndices, vk::ImageUsageFlags usageFlags,
		vk::SharingMode sharingMode, vk::MemoryPropertyFlags requiredMemoryFlags)
	{
		std::pair<vk::UniqueImage, vk::UniqueDeviceMemory> imageMemoryPair{};

		auto& sImageCreateInfo = vk::ImageCreateInfo{}
			.setImageType(vk::ImageType::e2D)
			.setFormat(format)
			.setExtent({ extent.width, extent.height, 1 })
			.setMipLevels(1)
			.setArrayLayers(1)
			.setSamples(vk::SampleCountFlagBits::e1)
			.setTiling(tiling)
			.setUsage(usageFlags)
			.setSharingMode(sharingMode)
			.setQueueFamilyIndices(queueFamilyIndices)
			.setInitialLayout(vk::ImageLayout::eUndefined);

		if (!(imageMemoryPair.first = uhDevice_->createImageUnique(sImageCreateInfo)))
		{
			throw wexception(L"VulkanSystem::createImageMemoryPair_() : Невозможно создать изображение (Image)");
		}

		auto imageMemoryRequirements = uhDevice_->getImageMemoryRequirements(imageMemoryPair.first.get());
		auto memoryIdx = findSuitableMemoryIndex_(imageMemoryRequirements.memoryTypeBits, requiredMemoryFlags);

		auto& sMemoryAllocateInfo = vk::MemoryAllocateInfo{}
			.setAllocationSize(imageMemoryRequirements.size)
			.setMemoryTypeIndex(memoryIdx);

		if (!(imageMemoryPair.second = uhDevice_->allocateMemoryUnique(sMemoryAllocateInfo)))
		{
			throw wexception(L"VulkanSystem::createImageMemoryPair_ : Невозможно выделить память изображения (ImageMemory)");
		}

		uhDevice_->bindImageMemory(imageMemoryPair.first.get(), imageMemoryPair.second.get(), 0u);
		postLogMessage_(LogLevelFlagBits::eAppInfo, L"VulkanSystem::createImageMemoryPair_ : Пара (Изображение - Память изображения) (Image, ImageMemory) создана");

		return imageMemoryPair;
	}

	void VulkanSystem::copyBuffer_(uint32_t transferIdx, vk::Buffer& hSrcBuffer, vk::Buffer& hDstBuffer, vk::DeviceSize size)
	{
		auto result = uhDevice_->waitForFences(uhTransferFences_[transferIdx].get(), vk::True, UINT64_MAX);
		if (result != vk::Result::eSuccess)
		{
			throw wexception(L"VulkanSystem::copyBuffer_() : Ошибка выполнения waitForFences");
		}

		auto& sCommandBufferBeginInfo = vk::CommandBufferBeginInfo{}
		.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

		auto& sBufferCopy = vk::BufferCopy{}
			.setSrcOffset(0)
			.setDstOffset(0)
			.setSize(size);

		hTransferCommandBuffers_[transferIdx].begin(sCommandBufferBeginInfo);
		hTransferCommandBuffers_[transferIdx].copyBuffer(hSrcBuffer, hDstBuffer, sBufferCopy);
		hTransferCommandBuffers_[transferIdx].end();

		auto& sSubmitInfo = vk::SubmitInfo{}
		.setCommandBuffers(hTransferCommandBuffers_[transferIdx]);

		uhDevice_->resetFences(uhTransferFences_[transferIdx].get());
		hTransferQueue_.submit(sSubmitInfo, uhTransferFences_[transferIdx].get());
	}



	void VulkanSystem::initGrid_()
	{
		// 1. Точки
		{
			auto& vertices = vertices_[UINT32(ObjectFlag::eGrid)][UINT32(TopologyTypeFlag::ePoints)];

			vertices.push_back({ { 0.0f, 0.0f, 0.0f }, state_.gridPointColor_ });

			for (uint32_t idx{ 1 }; idx < UINT32(state_.gridDimension_); idx++)
			{
				vertices.push_back({ { -state_.gridSpacing_.x * idx, 0.0f, 0.0f}, state_.gridPointColor_ });
				vertices.push_back({ { state_.gridSpacing_.x * idx,	0.0f, 0.0f}, state_.gridPointColor_ });
				vertices.push_back({ { 0.0f, -state_.gridSpacing_.y * idx, 0.0f}, state_.gridPointColor_ });
				vertices.push_back({ { 0.0f, state_.gridSpacing_.y * idx, 0.0f}, state_.gridPointColor_ });
			}

			for (uint32_t idxX{ 1 }; idxX < UINT32(state_.gridDimension_); idxX++)
			{
				for (uint32_t idxY{ 1 }; idxY < UINT32(state_.gridDimension_); idxY++)
				{
					vertices.push_back({ { -state_.gridSpacing_.x * idxX, -state_.gridSpacing_.y * idxY, 0.0f}, state_.gridPointColor_ });
					vertices.push_back({ { -state_.gridSpacing_.x * idxX, state_.gridSpacing_.y * idxY, 0.0f}, state_.gridPointColor_ });
					vertices.push_back({ { state_.gridSpacing_.x * idxX, -state_.gridSpacing_.y * idxY, 0.0f}, state_.gridPointColor_ });
					vertices.push_back({ { state_.gridSpacing_.x * idxX, state_.gridSpacing_.y * idxY, 0.0f}, state_.gridPointColor_ });
				}
			}

			auto& vertexIndices = vertexIndices_[UINT32(ObjectFlag::eGrid)][UINT32(TopologyTypeFlag::ePoints)];

			for (uint32_t idx{}; idx < UINT32(vertices.size()); idx++)
			{
				vertexIndices.push_back(idx);
			}
		}

		// 2. Линии
		{
			auto& vertices = vertices_[UINT32(ObjectFlag::eGrid)][UINT32(TopologyTypeFlag::eLines)];

			vertices.push_back({ { -state_.gridDimension_ * state_.gridSpacing_.x, 0.0f, 0.0f }, state_.gridAxisColors_[0] });
			vertices.push_back({ { state_.gridDimension_ * state_.gridSpacing_.x, 0.0f, 0.0f }, state_.gridAxisColors_[0] });

			vertices.push_back({ { 0.0f, -state_.gridDimension_ * state_.gridSpacing_.y, 0.0f }, state_.gridAxisColors_[1] });
			vertices.push_back({ { 0.0f, state_.gridDimension_ * state_.gridSpacing_.y, 0.0f }, state_.gridAxisColors_[1] });

			for (uint32_t idx{ 1 }; idx < UINT32(state_.gridDimension_); idx++)
			{
				vertices.push_back({ { -state_.gridDimension_ * state_.gridSpacing_.x, -state_.gridSpacing_.y * idx, 0.0f}, state_.gridColor_ });
				vertices.push_back({ { state_.gridDimension_ * state_.gridSpacing_.x, -state_.gridSpacing_.y * idx, 0.0f}, state_.gridColor_ });

				vertices.push_back({ { -state_.gridDimension_ * state_.gridSpacing_.x, state_.gridSpacing_.y * idx, 0.0f}, state_.gridColor_ });
				vertices.push_back({ { state_.gridDimension_ * state_.gridSpacing_.x, state_.gridSpacing_.y * idx, 0.0f}, state_.gridColor_ });

				vertices.push_back({ { -state_.gridSpacing_.x * idx, -state_.gridDimension_ * state_.gridSpacing_.y, 0.0f}, state_.gridColor_ });
				vertices.push_back({ { -state_.gridSpacing_.x * idx, state_.gridDimension_ * state_.gridSpacing_.y, 0.0f}, state_.gridColor_ });

				vertices.push_back({ { state_.gridSpacing_.x * idx, -state_.gridDimension_ * state_.gridSpacing_.y, 0.0f}, state_.gridColor_ });
				vertices.push_back({ { state_.gridSpacing_.x * idx, state_.gridDimension_ * state_.gridSpacing_.y, 0.0f}, state_.gridColor_ });
			}

			auto& vertexIndices = vertexIndices_[UINT32(ObjectFlag::eGrid)][UINT32(TopologyTypeFlag::eLines)];

			for (uint32_t idx{}; idx < UINT32(vertices.size()); idx++)
			{
				vertexIndices.push_back(idx);
			}
		}
	}

	void VulkanSystem::initCrosshair_()
	{
		// 1. Линии
		{
			auto& vertices = vertices_[UINT32(ObjectFlag::eCrosshair)][UINT32(TopologyTypeFlag::eLines)];

			vertices =
			{
				{ {-state_.crosshairAimSize_, 0.0f, 0.0f }, state_.crosschairColors_[0] },
				{ {state_.crosshairAimSize_, 0.0f, 0.0f }, state_.crosschairColors_[0] },
				{ {0.0f, -state_.crosshairAimSize_, 0.0f }, state_.crosschairColors_[1] },
				{ {0.0f, state_.crosshairAimSize_, 0.0f }, state_.crosschairColors_[1] },
				{ {0.0f, 0.0f, -state_.crosshairAimSize_ }, state_.crosschairColors_[2] },
				{ {0.0f, 0.0f, state_.crosshairAimSize_ }, state_.crosschairColors_[2] }
			};

			auto& vertexIndices = vertexIndices_[UINT32(ObjectFlag::eCrosshair)][UINT32(TopologyTypeFlag::eLines)];

			for (uint32_t idx{}; idx < UINT32(vertices.size()); idx++)
			{
				vertexIndices.push_back(idx);
			}
		}

		// 2. Точки
		{
			auto& vertices = vertices_[UINT32(ObjectFlag::eCrosshair)][UINT32(TopologyTypeFlag::ePoints)];

			vertices =
			{
				{ {0.0f, 0.0f, 0.0f }, state_.crosschairPointColor_ },
				{ {-state_.crosshairAimSize_, 0.0f, 0.0f }, state_.crosschairPointColor_ },
				{ {state_.crosshairAimSize_, 0.0f, 0.0f }, state_.crosschairPointColor_ },
				{ {0.0f, -state_.crosshairAimSize_, 0.0f }, state_.crosschairPointColor_ },
				{ {0.0f, state_.crosshairAimSize_, 0.0f }, state_.crosschairPointColor_ },
				{ {0.0f, 0.0f, -state_.crosshairAimSize_ }, state_.crosschairPointColor_ },
				{ {0.0f, 0.0f, state_.crosshairAimSize_ }, state_.crosschairPointColor_ }
			};

			auto& vertexIndices = vertexIndices_[UINT32(ObjectFlag::eCrosshair)][UINT32(TopologyTypeFlag::ePoints)];

			for (uint32_t idx{}; idx < UINT32(vertices.size()); idx++)
			{
				vertexIndices.push_back(idx);
			}
		}
	}

	void VulkanSystem::addPrimitiveToScene_(TopologyTypeFlag topologyType, geometry::Primitive& primitive)
	{
		auto& vertices = vertices_[UINT32(ObjectFlag::eScene)][UINT32(topologyType)];
		auto& vertexIndices = vertexIndices_[UINT32(ObjectFlag::eScene)][UINT32(topologyType)];

		auto addingVertices = primitive.getVertices(topologyType);
		auto addingvertexIndices = primitive.getVertexIndices(topologyType);

		uint32_t startVertexIdx{ UINT32(vertices.size()) };
		std::for_each(addingvertexIndices.begin(), addingvertexIndices.end(),
			[&](uint32_t& elem) { elem += startVertexIdx; });

		vertices.insert(vertices.end(), addingVertices.begin(), addingVertices.end());
		vertexIndices.insert(vertexIndices.end(), addingvertexIndices.begin(), addingvertexIndices.end());
	}


	void VulkanSystem::updateWindowTransformUniformBuffer_(ObjectFlag object)
	{
		auto& objectUniformTranformations = uniformTranformations_[UINT32(object)];

		uint32_t frameInFlightIdx{};
		for (auto& uniformTranformations : objectUniformTranformations)
		{
			uniformTranformations.model = glm::mat4(1.0f);
			uniformTranformations.view = glm::lookAt(state_.windowViewCameraEye_,
				state_.windowViewCameraTarget_, state_.windowViewCameraUp_);
			uniformTranformations.proj = glm::perspective(glm::radians(state_.windowViewPerspectiveFocusAngle_),
				state_.windowAspectRatio_,
				state_.windowViewPerspectiveNear_, state_.windowPerspectiveViewFar_);
			uniformTranformations.proj[1][1] *= -1;

			auto& pMappedUniformBufferMemorySection = pMappedUniformBufferMemorySections_[UINT32(object)][frameInFlightIdx];

			memcpy(pMappedUniformBufferMemorySection, &uniformTranformations, sizeof(geometry::Uniform));
			frameInFlightIdx++;
		}
	}

	void VulkanSystem::updateWindowSizeTransform_()
	{
		// Обновление соотношения сторон экрана
		state_.windowAspectRatio_ = state_.windowViewport_.width / state_.windowViewport_.height;

		state_.windowViewCameraForward_ = state_.windowViewCameraEye_ - state_.windowViewCameraTarget_;
		state_.windowViewCameraRight_ = glm::normalize(glm::cross(state_.windowViewCameraUp_, state_.windowViewCameraForward_));
		state_.windowViewCameraUp_ = glm::normalize(glm::cross(state_.windowViewCameraForward_, state_.windowViewCameraRight_));
	}

	void VulkanSystem::updateWindowViewCameraRightAndUpTransform_(float rightFactor, float upFactor)
	{
		// Смещение камеры и цели в стороны
		glm::vec3 translateVec{ rightFactor, -upFactor, 1.0f };
		glm::mat4 mattrix{ glm::translate(glm::mat4(1.0f), translateVec) };

		state_.windowViewCameraEye_ = mattrix * glm::vec4{ state_.windowViewCameraEye_, 1.0f };
		state_.windowViewCameraTarget_ = mattrix * glm::vec4{ state_.windowViewCameraTarget_, 1.0f };

		state_.windowViewCameraForward_ = state_.windowViewCameraEye_ - state_.windowViewCameraTarget_;
		state_.windowViewCameraRight_ = glm::normalize(glm::cross(state_.windowViewCameraUp_, state_.windowViewCameraForward_));
		state_.windowViewCameraUp_ = glm::normalize(glm::cross(state_.windowViewCameraForward_, state_.windowViewCameraRight_));
	}

	void VulkanSystem::updateWindowViewCameraForwardTransform_(float forwardFactor)
	{
		// Смещение камеры и цели вперёд/назад
		glm::vec3 translateVec{ forwardFactor * state_.windowViewCameraForward_ };
		glm::mat4 mattrix{ glm::translate(glm::mat4(1.0f), translateVec) };

		state_.windowViewCameraEye_ = mattrix * glm::vec4{ state_.windowViewCameraEye_, 1.0f };
		state_.windowViewCameraTarget_ = mattrix * glm::vec4{ state_.windowViewCameraTarget_, 1.0f };

		state_.windowViewCameraForward_ = state_.windowViewCameraEye_ - state_.windowViewCameraTarget_;
		state_.windowViewCameraRight_ = glm::normalize(glm::cross(state_.windowViewCameraUp_, state_.windowViewCameraForward_));
		state_.windowViewCameraUp_ = glm::normalize(glm::cross(state_.windowViewCameraForward_, state_.windowViewCameraRight_));
	}

	void VulkanSystem::updateWindowViewCameraRotateEyeTransform_(float rightFactor, float upFactor)
	{
		// Поворот камеры вокруг себя
		glm::mat4 mattrix{ glm::rotate(glm::mat4(1.0f), glm::radians(rightFactor), state_.windowViewCameraUp_) };
		mattrix = glm::rotate(mattrix, glm::radians(upFactor), state_.windowViewCameraRight_);

		state_.windowViewCameraUp_ = mattrix * glm::vec4{ state_.windowViewCameraUp_, 1.0f };
		state_.windowViewCameraEye_ = mattrix * glm::vec4{ state_.windowViewCameraEye_, 1.0f };
		state_.windowViewCameraTarget_ = mattrix * glm::vec4{ state_.windowViewCameraTarget_, 1.0f };

		state_.windowViewCameraForward_ = state_.windowViewCameraEye_ - state_.windowViewCameraTarget_;
		state_.windowViewCameraRight_ = glm::normalize(glm::cross(state_.windowViewCameraUp_, state_.windowViewCameraForward_));
		state_.windowViewCameraUp_ = glm::normalize(glm::cross(state_.windowViewCameraForward_, state_.windowViewCameraRight_));
	}

	void VulkanSystem::updateWindowViewCameraRotateTargetTransform_(float rightFactor, float upFactor)
	{
		// Поворот камеры вокруг цели

	}

	void VulkanSystem::updateCrosshairTransformUniformBuffer_()
	{
		glm::vec3 translateVec{ state_.crosshairPosition_.x, -state_.crosshairPosition_.y, 1.0f };

		auto& uniformTranformations = uniformTranformations_[UINT32(ObjectFlag::eCrosshair)][state_.frameInFlightIdx_];

		uniformTranformations.model = glm::translate(glm::mat4(1.0f), translateVec);
		uniformTranformations.view = glm::lookAt(glm::vec3{ 0.0f, 0.0f, FLOAT(state_.windowExtent_.width) / 2.0f },
			glm::vec3{ 0.0f, 0.0f, 0.0f }, glm::vec3{ 0.0f, 1.0f, 0.0f });
		uniformTranformations.proj = glm::perspective(glm::radians(state_.windowViewPerspectiveFocusAngle_),
			state_.windowAspectRatio_,
			state_.windowViewPerspectiveNear_, state_.windowPerspectiveViewFar_);
		uniformTranformations.proj[1][1] *= -1;

		auto& pMappedUniformBufferMemorySection = pMappedUniformBufferMemorySections_[UINT32(ObjectFlag::eCrosshair)][state_.frameInFlightIdx_];

		memcpy(pMappedUniformBufferMemorySection, &uniformTranformations, sizeof(geometry::Uniform));
	}


	void VulkanSystem::createVertexBuffer_(ObjectFlag object, TopologyTypeFlag topologyType)
	{
		auto& vertices = vertices_[UINT32(object)][UINT32(topologyType)];
		auto& uhVertexStagingBuffer = uhVertexStagingBuffers_[UINT32(object)][UINT32(topologyType)];
		auto& uhVertexBuffer = uhVertexBuffers_[UINT32(object)][UINT32(topologyType)];

		auto bufferSize = static_cast<vk::DeviceSize>(sizeof(geometry::Vertex3D) * vertices.size());

		uhVertexStagingBuffer = createBufferMemoryPair_(bufferSize, { transferQueueFamilyIdx_.value(), graphicsAndPresentQueueFamilyIdx_.value() },
			vk::BufferUsageFlagBits::eTransferSrc, vk::SharingMode::eConcurrent,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

		if (vertices.size())
		{
			auto mappedVertexBufferMemory = uhDevice_->mapMemory(uhVertexStagingBuffer.second.get(), 0, bufferSize);
			memcpy(mappedVertexBufferMemory, vertices.data(), bufferSize);
			uhDevice_->unmapMemory(uhVertexStagingBuffer.second.get());
		}

		uhVertexBuffer = createBufferMemoryPair_(bufferSize, { graphicsAndPresentQueueFamilyIdx_.value() },
			vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
			vk::SharingMode::eExclusive, vk::MemoryPropertyFlagBits::eDeviceLocal);

		if (vertices.size())
		{
			copyBuffer_(numFramesInFlight, uhVertexStagingBuffer.first.get(), uhVertexBuffer.first.get(), bufferSize);
		}
	}

	void VulkanSystem::createIndexBuffer_(ObjectFlag object, TopologyTypeFlag topologyType)
	{
		auto& vertexIndices = vertexIndices_[UINT32(object)][UINT32(topologyType)];
		auto& uhIndexStagingBuffer = uhIndexStagingBuffers_[UINT32(object)][UINT32(topologyType)];
		auto& uhIndexBuffer = uhIndexBuffers_[UINT32(object)][UINT32(topologyType)];

		auto bufferSize = static_cast<vk::DeviceSize>(sizeof(uint32_t) * vertexIndices.size());

		uhIndexStagingBuffer = createBufferMemoryPair_(bufferSize, { transferQueueFamilyIdx_.value(), graphicsAndPresentQueueFamilyIdx_.value() },
			vk::BufferUsageFlagBits::eTransferSrc, vk::SharingMode::eConcurrent,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

		if (vertexIndices.size())
		{
			auto mappedVertexBufferMemory = uhDevice_->mapMemory(uhIndexStagingBuffer.second.get(), 0, bufferSize);
			memcpy(mappedVertexBufferMemory, vertexIndices.data(), bufferSize);
			uhDevice_->unmapMemory(uhIndexStagingBuffer.second.get());
		}

		uhIndexBuffer = createBufferMemoryPair_(bufferSize, { graphicsAndPresentQueueFamilyIdx_.value() },
			vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
			vk::SharingMode::eExclusive, vk::MemoryPropertyFlagBits::eDeviceLocal);

		if (vertexIndices.size())
		{
			copyBuffer_(numFramesInFlight + 1, uhIndexStagingBuffer.first.get(), uhIndexBuffer.first.get(), bufferSize);
		}
	}

	void VulkanSystem::createUniformBuffers_(ObjectFlag object)
	{
		auto& uhUniformBuffers = uhUniformBuffers_[UINT32(object)];

		auto bufferSize = static_cast<vk::DeviceSize>(sizeof(geometry::Uniform));

		uint32_t frameInFlightIdx{};
		for (auto& uhUniformBuffer : uhUniformBuffers)
		{
			auto& pMappedUniformBufferMemorySection =
				pMappedUniformBufferMemorySections_[UINT32(object)][frameInFlightIdx];

			uhUniformBuffer = createBufferMemoryPair_(bufferSize, { graphicsAndPresentQueueFamilyIdx_.value() },
				vk::BufferUsageFlagBits::eUniformBuffer, vk::SharingMode::eExclusive,
				vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

			pMappedUniformBufferMemorySection = uhDevice_->mapMemory(uhUniformBuffer.second.get(), 0, bufferSize);

			frameInFlightIdx++;
		}
	}

	void VulkanSystem::allocateDescriptorSets_(ObjectFlag object)
	{
		auto& uhDescriptorSets = uhDescriptorSets_[UINT32(object)];
		auto& uhUniformBuffers = uhUniformBuffers_[UINT32(object)];

		std::vector<vk::DescriptorSetLayout> hDescriptorSetLayouts(numFramesInFlight, uhDescriptorSetLayout_.get());

		auto& sDescriptorSetAllocateInfo = vk::DescriptorSetAllocateInfo{}
			.setDescriptorPool(uhDescriptorPool_.get())
			.setDescriptorSetCount(numFramesInFlight)
			.setSetLayouts(hDescriptorSetLayouts);

		if (!(uhDescriptorSets = uhDevice_->allocateDescriptorSetsUnique(sDescriptorSetAllocateInfo)).size())
		{
			throw wexception(L"VulkanSystem::allocateDescriptorSets_() : Невозможно выделить наборы дескрипторов (DescriptorSets)");
		}
		postLogMessage_(LogLevelFlagBits::eAppInfo, L"VulkanSystem::allocateDescriptorSets_() : Наборы дескрипторов (DescriptorSets) выделены");

		uint32_t frameInFlightIdx{};
		for (auto& uhDescriptorSet : uhDescriptorSets)
		{
			auto& sDescriptorBufferInfo = vk::DescriptorBufferInfo{}
				.setBuffer(uhUniformBuffers[frameInFlightIdx].first.get())
				.setOffset(0)
				.setRange(static_cast<vk::DeviceSize>(sizeof(geometry::Uniform)));

			auto& sWriteDescriptorSet = vk::WriteDescriptorSet{}
				.setDstSet(uhDescriptorSet.get())
				.setDstBinding(0)
				.setDstArrayElement(0)
				.setDescriptorType(vk::DescriptorType::eUniformBuffer)
				.setDescriptorCount(1)
				.setBufferInfo(sDescriptorBufferInfo);

			uhDevice_->updateDescriptorSets(sWriteDescriptorSet, nullptr);

			frameInFlightIdx++;
		}
	}


	void VulkanSystem::recordObjectToCommandBuffer_(vk::CommandBuffer hCommandBuffer,
		TopologyTypeFlag topologyType, ObjectFlag object)
	{
		auto& hVertexBuffer = uhVertexBuffers_[UINT32(object)][UINT32(topologyType)].first.get();
		auto& hIndexBuffer = uhIndexBuffers_[UINT32(object)][UINT32(topologyType)].first.get();
		auto& vertexIndices = vertexIndices_[UINT32(object)][UINT32(topologyType)];
		auto& hDescriptorSet = uhDescriptorSets_[UINT32(object)][state_.frameInFlightIdx_].get();

		if (!vertexIndices.size())
		{
			return;
		}

		hCommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics,
			uhGraphicsPipelines_[UINT32(topologyType)].get());
		hCommandBuffer.setViewport(0, 1, &state_.windowViewport_);
		hCommandBuffer.setScissor(0, 1, &state_.windowScissor_);

		hCommandBuffer.bindVertexBuffers(0, hVertexBuffer, { 0 });
		hCommandBuffer.bindIndexBuffer(hIndexBuffer, 0, vk::IndexType::eUint32);
		hCommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, uhGraphicsPipelineLayout_.get(), 0, hDescriptorSet, nullptr);
		hCommandBuffer.drawIndexed(UINT32(vertexIndices.size()), 1, 0, 0, 0);
	}

	void VulkanSystem::recordGraphicsCommandBuffer_()
	{
		auto& hCommandBuffer = hGraphicsCommandBuffers_[state_.frameInFlightIdx_];
		auto& hFrameBuffer = uhFramebuffers_[state_.swapchainImageIdx_].get();

		std::array<vk::ClearValue, 2> clearValues;
		clearValues[0].setColor(state_.windowBackgroundColor_);
		clearValues[1].setDepthStencil({ 1.0f, 0 });

		auto& sRenderPassBeginInfo = vk::RenderPassBeginInfo{}
			.setRenderPass(uhRenderPass_.get())
			.setFramebuffer(hFrameBuffer)
			.setRenderArea(state_.windowScissor_)
			.setClearValues(clearValues);

		hCommandBuffer.reset();
		hCommandBuffer.begin(vk::CommandBufferBeginInfo{});
		hCommandBuffer.beginRenderPass(sRenderPassBeginInfo, vk::SubpassContents::eInline);

		recordObjectToCommandBuffer_(hCommandBuffer, TopologyTypeFlag::ePoints, ObjectFlag::eCrosshair);
		recordObjectToCommandBuffer_(hCommandBuffer, TopologyTypeFlag::eLines, ObjectFlag::eCrosshair);
		recordObjectToCommandBuffer_(hCommandBuffer, TopologyTypeFlag::ePoints, ObjectFlag::eGrid);
		recordObjectToCommandBuffer_(hCommandBuffer, TopologyTypeFlag::eLines, ObjectFlag::eGrid);
		recordObjectToCommandBuffer_(hCommandBuffer, TopologyTypeFlag::ePoints, ObjectFlag::eScene);
		recordObjectToCommandBuffer_(hCommandBuffer, TopologyTypeFlag::eLines, ObjectFlag::eScene);
		recordObjectToCommandBuffer_(hCommandBuffer, TopologyTypeFlag::eTriangles, ObjectFlag::eScene);

		hCommandBuffer.endRenderPass();
		hCommandBuffer.end();
	}

	void VulkanSystem::drawPaused_()
	{
	}

	void VulkanSystem::drawStarted_()
	{
		auto result = uhDevice_->waitForFences(uhInFlightFences_[state_.frameInFlightIdx_].get(), vk::True,
			std::numeric_limits<uint64_t>::max());
		if (result != vk::Result::eSuccess)
		{
			throw wexception(L"VulkanSystem::draw() : Ошибка выполнения waitForFences");
		}

		state_.swapchainImageIdx_ = uhDevice_->acquireNextImageKHR(uhSwapchain_.get(),
			UINT64_MAX, uhReadyForRenderingSemaphores_[state_.frameInFlightIdx_].get(), VK_NULL_HANDLE).value;

		updateCrosshairTransformUniformBuffer_();
		recordGraphicsCommandBuffer_();

		vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };

		auto& sSubmitInfo = vk::SubmitInfo{}
			.setCommandBuffers(hGraphicsCommandBuffers_[state_.frameInFlightIdx_])
			.setWaitSemaphores(uhReadyForRenderingSemaphores_[state_.frameInFlightIdx_].get())
			.setSignalSemaphores(uhReadyForPresentingSemaphores_[state_.frameInFlightIdx_].get())
			.setPWaitDstStageMask(waitStages);

		uhDevice_->resetFences(uhInFlightFences_[state_.frameInFlightIdx_].get());
		hGraphicsQueue_.submit(sSubmitInfo, uhInFlightFences_[state_.frameInFlightIdx_].get());

		auto& sPresentInfo = vk::PresentInfoKHR{}
			.setSwapchains(uhSwapchain_.get())
			.setImageIndices(state_.swapchainImageIdx_)
			.setWaitSemaphores(uhReadyForPresentingSemaphores_[state_.frameInFlightIdx_].get());

		result = hGraphicsQueue_.presentKHR(sPresentInfo);
		if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR)
		{
			throw wexception(L"VulkanSystem::draw() : Ошибка выполнения presentKHR");
		}

		state_.frameInFlightIdx_ = (state_.frameInFlightIdx_ + 1) % numFramesInFlight;
	}
}