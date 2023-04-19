#include "pcHeader.h"
#include "Renderer.h"
#include "VulkanContext.h"
#include "backends/imgui_impl_vulkan.h"
#include "backends/imgui_impl_win32.h"
#include "Core/Application.h"
#include "Core/Components/CLineChart.h"
#include "Renderer/Window.h"

bool Renderer::Init(VulkanContext* vulkanContext)
{
	LOG_FUNC_START();

	vulkanContext_ = vulkanContext;

	swapchain_.Init(vulkanContext_);

	renderPass_ = CreateRenderPass();
	descriptorSetLayout_ = CreateDescriptorSetLayout();
	pipelineLayout_ = CreatePipelineLayout();

	const std::vector<ShaderInfo> shaderInfos = {
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
		static_cast<float>(vulkanContext_->GetWindow().GetWidth()),
		static_cast<float>(vulkanContext_->GetWindow().GetHeight()),
		0.0f, 1.0f
	};
	
	scissor_ = vk::Rect2D{
		{
			0,
			0
		},
		{
			(vulkanContext_->GetWindow().GetWidth()),
			(vulkanContext_->GetWindow().GetHeight())
		}
	};

	GraphicsPipelineCreateInfo graphicsPipelineInfo{};
	graphicsPipelineInfo.logicalDevice = vulkanContext_->GetLogicalDevice();
	graphicsPipelineInfo.shaderInfos = shaderInfos;
	graphicsPipelineInfo.pipelineLayout = pipelineLayout_;
	graphicsPipelineInfo.renderPass = renderPass_;
	graphicsPipelineInfo.primitiveTopology = vk::PrimitiveTopology::eLineStrip;
	graphicsPipelineInfo.polygonMode = vk::PolygonMode::eFill;
	graphicsPipelineInfo.cullMode = vk::CullModeFlagBits::eNone;
	graphicsPipelineInfo.viewports = { mainViewport_};
	graphicsPipelineInfo.scissors = { scissor_ };
	graphicsPipelineInfo.dynamicStates = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };

	linePipeline_.Init(graphicsPipelineInfo);

	depthImage_ = CreateDepthImage();
	depthImageMemory_ = BindDepthImage();
	depthImageView_ = CreateDepthImageView();

	colourImage_ = CreateColourImage();
	colourImageMemory_ = BindColourImage();
	colourImageView_ = CreateColourImageView();

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

	lastWindowHeight_ = vulkanContext_->GetWindow().GetHeight();
	lastWindowWidth_ = vulkanContext_->GetWindow().GetWidth();

	LOG_FUNC_END();
	return true;
}

