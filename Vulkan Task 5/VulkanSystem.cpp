#include "VulkanSystem.h"
VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE


namespace ak
{
	VulkanSystem::VulkanSystem()
	{
	}

	VulkanSystem::~VulkanSystem()
	{
		if (uhDevice_)
		{
			uhDevice_->waitIdle();
		}






		if (uhSurface_)
		{
			uhInstance_->destroySurfaceKHR(uhSurface_.get());
		}

		if (uhDebugUtilsMessenger_)
		{
			uhInstance_->destroyDebugUtilsMessengerEXT(uhDebugUtilsMessenger_.get());
		}

		if (uhDevice_)
		{
			// TODO: Почему исключение? Уничтожение объекта производится 2 раза?
			//uhDevice_->destroy();
		}

		if (uhInstance_)
		{
			// TODO: Почему исключение? Уничтожение объекта производится 2 раза?
			//uhInstance_->destroy(nullptr);
		}
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
			pVulkanSystem->postLogMessage(LogLevel::eVulkanVerbose, message);
			return VK_TRUE;
		}
		else if (messageSeverity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo)
		{
			pVulkanSystem->postLogMessage(LogLevel::eVulkanInfo, message);
			return VK_TRUE;
		}
		else if (messageSeverity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning)
		{
			pVulkanSystem->postLogMessage(LogLevel::eVulkanWarning, message);
			return VK_FALSE;
		}
		else if (messageSeverity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eError)
		{
			pVulkanSystem->postLogMessage(LogLevel::eVulkanError, message);
			return VK_FALSE;
		}

		return VK_TRUE;
	}
