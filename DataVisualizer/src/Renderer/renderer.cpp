#include "pcHeader.h"
#include "renderer.h"
#include "VulkanContext.h"
#include "Core/Components/line_component.h"

bool renderer::init(vulkan_context* vulkan_context)
{
	vulkan_context_ = vulkan_context;

	swapchain_.init(vulkan_context_);

	render_pass_ = create_render_pass();
	descriptor_set_layout_ = create_descriptor_set_layout();
	pipeline_layout_ = create_pipeline_layout();

	const std::vector<shader_info> shader_infos = {
		{
			vk::ShaderStageFlagBits::eVertex,
			"../DataVisualizer/src/Shaders/vert.spv"
		},
		{
			vk::ShaderStageFlagBits::eFragment,
			"../DataVisualizer/src/Shaders/frag.spv"
		}
	};

	main_viewport_ = vk::Viewport{
		static_cast<float>(vulkan_context_->get_window().get_width()) * 0.2f,
		0.0f,
		static_cast<float>(vulkan_context_->get_window().get_width()) * 0.8f,
		static_cast<float>(vulkan_context_->get_window().get_height()),
		0.0f, 1.0f
	};

	sidebar_viewport_ = vk::Viewport{
		static_cast<float>(vulkan_context_->get_window().get_width()) * 0.0f,
		0.0f,
		static_cast<float>(vulkan_context_->get_window().get_width()) * 0.2f,
		static_cast<float>(vulkan_context_->get_window().get_height()),
		0.0f, 1.0f
	};

	vk::Rect2D scissor{
		{
			0,
			0
		},
		{
			static_cast<uint32_t>(vulkan_context_->get_window().get_width()),
			static_cast<uint32_t>(vulkan_context_->get_window().get_height())
		}
	};

	graphics_pipeline_create_info graphics_pipeline_info{};
	graphics_pipeline_info.logical_device = vulkan_context_->get_logical_device();
	graphics_pipeline_info.shader_infos = shader_infos;
	graphics_pipeline_info.pipeline_layout = pipeline_layout_;
	graphics_pipeline_info.render_pass = render_pass_;
	graphics_pipeline_info.primitive_topology = vk::PrimitiveTopology::eLineStrip;
	graphics_pipeline_info.polygon_mode = vk::PolygonMode::eFill;
	graphics_pipeline_info.cull_mode = vk::CullModeFlagBits::eNone;
	graphics_pipeline_info.viewports = { main_viewport_};
	graphics_pipeline_info.scissors = { scissor };
	graphics_pipeline_info.dynamic_states = { vk::DynamicState::eViewport };

	line_pipeline_.init(graphics_pipeline_info);

	depth_image_ = create_depth_image();
	depth_image_memory_ = bind_depth_image();
	depth_image_view_ = create_depth_image_view();

	frame_buffers_ = create_frame_buffers();

	command_pool_ = create_command_pool();

	uniform_buffer_ = create_uniform_buffer();

	uniform_buffer_memory_ = bind_uniform_buffer_memory();

	descriptor_pool_ = create_descriptor_pool();

	descriptor_sets_ = create_descriptor_set();

	update_descriptor_sets();

	command_buffers_ = create_command_buffer();

	image_available_semaphore_ = create_semaphore();
	render_finished_semaphore_ = create_semaphore();
	in_flight_fence_ = create_fence();

	LOG_INFO("Initialized Renderer");
	return true;
}

