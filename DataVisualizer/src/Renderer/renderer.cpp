#include "pcHeader.h"
#include "Renderer.h"
#include "VulkanContext.h"
#include "backends/imgui_impl_vulkan.h"
#include "backends/imgui_impl_win32.h"
#include "Core/Application.h"
#include "Core/Components/CLineChart.h"

bool Renderer::Init(VulkanContext* vulkanContext)
{
	LOG_FUNC_START();

	vulkanContext_ = vulkanContext;

	swapchain_.init(vulkanContext_);

	renderPass_ = CreateRenderPass();
	descriptorSetLayout_ = CreateDescriptorSetLayout();
	pipelineLayout_ = CreatePipelineLayout();

	const std::vector<shader_info> shaderInfos = {
		{
			vk::ShaderStageFlagBits::eVertex,
			"../DataVisualizer/src/Shaders/vert.spv"
		},
		{
			vk::ShaderStageFlagBits::eFragment,
			"../DataVisualizer/src/Shaders/frag.spv"
		}
	};

	mainViewport_ = vk::Viewport{
		0.0f,
		0.0f,
		static_cast<float>(vulkanContext_->get_window().get_width()),
		static_cast<float>(vulkanContext_->get_window().get_height()),
		0.0f, 1.0f
	};
	
	scissor_ = vk::Rect2D{
		{
			0,
			0
		},
		{
			(vulkanContext_->get_window().get_width()),
			(vulkanContext_->get_window().get_height())
		}
	};

	graphics_pipeline_create_info graphicsPipelineInfo{};
	graphicsPipelineInfo.logical_device = vulkanContext_->get_logical_device();
	graphicsPipelineInfo.shader_infos = shaderInfos;
	graphicsPipelineInfo.pipeline_layout = pipelineLayout_;
	graphicsPipelineInfo.render_pass = renderPass_;
	graphicsPipelineInfo.primitive_topology = vk::PrimitiveTopology::eLineStrip;
	graphicsPipelineInfo.polygon_mode = vk::PolygonMode::eFill;
	graphicsPipelineInfo.cull_mode = vk::CullModeFlagBits::eNone;
	graphicsPipelineInfo.viewports = { mainViewport_};
	graphicsPipelineInfo.scissors = { scissor_ };
	graphicsPipelineInfo.dynamic_states = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };

	linePipeline_.Init(graphicsPipelineInfo);

	depthImage_ = CreateDepthImage();
	depthImageMemory_ = BindDepthImage();
	depthImageView_ = CreateDepthImageView();

	frameBuffers_ = CreateFrameBuffers();

	commandPool_ = CreateCommandPool();

	uniformBuffer_ = CreateUniformBuffer();

	uniformBufferMemory_ = BindUniformBufferMemory();

	descriptorPool_ = CreateDescriptorPool();
	imGuiDescriptorPool_ = CreateImGuiDescriptorPool();

	descriptorSets_ = CreateDescriptorSet();

	UpdateDescriptorSets();

	commandBuffers_ = CreateCommandBuffer();

	imageAvailableSemaphore_ = CreateSemaphore();
	renderFinishedSemaphore_ = CreateSemaphore();
	inFlightFence_ = CreateFence();

	LOG_FUNC_END();
	return true;
}

