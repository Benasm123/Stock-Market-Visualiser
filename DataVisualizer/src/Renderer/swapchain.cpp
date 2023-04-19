#include "pcHeader.h"
#include "swapchain.h"

#include "VulkanContext.h"

bool Swapchain::Init(VulkanContext* vulkanContext)
{
	vulkanContext_ = vulkanContext;

	swapchain_ = CreateSwapchain();

	images_ = GetSwapchainImages();

	imageViews_ = CreateSwapchainImageViews();

	LOG_INFO("Initialized Swapchain");
	return true;
}

void Swapchain::Recreate()
{
	vulkanContext_->GetLogicalDevice().waitIdle();

	DestroyOldSwapchain();

	oldSwapchain_ = swapchain_;

	swapchain_ = CreateSwapchain();
	images_ = GetSwapchainImages();
	imageViews_ = CreateSwapchainImageViews();

	LOG_INFO("Recreated Swapchain");
}

void Swapchain::Shutdown()
{
	DestroyOldSwapchain();

	vulkanContext_->GetLogicalDevice().destroySwapchainKHR(swapchain_);

	LOG_INFO("Shutdown Swapchain");
}

void Swapchain::DestroyOldSwapchain()
{
	for (const auto& imageView : imageViews_)
	{
		vulkanContext_->GetLogicalDevice().destroyImageView(imageView);
	}
	imageViews_.clear();
	images_.clear();

	vulkanContext_->GetLogicalDevice().destroySwapchainKHR(oldSwapchain_);
}

vk::SwapchainKHR Swapchain::CreateSwapchain()
{
	swapchainDetails_ = GetSwapchainDetails();

	const vk::SwapchainCreateInfoKHR swapchainInfo(
		{},
		vulkanContext_->GetSurface(),
		swapchainDetails_.imageCount,
		swapchainDetails_.surfaceFormat.format,
		swapchainDetails_.surfaceFormat.colorSpace,
		swapchainDetails_.extent,
		1,
		vk::ImageUsageFlagBits::eColorAttachment,
		vk::SharingMode::eExclusive,
		0,
		nullptr,
		swapchainDetails_.surfaceTransform,
		vk::CompositeAlphaFlagBitsKHR::eOpaque,
		swapchainDetails_.presentMode,
		VK_TRUE,
		oldSwapchain_
	);

	LOG_VULK("Created Swapchain");
	return vulkanContext_->GetLogicalDevice().createSwapchainKHR(swapchainInfo);
}

Swapchain::SwapchainDetails Swapchain::GetSwapchainDetails() const
{
	SwapchainDetails swapchainDetails{};

	swapchainDetails.imageCount = GetSwapchainImageCount();

	swapchainDetails.surfaceFormat = GetSwapchainFormat();

	swapchainDetails.extent = GetSwapchainExtent();

	swapchainDetails.surfaceTransform = GetSwapchainSurfaceTransform();

	swapchainDetails.presentMode = GetSwapchainPresentMode();

	return swapchainDetails;
}

uint32_t Swapchain::GetSwapchainImageCount() const
{
	const vk::SurfaceCapabilitiesKHR surfaceCapabilities = vulkanContext_->GetPhysicalDevice().getSurfaceCapabilitiesKHR(vulkanContext_->GetSurface());

	uint32_t imageCount = surfaceCapabilities.minImageCount + 1;

	if (imageCount > surfaceCapabilities.maxImageCount)
	{
		imageCount = surfaceCapabilities.maxImageCount;
	}
	
	return imageCount;
}

vk::SurfaceFormatKHR Swapchain::GetSwapchainFormat() const
{
	const std::vector<vk::SurfaceFormatKHR> availableSurfaceFormats = vulkanContext_->GetPhysicalDevice().getSurfaceFormatsKHR(vulkanContext_->GetSurface());
	//Look for a specific surface_format, if not found just use the first available.
	for (const auto& availableSurfaceFormat : availableSurfaceFormats)
	{
		if (availableSurfaceFormat.format == vk::Format::eB8G8R8A8Sint &&
			availableSurfaceFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
		{
			return availableSurfaceFormat;
		}
	}
	
	return availableSurfaceFormats[0];
}

vk::Extent2D Swapchain::GetSwapchainExtent() const
{
	const vk::SurfaceCapabilitiesKHR surfaceCapabilities = vulkanContext_->GetPhysicalDevice().getSurfaceCapabilitiesKHR(vulkanContext_->GetSurface());

	if (surfaceCapabilities.currentExtent.height != ~0u)
	{
		return surfaceCapabilities.currentExtent;
	}
	else
	{
		LOG_ERROR("Cant Get Surface Extent!");
		return { 0,0 };
	}
}

vk::SurfaceTransformFlagBitsKHR Swapchain::GetSwapchainSurfaceTransform() const
{
	const vk::SurfaceCapabilitiesKHR surfaceCapabilities = vulkanContext_->GetPhysicalDevice().getSurfaceCapabilitiesKHR(vulkanContext_->GetSurface());

	return surfaceCapabilities.currentTransform;
}

vk::PresentModeKHR Swapchain::GetSwapchainPresentMode() const
{
	const std::vector<vk::PresentModeKHR> availablePresentModes = vulkanContext_->GetPhysicalDevice().getSurfacePresentModesKHR(vulkanContext_->GetSurface());

	for (const auto& availablePresentMode : availablePresentModes)
	{
		if (availablePresentMode == vk::PresentModeKHR::eMailbox)
		{
			return availablePresentMode;
		}
	}
	return availablePresentModes[0];
	return vk::PresentModeKHR::eFifo;
}

std::vector<vk::Image> Swapchain::GetSwapchainImages() const
{
	return vulkanContext_->GetLogicalDevice().getSwapchainImagesKHR(swapchain_);
}

std::vector<vk::ImageView> Swapchain::CreateSwapchainImageViews() const
{
	std::vector<vk::ImageView> imageViews = {};
	
	for (const auto& image : images_)
	{
		vk::ImageViewCreateInfo imageViewInfo(
			{},
			image,
			vk::ImageViewType::e2D,
			swapchainDetails_.surfaceFormat.format,
			{ // Components
				vk::ComponentSwizzle::eIdentity,
				vk::ComponentSwizzle::eIdentity,
				vk::ComponentSwizzle::eIdentity,
				vk::ComponentSwizzle::eIdentity
			},
			{ // Sub resource Range
				vk::ImageAspectFlagBits::eColor,
				0,
				1,
				0,
				1
			}
		);

		imageViews.push_back(vulkanContext_->GetLogicalDevice().createImageView(imageViewInfo));
	}

	return imageViews;
}