bool renderer::update()
{
	if (vulkan_context_->get_logical_device().waitForFences(1, &in_flight_fence_, VK_TRUE, 0) != vk::Result::eSuccess)
	{
		return true;
	}

	if (vulkan_context_->get_logical_device().resetFences(1, &in_flight_fence_) != vk::Result::eSuccess)
	{
		return false;
	}

	const vk::ResultValue<uint32_t> next_image_index_result = vulkan_context_->get_logical_device().acquireNextImageKHR(swapchain_.get_swapchain(), 0, image_available_semaphore_, VK_NULL_HANDLE);
	if (next_image_index_result.result != vk::Result::eSuccess)
	{
		return true;
	}

	const uint32_t next_image_index = next_image_index_result.value;

	vulkan_context_->get_logical_device().resetCommandPool(command_pool_);

	///////////
	constexpr vk::CommandBufferBeginInfo info(
		{},
		nullptr
	);

	constexpr vk::ClearValue clear_value(std::array<float, 4>({ { 0.0f, 0.0f, 0.0f, 0.0f } }));
	vk::ClearValue depth_value{};
	depth_value.depthStencil.depth = 1.0f;
	const std::vector<vk::ClearValue> clear_values = {
		clear_value,
		depth_value
	};

	const vk::RenderPassBeginInfo render_pass_begin_info(
		render_pass_,
		frame_buffers_[next_image_index],
		{
			{ 0, 0 },
			{ vulkan_context_->get_window().get_width(), vulkan_context_->get_window().get_height() }
		},
		static_cast<uint32_t>(clear_values.size()),
		clear_values.data()
	);

	float aspect_ratio = static_cast<float>(vulkan_context_->get_window().get_width()) * 0.8f / static_cast<float>(vulkan_context_->get_window().get_height());

	glm::mat4 p = glm::ortho(
		0.0f,
		aspect_ratio * 2,
		2.0f,
		0.0f,
		0.1f,
		1000.0f
	);


	glm::mat4 view{1.0f};

	glm::vec3 position{ 0.0f, 0.0f, 0.0f };

	glm::vec3 forward(0.0f, 0.0f, -1.0f);
	glm::vec3 up(0.0f, 1.0f, 0.0f);
	glm::vec3 right(1.0f, 0.0f, 0.0f);

	const float c3 = glm::cos(glm::radians(0.0f));
	const float s3 = glm::sin(glm::radians(0.0f));
	const float c2 = glm::cos(glm::radians(0.0f));
	const float s2 = glm::sin(glm::radians(0.0f));
	const float c1 = glm::cos(glm::radians(0.0f));
	const float s1 = glm::sin(glm::radians(0.0f));
	right = { (c1 * c3 + s1 * s2 * s3), (c2 * s3), (c1 * s2 * s3 - c3 * s1) };
	up = { (c3 * s1 * s2 - c1 * s3), (c2 * c3), (c1 * c3 * s2 + s1 * s3) };
	forward = { (c2 * s1), (-s2), (c1 * c2) };
	view[0][0] = right.x;
	view[1][0] = right.y;
	view[2][0] = right.z;
	view[0][1] = up.x;
	view[1][1] = up.y;
	view[2][1] = up.z;
	view[0][2] = forward.x;
	view[1][2] = forward.y;
	view[2][2] = forward.z;
	view[3][0] = -glm::dot(right, position);
	view[3][1] = -glm::dot(up, position);
	view[3][2] = -glm::dot(forward, position);

	const uniform_buffer_object mvp{ p * view * glm::scale(glm::mat4(1.0f), glm::vec3(2.0f, 2.0f, 2.0f)) };

	void* data = vulkan_context_->get_logical_device().mapMemory(uniform_buffer_memory_, 0, sizeof(uniform_buffer_object), {});
	memcpy(data, &mvp, sizeof(uniform_buffer_object));
	vulkan_context_->get_logical_device().unmapMemory(uniform_buffer_memory_);

	//TODO::Add List of meshes to draw and loop them in here.
	command_buffers_[0].begin(info);
	{
		command_buffers_[0].setViewport(0, 1, &main_viewport_);

		command_buffers_[0].beginRenderPass(&render_pass_begin_info, vk::SubpassContents::eInline);

		line_pipeline_.bind_pipeline(command_buffers_[0]);
		for (auto& line : lines_to_draw_)
		{
			//TODO::Pass Something useful to push constant including mvp.
			push_constant pc{};
			pc.color = glm::vec4(line->get_color(), 1.0f);
			pc.mvp = glm::translate(glm::mat4(1.0f), glm::vec3(line->get_transform().position, -line->get_transform().depth));
			pc.mvp = glm::rotate(pc.mvp, glm::radians(line->get_transform().rotation), glm::vec3(0, 0, 1));
			pc.mvp = p * view * glm::scale(pc.mvp, glm::vec3(line->get_transform().scale));
			command_buffers_[0].pushConstants(pipeline_layout_, vk::ShaderStageFlagBits::eVertex, 0, sizeof(push_constant), &pc);

			//TODO::Bind Index Buffer, Bind Descriptor Set.
			constexpr vk::DeviceSize offsets[] = { 0 };
			command_buffers_[0].bindVertexBuffers(0, 1, &line->get_vertex_buffer(), offsets);

			command_buffers_[0].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline_layout_, 0, 1, descriptor_sets_.data(), 0, nullptr);

			command_buffers_[0].draw(
				static_cast<uint32_t>(line->get_points().size()),
				1,
				0,
				0);
		}

		command_buffers_[0].endRenderPass();
	}
	command_buffers_[0].end();

	lines_to_draw_.clear();
	///////////

	constexpr vk::PipelineStageFlags wait_stages = { vk::PipelineStageFlagBits::eColorAttachmentOutput };

	vk::SubmitInfo submit_info(
		1,
		&image_available_semaphore_,
		&wait_stages,
		static_cast<uint32_t>(command_buffers_.size()),
		command_buffers_.data(),
		1,
		&render_finished_semaphore_
	);

	vulkan_context_->get_graphics_queue().submit(submit_info, in_flight_fence_);

	const vk::PresentInfoKHR present_info(
		1,
		&render_finished_semaphore_,
		1,
		&swapchain_.get_swapchain(),
		&next_image_index,
		nullptr
	);

	if (vulkan_context_->get_graphics_queue().presentKHR(&present_info) != vk::Result::eSuccess)
	{
		LOG_ERROR("Failed Presenting");
	}

	return true;
}

