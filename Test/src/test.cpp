#pragma once
#include <chrono>
#include <filesystem>
#include <iostream>
#include <valarray>

#include "data_manager.h"
#include "graph.h"
#include "test_actor.h"
#include "Core/Application.h"
#include "Core/main.h"
#include "imgui.h"
#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_vulkan.h"
#include "Core/python_caller.h"

class test final : public application
{
protected:

	graph* big_graph_ = nullptr;
	float rotation_speed = 360.0f;
	glm::vec3 col = { 1,1,1 };
	const char* selected_stock{};
	int start_date = 0;
	int end_date = 100;

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
		
		// SECOND GRAPH
		std::string stock = "./data/AAPL-Year.csv";

		data_table data = load_data(stock);

		std::vector<float> x_vals = {};
		for (int i = data.size() * (start_date/100); i < data["Date"].size() * (end_date/100); i++)
		{
			x_vals.push_back(i);
		}

		plot second_plot = plot(x_vals, data["Low"], { 1, 1, 1 });

		const auto small_graph = new graph(this, { second_plot });
		
		small_graph->get_transform().position = glm::vec2(2.0f, 0.25f);
		small_graph->get_transform().scale = 0.5f;
		
		small_graph->show();

		// SECOND GRAPH END
		
		// THIRD GRAPH
		
		data_table data_third = load_data("./data/AAPL-Max.csv");
		
		std::vector<float> x_vals_third = {};
		for (int i = 0; i < data_third["Date"].size(); i++)
		{
			x_vals_third.push_back(i);
		}
		
		plot third_plot = plot(x_vals_third, data_third["Low"], { 1, 1, 1 });
		
		const auto small_graph_2 = new graph(this, { third_plot });
		
		small_graph_2->get_transform().position = glm::vec2(2.0f, 1.25f);
		small_graph_2->get_transform().scale = 0.5f;
		
		small_graph_2->show();
		// THIRD GRAPH END

		std::string date_time_format = "%Y-%m-%d";
		
		std::istringstream string_date{"2020-1-28"};
		
		std::chrono::year_month_day date{};
		
		string_date >> std::chrono::parse(date_time_format, date);
		
		using namespace std::chrono;
		using namespace std;
		auto first = 2012y/1/24;

		std::cout << "Year: " << date.year() << "\n";
		std::cout << "Month: " << date.month() << "\n";
		std::cout << "Day: " << date.day() << "\n";
		
		std::cout << "Difference: " << std::chrono::sys_days{ date } - std::chrono::sys_days{first} << std::endl;
	}

	void load_graph()
	{
		delete big_graph_;

		// LOADING DATA
		std::string stock;
		if (selected_stock)
		{
			std::string t = "./data/";
			stock = t + selected_stock;
		} else
		{
			stock = "./data/AAPL-Year.csv";
		}
		data_table data = load_data(stock);

		std::vector<float> x_vals = {};
		for (int i = (int)(data["Date"].size() * (start_date / 100.0f)); i < (int)(data["Date"].size() * (end_date / 100.0f)); i++)
		{
			x_vals.push_back(i);
		}
		// DATA LOADED

		// ACTUAL PLOTS
		auto start = data["Low"].begin() + (int)(data["Low"].size() * (start_date / 100.0f));
		auto end = data["Low"].begin() + (int)(data["Low"].size() * (end_date / 100.0f));

		std::vector<float> to_plot((int)(data["Low"].size() * (end_date / 100.0f)) - (int)(data["Low"].size() * (start_date / 100.0f)));

		std::copy(start, end, to_plot.begin());
		
		const auto low_plot = plot(x_vals, to_plot, col);
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
		static bool need_to_load;

		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Debug Menu");

		static float f = 0.0f;
		static int counter = 0;
		static int selected = 0;

		ImGui::Text("Change Graph");

		static std::vector<std::string> stocks = {};

		if (stocks.empty())
		{
			for (const auto& entry : std::filesystem::directory_iterator("./data/"))
			{
				stocks.push_back(entry.path().filename().string());
			}

			for (const auto& stock : stocks)
			{
				LOG_INFO("%s", stock.c_str());
			}
		}

		std::vector<const char*> stock_p = {};

		stock_p.reserve(stocks.size());
		for (const auto& stock : stocks)
		{
			stock_p.push_back(stock.c_str());
		}

		if (ImGui::ListBox("Stock", &selected, stock_p.data(), stock_p.size()))
		{
			selected_stock = stock_p[selected];
		}

		if (ImGui::DragInt("Start Date", &start_date, 1, 0, end_date - 1))
			need_to_load = true;
		if (ImGui::DragInt("End Date", &end_date, 1, start_date + 1, 100))
			need_to_load = true;


		if (ImGui::Button("Python", {200, 20}))
		{
			static python_caller pyth("pytest");
			pyth.call_function("func1");
		}

		static float col_arr[3] = { 1, 1, 1 };
		ImGui::ColorPicker3("Graph Color", col_arr);
		col = { col_arr[0], col_arr[1], col_arr[2] };

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", static_cast<double>(1000.0f / ImGui::GetIO().Framerate), static_cast<double>(ImGui::GetIO().Framerate));
		ImGui::End();


		ImGui::Render();

		renderer_.draw_data = ImGui::GetDrawData();

		// if (renderer_.draw_data)
		// {
		// 	ImGui_ImplVulkan_RenderDrawData(renderer_.draw_data, renderer_.command_buffers_[0]);
		// }

		static const char* last_selected = "";
		static glm::vec3 last_color;
		if (selected_stock != last_selected ||
			col != last_color ||
			need_to_load)
		{
			vulkan_context_.get_logical_device().waitIdle();
			load_graph();
			last_selected = selected_stock;
			need_to_load = false;
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
