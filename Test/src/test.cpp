#pragma once
#include <chrono>
#include <iostream>

#include "data_manager.h"
#include "graph.h"
#include "test_actor.h"
#include "Core/Application.h"
#include "Core/main.h"
#include "imgui.h"
#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_vulkan.h"

class test final : public application
{
protected:

	graph* big_graph_ = nullptr;
	float rotation_speed = 360.0f;

	void on_startup() override
	{		
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		
		ImGui::StyleColorsDark();
		
		ImGui_ImplWin32_Init(vulkan_context_.get_window().get_hwnd());
		ImGui_ImplVulkan_InitInfo init_info = {};
		init_info.Instance = vulkan_context_.get_instance();
		init_info.PhysicalDevice = vulkan_context_.get_physical_device();
		init_info.Device = vulkan_context_.get_logical_device();
		init_info.QueueFamily = vulkan_context_.get_graphics_queue_index();
		init_info.Queue = vulkan_context_.get_graphics_queue();
		init_info.PipelineCache = nullptr;
		init_info.DescriptorPool = renderer_.im_gui_descriptor_pool_;
		init_info.Subpass = 0;
		init_info.MinImageCount = 3;
		init_info.ImageCount = renderer_.swapchain_.get_images().size();
		init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		init_info.Allocator = nullptr;
		init_info.CheckVkResultFn = nullptr;
		ImGui_ImplVulkan_Init(&init_info, renderer_.get_render_pass());
		
		// Upload Fonts
		{
			// Use any command queue
			VkCommandPool command_pool = renderer_.command_pool_;
			VkCommandBuffer command_buffer = renderer_.command_buffers_[0];
		
			vkResetCommandPool(vulkan_context_.get_logical_device(), command_pool, 0);
			VkCommandBufferBeginInfo begin_info = {};
			begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
			vkBeginCommandBuffer(command_buffer, &begin_info);
		
			ImGui_ImplVulkan_CreateFontsTexture(command_buffer);
		
			VkSubmitInfo end_info = {};
			end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			end_info.commandBufferCount = 1;
			end_info.pCommandBuffers = &command_buffer;
			vkEndCommandBuffer(command_buffer);
			vkQueueSubmit(vulkan_context_.get_graphics_queue(), 1, &end_info, VK_NULL_HANDLE);
		
			vkDeviceWaitIdle(vulkan_context_.get_logical_device());
			ImGui_ImplVulkan_DestroyFontUploadObjects();
		}

		load_graph();

		// FIRST GRAPH END
		//
		// // SECOND GRAPH
		// const auto small_graph = new graph(this, {high_plot});
		//
		// small_graph->get_transform().position = glm::vec2(2.0f, 0.25f);
		// small_graph->get_transform().scale = 0.5f;
		//
		// small_graph->show();
		// // SECOND GRAPH END
		//
		// // THIRD GRAPH
		// const auto small_graph_2 = new graph(this, { low_plot });
		//
		// small_graph_2->get_transform().position = glm::vec2(2.0f, 1.25f);
		// small_graph_2->get_transform().scale = 0.5f;
		//
		// small_graph_2->show();
		// // THIRD GRAPH END

		// std::string date_time_format = "%Y-%m-%d";
		//
		// std::istringstream string_date{"2020-1-28"};
		//
		// std::chrono::year_month_day date{};
		//
		// string_date >> std::chrono::parse(date_time_format, date);
		//
		// using namespace std::chrono;
		// using namespace std;
		// auto first = 2012y/1/24;

		// std::cout << "Year: " << date.year() << "\n";
		// std::cout << "Month: " << date.month() << "\n";
		// std::cout << "Day: " << date.day() << "\n";
		//
		// std::cout << "Difference: " << std::chrono::sys_days{ date } - std::chrono::sys_days{first} << std::endl;
	}

	void load_graph()
	{
		delete big_graph_;

		// LOADING DATA
		std::string stock;
		if (renderer_.selected_stock)
		{
			std::string t = "./data/";
			stock = t + renderer_.selected_stock + "-Year.csv";
		} else
		{
			stock = "./data/AAPL-Year.csv";
		}
		data_table data = load_data(stock);

		std::vector<float> x_vals = {};
		for (int i = 0; i < data["Date"].size(); i++)
		{
			x_vals.push_back(i);
		}
		// DATA LOADED

		// ACTUAL PLOTS
		const auto low_plot = plot(x_vals, data["Low"], renderer_.selected_color);
		// PLOTS

		// FIRST GRAPH
		big_graph_ = new graph(this, { low_plot });

		big_graph_->get_transform().scale = 1.5f;
		big_graph_->get_transform().rotation = 0.0f;
		big_graph_->get_transform().position = glm::vec2(0.25f, 0.25f);

		big_graph_->show();
	}

	void on_update(float delta_time) override
	{
		// big_graph_->get_transform().rotation += rotation_speed * delta_time;
		// if (big_graph_->get_transform().rotation > 360.0f || big_graph_->get_transform().rotation < -0.0f) 
		// {
		// 	rotation_speed *= -1;
		// }
		// big_graph_->get_transform().position.x = (big_graph_->get_transform().rotation / 360.0f) + 0.5f;
		// big_graph_->get_transform().scale += 0.1f * delta_time;
		static const char* last_selected = "";
		static glm::vec3 last_color = renderer_.selected_color;
		if (renderer_.selected_stock != last_selected ||
			renderer_.selected_color != last_color)
		{
			vulkan_context_.get_logical_device().waitIdle();
			load_graph();
			last_selected = renderer_.selected_stock;
		}
	}

	void on_shutdown() override
	{
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}

	void on_ui(float delta_time) override
	{
	}
	
};

application* create_application()
{
	return new test();
}