bool Renderer::Update()
{
	if (vulkanContext_->GetLogicalDevice().waitForFences(1, &inFlightFence_, VK_TRUE, 0) != vk::Result::eSuccess)
	{
		return true;
	}
	
	if (vulkanContext_->GetLogicalDevice().resetFences(1, &inFlightFence_) != vk::Result::eSuccess)
	{
		
		return false;
	}

	if ( vulkanContext_->GetWindow().GetHeight() != lastWindowHeight_ || vulkanContext_->GetWindow().GetWidth() != lastWindowWidth_ )
	{
		swapchain_.Recreate();

		vulkanContext_->GetLogicalDevice().freeMemory(depthImageMemory_);
		vulkanContext_->GetLogicalDevice().destroyImageView(depthImageView_);
		vulkanContext_->GetLogicalDevice().destroyImage(depthImage_);

		vulkanContext_->GetLogicalDevice().freeMemory(colourImageMemory_);
		vulkanContext_->GetLogicalDevice().destroyImageView(colourImageView_);
		vulkanContext_->GetLogicalDevice().destroyImage(colourImage_);

		for ( const auto& frameBuffer : frameBuffers_ )
		{
			vulkanContext_->GetLogicalDevice().destroyFramebuffer(frameBuffer);
		}

		depthImage_ = CreateDepthImage();
		depthImageMemory_ = BindDepthImage();
		depthImageView_ = CreateDepthImageView();

		colourImage_ = CreateColourImage();
		colourImageMemory_ = BindColourImage();
		colourImageView_ = CreateColourImageView();

		frameBuffers_ = CreateFrameBuffers();

		mainViewport_ = vk::Viewport{
			0.0f,
			0.0f,
			static_cast<float>(vulkanContext_->GetWindow().GetWidth()),
			static_cast<float>(vulkanContext_->GetWindow().GetHeight()),
			0.0f, 1.0f
		};

		scissor_ = vk::Rect2D{
			{
				0,
				0
			},
			{
				(vulkanContext_->GetWindow().GetWidth()),
				(vulkanContext_->GetWindow().GetHeight())
			}
		};

		lastWindowHeight_ = vulkanContext_->GetWindow().GetHeight();
		lastWindowWidth_ = vulkanContext_->GetWindow().GetWidth();
	}

	if ( Window::closed_ ) return false;

	const vk::ResultValue<uint32_t> nextImageIndexResult = vulkanContext_->GetLogicalDevice().acquireNextImageKHR(swapchain_.GetSwapchain(), 0, imageAvailableSemaphore_, VK_NULL_HANDLE);
	if (nextImageIndexResult.result != vk::Result::eSuccess)
	{
		return true;
	}

	const uint32_t nextImageIndex = nextImageIndexResult.value;

	vulkanContext_->GetLogicalDevice().resetCommandPool(commandPool_);

	///////////
	constexpr vk::CommandBufferBeginInfo info(
		{},
		nullptr
	);

	constexpr vk::ClearValue clearValue(std::array<float, 4>({ {0.9f, 0.94f, 0.93f, 1.0f } }));
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
			{ vulkanContext_->GetWindow().GetWidth(), vulkanContext_->GetWindow().GetHeight() }
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

	const dv_math::UniformBufferObject mvp{ p * view * glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f)) };

	void* data = vulkanContext_->GetLogicalDevice().mapMemory(uniformBufferMemory_, 0, sizeof(dv_math::UniformBufferObject), {});
	memcpy(data, &mvp, sizeof(dv_math::UniformBufferObject));
	vulkanContext_->GetLogicalDevice().unmapMemory(uniformBufferMemory_);

	//TODO::Add List of meshes to draw and loop them in here.
	commandBuffers_[0].begin(info);
	{
		commandBuffers_[0].setViewport(0, 1, &mainViewport_);
		commandBuffers_[0].setScissor(0, 1, &scissor_);

		commandBuffers_[0].beginRenderPass(&renderPassBeginInfo, vk::SubpassContents::eInline);


		linePipeline_.BindPipeline(commandBuffers_[0]);


		for (auto& line : linesToDraw_)
		{
			if ( line->name_.length() > 100 ) continue;
			if ( !line->GetVertexBuffer() ) continue;

			//TODO::Pass Something useful to push constant including mvp.
			dv_math::PushConstant pc{};
			pc.color = glm::vec4(line->GetColor(), 1.0f);
			pc.mvp = glm::translate(glm::mat4(1.0f), glm::vec3(line->GetTransform().position, -line->GetTransform().depth + line->GetOrder()));
			pc.mvp = glm::rotate(pc.mvp, glm::radians(line->GetTransform().rotation), glm::vec3(0, 0, 1));
			pc.mvp = p * view * glm::scale(pc.mvp, glm::vec3(line->GetTransform().scale.x, line->GetTransform().scale.y, 1.0f));

			commandBuffers_[0].pushConstants(pipelineLayout_, vk::ShaderStageFlagBits::eVertex, 0, sizeof(dv_math::PushConstant), &pc);
			
			constexpr vk::DeviceSize offsets[] = { 0 };

			commandBuffers_[0].bindVertexBuffers(0, 1, &line->GetVertexBuffer(), offsets);

			commandBuffers_[0].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout_, 0, 1, descriptorSets_.data(), 0, nullptr);

			commandBuffers_[0].draw(
				static_cast<uint32_t>(line->GetNumberOfPoints()),
				1,
				static_cast<uint32_t>(line->GetStartPoints()),
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

	constexpr vk::PipelineStageFlags waitStages = { vk::PipelineStageFlagBits::eColorAttachmentOutput };

	vk::SubmitInfo submitInfo(
		1,
		&imageAvailableSemaphore_,
		&waitStages,
		static_cast<uint32_t>(commandBuffers_.size()),
		commandBuffers_.data(),
		1,
		&renderFinishedSemaphore_
	);

	vulkanContext_->GetGraphicsQueue().submit(submitInfo, inFlightFence_);

	const vk::PresentInfoKHR presentInfo(
		1,
		&renderFinishedSemaphore_,
		1,
		&swapchain_.GetSwapchain(),
		&nextImageIndex,
		nullptr
	);

	if (vulkanContext_->GetGraphicsQueue().presentKHR(&presentInfo) != vk::Result::eSuccess)
	{
		LOG_ERROR("Failed Presenting");
	}

	return true;
}

void Renderer::Shutdown()
{
	vulkanContext_->GetLogicalDevice().waitIdle();

	vulkanContext_->GetLogicalDevice().freeMemory(depthImageMemory_);
	vulkanContext_->GetLogicalDevice().destroyImageView(depthImageView_);
	vulkanContext_->GetLogicalDevice().destroyImage(depthImage_);

	vulkanContext_->GetLogicalDevice().freeMemory(colourImageMemory_);
	vulkanContext_->GetLogicalDevice().destroyImageView(colourImageView_);
	vulkanContext_->GetLogicalDevice().destroyImage(colourImage_);

	vulkanContext_->GetLogicalDevice().destroyCommandPool(commandPool_);

	for (const auto& frameBuffer : frameBuffers_)
	{
		vulkanContext_->GetLogicalDevice().destroyFramebuffer(frameBuffer);
	}

	vulkanContext_->GetLogicalDevice().freeMemory(uniformBufferMemory_);
	vulkanContext_->GetLogicalDevice().destroyBuffer(uniformBuffer_);

	vulkanContext_->GetLogicalDevice().destroySemaphore(imageAvailableSemaphore_);
	vulkanContext_->GetLogicalDevice().destroySemaphore(renderFinishedSemaphore_);
	vulkanContext_->GetLogicalDevice().destroyFence(inFlightFence_);

	vulkanContext_->GetLogicalDevice().destroyDescriptorPool(descriptorPool_);
	vulkanContext_->GetLogicalDevice().destroyDescriptorPool(imGuiDescriptorPool_);

	linePipeline_.Shutdown();
	vulkanContext_->GetLogicalDevice().destroyPipelineLayout(pipelineLayout_);
	vulkanContext_->GetLogicalDevice().destroyDescriptorSetLayout(descriptorSetLayout_);
	vulkanContext_->GetLogicalDevice().destroyRenderPass(renderPass_);
	swapchain_.Shutdown();
	LOG_INFO("Shutdown Renderer");
}

vk::RenderPass Renderer::CreateRenderPass() const
{
	const std::vector<vk::AttachmentDescription> attachments = {
		{
			{},
			swapchain_.GetDetails().surfaceFormat.format,
			vk::SampleCountFlagBits::e8,
			vk::AttachmentLoadOp::eClear,
			vk::AttachmentStoreOp::eStore,
			vk::AttachmentLoadOp::eDontCare,
			vk::AttachmentStoreOp::eDontCare,
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::eColorAttachmentOptimal
		},
		{
			{},
			vk::Format::eD32Sfloat,
			vk::SampleCountFlagBits::e8,
			vk::AttachmentLoadOp::eClear,
			vk::AttachmentStoreOp::eStore,
			vk::AttachmentLoadOp::eClear,
			vk::AttachmentStoreOp::eDontCare,
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::eDepthStencilAttachmentOptimal
		},
		{
			{},
			swapchain_.GetDetails().surfaceFormat.format,
			vk::SampleCountFlagBits::e1,
			vk::AttachmentLoadOp::eDontCare,
			vk::AttachmentStoreOp::eStore,
			vk::AttachmentLoadOp::eDontCare,
			vk::AttachmentStoreOp::eDontCare,
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::ePresentSrcKHR
		}
	};

	std::vector<vk::AttachmentReference> colorAttachments = {
		{
			0,
			vk::ImageLayout::eColorAttachmentOptimal
		}
	};

	vk::AttachmentReference depthAttachment(
		1,
		vk::ImageLayout::eDepthStencilAttachmentOptimal
	);

	vk::AttachmentReference colourAttachment(
		2,
		vk::ImageLayout::eColorAttachmentOptimal
	);

	const std::vector<vk::SubpassDescription> subpasses = {
		{
			{},
			vk::PipelineBindPoint::eGraphics,
			0,
			nullptr,
			static_cast<uint32_t>(colorAttachments.size()),
			colorAttachments.data(),
			&colourAttachment,
			&depthAttachment,
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

	const vk::RenderPassCreateInfo renderPassInfo(
		{},
		static_cast<uint32_t>(attachments.size()),
		attachments.data(),
		static_cast<uint32_t>(subpasses.size()),
		subpasses.data(),
		static_cast<uint32_t>(dependencies.size()),
		dependencies.data()
	);

	LOG_VULK("Created Render Pass");
	return vulkanContext_->GetLogicalDevice().createRenderPass(renderPassInfo);
}

vk::DescriptorSetLayout Renderer::CreateDescriptorSetLayout() const
{
	const std::vector<vk::DescriptorSetLayoutBinding> uniformLayoutBindings = {
		{
			0,
			vk::DescriptorType::eUniformBuffer,
			1,
			vk::ShaderStageFlagBits::eVertex,
			nullptr
		}
	};

	const vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutInfo(
		{},
		static_cast<uint32_t>(uniformLayoutBindings.size()),
		uniformLayoutBindings.data()
	);

	LOG_VULK("Created Descriptor Set Layout");
	return vulkanContext_->GetLogicalDevice().createDescriptorSetLayout(descriptorSetLayoutInfo);
}

vk::PipelineLayout Renderer::CreatePipelineLayout() const
{
	const std::vector<vk::PushConstantRange> pushConstants = {
		{
			vk::ShaderStageFlagBits::eVertex,
		   0,
		   sizeof(dv_math::PushConstant)
		}
	};

	const vk::PipelineLayoutCreateInfo pipelineLayoutInfo(
		{},
		1,
		&descriptorSetLayout_,
		static_cast<uint32_t>(pushConstants.size()),
		pushConstants.data()
	);

	LOG_VULK("Created Pipeline Layout");
	return vulkanContext_->GetLogicalDevice().createPipelineLayout(pipelineLayoutInfo);
}

vk::Image Renderer::CreateDepthImage() const
{
	const vk::ImageCreateInfo imageInfo(
		{},
		vk::ImageType::e2D,
		vk::Format::eD32Sfloat,
		{
			vulkanContext_->GetWindow().GetWidth(),
			vulkanContext_->GetWindow().GetHeight(),
			1
		},
		1,
		1,
		vk::SampleCountFlagBits::e8,
		vk::ImageTiling::eOptimal,
		vk::ImageUsageFlagBits::eDepthStencilAttachment
	);

	LOG_VULK("Created Depth Image");
	return vulkanContext_->GetLogicalDevice().createImage(imageInfo);
}

vk::Image Renderer::CreateColourImage() const
{
	const vk::ImageCreateInfo imageInfo(
		{},
		vk::ImageType::e2D,
		swapchain_.GetDetails().surfaceFormat.format,
		{
			vulkanContext_->GetWindow().GetWidth(),
			vulkanContext_->GetWindow().GetHeight(),
			1
		},
		1,
		1,
		vk::SampleCountFlagBits::e8,
		vk::ImageTiling::eOptimal,
		vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransientAttachment
	);

	LOG_VULK("Created Colour Image");
	return vulkanContext_->GetLogicalDevice().createImage(imageInfo);
}

vk::DeviceMemory Renderer::BindDepthImage() const
{
	const vk::MemoryRequirements memoryRequirements = vulkanContext_->GetLogicalDevice().getImageMemoryRequirements(depthImage_);

	//TODO::Why is this memory type index set to 1? need to check gpu i think.
	const vk::MemoryAllocateInfo memoryAllocateInfo(
		memoryRequirements.size,
		1
	);

	const vk::DeviceMemory memory = vulkanContext_->GetLogicalDevice().allocateMemory(memoryAllocateInfo);
	vulkanContext_->GetLogicalDevice().bindImageMemory(depthImage_, memory, 0);

	LOG_VULK("Binded Depth Image Memory");
	return memory;
}

vk::DeviceMemory Renderer::BindColourImage() const
{
	const vk::MemoryRequirements memoryRequirements = vulkanContext_->GetLogicalDevice().getImageMemoryRequirements(colourImage_);

	//TODO::Why is this memory type index set to 1? need to check gpu i think.
	const vk::MemoryAllocateInfo memoryAllocateInfo(
		memoryRequirements.size,
		1
	);

	const vk::DeviceMemory memory = vulkanContext_->GetLogicalDevice().allocateMemory(memoryAllocateInfo);
	vulkanContext_->GetLogicalDevice().bindImageMemory(colourImage_, memory, 0);

	LOG_VULK("Binded Depth Image Memory");
	return memory;
}

vk::ImageView Renderer::CreateDepthImageView() const
{
	const vk::ImageViewCreateInfo depthImageViewInfo(
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
	return vulkanContext_->GetLogicalDevice().createImageView(depthImageViewInfo);
}

vk::ImageView Renderer::CreateColourImageView() const
{
	const vk::ImageViewCreateInfo colourImageViewInfo(
		{},
		colourImage_,
		vk::ImageViewType::e2D,
		swapchain_.GetDetails().surfaceFormat.format,
		{},
		{
			vk::ImageAspectFlagBits::eColor,
			0,
			1,
			0,
			1
		}
		);

	LOG_VULK("Created Colour Image View");
	return vulkanContext_->GetLogicalDevice().createImageView(colourImageViewInfo);
}

std::vector<vk::Framebuffer> Renderer::CreateFrameBuffers() const
{
	std::vector<vk::Framebuffer> frameBuffers(swapchain_.GetImages().size());

	for (size_t i = 0; i < swapchain_.GetImageViews().size(); i++)
	{
		std::vector<vk::ImageView> attachments = {
			colourImageView_,
			depthImageView_,
			swapchain_.GetImageViews()[i]
		};

		vk::FramebufferCreateInfo frameBufferInfo(
			{},
			renderPass_,
			static_cast<uint32_t>(attachments.size()),
			attachments.data(),
			vulkanContext_->GetWindow().GetWidth(),
			vulkanContext_->GetWindow().GetHeight(),
			1
		);

		frameBuffers[i] = vulkanContext_->GetLogicalDevice().createFramebuffer(frameBufferInfo);
	}

	LOG_VULK("Created Frame Buffers");
	return frameBuffers;
}

vk::CommandPool Renderer::CreateCommandPool() const
{
	const vk::CommandPoolCreateInfo commandPoolInfo(
		{},
		vulkanContext_->GetGraphicsQueueIndex()
	);

	LOG_VULK("Created Command Pool");
	return vulkanContext_->GetLogicalDevice().createCommandPool(commandPoolInfo);
}

vk::Buffer Renderer::CreateUniformBuffer() const
{
	constexpr vk::DeviceSize bufferSize = sizeof(dv_math::UniformBufferObject);

	constexpr vk::BufferCreateInfo bufferInfo(
		{},
		bufferSize,
		vk::BufferUsageFlagBits::eUniformBuffer
	);

	LOG_VULK("Created Uniform Buffer");
	return vulkanContext_->GetLogicalDevice().createBuffer(bufferInfo);
}

vk::DeviceMemory Renderer::BindUniformBufferMemory() const
{
	const vk::MemoryRequirements memoryRequirements = vulkanContext_->GetLogicalDevice().getBufferMemoryRequirements(uniformBuffer_);

	const vk::MemoryAllocateInfo allocateInfo(
		memoryRequirements.size,
		2
	);

	const vk::DeviceMemory memory = vulkanContext_->GetLogicalDevice().allocateMemory(allocateInfo);

	vulkanContext_->GetLogicalDevice().bindBufferMemory(uniformBuffer_, memory, 0);

	return memory;
}

vk::DescriptorPool Renderer::CreateDescriptorPool() const
{
	const std::vector<vk::DescriptorPoolSize> descriptorPoolSizes{
		{
			vk::DescriptorType::eUniformBuffer,
			1
		}
	};

	const vk::DescriptorPoolCreateInfo descriptorPoolInfo(
		{},
		1,
		static_cast<uint32_t>(descriptorPoolSizes.size()),
		descriptorPoolSizes.data()
	);

	LOG_VULK("Created Descriptor Pool");
	return vulkanContext_->GetLogicalDevice().createDescriptorPool(descriptorPoolInfo);
}



vk::DescriptorPool Renderer::CreateImGuiDescriptorPool() const
{
	const std::vector<vk::DescriptorPoolSize> poolSizes{
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

	const vk::DescriptorPoolCreateInfo descriptorPoolInfo(
		vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
		100u * static_cast<uint32_t>(poolSizes.size()),
		static_cast<uint32_t>(poolSizes.size()),
		poolSizes.data()
	);

	LOG_INFO("CREATED IMGUI DESCRIPTOR POOL");
	return vulkanContext_->GetLogicalDevice().createDescriptorPool(descriptorPoolInfo);
}

std::vector<vk::DescriptorSet> Renderer::CreateDescriptorSet() const
{
	const vk::DescriptorSetAllocateInfo allocateInfo(
		imGuiDescriptorPool_,
		1,
		&descriptorSetLayout_
	);

	return vulkanContext_->GetLogicalDevice().allocateDescriptorSets(allocateInfo);
}

void Renderer::UpdateDescriptorSets()
{
	const std::vector<vk::DescriptorBufferInfo> bufferInfos{
		{
			uniformBuffer_,
			0,
			sizeof(dv_math::UniformBufferObject)
		}
	};

	const std::vector<vk::WriteDescriptorSet> writeDescriptorSets = {
		{
			descriptorSets_[0],
		   0,
		   0,
		   static_cast<uint32_t>(bufferInfos.size()),
		   vk::DescriptorType::eUniformBuffer,
		   nullptr,
		   bufferInfos.data(),
		   nullptr
		}
	};

	const std::vector<vk::CopyDescriptorSet> copyDescriptorSets = {};

	vulkanContext_->GetLogicalDevice().updateDescriptorSets(
		static_cast<uint32_t>(writeDescriptorSets.size()),
		writeDescriptorSets.data(),
		static_cast<uint32_t>(copyDescriptorSets.size()),
		copyDescriptorSets.data()
	);
}

std::vector<vk::CommandBuffer> Renderer::CreateCommandBuffer() const
{
	const vk::CommandBufferAllocateInfo commandBufferAllocateInfo(
		commandPool_,
		vk::CommandBufferLevel::ePrimary,
		1
	);

	return vulkanContext_->GetLogicalDevice().allocateCommandBuffers(commandBufferAllocateInfo);
}

vk::Semaphore Renderer::CreateSemaphore() const
{
	const vk::SemaphoreCreateInfo semaphoreInfo(
		{}
	);

	return vulkanContext_->GetLogicalDevice().createSemaphore(semaphoreInfo);
}

vk::Fence Renderer::CreateFence() const
{
	constexpr vk::FenceCreateInfo fenceInfo(
		vk::FenceCreateFlagBits::eSignaled
	);

	return vulkanContext_->GetLogicalDevice().createFence(fenceInfo);
}

vk::Buffer Renderer::CreateVertexBuffer(const std::vector<dv_math::Vertex>& points) const
{
	if (points.empty())
	{
		LOG_WARN("Trying to create an empty buffer");
		return nullptr;
	}

	const vk::BufferCreateInfo bufferInfo(
		{},
		sizeof(points[0]) * points.size(),
		vk::BufferUsageFlagBits::eVertexBuffer
	);

	return vulkanContext_->GetLogicalDevice().createBuffer(bufferInfo);
}

vk::DeviceMemory Renderer::BindVertexBufferMemory(const vk::Buffer vertexBuffer, const std::vector<dv_math::Vertex>& points) const
{
	if (points.empty())
	{
		LOG_WARN("Trying to bind empty buffer");
		return nullptr;
	}

	const vk::MemoryRequirements memoryRequirements = vulkanContext_->GetLogicalDevice().getBufferMemoryRequirements(vertexBuffer);

	const vk::MemoryAllocateInfo allocateInfo(
		memoryRequirements.size,
		2
	);

	const vk::DeviceMemory memory = vulkanContext_->GetLogicalDevice().allocateMemory(allocateInfo);
	vulkanContext_->GetLogicalDevice().bindBufferMemory(vertexBuffer, memory, 0);

	return memory;
}

void* Renderer::MapMemory(const vk::Buffer vertexBuffer, const vk::DeviceMemory memory) const
{
	const vk::MemoryRequirements memoryRequirements = vulkanContext_->GetLogicalDevice().getBufferMemoryRequirements(vertexBuffer);

	const vk::MemoryAllocateInfo allocateInfo(
		memoryRequirements.size,
		2
	);

	void* data = vulkanContext_->GetLogicalDevice().mapMemory(memory, 0, allocateInfo.allocationSize, {});
	return data;
}

void Renderer::WriteToVertexBufferMemory(const vk::Buffer vertexBuffer, void* data, const std::vector<dv_math::Vertex>& points) const
{
	const vk::MemoryRequirements memoryRequirements = vulkanContext_->GetLogicalDevice().getBufferMemoryRequirements(vertexBuffer);

	const vk::MemoryAllocateInfo allocateInfo(
		memoryRequirements.size,
		2
	);

	memcpy(data, points.data(), allocateInfo.allocationSize);
}

void Renderer::UnMapMemory(const vk::DeviceMemory memory) const
{
	vulkanContext_->GetLogicalDevice().unmapMemory(memory);
}

void Renderer::DestroyBuffer(const vk::Buffer buffer) const
{
	vulkanContext_->GetLogicalDevice().destroyBuffer(buffer);
}

void Renderer::FreeMemory(const vk::DeviceMemory memory) const
{
	vulkanContext_->GetLogicalDevice().freeMemory(memory);
}

void Renderer::Draw(LineComponent* lineComponent)
{
	std::string name = lineComponent->name_;
	if (name.length() > 100)
	{
		
	}
	const auto iterator = std::ranges::find(linesToDraw_, lineComponent);
	if (iterator != linesToDraw_.end())
	{
		return;
	}
	linesToDraw_.emplace_back(lineComponent);
}
