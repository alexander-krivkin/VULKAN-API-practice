#pragma once

#include <vulkan/vulkan.hpp>


namespace ak
{
	class VulkanSystem final
	{
	public:
		VulkanSystem() {};
		explicit VulkanSystem(const VulkanSystem& obj) = delete;
		~VulkanSystem() {};
		VulkanSystem& operator=(const VulkanSystem& obj) = delete;

		void CreateInstance();

	private:
		vk::ApplicationInfo sApplicationInfo{};
		vk::InstanceCreateInfo sInstanceCreateInfo{};
		vk::UniqueInstance uhInstance{};
	};
}