void renderer::shutdown()
{
	vulkan_context_->get_logical_device().waitIdle();

	vulkan_context_->get_logical_device().freeMemory(depth_image_memory_);
	vulkan_context_->get_logical_device().destroyImageView(depth_image_view_);
	vulkan_context_->get_logical_device().destroyImage(depth_image_);

	vulkan_context_->get_logical_device().destroyCommandPool(command_pool_);

	for (const auto& frame_buffer : frame_buffers_)
	{
		vulkan_context_->get_logical_device().destroyFramebuffer(frame_buffer);
	}

	vulkan_context_->get_logical_device().freeMemory(uniform_buffer_memory_);
	vulkan_context_->get_logical_device().destroyBuffer(uniform_buffer_);

	vulkan_context_->get_logical_device().destroySemaphore(image_available_semaphore_);
	vulkan_context_->get_logical_device().destroySemaphore(render_finished_semaphore_);
	vulkan_context_->get_logical_device().destroyFence(in_flight_fence_);

	vulkan_context_->get_logical_device().destroyDescriptorPool(descriptor_pool_);

	line_pipeline_.shutdown();
	vulkan_context_->get_logical_device().destroyPipelineLayout(pipeline_layout_);
	vulkan_context_->get_logical_device().destroyDescriptorSetLayout(descriptor_set_layout_);
	vulkan_context_->get_logical_device().destroyRenderPass(render_pass_);
	swapchain_.shutdown();
	LOG_INFO("Shutdown Renderer");
}

