#pragma once
#include "GraphicsPipeline.h"
#include "imgui.h"
#include "swapchain.h"

class Renderer
{
public:
	Renderer() = default;
	~Renderer() = default;

	bool Init(VulkanContext* vulkanContext);
	bool Update();
	void Shutdown();

	[[nodiscard]] vk::Buffer CreateVertexBuffer(const std::vector<dv_math::Vertex>& points) const;
	[[nodiscard]] vk::DeviceMemory BindVertexBufferMemory(const vk::Buffer vertexBuffer, const std::vector<dv_math::Vertex>
	                                                      & points) const;
	void* MapMemory(vk::Buffer vertexBuffer, vk::DeviceMemory memory) const;
	void WriteToVertexBufferMemory(const vk::Buffer vertexBuffer, void* data, const std::vector<dv_math::Vertex>& points) const;
	void UnMapMemory(vk::DeviceMemory memory) const;
	void DestroyBuffer(vk::Buffer buffer) const;
	void FreeMemory(vk::DeviceMemory memory) const;

	void Draw(class LineComponent* lineComponent);

private:
	[[nodiscard]] vk::RenderPass CreateRenderPass() const;
	[[nodiscard]] vk::DescriptorSetLayout CreateDescriptorSetLayout() const;
	[[nodiscard]] vk::PipelineLayout CreatePipelineLayout() const;
	[[nodiscard]] vk::Image CreateDepthImage() const;
	[[nodiscard]] vk::Image CreateColourImage() const;
	[[nodiscard]] vk::DeviceMemory BindDepthImage() const;
	[[nodiscard]] vk::DeviceMemory BindColourImage() const;
	[[nodiscard]] vk::ImageView CreateDepthImageView() const;
	[[nodiscard]] vk::ImageView CreateColourImageView() const;
	[[nodiscard]] std::vector<vk::Framebuffer> CreateFrameBuffers() const;
	[[nodiscard]] vk::CommandPool CreateCommandPool() const;
	[[nodiscard]] vk::Buffer CreateUniformBuffer() const;
	[[nodiscard]] vk::DeviceMemory BindUniformBufferMemory() const;
	[[nodiscard]] vk::DescriptorPool CreateDescriptorPool() const;
	[[nodiscard]] vk::DescriptorPool CreateImGuiDescriptorPool() const;
	[[nodiscard]] std::vector < vk::DescriptorSet> CreateDescriptorSet() const;
	[[nodiscard]] std::vector <vk::CommandBuffer> CreateCommandBuffer() const;
	[[nodiscard]] vk::Semaphore CreateSemaphore() const;
	[[nodiscard]] vk::Fence CreateFence() const;

	void UpdateDescriptorSets();

public:
	void SetDrawData(ImDrawData* drawData) { drawData_ = drawData; }

	[[nodiscard]] VulkanContext* GetVulkanContext() const { return vulkanContext_; }
	[[nodiscard]] vk::DescriptorPool& GetImGuiDescriptorPool() { return imGuiDescriptorPool_; }
	[[nodiscard]] vk::RenderPass& GetRenderPass() { return renderPass_; }
	[[nodiscard]] vk::DescriptorPool& GetDescriptorPool() { return descriptorPool_; }
	[[nodiscard]] Swapchain& GetSwapChain() { return swapchain_; }
	[[nodiscard]] vk::CommandPool& GetCommandPool() { return commandPool_; }
	[[nodiscard]] std::vector<vk::CommandBuffer>& GetCommandBuffers() { return commandBuffers_; }
	[[nodiscard]] vk::Fence& GetInFlightFence() { return inFlightFence_; }

private:
	VulkanContext* vulkanContext_{};

	Swapchain swapchain_;

	vk::RenderPass renderPass_;
	vk::DescriptorSetLayout descriptorSetLayout_;
	vk::PipelineLayout pipelineLayout_;

	GraphicsPipeline linePipeline_;

	vk::Image depthImage_;
	vk::Image colourImage_;
	vk::DeviceMemory depthImageMemory_;
	vk::DeviceMemory colourImageMemory_;
	vk::ImageView depthImageView_;
	vk::ImageView colourImageView_;

	std::vector<vk::Framebuffer> frameBuffers_;

	vk::CommandPool commandPool_;

	vk::Buffer uniformBuffer_;
	vk::DeviceMemory uniformBufferMemory_;

	vk::DescriptorPool descriptorPool_;
	std::vector<vk::DescriptorSet> descriptorSets_;

	std::vector<vk::CommandBuffer> commandBuffers_;

	vk::Semaphore imageAvailableSemaphore_;
	vk::Semaphore renderFinishedSemaphore_;
	vk::Fence inFlightFence_;

	std::vector<LineComponent*> linesToDraw_;

	vk::Viewport mainViewport_;
	vk::Rect2D scissor_;

	vk::DescriptorPool imGuiDescriptorPool_;
	ImDrawData* drawData_{};

	uint32_t lastWindowHeight_;
	uint32_t lastWindowWidth_;
};