bool Renderer::Update()
{
	if (vulkanContext_->get_logical_device().waitForFences(1, &inFlightFence_, VK_TRUE, 0) != vk::Result::eSuccess)
	{
		return true;
	}
	
	if (vulkanContext_->get_logical_device().resetFences(1, &inFlightFence_) != vk::Result::eSuccess)
	{
		
		return false;
	}

	const vk::ResultValue<uint32_t> nextImageIndexResult = vulkanContext_->get_logical_device().acquireNextImageKHR(swapchain_.get_swapchain(), 0, imageAvailableSemaphore_, VK_NULL_HANDLE);
	if (nextImageIndexResult.result != vk::Result::eSuccess)
	{
		return true;
	}

	const uint32_t nextImageIndex = nextImageIndexResult.value;

	vulkanContext_->get_logical_device().resetCommandPool(commandPool_);

	///////////
	constexpr vk::CommandBufferBeginInfo info(
		{},
		nullptr
	);

	constexpr vk::ClearValue clearValue(std::array<float, 4>({ { 0.16f, 0.16f, 0.16f, 1.0f } }));
	vk::ClearValue depthValue{};
	depthValue.depthStencil.depth = 1.0f;
	const std::vector<vk::ClearValue> clearValues = {
		clearValue,
		depthValue
	};

	const vk::RenderPassBeginInfo renderPassBeginInfo(
		renderPass_,
		frameBuffers_[nextImageIndex],
		{
			{ 0, 0 },
			{ vulkanContext_->get_window().get_width(), vulkanContext_->get_window().get_height() }
		},
		static_cast<uint32_t>(clearValues.size()),
		clearValues.data()
	);

	glm::mat4 p = glm::ortho(
		0.0f,
		1.0f,
		1.0f,
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

	const PMATH::uniform_buffer_object mvp{ p * view * glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f)) };

	void* data = vulkanContext_->get_logical_device().mapMemory(uniformBufferMemory_, 0, sizeof(PMATH::uniform_buffer_object), {});
	memcpy(data, &mvp, sizeof(PMATH::uniform_buffer_object));
	vulkanContext_->get_logical_device().unmapMemory(uniformBufferMemory_);

	//TODO::Add List of meshes to draw and loop them in here.
	commandBuffers_[0].begin(info);
	{
		commandBuffers_[0].setViewport(0, 1, &mainViewport_);
		commandBuffers_[0].setScissor(0, 1, &scissor_);

		commandBuffers_[0].beginRenderPass(&renderPassBeginInfo, vk::SubpassContents::eInline);


		linePipeline_.BindPipeline(commandBuffers_[0]);


		for (auto& line : linesToDraw_)
		{
			//TODO::Pass Something useful to push constant including mvp.
			PMATH::push_constant pc{};
			pc.color = glm::vec4(line->GetColor(), 1.0f);
			pc.mvp = glm::translate(glm::mat4(1.0f), glm::vec3(line->GetTransform().position, -line->GetTransform().depth));
			pc.mvp = glm::rotate(pc.mvp, glm::radians(line->GetTransform().rotation), glm::vec3(0, 0, 1));
			pc.mvp = p * view * glm::scale(pc.mvp, glm::vec3(line->GetTransform().scale.x, line->GetTransform().scale.y, 1.0f));
			commandBuffers_[0].pushConstants(pipelineLayout_, vk::ShaderStageFlagBits::eVertex, 0, sizeof(PMATH::push_constant), &pc);

			//TODO::Bind Index Buffer, Bind Descriptor Set.
			constexpr vk::DeviceSize offsets[] = { 0 };

			commandBuffers_[0].bindVertexBuffers(0, 1, &line->GetVertexBuffer(), offsets);

			commandBuffers_[0].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout_, 0, 1, descriptorSets_.data(), 0, nullptr);

			commandBuffers_[0].draw(
				static_cast<uint32_t>(line->GetPoints().size()),
				1,
				0,
				0);
		}

		if (drawData_)
		{
			ImGui_ImplVulkan_RenderDrawData(drawData_, commandBuffers_[0]);
		}

		commandBuffers_[0].endRenderPass();
	}
	commandBuffers_[0].end();

	linesToDraw_.clear();
	///////////

	constexpr vk::PipelineStageFlags wait_stages = { vk::PipelineStageFlagBits::eColorAttachmentOutput };

	vk::SubmitInfo submit_info(
		1,
		&imageAvailableSemaphore_,
		&wait_stages,
		static_cast<uint32_t>(commandBuffers_.size()),
		commandBuffers_.data(),
		1,
		&renderFinishedSemaphore_
	);

	vulkanContext_->get_graphics_queue().submit(submit_info, inFlightFence_);

	const vk::PresentInfoKHR present_info(
		1,
		&renderFinishedSemaphore_,
		1,
		&swapchain_.get_swapchain(),
		&nextImageIndex,
		nullptr
	);

	if (vulkanContext_->get_graphics_queue().presentKHR(&present_info) != vk::Result::eSuccess)
	{
		LOG_ERROR("Failed Presenting");
	}

	return true;
}

