#pragma once
#include <chrono>
#include <filesystem>
#include <functional>
#include <iostream>
#include <numeric>
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
#include "Visualisation/AGraph.h"

class test final : public application
{
protected:

	AGraph* big_graph_ = nullptr;
	AGraph* volumeGraph_ = nullptr;
	plot* model_plot_ = nullptr;
	graph* small_graph_ = nullptr;
	glm::vec3 col_ = { 0.86f, 0.43f, 0.1f };
	std::string selected_stock_{};
	std::string selected_algorithm_{};
	int start_date_ = 0;
	int end_date_ = 100;
	std::vector<int> x_vals_model_;
	std::vector<float>  y_vals_model_;
	bool is_updating_ = false;
	bool new_portfolio_sim = true;
	data_table data_ {};
	python_caller python_class{""};
	AGraph* testGraph = nullptr;

	std::vector<std::string> stocks_ = {};
	std::vector<std::string> algorithms_ = {};

	bool graphNeedsRefreshing = true;

	ImVec2 mousePos{};
	ImVec2 graphStart{ 502, 688 };
	ImVec2 graphEnd{ 1487, 73 };
	ImVec2 graphSize = { graphEnd.x - graphStart.x, graphEnd.y - graphStart.y };
	ImVec2 mouseOverGraphPosition{};

	graph::GraphType type = graph::GraphType::Line;

	// Default flags for a ImGui window to use for all main UI elements.
	ImGuiWindowFlags UIFlags = ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize;

	void on_startup() override
	{
		SetupImGui();

		LoadData();

		load_graph();
	}

	void SetupImGui()
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;

		ImGui::StyleColorsDark();

