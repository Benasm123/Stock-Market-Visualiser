#pragma once
#include "pcHeader.h"

class VulkanContext;

class Swapchain
{
public:
	Swapchain() = default;
	~Swapchain() = default;

	bool Init(VulkanContext* vulkanContext);
	void Recreate();
	void Shutdown();

private:
	struct SwapchainDetails
	{
		uint32_t imageCount;
		vk::SurfaceFormatKHR surfaceFormat;
		vk::Extent2D extent;
		vk::SurfaceTransformFlagBitsKHR surfaceTransform;
		vk::PresentModeKHR presentMode;
	};

	void DestroyOldSwapchain();

	[[nodiscard]] vk::SwapchainKHR CreateSwapchain();
	[[nodiscard]] SwapchainDetails GetSwapchainDetails() const;
	[[nodiscard]] uint32_t GetSwapchainImageCount() const;
	[[nodiscard]] vk::SurfaceFormatKHR GetSwapchainFormat() const;
	[[nodiscard]] vk::Extent2D GetSwapchainExtent() const;
	[[nodiscard]] vk::SurfaceTransformFlagBitsKHR GetSwapchainSurfaceTransform() const;
	[[nodiscard]] vk::PresentModeKHR GetSwapchainPresentMode() const;

	[[nodiscard]] std::vector<vk::Image> GetSwapchainImages() const;
	[[nodiscard]] std::vector<vk::ImageView> CreateSwapchainImageViews() const;

public:
	[[nodiscard]] const vk::SwapchainKHR& GetSwapchain() const { return swapchain_; }
	[[nodiscard]] const std::vector<vk::Image>& GetImages() const { return images_; }
	[[nodiscard]] const std::vector<vk::ImageView>& GetImageViews() const { return imageViews_; }
	[[nodiscard]] SwapchainDetails GetDetails() const { return swapchainDetails_; }

private:
	VulkanContext* vulkanContext_{};

	vk::SwapchainKHR swapchain_;
	vk::SwapchainKHR oldSwapchain_{};

	SwapchainDetails swapchainDetails_;

	std::vector<vk::Image> images_;
	std::vector<vk::ImageView> imageViews_;
};

