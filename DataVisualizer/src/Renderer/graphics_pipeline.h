#pragma once
#include "pcHeader.h"

class graphics_pipeline
{
public:
	graphics_pipeline() = default;
	~graphics_pipeline() = default;

	bool init(const graphics_pipeline_create_info& create_info);
	void shutdown() const;

	void bind_pipeline(vk::CommandBuffer command_buffer) const;

public:
	vk::Pipeline get_pipeline() { return pipeline_; }

private:
	vk::Pipeline create_graphics_pipeline(const graphics_pipeline_create_info& create_info);
	
	std::vector<vk::PipelineShaderStageCreateInfo> create_shader_stage_infos(const vk::Device& logical_device, const std::vector<shader_info>& shader_infos, const std::vector<vk::ShaderModule>
	                                                                         shader_modules);
	static std::vector<vk::ShaderModule> load_shaders(const vk::Device& logical_device, const std::vector<shader_info>& shader_infos);
	static std::vector<vk::VertexInputBindingDescription> create_vertex_input_binding_descriptions();
	static std::vector<vk::VertexInputAttributeDescription> create_vertex_input_attribute_descriptions();
	static vk::PipelineVertexInputStateCreateInfo create_vertex_input_state_info(const std::vector<vk::VertexInputBindingDescription>& vertex_input_binding_descriptions, const std::vector<vk::
		                                                                             VertexInputAttributeDescription>& vertex_input_attribute_descriptions);
	static vk::PipelineInputAssemblyStateCreateInfo create_input_assembly_state_info(vk::PrimitiveTopology primitive_topology);
	static vk::PipelineViewportStateCreateInfo create_viewport_state_info(const std::vector<vk::Viewport>& viewports, const std::vector<vk::Rect2D>& scissors);
	static vk::PipelineRasterizationStateCreateInfo create_rasterization_state_info(vk::PolygonMode polygon_mode, vk::CullModeFlagBits cull_mode);
	static vk::PipelineDynamicStateCreateInfo create_dynamic_state_info(const std::vector<vk::DynamicState>& dynamic_states);
	static vk::PipelineMultisampleStateCreateInfo create_multisample_state_info();
	static std::vector<vk::PipelineColorBlendAttachmentState> create_color_blend_attachment_states();
	static vk::PipelineColorBlendStateCreateInfo create_color_blend_state_info(const std::vector<vk::PipelineColorBlendAttachmentState>& color_blend_attachment_states);
	static vk::PipelineDepthStencilStateCreateInfo create_depth_stencil_state_info();

private:
	vk::Pipeline pipeline_{};
	vk::Device logical_device_{};
};

