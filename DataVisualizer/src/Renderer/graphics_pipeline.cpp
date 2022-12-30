#include "pcHeader.h"
#include "graphics_pipeline.h"

bool graphics_pipeline::init(const graphics_pipeline_create_info& create_info)
{
	logical_device_ = create_info.logical_device;

	pipeline_ = create_graphics_pipeline(create_info);
	if (!pipeline_)
	{
		LOG_INFO("Faile To Initialize Graphics Pipeline");
		return false;
	}

	LOG_INFO("Initialized Graphics Pipeline");
	return true;
}

void graphics_pipeline::shutdown() const
{
	logical_device_.destroyPipeline(pipeline_);
	LOG_INFO("Shutdown Graphics Pipeline");
}

void graphics_pipeline::bind_pipeline(const vk::CommandBuffer command_buffer) const
{
	command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline_);
}

vk::Pipeline graphics_pipeline::create_graphics_pipeline(const graphics_pipeline_create_info& create_info)
{
	const std::vector<vk::ShaderModule> shader_modules = load_shaders(logical_device_, create_info.shader_infos);

	std::vector<vk::PipelineShaderStageCreateInfo> shader_stage_infos = create_shader_stage_infos(create_info.logical_device, create_info.shader_infos, shader_modules);
	
	std::vector<vk::VertexInputBindingDescription> vertex_input_binding_descriptions = create_vertex_input_binding_descriptions();

	std::vector<vk::VertexInputAttributeDescription> vertex_input_attribute_descriptions = create_vertex_input_attribute_descriptions();

	vk::PipelineVertexInputStateCreateInfo vertex_input_state_info = create_vertex_input_state_info(vertex_input_binding_descriptions, vertex_input_attribute_descriptions);

	vk::PipelineInputAssemblyStateCreateInfo input_assembly_state_info = create_input_assembly_state_info(create_info.primitive_topology);

	vk::PipelineViewportStateCreateInfo viewport_state_info = create_viewport_state_info(create_info.viewports, create_info.scissors);

	vk::PipelineRasterizationStateCreateInfo rasterization_state_info = create_rasterization_state_info(create_info.polygon_mode, create_info.cull_mode);

	vk::PipelineDynamicStateCreateInfo dynamic_state_info = create_dynamic_state_info(create_info.dynamic_states);

	vk::PipelineMultisampleStateCreateInfo multisample_state_info = create_multisample_state_info();

	std::vector<vk::PipelineColorBlendAttachmentState> color_blend_attachment_states = create_color_blend_attachment_states();

	vk::PipelineColorBlendStateCreateInfo color_blend_state_info = create_color_blend_state_info(color_blend_attachment_states);

	vk::PipelineDepthStencilStateCreateInfo depth_stencil_state_info = create_depth_stencil_state_info();

	const vk::GraphicsPipelineCreateInfo graphics_pipeline_info(
		{},
		static_cast<uint32_t>(shader_stage_infos.size()),
		shader_stage_infos.data(),
		&vertex_input_state_info,
		&input_assembly_state_info,
		nullptr, // Tessellation Stage
		&viewport_state_info,
		&rasterization_state_info,
		&multisample_state_info,
		&depth_stencil_state_info,
		&color_blend_state_info,
		&dynamic_state_info,
		create_info.pipeline_layout,
		create_info.render_pass,
		0,
		VK_NULL_HANDLE,
		0
	);

	vk::ResultValue<vk::Pipeline> graphics_pipeline_result = create_info.logical_device.createGraphicsPipeline(nullptr, graphics_pipeline_info);
	if (graphics_pipeline_result.result != vk::Result::eSuccess)
	{
		LOG_VULK("Failed To Create Graphics Pipeline");
		return nullptr;
	}

	for (auto& shader_module : shader_modules)
	{
		logical_device_.destroyShaderModule(shader_module);
	}

	LOG_VULK("Created Graphics Pipeline");
	return graphics_pipeline_result.value;
}

std::vector<vk::PipelineShaderStageCreateInfo> graphics_pipeline::create_shader_stage_infos(
	const vk::Device& logical_device, const std::vector<shader_info>& shader_infos,
	const std::vector<vk::ShaderModule> shader_modules)
{
	std::vector<vk::PipelineShaderStageCreateInfo> shader_stage_infos;

	int index = 0;
	for (auto& shader_info : shader_infos)
	{
		vk::PipelineShaderStageCreateInfo shader_stage(
			{},
			shader_info.type,
			shader_modules[index],
			"main",
			nullptr
		);

		shader_stage_infos.push_back(shader_stage);
		index++;
	}

	return shader_stage_infos;
}

