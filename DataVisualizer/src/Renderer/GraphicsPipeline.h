#pragma once
#include "pcHeader.h"

class GraphicsPipeline
{
public:
	GraphicsPipeline() = default;
	~GraphicsPipeline() = default;

	bool Init(const graphics_pipeline_create_info& createInfo);
	void Shutdown() const;

	void BindPipeline(vk::CommandBuffer commandBuffer) const;

public:
	[[nodiscard]] vk::Pipeline GetPipeline() const { return pipeline_; }

private:
	vk::Pipeline CreateGraphicsPipeline(const graphics_pipeline_create_info& createInfo) const;

	static std::vector<vk::PipelineShaderStageCreateInfo> CreateShaderStageInfos(const vk::Device& logicalDevice, const std::vector<shader_info>& shaderInfos, const std::vector<vk::ShaderModule>
		&
		shaderModules);
	static std::vector<vk::ShaderModule> LoadShaders(const vk::Device& logicalDevice, const std::vector<shader_info>& shaderInfos);
	static std::vector<vk::VertexInputBindingDescription> CreateVertexInputBindingDescriptions();
	static std::vector<vk::VertexInputAttributeDescription> CreateVertexInputAttributeDescriptions();
	static vk::PipelineVertexInputStateCreateInfo CreateVertexInputStateInfo(const std::vector<vk::VertexInputBindingDescription>& vertexInputBindingDescriptions, const std::vector<vk::
		                                                                             VertexInputAttributeDescription>& vertexInputAttributeDescriptions);
	static vk::PipelineInputAssemblyStateCreateInfo CreateInputAssemblyStateInfo(vk::PrimitiveTopology primitiveTopology);
	static vk::PipelineViewportStateCreateInfo CreateViewportStateInfo(const std::vector<vk::Viewport>& viewports, const std::vector<vk::Rect2D>& scissors);
	static vk::PipelineRasterizationStateCreateInfo CreateRasterizationStateInfo(vk::PolygonMode polygonMode, vk::CullModeFlagBits cullMode);
	static vk::PipelineDynamicStateCreateInfo CreateDynamicStateInfo(const std::vector<vk::DynamicState>& dynamicStates);
	static vk::PipelineMultisampleStateCreateInfo CreateMultiSampleStateInfo();
	static std::vector<vk::PipelineColorBlendAttachmentState> CreateColorBlendAttachmentStates();
	static vk::PipelineColorBlendStateCreateInfo CreateColorBlendStateInfo(const std::vector<vk::PipelineColorBlendAttachmentState>& colorBlendAttachmentStates);
	static vk::PipelineDepthStencilStateCreateInfo CreateDepthStencilStateInfo();

private:
	vk::Pipeline pipeline_{};
	vk::Device logicalDevice_{};
};

