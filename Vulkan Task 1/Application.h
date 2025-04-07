#pragma once

#include <memory>
#include "VulkanSystem.h"


namespace ak
{
	class Application final
	{
	public:
		void run();

	private:
		void initWin32Platform();
		void initVulkanSystem();
		void mainLoop();
		void cleanup();

		std::unique_ptr<VulkanSystem> upVulkanSystem{};
	};
}