std::vector<vk::ShaderModule> graphics_pipeline::load_shaders(const vk::Device& logical_device, const std::vector<shader_info>& shader_infos)
{
	std::vector<vk::ShaderModule> shader_modules;

	for (auto& shader_info : shader_infos)
	{
		std::ifstream file(shader_info.file_name, std::ios::ate | std::ios::binary);

		if (!file.is_open())
		{
			LOG_ERROR("Failed To Open File: %s", shader_info.file_name);
			continue;
		}

		const size_t file_size = file.tellg();
		std::vector<char> file_buffer(file_size);

		file.seekg(0);
		file.read(file_buffer.data(), static_cast<std::streamsize>(file_size));

		file.close();

		vk::ShaderModuleCreateInfo shader_module_info(
			{},
			file_buffer.size(),
			reinterpret_cast<const uint32_t*>(file_buffer.data())
		);

		shader_modules.push_back(logical_device.createShaderModule(shader_module_info));
	}

	return shader_modules;
}

std::vector<vk::VertexInputBindingDescription> graphics_pipeline::create_vertex_input_binding_descriptions()
{
	return std::vector<vk::VertexInputBindingDescription> {
		   {
			   0,
			   sizeof(vertex),
			   vk::VertexInputRate::eVertex
		   }
	};
}

std::vector<vk::VertexInputAttributeDescription> graphics_pipeline::create_vertex_input_attribute_descriptions()
{
	return std::vector<vk::VertexInputAttributeDescription> {
		{
			0,
			0,
			vk::Format::eR32G32Sfloat,
			0
		}
	};
	
}

vk::PipelineVertexInputStateCreateInfo graphics_pipeline::create_vertex_input_state_info(
	const std::vector<vk::VertexInputBindingDescription>& vertex_input_binding_descriptions,
	const std::vector<vk::VertexInputAttributeDescription>& vertex_input_attribute_descriptions)
{
	return vk::PipelineVertexInputStateCreateInfo(
		{},
		static_cast<uint32_t>(vertex_input_binding_descriptions.size()),
		vertex_input_binding_descriptions.data(),
		static_cast<uint32_t>(vertex_input_attribute_descriptions.size()),
		vertex_input_attribute_descriptions.data()
	);
}

vk::PipelineInputAssemblyStateCreateInfo graphics_pipeline::create_input_assembly_state_info(
	const vk::PrimitiveTopology primitive_topology)
{
	return vk::PipelineInputAssemblyStateCreateInfo(
		{},
		primitive_topology,
		VK_FALSE
	);
}

vk::PipelineViewportStateCreateInfo graphics_pipeline::create_viewport_state_info(
	const std::vector<vk::Viewport>& viewports, const std::vector<vk::Rect2D>& scissors)
{
	return vk::PipelineViewportStateCreateInfo(
		{},
		static_cast<uint32_t>(viewports.size()),
		viewports.data(),
		static_cast<uint32_t>(scissors.size()),
		scissors.data()
	);
}

vk::PipelineRasterizationStateCreateInfo graphics_pipeline::create_rasterization_state_info(
	const vk::PolygonMode polygon_mode, const vk::CullModeFlagBits cull_mode)
{
	return vk::PipelineRasterizationStateCreateInfo(
		{},
		VK_FALSE,
		VK_FALSE,
		polygon_mode,
		cull_mode,
		vk::FrontFace::eCounterClockwise,
		VK_TRUE,
		0.0f,
		0.0f,
		0.0f,
		1.0f
	);
}

vk::PipelineDynamicStateCreateInfo graphics_pipeline::create_dynamic_state_info(
	const std::vector<vk::DynamicState>& dynamic_states)
{
	return vk::PipelineDynamicStateCreateInfo(
		{},
		static_cast<uint32_t>(dynamic_states.size()),
		dynamic_states.data()
	);
}

vk::PipelineMultisampleStateCreateInfo graphics_pipeline::create_multisample_state_info()
{
	return vk::PipelineMultisampleStateCreateInfo(
		{},
		vk::SampleCountFlagBits::e1,
		VK_FALSE,
		4.0f,
		nullptr,
		VK_FALSE,
		VK_FALSE
	);
}

std::vector<vk::PipelineColorBlendAttachmentState> graphics_pipeline::create_color_blend_attachment_states()
{
	return std::vector<vk::PipelineColorBlendAttachmentState>{
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

vk::PipelineColorBlendStateCreateInfo graphics_pipeline::create_color_blend_state_info(const std::vector<vk::PipelineColorBlendAttachmentState>& color_blend_attachment_states)
{
	

	return vk::PipelineColorBlendStateCreateInfo(
		{},
		VK_FALSE,
		vk::LogicOp::eCopy,
		static_cast<uint32_t>(color_blend_attachment_states.size()),
		color_blend_attachment_states.data(),
		{ // Blend Constants
			0,
			0,
			0,
			1
		}
	);
}

vk::PipelineDepthStencilStateCreateInfo graphics_pipeline::create_depth_stencil_state_info()
{
	return vk::PipelineDepthStencilStateCreateInfo(
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
	);
}