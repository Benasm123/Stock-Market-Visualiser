#pragma once
#include <chrono>
#include <filesystem>
#include <functional>
#include <iostream>
#include <numeric>
#include <valarray>

#include "Core/DataUtilities.h"
#include "graph.h"
#include "test_actor.h"
#include "Core/Application.h"
#include "Core/main.h"
#include "imgui.h"
#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_vulkan.h"
#include "Core/PythonUtilities.h"
#include "Visualisation/AGraph.h"

class test final : public Application
{
protected:

	AGraph* bigGraph_ = nullptr;
	AGraph* volumeGraph_ = nullptr;

	glm::vec3 col_ = { 0.86f, 0.43f, 0.1f };

	std::string selectedStock_{};
	std::string selectedAlgorithm_{};

	int startDate_ = 0;
	int endDate_ = 100;

	std::vector<int> xValsModel_;
	std::vector<float>  yValsModel_;
	bool isUpdating_ = false;
	bool newPortfolioSim_ = true;
	DataTable data_ {};
	PythonCaller pythonClass_{};
	AGraph* testGraph_ = nullptr;

	std::vector<std::string> stocks_ = {};
	std::vector<std::string> algorithms_ = {};

	bool graphNeedsRefreshing_ = true;

	ImVec2 mousePos_{};
	ImVec2 graphStart_{ 502, 688 };
	ImVec2 graphEnd_{ 1487, 73 };
	ImVec2 graphSize_ = { graphEnd_.x - graphStart_.x, graphEnd_.y - graphStart_.y };
	ImVec2 mouseOverGraphPosition_{};

	graph::GraphType type_ = graph::GraphType::Line;

	// Default flags for a ImGui window to use for all main UI elements.
	ImGuiWindowFlags uiFlags_ = ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize;

	void OnStartup() override
	{
		// Not ideal but just using for now to make sure rendering works without crashes.
		vkDeviceWaitIdle(vulkanContext_.get_logical_device());

		SetupImGui();

		SetupData();

		LoadGraph();
	}

	void SetupImGui()
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		const ImGuiIO& io = ImGui::GetIO(); (void)io;

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

		ImGui_ImplWin32_Init(vulkanContext_.get_window().get_hwnd());
		ImGui_ImplVulkan_InitInfo initInfo = {};
		initInfo.Instance = vulkanContext_.get_instance();
		initInfo.PhysicalDevice = vulkanContext_.get_physical_device();
		initInfo.Device = vulkanContext_.get_logical_device();
		initInfo.QueueFamily = vulkanContext_.get_graphics_queue_index();
		initInfo.Queue = vulkanContext_.get_graphics_queue();
		initInfo.PipelineCache = nullptr;
		initInfo.DescriptorPool = renderer_.GetImGuiDescriptorPool();
		initInfo.Subpass = 0;
		initInfo.MinImageCount = 3;
		initInfo.ImageCount = renderer_.GetSwapChain().get_images().size();
		initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		initInfo.Allocator = nullptr;
		initInfo.CheckVkResultFn = nullptr;
		ImGui_ImplVulkan_Init(&initInfo, renderer_.GetRenderPass());

		// Upload Fonts
		{
			// Use any command queue
			const VkCommandPool commandPool = renderer_.GetCommandPool();
			const VkCommandBuffer commandBuffer = renderer_.GetCommandBuffers()[0];

			vkResetCommandPool(vulkanContext_.get_logical_device(), commandPool, 0);
			VkCommandBufferBeginInfo begin_info = {};
			begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
			vkBeginCommandBuffer(commandBuffer, &begin_info);

			ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);

			VkSubmitInfo end_info = {};
			end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			end_info.commandBufferCount = 1;
			end_info.pCommandBuffers = &commandBuffer;
			vkEndCommandBuffer(commandBuffer);
			vkQueueSubmit(vulkanContext_.get_graphics_queue(), 1, &end_info, VK_NULL_HANDLE);

