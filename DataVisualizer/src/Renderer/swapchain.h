#pragma once
#include "pcHeader.h"

class VulkanContext;

class swapchain
{
public:
	swapchain() = default;
	~swapchain() = default;

	bool init(VulkanContext* vulkan_context);
	void recreate();
	void shutdown();

private:
	struct swapchain_details
	{
		uint32_t image_count;
		vk::SurfaceFormatKHR surface_format;
		vk::Extent2D extent;
		vk::SurfaceTransformFlagBitsKHR surface_transform;
		vk::PresentModeKHR present_mode;
	};

	void destroy_old_swapchain();

	[[nodiscard]] vk::SwapchainKHR create_swapchain();
	[[nodiscard]] swapchain_details get_swapchain_details() const;
	[[nodiscard]] uint32_t get_swapchain_image_count() const;
	[[nodiscard]] vk::SurfaceFormatKHR get_swapchain_format() const;
	[[nodiscard]] vk::Extent2D get_swapchain_extent() const;
	[[nodiscard]] vk::SurfaceTransformFlagBitsKHR get_swapchain_surface_transform() const;
	[[nodiscard]] vk::PresentModeKHR get_swapchain_present_mode() const;

	[[nodiscard]] std::vector<vk::Image> get_swapchain_images() const;
	[[nodiscard]] std::vector<vk::ImageView> create_swapchain_image_views() const;

public:
	[[nodiscard]] const vk::SwapchainKHR& get_swapchain() const { return swapchain_; }
	[[nodiscard]] const std::vector<vk::Image>& get_images() const { return images_; }
	[[nodiscard]] const std::vector<vk::ImageView>& get_image_views() const { return image_views_; }
	[[nodiscard]] swapchain_details get_details() const { return swapchain_details_; }

private:
	VulkanContext* vulkan_context_{};

	vk::SwapchainKHR swapchain_;
	vk::SwapchainKHR old_swapchain_{};

	swapchain_details swapchain_details_;

	std::vector<vk::Image> images_;
	std::vector<vk::ImageView> image_views_;
};

