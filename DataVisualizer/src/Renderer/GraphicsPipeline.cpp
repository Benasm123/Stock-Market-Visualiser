#include "pcHeader.h"
#include "GraphicsPipeline.h"

bool GraphicsPipeline::Init(const graphics_pipeline_create_info& createInfo)
{
	LOG_FUNC_START();
	logicalDevice_ = createInfo.logical_device;

	pipeline_ = CreateGraphicsPipeline(createInfo);
	if (!pipeline_)
	{
		LOG_INFO("Failed To Initialize Graphics Pipeline");
		return false;
	}
	
	LOG_FUNC_END();
	return true;
}

void GraphicsPipeline::Shutdown() const
{
	LOG_FUNC_START();
	logicalDevice_.destroyPipeline(pipeline_);
	LOG_FUNC_END();
}

void GraphicsPipeline::BindPipeline(const vk::CommandBuffer commandBuffer) const
{
	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline_);
}

vk::Pipeline GraphicsPipeline::CreateGraphicsPipeline(const graphics_pipeline_create_info& createInfo) const
{
	LOG_FUNC_START();
	const std::vector<vk::ShaderModule> shaderModules = LoadShaders(logicalDevice_, createInfo.shader_infos);

	std::vector<vk::PipelineShaderStageCreateInfo> shaderStageInfos = CreateShaderStageInfos(createInfo.logical_device, createInfo.shader_infos, shaderModules);
	
	std::vector<vk::VertexInputBindingDescription> vertexInputBindingDescriptions = CreateVertexInputBindingDescriptions();

	std::vector<vk::VertexInputAttributeDescription> vertexInputAttributeDescriptions = CreateVertexInputAttributeDescriptions();

	vk::PipelineVertexInputStateCreateInfo vertexInputStateInfo = CreateVertexInputStateInfo(vertexInputBindingDescriptions, vertexInputAttributeDescriptions);

	vk::PipelineInputAssemblyStateCreateInfo inputAssemblyStateInfo = CreateInputAssemblyStateInfo(createInfo.primitive_topology);

	vk::PipelineViewportStateCreateInfo viewportStateInfo = CreateViewportStateInfo(createInfo.viewports, createInfo.scissors);

	vk::PipelineRasterizationStateCreateInfo rasterizationStateInfo = CreateRasterizationStateInfo(createInfo.polygon_mode, createInfo.cull_mode);

	vk::PipelineDynamicStateCreateInfo dynamicStateInfo = CreateDynamicStateInfo(createInfo.dynamic_states);

	vk::PipelineMultisampleStateCreateInfo multiSampleStateInfo = CreateMultiSampleStateInfo();

	std::vector<vk::PipelineColorBlendAttachmentState> colorBlendAttachmentStates = CreateColorBlendAttachmentStates();

	vk::PipelineColorBlendStateCreateInfo colorBlendStateInfo = CreateColorBlendStateInfo(colorBlendAttachmentStates);

	vk::PipelineDepthStencilStateCreateInfo depthStencilStateInfo = CreateDepthStencilStateInfo();

	const vk::GraphicsPipelineCreateInfo graphicsPipelineInfo(
		{},
		static_cast<uint32_t>(shaderStageInfos.size()),
		shaderStageInfos.data(),
		&vertexInputStateInfo,
		&inputAssemblyStateInfo,
		nullptr, // Tessellation Stage
		&viewportStateInfo,
		&rasterizationStateInfo,
		&multiSampleStateInfo,
		&depthStencilStateInfo,
		&colorBlendStateInfo,
		&dynamicStateInfo,
		createInfo.pipeline_layout,
		createInfo.render_pass,
		0,
		VK_NULL_HANDLE,
		0
	);

	vk::ResultValue<vk::Pipeline> graphicsPipelineResult = createInfo.logical_device.createGraphicsPipeline(nullptr, graphicsPipelineInfo);
	if (graphicsPipelineResult.result != vk::Result::eSuccess)
	{
		LOG_VULK("Failed To Create Graphics Pipeline");
		return nullptr;
	}

	for (auto& shaderModule : shaderModules)
	{
		logicalDevice_.destroyShaderModule(shaderModule);
	}

	LOG_FUNC_END();
	return graphicsPipelineResult.value;
}

std::vector<vk::PipelineShaderStageCreateInfo> GraphicsPipeline::CreateShaderStageInfos(
	const vk::Device& logicalDevice, const std::vector<shader_info>& shaderInfos,
	const std::vector<vk::ShaderModule>& shaderModules)
{
	LOG_FUNC_START();
	std::vector<vk::PipelineShaderStageCreateInfo> shaderStageInfos;

	int index = 0;
	for (auto& shaderInfo : shaderInfos)
	{
		vk::PipelineShaderStageCreateInfo shaderStage(
			{},
			shaderInfo.type,
			shaderModules[index],
			"main",
			nullptr
		);

		shaderStageInfos.push_back(shaderStage);
		index++;
	}

	LOG_FUNC_END();
	return shaderStageInfos;
}

std::vector<vk::ShaderModule> GraphicsPipeline::LoadShaders(const vk::Device& logicalDevice, const std::vector<shader_info>& shaderInfos)
{
	LOG_FUNC_START();
	std::vector<vk::ShaderModule> shaderModules;

	for (auto& shaderInfo : shaderInfos)
	{
		std::ifstream file(shaderInfo.file_name, std::ios::ate | std::ios::binary);

		if (!file.is_open())
		{
			LOG_ERROR("Failed To Open File: %s", shaderInfo.file_name);
			continue;
		}

		const size_t fileSize = file.tellg();
		std::vector<char> fileBuffer(fileSize);

		file.seekg(0);
		file.read(fileBuffer.data(), static_cast<std::streamsize>(fileSize));

		file.close();

		vk::ShaderModuleCreateInfo shaderModuleInfo(
			{},
			fileBuffer.size(),
			reinterpret_cast<const uint32_t*>(fileBuffer.data())
		);

		shaderModules.push_back(logicalDevice.createShaderModule(shaderModuleInfo));
	}

	LOG_FUNC_END();
	return shaderModules;
}