			vkDeviceWaitIdle(vulkanContext_.get_logical_device());
			ImGui_ImplVulkan_DestroyFontUploadObjects();
		}
	}

	void SetupData()
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

	void LoadGraph()
	{
		if ( vulkanContext_.get_logical_device().waitForFences(1, &renderer_.GetInFlightFence(), VK_TRUE, 0) != vk::Result::eSuccess )
		{
			return;
		}

		// LOADING DATA
		LOG_INFO("Loading Selected Stock values");
		std::string stock;
		if (!selectedStock_.empty())
		{
			std::string t = "./data/";
			stock = t + selectedStock_;
		} else
		{
			stock = "./data/ABDN.csv";
		}
		data_ = LoadData(stock);
		LOG_INFO("Loaded Selected Stock values");


		LOG_INFO("Generating X_vals for graph");
		std::vector<float> x_vals = {};
		for (int i = static_cast<int>(data_.dates.size() * (startDate_ / 100.0f)); i < static_cast<int>(data_.dates.size() * (endDate_ / 100.0f)); i++)
		{
			x_vals.push_back(i);
		}
		LOG_INFO("Generated X_vals for graph");
		// DATA LOADED

		// ACTUAL PLOTS
		LOG_INFO("Creating stock Graph plot");
		const auto start = data_.values["Low"].begin() + static_cast<int>(static_cast<float>(data_.values["Low"].size()) * (static_cast<float>(startDate_) / 100.0f));
		const auto end = data_.values["Low"].begin() + static_cast<int>(static_cast<float>(data_.values["Low"].size()) * (static_cast<float>(endDate_) / 100.0f));

		const std::vector<float> to_plot(start, end);
		
		const auto lowPlot = plot(x_vals, to_plot, col_);
		LOG_INFO("Created stock Graph plot");
		// PLOTS

		std::string startDateString = data_.dates[(int)(data_.dates.size() * (startDate_ / 100.0f))];
		std::string endDateString = data_.dates[(int)(data_.dates.size() * (endDate_ / 100.0f)) - 1];

		// FIRST GRAPH
		// On Loading graph, ensure were not using the gpu as we need to rewrite graph values.
		LOG_INFO("Recreating Graph");
		delete bigGraph_;

		bigGraph_ = new AGraph(this);

		bigGraph_->AddPlot(stock, "High", {1, 0, 0}, GRAPH_TYPE::LINE);

		if (!xValsModel_.empty() )
			bigGraph_->AddPlot(xValsModel_, yValsModel_, { 0, 1, 1 }, GRAPH_TYPE::LINE);

		bigGraph_->GetTransform().scale = {0.5f, 0.6f};
		bigGraph_->GetTransform().rotation = 0.0f;
		bigGraph_->GetTransform().position = glm::vec2(0.25f, 0.3f);

		bigGraph_->SetRange(startDateString, endDateString);

		graphStart_ = { (bigGraph_->GetTransform().position.x * static_cast<float>(GetWindowSize().x)), static_cast<float>(GetWindowSize().y) - (bigGraph_->GetTransform().position.y * static_cast<float>(GetWindowSize().y)) };
		graphEnd_ = { ((bigGraph_->GetTransform().position.x + bigGraph_->GetTransform().scale.x) * static_cast<float>(GetWindowSize().x)), static_cast<float>(GetWindowSize().y) - ((bigGraph_->GetTransform().position.y + bigGraph_->GetTransform().scale.y) * static_cast<float>(GetWindowSize().y)) };
		graphSize_ = { graphEnd_.x - graphStart_.x, graphEnd_.y - graphStart_.y };

		bigGraph_->Draw();

		LOG_INFO("Creating stock Graph plot");

		const auto startVolume = data_.values["Volume"].begin() + static_cast<int>(static_cast<float>(data_.values["Volume"].size()) * (static_cast<float>(startDate_) / 100.0f));
		const auto endVolume = data_.values["Volume"].begin() + static_cast<int>(static_cast<float>(data_.values["Volume"].size()) * (static_cast<float>(endDate_) / 100.0f));

		const std::vector<float> volumePlotVals(startVolume, endVolume);

		std::cout << "X SIZE: " << x_vals.size() << std::endl;
		std::cout << "Y SIZE: " << data_.values["Volume"].size() << std::endl;

		const auto volumePlot = plot(x_vals, volumePlotVals, col_);

		delete volumeGraph_;
		volumeGraph_ = new AGraph(this);
		volumeGraph_->AddPlot(stock, "Volume", { 1, 0, 0 }, GRAPH_TYPE::BAR);

		volumeGraph_->GetTransform().scale = { 0.5f, 0.15f };
		volumeGraph_->GetTransform().rotation = 0.0f;
		volumeGraph_->GetTransform().position = glm::vec2(0.25f, 0.1f);

		volumeGraph_->SetRange(startDateString, endDateString);

		volumeGraph_->Draw();

		LOG_INFO("Recreated Graph");
	}

	void OnUpdate(float delta_time) override
	{
		vkDeviceWaitIdle(vulkanContext_.get_logical_device());
		UpdateUI();
	}

	void UpdateUI()
	{
		if ( vulkanContext_.get_logical_device().waitForFences(1, &renderer_.GetInFlightFence(), VK_TRUE, 0) != vk::Result::eSuccess )
		{
			return;
		}

		//TODO::USE THIS FOR MODEL PERCENTAGE UPDATE!
		// static int i = 0;
		// i++;
		// printf("\33[2K\r");
		// printf("%i", i);

		if ( isUpdating_ )
		{
			update_graph();
		}

		// ImGui Frame Setup
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		mousePos_ = ImGui::GetMousePos();
		mouseOverGraphPosition_ = { (mousePos_.x - graphStart_.x) / graphSize_.x, (mousePos_.y - graphStart_.y) / graphSize_.y };

		ImGui::ShowDemoWindow();

		// Main selection menu for user to interact with.
		ImGui::Begin("Selection Menu", nullptr, uiFlags_);
		{
			// Set position to top left of the screen.
			ImGui::SetWindowPos({ 0, 0 }, ImGuiCond_Once);

			if (GenerateListBox("Stocks", { -FLT_MIN, 200 }, stocks_, selectedStock_))
			{
				graphNeedsRefreshing_ = true;
			}

			UISpacing();

			if (GenerateListBox("Algorithms", { -FLT_MIN, 200 }, algorithms_, selectedAlgorithm_))
			{
				graphNeedsRefreshing_ = true;
				pythonClass_.LoadNewModule(selectedAlgorithm_);
			}

			UISpacing();

			CreateDateSelectionElement();

			CreatePythonInteractionElements();

			if (ImGui::Button("Toggle Graph", {200, 50}))
			{
				if (type_ == graph::GraphType::Bar)
				{
					type_ = graph::GraphType::Line;
				} else
				{
					type_ = graph::GraphType::Bar;
				}
				graphNeedsRefreshing_ = true;
			}
		}
		ImGui::End();

		ImGui::Begin("Information Panel", nullptr, uiFlags_);
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
		renderer_.SetDrawData(ImGui::GetDrawData());

		static int last_update = 0;
		last_update++;

		if (graphNeedsRefreshing_)
		{
			if (last_update > 100)
			{
				LoadGraph();
				graphNeedsRefreshing_ = false;
				last_update = 0;
			}
		}
	}

	[[nodiscard]] bool MouseHoveringGraph() const
	{
		return mouseOverGraphPosition_.x >= 0 && mouseOverGraphPosition_.x <= 1.0f && mouseOverGraphPosition_.y >= 0 && mouseOverGraphPosition_.y <= 1.0f;
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
		ImGui::Text("Mouse Position x:%f y:%f)", static_cast<double>(mousePos_.x), static_cast<double>(mousePos_.y));
		ImGui::Text("Mouse Position On Graph x:%.4f y:%.4f)", static_cast<double>(mouseOverGraphPosition_.x), static_cast<double>(mouseOverGraphPosition_.y));
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
		if ( ImGui::DragInt("Start Date", &startDate_, 1, 0, endDate_ - 1) )
			graphNeedsRefreshing_ = true;

		if ( ImGui::DragInt("End Date", &endDate_, 1, startDate_ + 1, 100) )
			graphNeedsRefreshing_ = true;
	}

	void CreatePythonInteractionElements()
	{
		if ( ImGui::Button("Setup", { 400, 20 }) )
		{
			pythonClass_.Initialise();
		}

		static int epochs;
		ImGui::DragInt("Epochs", &epochs, 1, 0, 1000);
		if ( ImGui::Button("Train", { 400, 20 }) )
		{
			pythonClass_.Train(epochs);
		}

		if ( ImGui::Button("load Weights", { 400, 20 }) )
		{
			pythonClass_.LoadWeights("Load Name Test");
		}

		if ( ImGui::Button("Save Weights", { 400, 20 }) )
		{
			pythonClass_.SaveWeights("Save Name Test");
		}

		if ( ImGui::Button("Evaluate", { 400, 20 }) )
		{
			pythonClass_.Evaluate({1});
		}

		if ( ImGui::Button("Simulate Portfolio", { 400, 20 }) )
		{
			isUpdating_ = true;
			newPortfolioSim_ = true;
		}
	}

	void OnShutdown() override
	{
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}

	void update_graph()
	{
		if ( vulkanContext_.get_logical_device().waitForFences(1, &renderer_.GetInFlightFence(), VK_TRUE, 0) != vk::Result::eSuccess )
		{
			return;
		}

		static int sellCount = 0;
		static int buyCount = 0;
		static bool isBuying = false;

		if ( newPortfolioSim_ )
		{
			newPortfolioSim_ = false;
			sellCount = 0;
			buyCount = 0;
			isBuying = false;
			xValsModel_.clear();
			yValsModel_.clear();

		}

		static int i = 0;
		// for (int i = 0; i < x_vals.size(); ++i)
		if ( i < (int)(data_.dates.size() * (endDate_ / 100.0f)) - (int)(data_.dates.size() * (startDate_ / 100.0f)))
		{
			xValsModel_.push_back((int)(data_.DatesInt()[data_.dates.size() * (startDate_ / 100.0f) + i]));
	
			if (i < 15)
			{
				yValsModel_.push_back(data_.values["High"][15]);
			}
			else
			{
				if ( isBuying )
				{
					yValsModel_.push_back(yValsModel_[i - 1] + (data_.values["High"][i] - data_.values["High"][i - 1]));
				}
				else
				{
					yValsModel_.push_back(yValsModel_[i - 1]);
				}
	
				const std::vector<float>::const_iterator first = data_.values["High"].begin() + (i - 15);
				const auto last = first + 15;
	
				const std::vector<float> new_vec(first, last);
	
				// const int prediction = static_cast<int>(python_class.Predict(new_vec));
				int prediction = std::rand() % 2 + 1;  // NOLINT(concurrency-mt-unsafe)
				if (prediction == 1)
				{
					buyCount++;
					sellCount = 0;
					if ( buyCount > 0 )
					{
						isBuying = true;
					}
				} else if (prediction == 2)
				{
					sellCount++;
					buyCount = 0;
					if ( sellCount > 0 )
					{
						isBuying = false;
					}
				}
			}
			graphNeedsRefreshing_ = true;
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
	
			isUpdating_ = false;
		}
	}
	
};

Application* CreateApplication()
{
	return new test();
}