void Renderer::Shutdown()
{
	vulkanContext_->get_logical_device().waitIdle();

	vulkanContext_->get_logical_device().freeMemory(depthImageMemory_);
	vulkanContext_->get_logical_device().destroyImageView(depthImageView_);
	vulkanContext_->get_logical_device().destroyImage(depthImage_);

	vulkanContext_->get_logical_device().destroyCommandPool(commandPool_);

	for (const auto& frame_buffer : frameBuffers_)
	{
		vulkanContext_->get_logical_device().destroyFramebuffer(frame_buffer);
	}

	vulkanContext_->get_logical_device().freeMemory(uniformBufferMemory_);
	vulkanContext_->get_logical_device().destroyBuffer(uniformBuffer_);

	vulkanContext_->get_logical_device().destroySemaphore(imageAvailableSemaphore_);
	vulkanContext_->get_logical_device().destroySemaphore(renderFinishedSemaphore_);
	vulkanContext_->get_logical_device().destroyFence(inFlightFence_);

	vulkanContext_->get_logical_device().destroyDescriptorPool(descriptorPool_);
	vulkanContext_->get_logical_device().destroyDescriptorPool(imGuiDescriptorPool_);

	linePipeline_.Shutdown();
	vulkanContext_->get_logical_device().destroyPipelineLayout(pipelineLayout_);
	vulkanContext_->get_logical_device().destroyDescriptorSetLayout(descriptorSetLayout_);
	vulkanContext_->get_logical_device().destroyRenderPass(renderPass_);
	swapchain_.shutdown();
	LOG_INFO("Shutdown Renderer");
}

vk::RenderPass Renderer::CreateRenderPass() const
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
	return vulkanContext_->get_logical_device().createRenderPass(render_pass_info);
}

vk::DescriptorSetLayout Renderer::CreateDescriptorSetLayout() const
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
	return vulkanContext_->get_logical_device().createDescriptorSetLayout(descriptor_set_layout_info);
}

vk::PipelineLayout Renderer::CreatePipelineLayout() const
{
	const std::vector<vk::PushConstantRange> push_constants = {
		{
			vk::ShaderStageFlagBits::eVertex,
		   0,
		   sizeof(PMATH::push_constant)
		}
	};

	const vk::PipelineLayoutCreateInfo pipeline_layout_info(
		{},
		1,
		&descriptorSetLayout_,
		static_cast<uint32_t>(push_constants.size()),
		push_constants.data()
	);

	LOG_VULK("Created Pipeline Layout");
	return vulkanContext_->get_logical_device().createPipelineLayout(pipeline_layout_info);
}

vk::Image Renderer::CreateDepthImage() const
{
	const vk::ImageCreateInfo image_info(
		{},
		vk::ImageType::e2D,
		vk::Format::eD32Sfloat,
		{
			vulkanContext_->get_window().get_width(),
			vulkanContext_->get_window().get_height(),
			1
		},
		1,
		1,
		vk::SampleCountFlagBits::e1,
		vk::ImageTiling::eOptimal,
		vk::ImageUsageFlagBits::eDepthStencilAttachment
	);

	LOG_VULK("Created Depth Image");
	return vulkanContext_->get_logical_device().createImage(image_info);
}

