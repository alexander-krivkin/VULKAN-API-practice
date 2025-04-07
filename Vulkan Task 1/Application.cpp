#include "Application.h"


namespace ak
{
	void Application::run()
	{
		initWin32Platform();
		initVulkanSystem();
		mainLoop();
		cleanup();
	}

	void Application::initVulkanSystem()
	{
		upVulkanSystem = std::unique_ptr<VulkanSystem>(new VulkanSystem);
		upVulkanSystem->CreateInstance();
	}

	void Application::mainLoop()
	{

	}

	void Application::cleanup()
	{

	}
}