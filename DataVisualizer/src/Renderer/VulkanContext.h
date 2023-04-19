#pragma once
#include "pcHeader.h"

#include "Window.h"

class VulkanContext
{
	// PUBLIC FUNCTIONS
public:
	VulkanContext() = default;
	~VulkanContext() = default;

	bool Init();
	[[nodiscard]] bool Update() const;
	void Shutdown() const;

	// PRIVATE FUNCTIONS
private:
	[[nodiscard]] vk::Instance CreateInstance() const;
	[[nodiscard]] bool VerifyLayers(const std::vector<const char*>& layerNames) const;
	[[nodiscard]] bool VerifyExtensions(const std::vector<const char*>& extensionNames) const;
	[[nodiscard]] vk::PhysicalDevice GetBestPhysicalDevice() const;
	[[nodiscard]] vk::Device CreateLogicalDevice();
	[[nodiscard]] vk::SurfaceKHR CreateSurface() const;

	// GETTERS AND SETTERS
public:
	vk::Instance& GetInstance() { return instance_; }
	vk::PhysicalDevice& GetPhysicalDevice() { return physicalDevice_; }
	vk::Device& GetLogicalDevice() { return logicalDevice_; }
	vk::SurfaceKHR& GetSurface() { return surface_; }
	Window& GetWindow() { return window_; }
	[[nodiscard]] uint32_t GetGraphicsQueueIndex() const { return graphicsQueueIndex_; }
	vk::Queue& GetGraphicsQueue() { return graphicsQueue_; }

	// MEMBER VARIABLES
private:
	vk::Instance instance_;
	vk::PhysicalDevice physicalDevice_;
	vk::Device logicalDevice_;
	vk::SurfaceKHR surface_;
	Window window_;

	uint32_t graphicsQueueIndex_;
	vk::Queue graphicsQueue_;
};