std::vector<vk::VertexInputBindingDescription> GraphicsPipeline::CreateVertexInputBindingDescriptions()
{
	LOG_FUNC_START();
	LOG_FUNC_END();
	return std::vector<vk::VertexInputBindingDescription> {
		   {
			   0,
			   sizeof(PMATH::vertex),
			   vk::VertexInputRate::eVertex
		   }
	};
}

std::vector<vk::VertexInputAttributeDescription> GraphicsPipeline::CreateVertexInputAttributeDescriptions()
{
	LOG_FUNC_START();
	LOG_FUNC_END();
	return std::vector<vk::VertexInputAttributeDescription> {
		{
			0,
			0,
			vk::Format::eR32G32Sfloat,
			0
		}
	};
	
}

vk::PipelineVertexInputStateCreateInfo GraphicsPipeline::CreateVertexInputStateInfo(
	const std::vector<vk::VertexInputBindingDescription>& vertexInputBindingDescriptions,
	const std::vector<vk::VertexInputAttributeDescription>& vertexInputAttributeDescriptions)
{
	LOG_FUNC_START();
	LOG_FUNC_END();
	return {
		{},
		static_cast<uint32_t>(vertexInputBindingDescriptions.size()),
		vertexInputBindingDescriptions.data(),
		static_cast<uint32_t>(vertexInputAttributeDescriptions.size()),
		vertexInputAttributeDescriptions.data()
	};
}

vk::PipelineInputAssemblyStateCreateInfo GraphicsPipeline::CreateInputAssemblyStateInfo(
	const vk::PrimitiveTopology primitiveTopology)
{
	LOG_FUNC_START();
	LOG_FUNC_END();
	return {
		{},
		primitiveTopology,
		VK_FALSE
	};
}

vk::PipelineViewportStateCreateInfo GraphicsPipeline::CreateViewportStateInfo(
	const std::vector<vk::Viewport>& viewports, const std::vector<vk::Rect2D>& scissors)
{
	LOG_FUNC_START();
	LOG_FUNC_END();
	return {
		{},
		static_cast<uint32_t>(viewports.size()),
		viewports.data(),
		static_cast<uint32_t>(scissors.size()),
		scissors.data()
	};
}

vk::PipelineRasterizationStateCreateInfo GraphicsPipeline::CreateRasterizationStateInfo(
	const vk::PolygonMode polygonMode, const vk::CullModeFlagBits cullMode)
{
	LOG_FUNC_START();
	LOG_FUNC_END();
	return {
		{},
		VK_FALSE,
		VK_FALSE,
		polygonMode,
		cullMode,
		vk::FrontFace::eCounterClockwise,
		VK_TRUE,
		0.0f,
		0.0f,
		0.0f,
		1.0f
	};
}

vk::PipelineDynamicStateCreateInfo GraphicsPipeline::CreateDynamicStateInfo(
	const std::vector<vk::DynamicState>& dynamicStates)
{
	LOG_FUNC_START();
	LOG_FUNC_END();
	return {
		{},
		static_cast<uint32_t>(dynamicStates.size()),
		dynamicStates.data()
	};
}

vk::PipelineMultisampleStateCreateInfo GraphicsPipeline::CreateMultiSampleStateInfo()
{
	LOG_FUNC_START();
	LOG_FUNC_END();
	return {
		{},
		vk::SampleCountFlagBits::e1,
		VK_FALSE,
		4.0f,
		nullptr,
		VK_FALSE,
		VK_FALSE
	};
}

std::vector<vk::PipelineColorBlendAttachmentState> GraphicsPipeline::CreateColorBlendAttachmentStates()
{
	LOG_FUNC_START();
	LOG_FUNC_END();
	return {
		{
			VK_FALSE,
		   vk::BlendFactor::eSrcColor,
		   vk::BlendFactor::eDstColor,
		   vk::BlendOp::eAdd,
		   vk::BlendFactor::eSrcColor,
		   vk::BlendFactor::eDstColor,
		   vk::BlendOp::eAdd,
		   vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
		}
	};
}

vk::PipelineColorBlendStateCreateInfo GraphicsPipeline::CreateColorBlendStateInfo(const std::vector<vk::PipelineColorBlendAttachmentState>& colorBlendAttachmentStates)
{
	LOG_FUNC_START();
	LOG_FUNC_END();
	return vk::PipelineColorBlendStateCreateInfo(
		{},
		VK_FALSE,
		vk::LogicOp::eCopy,
		static_cast<uint32_t>(colorBlendAttachmentStates.size()),
		colorBlendAttachmentStates.data(),
		{ // Blend Constants
			0,
			0,
			0,
			1
		}
	);
}

vk::PipelineDepthStencilStateCreateInfo GraphicsPipeline::CreateDepthStencilStateInfo()
{
	LOG_FUNC_START();
	LOG_FUNC_END();
	return {
		{},
		VK_TRUE,
		VK_TRUE,
		vk::CompareOp::eLess,
		VK_FALSE,
		VK_FALSE,
		{},
		{},
		0.0f,
		0.0f
	};
}