#include "VulkanSystem.h"


namespace ak
{
	void VulkanSystem::registerLogger(const std::shared_ptr<LoggerSystem>& shpLoggerSystem)
	{
		shpLoggerSystem_ = shpLoggerSystem;
	}

	void VulkanSystem::createInstance()
	{
		sApplicationInfo_
			.setPApplicationName("Vulkan API практика")
			.setApplicationVersion(vk::makeApiVersion(0, 0, 1, 0))
			.setPEngineName("ak engine")
			.setEngineVersion(vk::makeApiVersion(0, 0, 1, 0))
			.setApiVersion(vk::makeApiVersion(0, 1, 4, 0));

		sInstanceCreateInfo_
			.setPApplicationInfo(&sApplicationInfo_)
			.setPEnabledLayerNames(instanceLayerNames_)
			.setPEnabledExtensionNames(instanceExtensionNames_);

		if (!(uhInstance_ = vk::createInstanceUnique(sInstanceCreateInfo_)))
		{
			throw wexception(L"Невозможно создать экземпляр");
		}

		shpLoggerSystem_->postMessage(LoggerLevel::LOG_INFO, L"VulkanSystem::createInstance() : Экземпляр создан");
	}
}