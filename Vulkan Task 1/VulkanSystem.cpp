#include <iostream>
#include "VulkanSystem.h"


namespace ak
{
	void VulkanSystem::CreateInstance()
	{
		sApplicationInfo
			.setPApplicationName("Vulkan API практика")
			.setApplicationVersion(vk::makeApiVersion(0, 0, 1, 0))
			.setPEngineName("ak engine")
			.setEngineVersion(vk::makeApiVersion(0, 0, 1, 0))
			.setApiVersion(vk::makeApiVersion(0, 1, 4, 0));

		sInstanceCreateInfo
			.setPApplicationInfo(&sApplicationInfo);

		uhInstance = vk::createInstanceUnique(sInstanceCreateInfo);
	}
}