#endif // _DEBUG

	void VulkanSystem::postLogMessage(LogLevel logLevel, const char* message)
	{
#ifdef _DEBUG
		shpLoggingSystem_->postLogMessage(logLevel, message);
#else
		UNREFERENCED_PARAMETER(logLevel);
		UNREFERENCED_PARAMETER(message);
#endif // _DEBUG
	}

	void VulkanSystem::postLogMessage(LogLevel logLevel, const std::string& message)
	{
#ifdef _DEBUG
		shpLoggingSystem_->postLogMessage(logLevel, message);
#else
		UNREFERENCED_PARAMETER(logLevel);
		UNREFERENCED_PARAMETER(message);
#endif // _DEBUG
	}

	void VulkanSystem::postLogMessage(LogLevel logLevel, const std::wstring& message)
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
	}

	void VulkanSystem::createInstance()
	{
		static vk::detail::DynamicLoader dynamicLoader;
		auto vkGetInstanceProcAddr = dynamicLoader.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
		VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

		sApplicationInfo_
			.setPApplicationName("Vulkan API практика")
			.setApplicationVersion(vk::makeApiVersion(0, 0, 1, 0))
			.setPEngineName("ak engine")
			.setEngineVersion(vk::makeApiVersion(0, 0, 1, 0))
			.setApiVersion(vk::makeApiVersion(0, 1, 3, 0));

		sInstanceCreateInfo_
			.setPApplicationInfo(&sApplicationInfo_)
			.setPEnabledLayerNames(instanceLayerNames_)
			.setPEnabledExtensionNames(instanceExtensionNames_);

#ifdef _DEBUG
		sDebugUtilsMessengerCreateInfo_
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

		sInstanceCreateInfo_
			.setPNext(&sDebugUtilsMessengerCreateInfo_);
#endif // _DEBUG

		if (!(uhInstance_ = vk::createInstanceUnique(sInstanceCreateInfo_)))
		{
			throw wexception(L"VulkanSystem::createInstance() : Невозможно создать экземпляр");
		}
		vk::detail::defaultDispatchLoaderDynamic.init(uhInstance_.get());
		postLogMessage(LogLevel::eAppInfo, L"VulkanSystem::createInstance() : Экземпляр создан");

#ifdef _DEBUG
		if (!(uhDebugUtilsMessenger_ = uhInstance_->createDebugUtilsMessengerEXTUnique(
			sDebugUtilsMessengerCreateInfo_, nullptr)))
		{
			throw wexception(L"VulkanSystem::createInstance() : Невозможно создать обработчик диагностических сообщений");
		}
		postLogMessage(LogLevel::eAppInfo, L"VulkanSystem::createInstance() : Обработчик диагностических сообщений создан");
#endif // _DEBUG
	}

	void VulkanSystem::createSurface()
	{
		sWin32SurfaceCreateInfo_
			.setHwnd(hWnd_)
			.setHinstance(GetModuleHandle(nullptr));

		if (!(uhSurface_ = uhInstance_->createWin32SurfaceKHRUnique(sWin32SurfaceCreateInfo_)))
		{
			throw wexception(L"VulkanSystem::createSurface() : Невозможно создать поверхность Win32");
		}
		postLogMessage(LogLevel::eAppInfo, L"VulkanSystem::createSurface() : Поверхность Win32 создана");
	}

	void VulkanSystem::pickPhysicalDevice()
	{
		enum class PhysicalDeviceCriteria
		{
			eGeometryShaderSupport = 0,
			eGraphicsAndPresentQueueFamilySupport = 1
		};

		const auto physicalDevices = uhInstance_->enumeratePhysicalDevices();

		if (!physicalDevices.size())
		{
			throw wexception(L"VulkanSystem::pickPhysicalDevice() : GPU с драйвером Vulkan API отсутствует");
		}

		std::vector<std::array<bool, 2>> PhysicalDevicesCriteria{};
		PhysicalDevicesCriteria.resize(physicalDevices.size());

		uint32_t physicalDeviceIdx{};
		for (auto& physicalDevice : physicalDevices)
		{
			//auto physicalDeviceSurfaceFormats = physicalDevice.getSurfaceFormatsKHR(uhSurface_.get());
			auto physicalDeviceProperties = physicalDevice.getProperties();
			auto physicalDeviceFeatures = physicalDevice.getFeatures();
			auto physicalDeviceQueueFamilyProperties = physicalDevice.getQueueFamilyProperties();

			PhysicalDevicesCriteria[physicalDeviceIdx][static_cast<int>(PhysicalDeviceCriteria::eGeometryShaderSupport)] =
				bool(physicalDeviceFeatures.geometryShader);

			uint32_t queueFamilyIdx{};

			int physicalDeviceQueueFamilyIdx{};
			for (auto& physicalDeviceQueueFamilyProperty : physicalDeviceQueueFamilyProperties)
			{
				PhysicalDevicesCriteria[physicalDeviceIdx][static_cast<int>(PhysicalDeviceCriteria::eGraphicsAndPresentQueueFamilySupport)] =
					bool(physicalDeviceQueueFamilyProperty.queueFlags & vk::QueueFlagBits::eGraphics);

				PhysicalDevicesCriteria[physicalDeviceIdx][static_cast<int>(PhysicalDeviceCriteria::eGraphicsAndPresentQueueFamilySupport)] &=
					bool(physicalDevice.getWin32PresentationSupportKHR(queueFamilyIdx));

				PhysicalDevicesCriteria[physicalDeviceIdx][static_cast<int>(PhysicalDeviceCriteria::eGraphicsAndPresentQueueFamilySupport)] &=
					bool(physicalDevice.getSurfaceSupportKHR(queueFamilyIdx, uhSurface_.get()));

				if (PhysicalDevicesCriteria[physicalDeviceIdx][static_cast<int>(PhysicalDeviceCriteria::eGraphicsAndPresentQueueFamilySupport)])
				{
					break;
				}
				queueFamilyIdx++;
			}

			if (std::find(PhysicalDevicesCriteria[physicalDeviceIdx].begin(), PhysicalDevicesCriteria[physicalDeviceIdx].end(), false) ==
				PhysicalDevicesCriteria[physicalDeviceIdx].end())
			{
				physicalDevice_ = physicalDevice;
				graphicsAndPresentQueueFamilyIdx_ = queueFamilyIdx;
				break;
			}
			physicalDeviceIdx++;
		}

		if (physicalDevice_)
		{
			sPhysicalDeviceProperties_ = physicalDevice_.getProperties();
			sPhysicalDeviceFeatures_ = physicalDevice_.getFeatures();

			postLogMessage(LogLevel::eAppInfo, std::string("VulkanSystem::pickPhysicalDevice() : GPU: ") +
				std::string(sPhysicalDeviceProperties_.deviceName.data()) + std::string("\r\n"));
			postLogMessage(LogLevel::eAppInfo, std::string("VulkanSystem::pickPhysicalDevice() : Семейство очередей GPU: ") +
				std::to_string(graphicsAndPresentQueueFamilyIdx_.value()) + std::string("\r\n"));
		}
		else
		{
			throw wexception(L"VulkanSystem::pickPhysicalDevice() : GPU с заданными критериями отсутствует");
		}
	}

	void VulkanSystem::createLogicalDevice()
	{
		const auto queuePriority = 1.0f;

		sDeviceQueueCreateInfos_.resize(1);
		sDeviceQueueCreateInfos_[0]
			.setQueueFamilyIndex(graphicsAndPresentQueueFamilyIdx_.value())
			.setQueueCount(1)
			.setQueuePriorities(queuePriority);

		sDeviceCreateInfo_
			.setPEnabledLayerNames(instanceLayerNames_)
			.setPEnabledExtensionNames(nullptr)
			.setPEnabledFeatures(&sPhysicalDeviceFeatures_)
			.setQueueCreateInfos(sDeviceQueueCreateInfos_);

		if (!(uhDevice_ = physicalDevice_.createDeviceUnique(sDeviceCreateInfo_)))
		{
			throw wexception(L"VulkanSystem::createLogicalDevice() : Невозможно создать логическое устройство");
		}
		postLogMessage(LogLevel::eAppInfo, L"VulkanSystem::createLogicalDevice() : Логическое устройство создано");

		queue_ = uhDevice_->getQueue(graphicsAndPresentQueueFamilyIdx_.value(), 0);
	}
}