#include "pcHeader.h"
#include "VulkanContext.h"

#include "Window.h"

bool vulkan_context::init()
{
	instance_ = create_instance();
	if (!instance_)
	{
		return false;
	}

	physical_device_ = get_best_physical_device();
	if (!physical_device_)
	{
		return false;
	}

	logical_device_ = create_logical_device();
	if (!logical_device_)
	{
		return false;
	}

	graphics_queue_ = logical_device_.getQueue(graphics_queue_index_, 0);

	window_.init(L"Data Visualization", 1920, 1080);

	surface_ = create_surface();
	if (!surface_)
	{
		return false;
	}

	LOG_INFO("Initialized Vulkan Context");
	return true;
}

bool vulkan_context::update() const
{
	bool successful = true;

	successful &= window_.update();

	return successful;
}

void vulkan_context::shutdown() const
{
	logical_device_.waitIdle();

	instance_.destroySurfaceKHR(surface_);
	window_.shutdown();
	logical_device_.destroy();
	instance_.destroy();

	LOG_INFO("Shutdown Vulkan Context");
}

vk::Instance vulkan_context::create_instance() const
{

	const std::vector<const char*> layer_names = {

#ifndef DV_RELEASE

		"VK_LAYER_KHRONOS_validation",
		"VK_LAYER_LUNARG_monitor"

#endif

	};

	const std::vector<const char*> extension_names = {
		VK_KHR_SURFACE_EXTENSION_NAME,
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
	};

	if (!verify_layers(layer_names))
	{
		return nullptr;
	}

	if (!verify_extensions(extension_names))
	{
		return nullptr;
	}

	constexpr vk::ApplicationInfo app_info(
		ENGINE_NAME,
		APPLICATION_VERSION,
		ENGINE_NAME,
		ENGINE_VERSION,
		VK_API_VERSION_1_1
	);

	const vk::InstanceCreateInfo instance_info(
		{},
		&app_info,
		static_cast<uint32_t>(layer_names.size()),
		layer_names.data(),
		static_cast<uint32_t>(extension_names.size()),
		extension_names.data()
	);

	LOG_INFO("Created Instance");
	return vk::createInstance(instance_info);
}

bool vulkan_context::verify_layers(const std::vector<const char*>& layer_names) const
{
	std::vector<vk::LayerProperties> supported_layers = vk::enumerateInstanceLayerProperties();

	for (const auto& layer_name : layer_names)
	{
		bool layer_found = false;
		for (auto& supported_layer : supported_layers)
		{
			if (strcmp(layer_name, supported_layer.layerName) == 0)
			{
				layer_found = true;
			}
		}
		if (!layer_found)
		{
			LOG_ERROR("UNSUPPORTED LAYER: %s", layer_name);
			return false;
		}
	}

	return true;
}

bool vulkan_context::verify_extensions(const std::vector<const char*>& extension_names) const
{
	//TODO::This currently checks all base layer extensions, but there are more available at each layer.
	//TODO::Need to pass layers used and add extensions for those layers here as well.
	std::vector<vk::ExtensionProperties> supported_extensions = vk::enumerateInstanceExtensionProperties();

	std::vector<vk::LayerProperties> supported_layers = vk::enumerateInstanceLayerProperties();

	for (const auto& extension_name : extension_names)
	{
		bool extension_found = false;
		for (auto& supported_extension : supported_extensions)
		{
			if (strcmp(extension_name, supported_extension.extensionName) == 0)
			{
				extension_found = true;
			}
		}
		if (!extension_found)
		{
			LOG_ERROR("UNSUPPORTED EXTENSION: %s", extension_name);
			return false;
		}
	}

	return true;
}

vk::PhysicalDevice vulkan_context::get_best_physical_device() const
{
	const std::vector<vk::PhysicalDevice> physical_devices_available = instance_.enumeratePhysicalDevices();

	if (physical_devices_available.empty())
	{
		LOG_ERROR("FOUND NO VULKAN SUPPORTED DEVICES");
		return nullptr;
	}

	vk::PhysicalDevice best_device = nullptr;

	//TODO::Add better checks here for best GPU, although this is low priority for this application.
	for (const auto& physical_device : physical_devices_available)
	{
		if (!best_device)
		{
			best_device = physical_device;
			if (best_device.getProperties().deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
			{
				break;
			}
			continue;
		}

		if (physical_device.getProperties().deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
		{
			best_device = physical_device;
			break;
		}
	}

	LOG_INFO("Using GPU: %s", best_device.getProperties().deviceName.data());
	return best_device;
}


vk::Device vulkan_context::create_logical_device()
{
	const std::vector<vk::QueueFamilyProperties> queue_family_properties = physical_device_.getQueueFamilyProperties();

	uint32_t graphics_queue_family_index = ~0u;

	uint32_t index = 0;
	for (auto& queue_family_property : queue_family_properties)
	{
		if (queue_family_property.queueFlags & vk::QueueFlagBits::eGraphics &&
			physical_device_.getWin32PresentationSupportKHR(index))
		{
			graphics_queue_family_index = index;
			break;
		}
		index++;
	}

	if (graphics_queue_family_index == ~0u)
	{
		LOG_ERROR("NO SUPPORTED GRAPHICS QUEUE FOUND");
		return nullptr;
	}

	graphics_queue_index_ = graphics_queue_family_index;

	float priorities[] = { 1.0f };

	const std::vector<vk::DeviceQueueCreateInfo> queue_infos = {
		{
			{},
			graphics_queue_family_index,
			1,
			priorities
		}
	};

	const std::vector<const char*> layer_names = {
#ifndef DV_RELEASE
		"VK_LAYER_KHRONOS_validation",
		"VK_LAYER_LUNARG_monitor"
#endif
	};

	const std::vector<const char*> extension_names = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	vk::PhysicalDeviceFeatures features{};
	features.multiViewport = VK_TRUE;

	const vk::DeviceCreateInfo device_info(
		{},
		static_cast<uint32_t>(queue_infos.size()),
		queue_infos.data(),
		static_cast<uint32_t>(layer_names.size()),
		layer_names.data(),
		static_cast<uint32_t>(extension_names.size()),
		extension_names.data(),
		&features
	);

	LOG_INFO("Created Logical Device");
	return physical_device_.createDevice(device_info);
}

vk::SurfaceKHR vulkan_context::create_surface() const
{
	const vk::Win32SurfaceCreateInfoKHR surface_info(
		{},
		window_.get_hinstance(),
		window_.get_hwnd()
	);

	LOG_INFO("Created Win32 Surface");
	return instance_.createWin32SurfaceKHR(surface_info);
}
