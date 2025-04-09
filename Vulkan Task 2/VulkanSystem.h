#pragma once

#include <iostream>
#include <string>
#include <memory>

#include <vulkan/vulkan.hpp>

#include "wexception.h"


namespace ak
{
	class VulkanSystem final
	{
	public:
		VulkanSystem() {};
		explicit VulkanSystem(const VulkanSystem& obj) = delete;
		~VulkanSystem() {};
		VulkanSystem& operator=(const VulkanSystem& obj) = delete;

		void createInstance();
		void setupDebugMessenger();
		
	private:
		vk::ApplicationInfo sApplicationInfo_{};
		vk::InstanceCreateInfo sInstanceCreateInfo_{};
		vk::UniqueInstance uhInstance_{};
	};
}