vk::RenderPass renderer::create_render_pass() const
{
	const std::vector<vk::AttachmentDescription> attachments = {
		{
			{},
			swapchain_.get_details().surface_format.format,
			vk::SampleCountFlagBits::e1,
			vk::AttachmentLoadOp::eClear,
			vk::AttachmentStoreOp::eStore,
			vk::AttachmentLoadOp::eDontCare,
			vk::AttachmentStoreOp::eDontCare,
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::ePresentSrcKHR
		},
		{
			{},
			vk::Format::eD32Sfloat,
			vk::SampleCountFlagBits::e1,
			vk::AttachmentLoadOp::eClear,
			vk::AttachmentStoreOp::eStore,
			vk::AttachmentLoadOp::eClear,
			vk::AttachmentStoreOp::eDontCare,
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::eDepthStencilAttachmentOptimal
		}
	};

	std::vector<vk::AttachmentReference> color_attachments = {
		{
			0,
			vk::ImageLayout::eColorAttachmentOptimal
		}
	};

	vk::AttachmentReference depth_attachment(
		1,
		vk::ImageLayout::eDepthStencilAttachmentOptimal
	);

	const std::vector<vk::SubpassDescription> subpasses = {
		{
			{},
			vk::PipelineBindPoint::eGraphics,
			0,
			nullptr,
			static_cast<uint32_t>(color_attachments.size()),
			color_attachments.data(),
			nullptr,
			&depth_attachment,
			0,
			nullptr
		}
	};

	const std::vector<vk::SubpassDependency> dependencies = {
		{ // For color attachment
			VK_SUBPASS_EXTERNAL,
		   0,
		   vk::PipelineStageFlagBits::eColorAttachmentOutput,
		   vk::PipelineStageFlagBits::eColorAttachmentOutput,
		   vk::AccessFlagBits::eNone,
		   vk::AccessFlagBits::eColorAttachmentWrite
		},
		{ // For depth attachment
			VK_SUBPASS_EXTERNAL,
			0,
			vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests,
			vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests,
			vk::AccessFlagBits::eNone,
			vk::AccessFlagBits::eDepthStencilAttachmentWrite
		}
	};

	const vk::RenderPassCreateInfo render_pass_info(
		{},
		static_cast<uint32_t>(attachments.size()),
		attachments.data(),
		static_cast<uint32_t>(subpasses.size()),
		subpasses.data(),
		static_cast<uint32_t>(dependencies.size()),
		dependencies.data()
	);

	LOG_VULK("Created Render Pass");
	return vulkan_context_->get_logical_device().createRenderPass(render_pass_info);
}

vk::DescriptorSetLayout renderer::create_descriptor_set_layout() const
{
	const std::vector<vk::DescriptorSetLayoutBinding> uniform_layout_bindings = {
		{
			0,
			vk::DescriptorType::eUniformBuffer,
			1,
			vk::ShaderStageFlagBits::eVertex,
			nullptr
		}
	};

	const vk::DescriptorSetLayoutCreateInfo descriptor_set_layout_info(
		{},
		static_cast<uint32_t>(uniform_layout_bindings.size()),
		uniform_layout_bindings.data()
	);

	LOG_VULK("Created Descriptor Set Layout");
	return vulkan_context_->get_logical_device().createDescriptorSetLayout(descriptor_set_layout_info);
}

vk::PipelineLayout renderer::create_pipeline_layout() const
{
	const std::vector<vk::PushConstantRange> push_constants = {
		{
			vk::ShaderStageFlagBits::eVertex,
		   0,
		   sizeof(push_constant)
		}
	};

	const vk::PipelineLayoutCreateInfo pipeline_layout_info(
		{},
		1,
		&descriptor_set_layout_,
		static_cast<uint32_t>(push_constants.size()),
		push_constants.data()
	);

	LOG_VULK("Created Pipeline Layout");
	return vulkan_context_->get_logical_device().createPipelineLayout(pipeline_layout_info);
}

vk::Image renderer::create_depth_image() const
{
	const vk::ImageCreateInfo image_info(
		{},
		vk::ImageType::e2D,
		vk::Format::eD32Sfloat,
		{
			vulkan_context_->get_window().get_width(),
			vulkan_context_->get_window().get_height(),
			1
		},
		1,
		1,
		vk::SampleCountFlagBits::e1,
		vk::ImageTiling::eOptimal,
		vk::ImageUsageFlagBits::eDepthStencilAttachment
	);

	LOG_VULK("Created Depth Image");
	return vulkan_context_->get_logical_device().createImage(image_info);
}

