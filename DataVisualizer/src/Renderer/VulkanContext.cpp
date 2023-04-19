#include "pcHeader.h"
#include "VulkanContext.h"

#include "Window.h"

bool VulkanContext::Init()
{
	LOG_FUNC_START();
	instance_ = CreateInstance();
	if (!instance_)
	{
		return false;
	}

	physicalDevice_ = GetBestPhysicalDevice();
	if (!physicalDevice_)
	{
		return false;
	}

	logicalDevice_ = CreateLogicalDevice();
	if (!logicalDevice_)
	{
		return false;
	}

	graphicsQueue_ = logicalDevice_.getQueue(graphicsQueueIndex_, 0);

	window_.Init(L"Data Visualization", 1800, 900);

	surface_ = CreateSurface();
	if (!surface_)
	{
		return false;
	}

	LOG_FUNC_END();
	return true;
}

bool VulkanContext::Update() const
{
	bool successful = true;

	successful &= window_.Update();

	return successful;
}

void VulkanContext::Shutdown() const
{
	LOG_FUNC_START();
	logicalDevice_.waitIdle();

	instance_.destroySurfaceKHR(surface_);
	window_.Shutdown();
	logicalDevice_.destroy();
	instance_.destroy();

	LOG_FUNC_END();
}

vk::Instance VulkanContext::CreateInstance() const
{
	LOG_FUNC_START();
	const std::vector<const char*> layerNames = {

#ifndef DV_RELEASE

		"VK_LAYER_KHRONOS_validation",
		"VK_LAYER_LUNARG_monitor"

#endif

	};

	const std::vector<const char*> extensionNames = {
		VK_KHR_SURFACE_EXTENSION_NAME,
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
	};

	if (!VerifyLayers(layerNames))
	{
		return nullptr;
	}

	if (!VerifyExtensions(extensionNames))
	{
		return nullptr;
	}

	constexpr vk::ApplicationInfo appInfo(
		kEngineName,
		kApplicationVersion,
		kEngineName,
		kEngineVersion,
		VK_API_VERSION_1_1
	);

	const vk::InstanceCreateInfo instanceInfo(
		{},
		&appInfo,
		static_cast<uint32_t>(layerNames.size()),
		layerNames.data(),
		static_cast<uint32_t>(extensionNames.size()),
		extensionNames.data()
	);

	LOG_FUNC_END();
	return vk::createInstance(instanceInfo);
}

bool VulkanContext::VerifyLayers(const std::vector<const char*>& layerNames) const
{
	LOG_FUNC_START();
	std::vector<vk::LayerProperties> supportedLayers = vk::enumerateInstanceLayerProperties();

	for (const auto& layerName : layerNames)
	{
		bool layerFound = false;
		for (auto& supportedLayer : supportedLayers)
		{
			if (strcmp(layerName, supportedLayer.layerName) == 0)
			{
				layerFound = true;
			}
		}
		if (!layerFound)
		{
			LOG_ERROR("UNSUPPORTED LAYER: %s", layerName);
			return false;
		}
	}

	LOG_FUNC_END();
	return true;
}

bool VulkanContext::VerifyExtensions(const std::vector<const char*>& extensionNames) const
{
	LOG_FUNC_START();
	//TODO::This currently checks all base layer extensions, but there are more available at each layer.
	//TODO::Need to pass layers used and add extensions for those layers here as well.
	std::vector<vk::ExtensionProperties> supportedExtensions = vk::enumerateInstanceExtensionProperties();

	std::vector<vk::LayerProperties> supportedLayers = vk::enumerateInstanceLayerProperties();

	for (const auto& extensionName : extensionNames)
	{
		bool extensionFound = false;
		for (auto& supportedExtension : supportedExtensions)
		{
			if (strcmp(extensionName, supportedExtension.extensionName) == 0)
			{
				extensionFound = true;
			}
		}
		if (!extensionFound)
		{
			LOG_ERROR("UNSUPPORTED EXTENSION: %s", extensionName);
			return false;
		}
	}

	LOG_FUNC_END();
	return true;
}

vk::PhysicalDevice VulkanContext::GetBestPhysicalDevice() const
{
	LOG_FUNC_START();
	const std::vector<vk::PhysicalDevice> physicalDevicesAvailable = instance_.enumeratePhysicalDevices();

	if (physicalDevicesAvailable.empty())
	{
		LOG_ERROR("FOUND NO VULKAN SUPPORTED DEVICES");
		return nullptr;
	}

	vk::PhysicalDevice bestDevice = nullptr;

	//TODO::Add better checks here for best GPU, although this is low priority for this application.
	for (const auto& physicalDevice : physicalDevicesAvailable)
	{
		if (!bestDevice)
		{
			bestDevice = physicalDevice;
			if (bestDevice.getProperties().deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
			{
				break;
			}
			continue;
		}

		if (physicalDevice.getProperties().deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
		{
			bestDevice = physicalDevice;
			break;
		}
	}
	
	LOG_INFO("Using GPU: %s", bestDevice.getProperties().deviceName.data());
	LOG_FUNC_END();
	return bestDevice;
}


vk::Device VulkanContext::CreateLogicalDevice()
{
	LOG_FUNC_START();
	const std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice_.getQueueFamilyProperties();

	uint32_t graphicsQueueFamilyIndex = ~0u;

	uint32_t index = 0;
	for (auto& queueFamilyProperty : queueFamilyProperties)
	{
		if (queueFamilyProperty.queueFlags & vk::QueueFlagBits::eGraphics &&
			physicalDevice_.getWin32PresentationSupportKHR(index))
		{
			graphicsQueueFamilyIndex = index;
			break;
		}
		index++;
	}

	if (graphicsQueueFamilyIndex == ~0u)
	{
		LOG_ERROR("NO SUPPORTED GRAPHICS QUEUE FOUND");
		return nullptr;
	}

	graphicsQueueIndex_ = graphicsQueueFamilyIndex;

	float priorities[] = { 1.0f };

	const std::vector<vk::DeviceQueueCreateInfo> queueInfos = {
		{
			{},
			graphicsQueueFamilyIndex,
			1,
			priorities
		}
	};

	const std::vector<const char*> layerNames = {
#ifndef DV_RELEASE
		"VK_LAYER_KHRONOS_validation",
		"VK_LAYER_LUNARG_monitor"
#endif
	};

	const std::vector<const char*> extensionNames = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	vk::PhysicalDeviceFeatures features{};
	// features.multiViewport = VK_TRUE;
	features.setWideLines(VK_TRUE);

	const vk::DeviceCreateInfo deviceInfo(
		{},
		static_cast<uint32_t>(queueInfos.size()),
		queueInfos.data(),
		static_cast<uint32_t>(layerNames.size()),
		layerNames.data(),
		static_cast<uint32_t>(extensionNames.size()),
		extensionNames.data(),
		&features
	);

	LOG_FUNC_END();
	return physicalDevice_.createDevice(deviceInfo);
}

vk::SurfaceKHR VulkanContext::CreateSurface() const
{
	LOG_FUNC_START();
	const vk::Win32SurfaceCreateInfoKHR surfaceInfo(
		{},
		window_.GetHInstance(),
		window_.GetHwnd()
	);

	LOG_FUNC_END();
	return instance_.createWin32SurfaceKHR(surfaceInfo);
}
