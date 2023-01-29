#include "pcHeader.h"
#include "swapchain.h"

#include "VulkanContext.h"

bool swapchain::init(VulkanContext* vulkan_context)
{
	vulkan_context_ = vulkan_context;

	swapchain_ = create_swapchain();

	images_ = get_swapchain_images();

	image_views_ = create_swapchain_image_views();

	LOG_INFO("Initialized Swapchain");
	return true;
}

void swapchain::recreate()
{
	vulkan_context_->get_logical_device().waitIdle();

	destroy_old_swapchain();

	old_swapchain_ = swapchain_;

	swapchain_ = create_swapchain();
	images_ = get_swapchain_images();
	image_views_ = create_swapchain_image_views();

	LOG_INFO("Recreated Swapchain");
}

void swapchain::shutdown()
{
	destroy_old_swapchain();

	vulkan_context_->get_logical_device().destroySwapchainKHR(swapchain_);

	LOG_INFO("Shutdown Swapchain");
}

void swapchain::destroy_old_swapchain()
{
	for (const auto& image_view : image_views_)
	{
		vulkan_context_->get_logical_device().destroyImageView(image_view);
	}
	image_views_.clear();
	images_.clear();

	vulkan_context_->get_logical_device().destroySwapchainKHR(old_swapchain_);
}

vk::SwapchainKHR swapchain::create_swapchain()
{
	swapchain_details_ = get_swapchain_details();

	const vk::SwapchainCreateInfoKHR swapchain_info(
		{},
		vulkan_context_->get_surface(),
		swapchain_details_.image_count,
		swapchain_details_.surface_format.format,
		swapchain_details_.surface_format.colorSpace,
		swapchain_details_.extent,
		1,
		vk::ImageUsageFlagBits::eColorAttachment,
		vk::SharingMode::eExclusive,
		0,
		nullptr,
		swapchain_details_.surface_transform,
		vk::CompositeAlphaFlagBitsKHR::eOpaque,
		swapchain_details_.present_mode,
		VK_TRUE,
		old_swapchain_
	);

	LOG_VULK("Created Swapchain");
	return vulkan_context_->get_logical_device().createSwapchainKHR(swapchain_info);
}

swapchain::swapchain_details swapchain::get_swapchain_details() const
{
	swapchain_details swapchain_details{};

	swapchain_details.image_count = get_swapchain_image_count();

	swapchain_details.surface_format = get_swapchain_format();

	swapchain_details.extent = get_swapchain_extent();

	swapchain_details.surface_transform = get_swapchain_surface_transform();

	swapchain_details.present_mode = get_swapchain_present_mode();

	return swapchain_details;
}

uint32_t swapchain::get_swapchain_image_count() const
{
	const vk::SurfaceCapabilitiesKHR surface_capabilities = vulkan_context_->get_physical_device().getSurfaceCapabilitiesKHR(vulkan_context_->get_surface());

	uint32_t image_count = surface_capabilities.minImageCount + 1;

	if (image_count > surface_capabilities.maxImageCount)
	{
		image_count = surface_capabilities.maxImageCount;
	}
	
	return image_count;
}

vk::SurfaceFormatKHR swapchain::get_swapchain_format() const
{
	const std::vector<vk::SurfaceFormatKHR> available_surface_formats = vulkan_context_->get_physical_device().getSurfaceFormatsKHR(vulkan_context_->get_surface());
	//Look for a specific surface_format, if not found just use the first available.
	for (const auto& available_surface_format : available_surface_formats)
	{
		if (available_surface_format.format == vk::Format::eB8G8R8A8Sint &&
			available_surface_format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
		{
			return available_surface_format;
		}
	}
	
	return available_surface_formats[0];
}

vk::Extent2D swapchain::get_swapchain_extent() const
{
	const vk::SurfaceCapabilitiesKHR surface_capabilities = vulkan_context_->get_physical_device().getSurfaceCapabilitiesKHR(vulkan_context_->get_surface());

	if (surface_capabilities.currentExtent.height != ~0u)
	{
		return surface_capabilities.currentExtent;
	}
	else
	{
		LOG_ERROR("Cant Get Surface Extent!");
		return { 0,0 };
	}
}

vk::SurfaceTransformFlagBitsKHR swapchain::get_swapchain_surface_transform() const
{
	const vk::SurfaceCapabilitiesKHR surface_capabilities = vulkan_context_->get_physical_device().getSurfaceCapabilitiesKHR(vulkan_context_->get_surface());

	return surface_capabilities.currentTransform;
}

vk::PresentModeKHR swapchain::get_swapchain_present_mode() const
{
	const std::vector<vk::PresentModeKHR> available_present_modes = vulkan_context_->get_physical_device().getSurfacePresentModesKHR(vulkan_context_->get_surface());

	for (const auto& available_present_mode : available_present_modes)
	{
		if (available_present_mode == vk::PresentModeKHR::eMailbox)
		{
			return available_present_mode;
		}
	}
	return available_present_modes[0];
	return vk::PresentModeKHR::eFifo;
}

std::vector<vk::Image> swapchain::get_swapchain_images() const
{
	return vulkan_context_->get_logical_device().getSwapchainImagesKHR(swapchain_);
}

std::vector<vk::ImageView> swapchain::create_swapchain_image_views() const
{
	std::vector<vk::ImageView> image_views = {};
	
	for (const auto& image : images_)
	{
		vk::ImageViewCreateInfo image_view_info(
			{},
			image,
			vk::ImageViewType::e2D,
			swapchain_details_.surface_format.format,
			{ // Components
				vk::ComponentSwizzle::eIdentity,
				vk::ComponentSwizzle::eIdentity,
				vk::ComponentSwizzle::eIdentity,
				vk::ComponentSwizzle::eIdentity
			},
			{ // Subresource Range
				vk::ImageAspectFlagBits::eColor,
				0,
				1,
				0,
				1
			}
		);

		image_views.push_back(vulkan_context_->get_logical_device().createImageView(image_view_info));
	}

	return image_views;
}