vk::DeviceMemory renderer::bind_depth_image() const
{
	const vk::MemoryRequirements memory_requirements = vulkan_context_->get_logical_device().getImageMemoryRequirements(depth_image_);

	//TODO::Why is this memory type index set to 1? need to check gpu i think.
	const vk::MemoryAllocateInfo memory_allocate_info(
		memory_requirements.size,
		1
	);

	const vk::DeviceMemory memory = vulkan_context_->get_logical_device().allocateMemory(memory_allocate_info);
	vulkan_context_->get_logical_device().bindImageMemory(depth_image_, memory, 0);

	LOG_VULK("Binded Depth Image Memory");
	return memory;
}

vk::ImageView renderer::create_depth_image_view() const
{
	const vk::ImageViewCreateInfo depth_image_view_info(
		{},
		depth_image_,
		vk::ImageViewType::e2D,
		vk::Format::eD32Sfloat,
		{},
		{
			vk::ImageAspectFlagBits::eDepth,
			0,
			1,
			0,
			1
		}
	);

	LOG_VULK("Created Depth Image View");
	return vulkan_context_->get_logical_device().createImageView(depth_image_view_info);
}

std::vector<vk::Framebuffer> renderer::create_frame_buffers() const
{
	std::vector<vk::Framebuffer> frame_buffers(swapchain_.get_images().size());

	for (size_t i = 0; i < swapchain_.get_image_views().size(); i++)
	{
		std::vector<vk::ImageView> attachments = {
			swapchain_.get_image_views()[i],
			depth_image_view_
		};

		vk::FramebufferCreateInfo frame_buffer_info(
			{},
			render_pass_,
			static_cast<uint32_t>(attachments.size()),
			attachments.data(),
			vulkan_context_->get_window().get_width(),
			vulkan_context_->get_window().get_height(),
			1
		);

		frame_buffers[i] = vulkan_context_->get_logical_device().createFramebuffer(frame_buffer_info);
	}

	LOG_VULK("Created Frame Buffers");
	return frame_buffers;
}

vk::CommandPool renderer::create_command_pool() const
{
	const vk::CommandPoolCreateInfo command_pool_info(
		{},
		vulkan_context_->get_graphics_queue_index()
	);

	LOG_VULK("Created Command Pool");
	return vulkan_context_->get_logical_device().createCommandPool(command_pool_info);
}

vk::Buffer renderer::create_uniform_buffer() const
{
	constexpr vk::DeviceSize buffer_size = sizeof(uniform_buffer_object);

	const vk::BufferCreateInfo buffer_info(
		{},
		buffer_size,
		vk::BufferUsageFlagBits::eUniformBuffer
	);

	LOG_VULK("Created Uniform Buffer");
	return vulkan_context_->get_logical_device().createBuffer(buffer_info);
}

vk::DeviceMemory renderer::bind_uniform_buffer_memory() const
{
	const vk::MemoryRequirements memory_requirements = vulkan_context_->get_logical_device().getBufferMemoryRequirements(uniform_buffer_);

	const vk::MemoryAllocateInfo allocate_info(
		memory_requirements.size,
		2
	);

	const vk::DeviceMemory memory = vulkan_context_->get_logical_device().allocateMemory(allocate_info);

	vulkan_context_->get_logical_device().bindBufferMemory(uniform_buffer_, memory, 0);

	return memory;
}

vk::DescriptorPool renderer::create_descriptor_pool() const
{
	const std::vector<vk::DescriptorPoolSize> descriptor_pool_sizes{
		{
			vk::DescriptorType::eUniformBuffer,
			1
		}
	};

	const vk::DescriptorPoolCreateInfo descriptor_pool_info(
		{},
		1,
		static_cast<uint32_t>(descriptor_pool_sizes.size()),
		descriptor_pool_sizes.data()
	);

	LOG_VULK("Created Descriptor Pool");
	return vulkan_context_->get_logical_device().createDescriptorPool(descriptor_pool_info);
}

