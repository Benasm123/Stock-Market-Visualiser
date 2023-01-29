#pragma once
#include "pcHeader.h"

#include "Window.h"

class VulkanContext
{
// PUBLIC FUNCTIONS
public:
	VulkanContext() = default;
	~VulkanContext() = default;

	bool init();
	bool update() const;
	void shutdown() const;

// PRIVATE FUNCTIONS
private:
	[[nodiscard]] vk::Instance create_instance() const;
	[[nodiscard]] bool verify_layers(const std::vector<const char*>& layer_names) const;
	[[nodiscard]] bool verify_extensions(const std::vector<const char*>& extension_names) const;
	[[nodiscard]] vk::PhysicalDevice get_best_physical_device() const;
	[[nodiscard]] vk::Device create_logical_device();
	[[nodiscard]] vk::SurfaceKHR create_surface() const;

// GETTERS AND SETTERS
public:
	vk::Instance& get_instance() { return instance_; }
	vk::PhysicalDevice& get_physical_device() { return physical_device_; }
	vk::Device& get_logical_device() { return logical_device_; }
	vk::SurfaceKHR& get_surface() { return surface_; }
	window& get_window() { return window_; }
	[[nodiscard]] uint32_t get_graphics_queue_index() const { return graphics_queue_index_; }
	vk::Queue& get_graphics_queue() { return graphics_queue_; }

// MEMBER VARIABLES
private:
	vk::Instance instance_;
	vk::PhysicalDevice physical_device_;
	vk::Device logical_device_;
	vk::SurfaceKHR surface_;
	window window_;

	uint32_t graphics_queue_index_;
	vk::Queue graphics_queue_;
};

