#include "VulkanSystem.h"


namespace ak
{
	void VulkanSystem::createInstance()
	{
		sApplicationInfo_
			.setPApplicationName("Vulkan API ��������")
			.setApplicationVersion(vk::makeApiVersion(0, 0, 1, 0))
			.setPEngineName("ak engine")
			.setEngineVersion(vk::makeApiVersion(0, 0, 1, 0))
			.setApiVersion(vk::makeApiVersion(0, 1, 4, 0));

		sInstanceCreateInfo_
			.setPApplicationInfo(&sApplicationInfo_);

		if (!(uhInstance_ = vk::createInstanceUnique(sInstanceCreateInfo_)))
		{
			throw wexception(L"���������� ������� ���������");
		}

		std::cout << __FUNCSIG__ << " : ��������� ������" << std::endl; // MOVE TO DEBUG WINDOW
	}
}