std::vector<vk::DescriptorSet> renderer::create_descriptor_set() const
{
	const vk::DescriptorSetAllocateInfo allocate_info(
		descriptor_pool_,
		1,
		&descriptor_set_layout_
	);

	return vulkan_context_->get_logical_device().allocateDescriptorSets(allocate_info);
}

void renderer::update_descriptor_sets()
{
	const std::vector<vk::DescriptorBufferInfo> buffer_infos{
		{
			uniform_buffer_,
			0,
			sizeof(uniform_buffer_object)
		}
	};

	const std::vector<vk::WriteDescriptorSet> write_descriptor_sets = {
		{
			descriptor_sets_[0],
		   0,
		   0,
		   static_cast<uint32_t>(buffer_infos.size()),
		   vk::DescriptorType::eUniformBuffer,
		   nullptr,
		   buffer_infos.data(),
		   nullptr
		}
	};

	const std::vector<vk::CopyDescriptorSet> copy_descriptor_sets = {};

	vulkan_context_->get_logical_device().updateDescriptorSets(
		static_cast<uint32_t>(write_descriptor_sets.size()),
		write_descriptor_sets.data(),
		static_cast<uint32_t>(copy_descriptor_sets.size()),
		copy_descriptor_sets.data()
	);
}

std::vector<vk::CommandBuffer> renderer::create_command_buffer() const
{
	const vk::CommandBufferAllocateInfo command_buffer_allocate_info(
		command_pool_,
		vk::CommandBufferLevel::ePrimary,
		1
	);

	return vulkan_context_->get_logical_device().allocateCommandBuffers(command_buffer_allocate_info);
}

vk::Semaphore renderer::create_semaphore() const
{
	const vk::SemaphoreCreateInfo semaphore_info(
		{}
	);

	return vulkan_context_->get_logical_device().createSemaphore(semaphore_info);
}

vk::Fence renderer::create_fence() const
{
	const vk::FenceCreateInfo fence_info(
		vk::FenceCreateFlagBits::eSignaled
	);

	return vulkan_context_->get_logical_device().createFence(fence_info);
}

vk::Buffer renderer::create_vertex_buffer(const std::vector<vertex>& points) const
{
	if (points.empty())
	{
		LOG_WARN("Trying to create an empty buffer");
		return nullptr;
	}

	const vk::BufferCreateInfo buffer_info(
		{},
		sizeof(points[0]) * points.size(),
		vk::BufferUsageFlagBits::eVertexBuffer
	);

	return vulkan_context_->get_logical_device().createBuffer(buffer_info);
}

vk::DeviceMemory renderer::bind_vertex_buffer_memory(const vk::Buffer vertex_buffer, const std::vector<vertex>& points) const
{
	if (points.empty())
	{
		LOG_WARN("Trying to bind empty buffer");
		return nullptr;
	}

	const vk::MemoryRequirements memory_requirements = vulkan_context_->get_logical_device().getBufferMemoryRequirements(vertex_buffer);

	const vk::MemoryAllocateInfo allocate_info(
		memory_requirements.size,
		2
	);

	const vk::DeviceMemory memory = vulkan_context_->get_logical_device().allocateMemory(allocate_info);

	vulkan_context_->get_logical_device().bindBufferMemory(vertex_buffer, memory, 0);

	void* data = vulkan_context_->get_logical_device().mapMemory(memory, 0, allocate_info.allocationSize, {});
	memcpy(data, points.data(), allocate_info.allocationSize);
	vulkan_context_->get_logical_device().unmapMemory(memory);

	return memory;
}

void renderer::destroy_buffer(const vk::Buffer buffer) const
{
	vulkan_context_->get_logical_device().destroyBuffer(buffer);
}

void renderer::free_memory(const vk::DeviceMemory memory) const
{
	vulkan_context_->get_logical_device().freeMemory(memory);
}

void renderer::draw(line_component* line_component)
{
	const auto iterator = std::ranges::find(lines_to_draw_, line_component);
	if (iterator != lines_to_draw_.end())
	{
		return;
	}
	lines_to_draw_.emplace_back(line_component);
}