vk::DeviceMemory Renderer::BindDepthImage() const
{
	const vk::MemoryRequirements memory_requirements = vulkanContext_->get_logical_device().getImageMemoryRequirements(depthImage_);

	//TODO::Why is this memory type index set to 1? need to check gpu i think.
	const vk::MemoryAllocateInfo memory_allocate_info(
		memory_requirements.size,
		1
	);

	const vk::DeviceMemory memory = vulkanContext_->get_logical_device().allocateMemory(memory_allocate_info);
	vulkanContext_->get_logical_device().bindImageMemory(depthImage_, memory, 0);

	LOG_VULK("Binded Depth Image Memory");
	return memory;
}

vk::ImageView Renderer::CreateDepthImageView() const
{
	const vk::ImageViewCreateInfo depth_image_view_info(
		{},
		depthImage_,
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
	return vulkanContext_->get_logical_device().createImageView(depth_image_view_info);
}

std::vector<vk::Framebuffer> Renderer::CreateFrameBuffers() const
{
	std::vector<vk::Framebuffer> frame_buffers(swapchain_.get_images().size());

	for (size_t i = 0; i < swapchain_.get_image_views().size(); i++)
	{
		std::vector<vk::ImageView> attachments = {
			swapchain_.get_image_views()[i],
			depthImageView_
		};

		vk::FramebufferCreateInfo frame_buffer_info(
			{},
			renderPass_,
			static_cast<uint32_t>(attachments.size()),
			attachments.data(),
			vulkanContext_->get_window().get_width(),
			vulkanContext_->get_window().get_height(),
			1
		);

		frame_buffers[i] = vulkanContext_->get_logical_device().createFramebuffer(frame_buffer_info);
	}

	LOG_VULK("Created Frame Buffers");
	return frame_buffers;
}

vk::CommandPool Renderer::CreateCommandPool() const
{
	const vk::CommandPoolCreateInfo command_pool_info(
		{},
		vulkanContext_->get_graphics_queue_index()
	);

	LOG_VULK("Created Command Pool");
	return vulkanContext_->get_logical_device().createCommandPool(command_pool_info);
}

vk::Buffer Renderer::CreateUniformBuffer() const
{
	constexpr vk::DeviceSize buffer_size = sizeof(PMATH::uniform_buffer_object);

	const vk::BufferCreateInfo buffer_info(
		{},
		buffer_size,
		vk::BufferUsageFlagBits::eUniformBuffer
	);

	LOG_VULK("Created Uniform Buffer");
	return vulkanContext_->get_logical_device().createBuffer(buffer_info);
}

vk::DeviceMemory Renderer::BindUniformBufferMemory() const
{
	const vk::MemoryRequirements memory_requirements = vulkanContext_->get_logical_device().getBufferMemoryRequirements(uniformBuffer_);

	const vk::MemoryAllocateInfo allocate_info(
		memory_requirements.size,
		2
	);

	const vk::DeviceMemory memory = vulkanContext_->get_logical_device().allocateMemory(allocate_info);

	vulkanContext_->get_logical_device().bindBufferMemory(uniformBuffer_, memory, 0);

	return memory;
}

vk::DescriptorPool Renderer::CreateDescriptorPool() const
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
	return vulkanContext_->get_logical_device().createDescriptorPool(descriptor_pool_info);
}



vk::DescriptorPool Renderer::CreateImGuiDescriptorPool() const
{
	const std::vector<vk::DescriptorPoolSize> pool_sizes{
			{ vk::DescriptorType::eSampler, 100 },
			{ vk::DescriptorType::eCombinedImageSampler, 100 },
			{ vk::DescriptorType::eSampledImage, 100 },
			{ vk::DescriptorType::eStorageImage, 100 },
			{ vk::DescriptorType::eUniformTexelBuffer, 100 },
			{ vk::DescriptorType::eStorageTexelBuffer, 100 },
			{ vk::DescriptorType::eUniformBuffer, 100 },
			{ vk::DescriptorType::eStorageBuffer, 100 },
			{ vk::DescriptorType::eUniformBufferDynamic, 100 },
			{ vk::DescriptorType::eStorageBufferDynamic, 100 },
			{ vk::DescriptorType::eInputAttachment, 100 }
	};

	const vk::DescriptorPoolCreateInfo descriptor_pool_info(
		vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
		100 * pool_sizes.size(),
		pool_sizes.size(),
		pool_sizes.data()
	);

	LOG_INFO("CREATED IMGUI DESCRIPTOR POOL");
	return vulkanContext_->get_logical_device().createDescriptorPool(descriptor_pool_info);
}

