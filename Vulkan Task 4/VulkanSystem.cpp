#include "VulkanSystem.h"
VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE


namespace ak
{
#ifdef _DEBUG
	void VulkanSystem::registerLogger(const std::shared_ptr<LoggerSystem>& shpLoggerSystem)
	{
		shpLoggerSystem_ = shpLoggerSystem;
	}
#endif // _DEBUG

	void VulkanSystem::createInstance()
	{
		static vk::detail::DynamicLoader dynamicLoader;
		PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = dynamicLoader.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
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
				//vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
				//vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
				vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
				vk::DebugUtilsMessageSeverityFlagBitsEXT::eError)
			.setMessageType(
				vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
				vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
				vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
				vk::DebugUtilsMessageTypeFlagBitsEXT::eDeviceAddressBinding)
			.setPfnUserCallback(&debugCallback_)
			.setPUserData(this);

		sInstanceCreateInfo_
			.setPNext(&sDebugUtilsMessengerCreateInfo_);
#endif // _DEBUG

		if (!(uhInstance_ = vk::createInstanceUnique(sInstanceCreateInfo_)))
		{
			throw wexception(L"VulkanSystem::createInstance() : Невозможно создать экземпляр");
		}

		postMessage(LoggerLevel::APP_INFO, L"VulkanSystem::createInstance() : Экземпляр создан");

		vk::detail::defaultDispatchLoaderDynamic.init(uhInstance_.get());

#ifdef _DEBUG
		if (!(uhDebugUtilsMessenger_ = uhInstance_->createDebugUtilsMessengerEXTUnique(
			sDebugUtilsMessengerCreateInfo_, nullptr)))
		{
			throw wexception(L"VulkanSystem::createInstance() : Невозможно создать обработчик диагностических сообщений");
		}

		postMessage(LoggerLevel::APP_INFO, L"VulkanSystem::createInstance() : Обработчик диагностических сообщений создан");
#endif // _DEBUG
	}

	void VulkanSystem::postMessage(LoggerLevel loggerLevel, const std::wstring& message)
	{
#ifdef _DEBUG
		shpLoggerSystem_->postMessage(loggerLevel, message);
#else
		UNREFERENCED_PARAMETER(loggerLevel);
		UNREFERENCED_PARAMETER(message);
#endif // _DEBUG
	}
}