		{
			ImGuiStyle* style = &ImGui::GetStyle();
			style->WindowPadding = ImVec2(15, 15);
			style->ScrollbarSize = 15;
			style->WindowBorderSize = 0;
			style->FrameRounding = 2;
			style->WindowRounding = 2;
			style->ScrollbarRounding = 2;
			style->GrabRounding = 2;
			style->WindowTitleAlign = ImVec2(0.5f, 0.5f);
			style->WindowMenuButtonPosition = ImGuiDir_None;
			style->SelectableTextAlign = ImVec2(0.0f, 0.0f);
			style->Colors[ImGuiCol_FrameBg] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
			style->Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.86f, 0.43f, 0.1f, 1.0f);
			style->Colors[ImGuiCol_FrameBgActive] = ImVec4(0.86f, 0.43f, 0.1f, 1.0f);
			style->Colors[ImGuiCol_TitleBg] = ImVec4(0.86f, 0.43f, 0.1f, 1.0f);
			style->Colors[ImGuiCol_TitleBgActive] = ImVec4(0.86f, 0.43f, 0.1f, 1.0f);
			style->Colors[ImGuiCol_MenuBarBg] = ImVec4(0.86f, 0.43f, 0.1f, 0.5f);
			style->Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.86f, 0.43f, 0.1f, 0.5f);
			style->Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.86f, 0.43f, 0.1f, 1.0f);
			style->Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.86f, 0.43f, 0.1f, 0.8f);
			style->Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.86f, 0.43f, 0.1f, 0.8f);
			style->Colors[ImGuiCol_CheckMark] = ImVec4(0.86f, 0.43f, 0.1f, 1.0f);
			style->Colors[ImGuiCol_SliderGrab] = ImVec4(0.86f, 0.43f, 0.1f, 1.0f);
			style->Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.86f, 0.43f, 0.1f, 1.0f);
			style->Colors[ImGuiCol_Button] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
			style->Colors[ImGuiCol_ButtonActive] = ImVec4(0.86f, 0.43f, 0.1f, 1.0f);
			style->Colors[ImGuiCol_ButtonHovered] = ImVec4(0.86f, 0.43f, 0.1f, 0.9f);
			style->Colors[ImGuiCol_Header] = ImVec4(0.86f, 0.43f, 0.1f, 0.8f);
			style->Colors[ImGuiCol_HeaderHovered] = ImVec4(0.86f, 0.43f, 0.1f, 0.9f);
			style->Colors[ImGuiCol_HeaderActive] = ImVec4(0.86f, 0.43f, 0.1f, 1.0f);
			style->Colors[ImGuiCol_Separator] = ImVec4(0.86f, 0.43f, 0.1f, 1.0f);
			style->Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.86f, 0.43f, 0.1f, 1.0f);
			style->Colors[ImGuiCol_SeparatorActive] = ImVec4(0.86f, 0.43f, 0.1f, 1.0f);


			// Color for stuff ---> 219, 110, 25 ---> 0.86f, 0.43f, 0.1f, 1.0f
		}

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
	}

	void LoadData()
	{
		if ( stocks_.empty() )
		{
			LOG_INFO("Gathering Stock Files");
			for ( const auto& entry : std::filesystem::directory_iterator("./data/") )
			{
				stocks_.push_back(entry.path().filename().string());
			}
			LOG_INFO("Stock Files Loaded");
		}

		if ( algorithms_.empty() )
		{
			LOG_INFO("Gathering Algorithm Files");
			for ( const auto& entry : std::filesystem::directory_iterator("./Models/Python/") )
			{
				if ( is_directory(entry.path()) )
					algorithms_.push_back(entry.path().filename().string());
			}
			LOG_INFO("Algorithm Files Loaded");
		}
	}

	void load_graph()
	{
		if ( vulkan_context_.get_logical_device().waitForFences(1, &renderer_.in_flight_fence_, VK_TRUE, 1) != vk::Result::eSuccess )
		{
			LOG_INFO("Waiting on in flight fence");
			return;
		}

		// LOADING DATA
		LOG_INFO("Loading Selected Stock values");
		std::string stock;
		if (!selected_stock_.empty())
		{
			std::string t = "./data/";
			stock = t + selected_stock_;
		} else
		{
			stock = "./data/ABDN.csv";
		}
		data_ = load_data(stock);
		LOG_INFO("Loaded Selected Stock values");


		LOG_INFO("Generating X_vals for graph");
		std::vector<float> x_vals = {};
		for (int i = static_cast<int>(data_.dates.size() * (start_date_ / 100.0f)); i < static_cast<int>(data_.dates.size() * (end_date_ / 100.0f)); i++)
		{
			x_vals.push_back(i);
		}
		LOG_INFO("Generated X_vals for graph");
		// DATA LOADED

		// ACTUAL PLOTS
		LOG_INFO("Creating stock Graph plot");
		const auto start = data_.values["Low"].begin() + static_cast<int>(static_cast<float>(data_.values["Low"].size()) * (static_cast<float>(start_date_) / 100.0f));
		const auto end = data_.values["Low"].begin() + static_cast<int>(static_cast<float>(data_.values["Low"].size()) * (static_cast<float>(end_date_) / 100.0f));

		const std::vector<float> to_plot(start, end);
		
		const auto low_plot = plot(x_vals, to_plot, col_);
		LOG_INFO("Created stock Graph plot");
		// PLOTS

		std::string startDateString = data_.dates[(int)(data_.dates.size() * (start_date_ / 100.0f))];
		std::string endDateString = data_.dates[(int)(data_.dates.size() * (end_date_ / 100.0f)) - 1];

		// FIRST GRAPH
		// On Loading graph, ensure were not using the gpu as we need to rewrite graph values.
		LOG_INFO("Recreating Graph");
		delete big_graph_;
		big_graph_ = new AGraph(this);
		big_graph_->AddPlot(stock, "High", {1, 0, 0}, GraphType::LINE);
		if (!x_vals_model_.empty() )
			big_graph_->AddPlot(x_vals_model_, y_vals_model_, { 0, 1, 1 }, GraphType::LINE);

		big_graph_->get_transform().scale = {0.5f, 0.6f};
		big_graph_->get_transform().rotation = 0.0f;
		big_graph_->get_transform().position = glm::vec2(0.25f, 0.3f);

		big_graph_->SetRange(startDateString, endDateString);

		graphStart = { (big_graph_->get_transform().position.x * static_cast<float>(GetWindowSize().x)), static_cast<float>(GetWindowSize().y) - (big_graph_->get_transform().position.y * static_cast<float>(GetWindowSize().y)) };
		graphEnd = { ((big_graph_->get_transform().position.x + big_graph_->get_transform().scale.x) * static_cast<float>(GetWindowSize().x)), static_cast<float>(GetWindowSize().y) - ((big_graph_->get_transform().position.y + big_graph_->get_transform().scale.y) * static_cast<float>(GetWindowSize().y)) };
		graphSize = { graphEnd.x - graphStart.x, graphEnd.y - graphStart.y };

		big_graph_->Draw();

		LOG_INFO("Creating stock Graph plot");

		const auto startVolume = data_.values["Volume"].begin() + static_cast<int>(static_cast<float>(data_.values["Volume"].size()) * (static_cast<float>(start_date_) / 100.0f));
		const auto endVolume = data_.values["Volume"].begin() + static_cast<int>(static_cast<float>(data_.values["Volume"].size()) * (static_cast<float>(end_date_) / 100.0f));

		const std::vector<float> volumePlotVals(startVolume, endVolume);

		std::cout << "X SIZE: " << x_vals.size() << std::endl;
		std::cout << "Y SIZE: " << data_.values["Volume"].size() << std::endl;

		const auto volumePlot = plot(x_vals, volumePlotVals, col_);

		delete volumeGraph_;
		volumeGraph_ = new AGraph(this);
		volumeGraph_->AddPlot(stock, "Volume", { 1, 0, 0 }, GraphType::LINE);

		volumeGraph_->get_transform().scale = { 0.5f, 0.15f };
		volumeGraph_->get_transform().rotation = 0.0f;
		volumeGraph_->get_transform().position = glm::vec2(0.25f, 0.1f);

		volumeGraph_->SetRange(startDateString, endDateString);

		volumeGraph_->Draw();

		LOG_INFO("Recreated Graph");
	}

	void on_update(float delta_time) override
	{
		//TODO::USE THIS FOR MODEL PERCENTAGE UPDATE!
		// static int i = 0;
		// i++;
		// printf("\33[2K\r");
		// printf("%i", i);

		UpdateUI();
	}

	void UpdateUI()
	{
		if ( vulkan_context_.get_logical_device().waitForFences(1, &renderer_.in_flight_fence_, VK_TRUE, 1) != vk::Result::eSuccess )
		{
			return;
		}

		if ( is_updating_ )
		{
			update_graph();
		}

		// ImGui Frame Setup
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		mousePos = ImGui::GetMousePos();
		mouseOverGraphPosition = { (mousePos.x - graphStart.x) / graphSize.x, (mousePos.y - graphStart.y) / graphSize.y };

		ImGui::ShowDemoWindow();

		// Main selection menu for user to interact with.
		ImGui::Begin("Selection Menu", nullptr, UIFlags);
		{
			// Set position to top left of the screen.
			ImGui::SetWindowPos({ 0, 0 }, ImGuiCond_Once);

			if (GenerateListBox("Stocks", { -FLT_MIN, 200 }, stocks_, selected_stock_))
			{
				graphNeedsRefreshing = true;
			}

			UISpacing();

			if (GenerateListBox("Algorithms", { -FLT_MIN, 200 }, algorithms_, selected_algorithm_))
			{
				graphNeedsRefreshing = true;
				python_class.loadNewModule(selected_algorithm_);
			}

			UISpacing();

			CreateDateSelectionElement();

			CreatePythonInteractionElements();

			if (ImGui::Button("Toggle Graph", {200, 50}))
			{
				if (type == graph::GraphType::Bar)
				{
					type = graph::GraphType::Line;
				} else
				{
					type = graph::GraphType::Bar;
				}
				graphNeedsRefreshing = true;
			}
		}
		ImGui::End();

		ImGui::Begin("Information Panel", nullptr, UIFlags);
		{
			// Setting Info panel position to right side of the screen TODO::Change this to be variable not hard coded, depending on screen size. 
			ImGui::SetWindowPos({ 1500, 100 }, ImGuiCond_Once);

			if (MouseHoveringGraph())
			{
				ShowInformationAtMousePosition();
			}
			else
			{
				ImGui::Text("Hover over the graph to see values");
			}
		}
		ImGui::End();

#ifndef DV_RELEASE
		ImGui::Begin("Output", nullptr, ImGuiWindowFlags_NoTitleBar);
		{
			ShowDebugInformation();
		}
		ImGui::End();
#endif

		ImGui::Render();
		renderer_.draw_data = ImGui::GetDrawData();

		static int last_update = 0;
		last_update++;

		if (graphNeedsRefreshing)
		{
			if (last_update > 100)
			{
				load_graph();
				graphNeedsRefreshing = false;
				last_update = 0;
			}
		}
	}

	[[nodiscard]] bool MouseHoveringGraph() const
	{
		return mouseOverGraphPosition.x >= 0 && mouseOverGraphPosition.x <= 1.0f && mouseOverGraphPosition.y >= 0 && mouseOverGraphPosition.y <= 1.0f;
	}

	void ShowInformationAtMousePosition()
	{
		// const std::vector<float> x_values = big_graph_->get_plots()[0].get_x_values();
		// const std::vector<float> y_values = big_graph_->get_values();
		// ImGui::Text("Graph Values at mouse position x: %.2f", static_cast<double>(y_values[static_cast<int>(mouseOverGraphPosition.x * static_cast<float>(y_values.size()))]));
		// ImGui::Text("Date: %s", (data_.dates[static_cast<int>(x_values[static_cast<int>(mouseOverGraphPosition.x * static_cast<float>(x_values.size()))])]));
	}

	void ShowDebugInformation() const
	{
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", static_cast<double>(1000.0f / ImGui::GetIO().Framerate), static_cast<double>(ImGui::GetIO().Framerate));
		ImGui::Text("Mouse Position x:%f y:%f)", static_cast<double>(mousePos.x), static_cast<double>(mousePos.y));
		ImGui::Text("Mouse Position On Graph x:%.4f y:%.4f)", static_cast<double>(mouseOverGraphPosition.x), static_cast<double>(mouseOverGraphPosition.y));
	}

	static void TextAsHeader(const std::string& text)
	{
		ImGui::SetWindowFontScale(2);
		ImGui::Text(text.c_str());
		ImGui::SetWindowFontScale(1);
	}

	bool GenerateListBox(const std::string& header, const ImVec2 size, const std::vector<std::string>& values, std::string& selection) const
	{
		TextAsHeader(header);
		bool valueChanged = false;
		if ( ImGui::BeginListBox(std::string("##" + header).c_str(), size) )
		{
			for ( const auto& value : values )
			{
				if ( ImGui::Selectable(value.c_str(), value == selection) )
				{
					valueChanged = true;
					selection = value;
				}
			}
			ImGui::EndListBox();
		}
		return valueChanged;
	}

	static void UISpacing()
	{
		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
	}

	void CreateDateSelectionElement()
	{
		if ( ImGui::DragInt("Start Date", &start_date_, 1, 0, end_date_ - 1) )
			graphNeedsRefreshing = true;

		if ( ImGui::DragInt("End Date", &end_date_, 1, start_date_ + 1, 100) )
			graphNeedsRefreshing = true;
	}

	void CreatePythonInteractionElements()
	{
		if ( ImGui::Button("Setup", { 400, 20 }) )
		{
			python_class.Initialise();
		}

		static int epochs;
		ImGui::DragInt("Epochs", &epochs, 1, 0, 1000);
		if ( ImGui::Button("Train", { 400, 20 }) )
		{
			python_class.Train(epochs);
		}

		if ( ImGui::Button("load Weights", { 400, 20 }) )
		{
			python_class.LoadWeights("Load Name Test");
		}

		if ( ImGui::Button("Save Weights", { 400, 20 }) )
		{
			python_class.SaveWeights("Save Name Test");
		}

		if ( ImGui::Button("Evaluate", { 400, 20 }) )
		{
			python_class.Evaluate({1});
		}

		if ( ImGui::Button("Simulate Portfolio", { 400, 20 }) )
		{
			is_updating_ = true;
			new_portfolio_sim = true;
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

	void update_graph()
	{
		if ( vulkan_context_.get_logical_device().waitForFences(1, &renderer_.in_flight_fence_, VK_TRUE, 2) != vk::Result::eSuccess )
		{
			return;
		}

		static int sell_count = 0;
		static int buy_count = 0;
		static bool is_buying = false;

		if ( new_portfolio_sim )
		{
			new_portfolio_sim = false;
			sell_count = 0;
			buy_count = 0;
			is_buying = false;
			x_vals_model_.clear();
			y_vals_model_.clear();

		}

		static int i = 0;
		// for (int i = 0; i < x_vals.size(); ++i)
		if ( i < (int)(data_.dates.size() * (end_date_ / 100.0f)) - (int)(data_.dates.size() * (start_date_ / 100.0f)))
		{
			x_vals_model_.push_back((int)(data_.dates_int()[data_.dates.size() * (start_date_ / 100.0f) + i]));
	
			if (i < 15)
			{
				y_vals_model_.push_back(data_.values["High"][15]);
			}
			else
			{
				if ( is_buying )
				{
					y_vals_model_.push_back(y_vals_model_[i - 1] + (data_.values["High"][i] - data_.values["High"][i - 1]));
				}
				else
				{
					y_vals_model_.push_back(y_vals_model_[i - 1]);
				}
	
				const std::vector<float>::const_iterator first = data_.values["High"].begin() + (i - 15);
				const auto last = first + 15;
	
				const std::vector<float> new_vec(first, last);
	
				// const int prediction = static_cast<int>(python_class.Predict(new_vec));
				int prediction = std::rand() % 2 + 1;  // NOLINT(concurrency-mt-unsafe)
				if (prediction == 1)
				{
					buy_count++;
					sell_count = 0;
					if ( buy_count > 0 )
					{
						is_buying = true;
					}
				} else if (prediction == 2)
				{
					sell_count++;
					buy_count = 0;
					if ( sell_count > 0 )
					{
						is_buying = false;
					}
				}
			}
			delete model_plot_;
			// model_plot_ = new plot(x_vals_model_, y_vals_model_, { 0, 1, 1 });
			graphNeedsRefreshing = true;
			i++;
		} else
		{
			i = 0;
	
			// delete small_graph_;
			//
			// small_graph_ = new graph(this, { *model_plot_ }, graph::GraphType::Line);
			//
			// small_graph_->get_transform().position = glm::vec2(2.35f, 0.6f);
			// small_graph_->get_transform().scale = glm::vec2{0.5f};
			//
			// small_graph_->show();
	
			is_updating_ = false;
		}
	}
	
};

application* create_application()
{
	return new test();
}