std::vector<vk::DescriptorSet> Renderer::CreateDescriptorSet() const
{
	const vk::DescriptorSetAllocateInfo allocate_info(
		imGuiDescriptorPool_,
		1,
		&descriptorSetLayout_
	);

	return vulkanContext_->get_logical_device().allocateDescriptorSets(allocate_info);
}

void Renderer::UpdateDescriptorSets()
{
	const std::vector<vk::DescriptorBufferInfo> buffer_infos{
		{
			uniformBuffer_,
			0,
			sizeof(PMATH::uniform_buffer_object)
		}
	};

	const std::vector<vk::WriteDescriptorSet> write_descriptor_sets = {
		{
			descriptorSets_[0],
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

	vulkanContext_->get_logical_device().updateDescriptorSets(
		static_cast<uint32_t>(write_descriptor_sets.size()),
		write_descriptor_sets.data(),
		static_cast<uint32_t>(copy_descriptor_sets.size()),
		copy_descriptor_sets.data()
	);
}

std::vector<vk::CommandBuffer> Renderer::CreateCommandBuffer() const
{
	const vk::CommandBufferAllocateInfo command_buffer_allocate_info(
		commandPool_,
		vk::CommandBufferLevel::ePrimary,
		1
	);

	return vulkanContext_->get_logical_device().allocateCommandBuffers(command_buffer_allocate_info);
}

vk::Semaphore Renderer::CreateSemaphore() const
{
	const vk::SemaphoreCreateInfo semaphore_info(
		{}
	);

	return vulkanContext_->get_logical_device().createSemaphore(semaphore_info);
}

vk::Fence Renderer::CreateFence() const
{
	const vk::FenceCreateInfo fence_info(
		vk::FenceCreateFlagBits::eSignaled
	);

	return vulkanContext_->get_logical_device().createFence(fence_info);
}

vk::Buffer Renderer::CreateVertexBuffer(const std::vector<PMATH::vertex> points) const
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

	return vulkanContext_->get_logical_device().createBuffer(buffer_info);
}

vk::DeviceMemory Renderer::BindVertexBufferMemory(const vk::Buffer vertexBuffer, const std::vector<PMATH::vertex> points) const
{
	if (points.empty())
	{
		LOG_WARN("Trying to bind empty buffer");
		return nullptr;
	}

	const vk::MemoryRequirements memory_requirements = vulkanContext_->get_logical_device().getBufferMemoryRequirements(vertexBuffer);

	const vk::MemoryAllocateInfo allocate_info(
		memory_requirements.size,
		2
	);

	const vk::DeviceMemory memory = vulkanContext_->get_logical_device().allocateMemory(allocate_info);

	vulkanContext_->get_logical_device().bindBufferMemory(vertexBuffer, memory, 0);

	void* data = vulkanContext_->get_logical_device().mapMemory(memory, 0, allocate_info.allocationSize, {});
	memcpy(data, points.data(), allocate_info.allocationSize);
	vulkanContext_->get_logical_device().unmapMemory(memory);

	return memory;
}

void Renderer::DestroyBuffer(const vk::Buffer buffer) const
{
	vulkanContext_->get_logical_device().destroyBuffer(buffer);
}

void Renderer::FreeMemory(const vk::DeviceMemory memory) const
{
	vulkanContext_->get_logical_device().freeMemory(memory);
}

void Renderer::Draw(LineComponent* lineComponent)
{
	const auto iterator = std::ranges::find(linesToDraw_, lineComponent);
	if (iterator != linesToDraw_.end())
	{
		return;
	}
	linesToDraw_.emplace_back(lineComponent);
}
