#pragma once
#include "graphics_pipeline.h"
#include "LineMesh.h"
#include "swapchain.h"

class vulkan_context;

class renderer
{
public:
	renderer() = default;
	~renderer() = default;

	bool init(vulkan_context* vulkan_context);
	bool update();
	void shutdown();

	[[nodiscard]] vk::Buffer create_vertex_buffer(const std::vector<vertex>& points) const;
	[[nodiscard]] vk::DeviceMemory bind_vertex_buffer_memory(const vk::Buffer vertex_buffer, const std::vector<vertex>& points) const;
	void destroy_buffer(vk::Buffer buffer) const;
	void free_memory(vk::DeviceMemory memory) const;

	void draw(class line_component* line_component);

private:
	[[nodiscard]] vk::RenderPass create_render_pass() const;
	[[nodiscard]] vk::DescriptorSetLayout create_descriptor_set_layout() const;
	[[nodiscard]] vk::PipelineLayout create_pipeline_layout() const;
	[[nodiscard]] vk::Image create_depth_image() const;
	[[nodiscard]] vk::DeviceMemory bind_depth_image() const;
	[[nodiscard]] vk::ImageView create_depth_image_view() const;
	[[nodiscard]] std::vector<vk::Framebuffer> create_frame_buffers() const;
	[[nodiscard]] vk::CommandPool create_command_pool() const;
	[[nodiscard]] vk::Buffer create_uniform_buffer() const;
	[[nodiscard]] vk::DeviceMemory bind_uniform_buffer_memory() const;
	[[nodiscard]] vk::DescriptorPool create_descriptor_pool() const;
	[[nodiscard]] std::vector < vk::DescriptorSet> create_descriptor_set() const;
	void update_descriptor_sets();
	[[nodiscard]] std::vector <vk::CommandBuffer> create_command_buffer() const;
	[[nodiscard]] vk::Semaphore create_semaphore() const;
	[[nodiscard]] vk::Fence create_fence() const;

private:
	vulkan_context* vulkan_context_{};

	swapchain swapchain_;

	vk::RenderPass render_pass_;
	vk::DescriptorSetLayout descriptor_set_layout_;
	vk::PipelineLayout pipeline_layout_;

	graphics_pipeline line_pipeline_;

	vk::Image depth_image_;
	vk::DeviceMemory depth_image_memory_;
	vk::ImageView depth_image_view_;

	std::vector<vk::Framebuffer> frame_buffers_;

	vk::CommandPool command_pool_;

	vk::Buffer uniform_buffer_;
	vk::DeviceMemory uniform_buffer_memory_;

	vk::DescriptorPool descriptor_pool_;
	std::vector < vk::DescriptorSet> descriptor_sets_;

	std::vector <vk::CommandBuffer> command_buffers_;

	vk::Semaphore image_available_semaphore_;
	vk::Semaphore render_finished_semaphore_;
	vk::Fence in_flight_fence_;

	std::vector<line_component*> lines_to_draw_;

	vk::Viewport main_viewport_;
	vk::Viewport sidebar_viewport_;

	int count = 0;
};

