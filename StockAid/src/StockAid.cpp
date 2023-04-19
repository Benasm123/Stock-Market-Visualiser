#include "StockAid.h"

#include <numeric>

#include "Core/DataUtilities.h"
#include "imgui.h"
#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_vulkan.h"
#include "Core/PythonUtilities.h"
#include "Visualisation/AGraph.h"

// UI CONSTANTS
constexpr ImGuiWindowFlags kUiFlags = ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMove |
	ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize;
#define IM_COLOUR_MAIN(alpha) (ImVec4(0.09f, 0.31f, 0.33f, alpha))
#define IM_COLOUR_HOVERED(alpha) (ImVec4(0.58f, 0.67f, 0.70f, alpha))
#define IM_COLOUR_BACKGROUND(alpha) (ImVec4(0.9f, 0.94f, 0.93f, alpha))
#define IM_COLOUR_BUTTON(alpha) (ImVec4(0.8f, 0.84f, 0.83f, alpha))
#define IM_COLOUR_SECONDARY(alpha) (ImVec4(0.1f, 0.1f, 0.2f, alpha))

void StockAid::OnStartup()
{
	Window::AddCallbackOnResize([this]() -> void
	{
		graphNeedsRefreshing_ = true;
	});

	SetupImGui();

	SetupData();

	LoadStockGraphs();

	LoadAlgorithmGraphs();
}

void StockAid::OnShutdown()
{
	trainThread_->join();
	delete trainThread_;
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void StockAid::SetupImGui()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	const ImGuiIO& io = ImGui::GetIO();

	io.Fonts->Clear();

	ImFontConfig config;
	config.GlyphExtraSpacing.x = 1.0f;

	//TODO->Move fonts to folder in project folder Fonts.
	mainFont_ = io.Fonts->AddFontFromFileTTF("../ext/imgui/misc/fonts/Roboto-Medium.ttf", 16.0, &config);
	headerFont_ = io.Fonts->AddFontFromFileTTF("../ext/imgui/misc/fonts/Roboto-Medium.ttf", 40.0, &config);
	titleFont_ = io.Fonts->AddFontFromFileTTF("../ext/imgui/misc/fonts/Roboto-Medium.ttf", 56.0, &config);

	ImGui_ImplWin32_Init(vulkanContext_.GetWindow().GetHwnd());
	ImGui_ImplVulkan_InitInfo initInfo = {};
	initInfo.Instance = vulkanContext_.GetInstance();
	initInfo.PhysicalDevice = vulkanContext_.GetPhysicalDevice();
	initInfo.Device = vulkanContext_.GetLogicalDevice();
	initInfo.QueueFamily = vulkanContext_.GetGraphicsQueueIndex();
	initInfo.Queue = vulkanContext_.GetGraphicsQueue();
	initInfo.PipelineCache = nullptr;
	initInfo.DescriptorPool = renderer_.GetImGuiDescriptorPool();
	initInfo.Subpass = 0;
	initInfo.MinImageCount = 3;
	initInfo.ImageCount = static_cast<uint32_t>(renderer_.GetSwapChain().GetImages().size());
	initInfo.MSAASamples = VK_SAMPLE_COUNT_8_BIT;
	initInfo.Allocator = nullptr;
	initInfo.CheckVkResultFn = nullptr;
	ImGui_ImplVulkan_Init(&initInfo, renderer_.GetRenderPass());

	// Upload Fonts
	{
		// Use any command queue
		const VkCommandPool commandPool = renderer_.GetCommandPool();
		const VkCommandBuffer commandBuffer = renderer_.GetCommandBuffers()[0];

		vkResetCommandPool(vulkanContext_.GetLogicalDevice(), commandPool, 0);
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);

		VkSubmitInfo endInfo = {};
		endInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		endInfo.commandBufferCount = 1;
		endInfo.pCommandBuffers = &commandBuffer;
		vkEndCommandBuffer(commandBuffer);
		vkQueueSubmit(vulkanContext_.GetGraphicsQueue(), 1, &endInfo, VK_NULL_HANDLE);

		vkDeviceWaitIdle(vulkanContext_.GetLogicalDevice());

		ImGui_ImplVulkan_DestroyFontUploadObjects();
	}

	ImGui::StyleColorsLight();
	{
		ImGuiStyle* style = &ImGui::GetStyle();
		style->WindowPadding = ImVec2(15, 15);
		style->ScrollbarSize = 5;
		style->WindowBorderSize = 0;
		style->FrameRounding = 0;
		style->WindowRounding = 0;
		style->ScrollbarRounding = 0;
		style->GrabRounding = 2;
		style->WindowTitleAlign = ImVec2(0.5f, 0.5f);
		style->WindowMenuButtonPosition = ImGuiDir_None;
		style->SelectableTextAlign = ImVec2(0.0f, 0.0f);

		style->Colors[ImGuiCol_FrameBg] = IM_COLOUR_BUTTON(1.0f);
		style->Colors[ImGuiCol_FrameBgHovered] = IM_COLOUR_HOVERED(1.0f);
		style->Colors[ImGuiCol_FrameBgActive] = IM_COLOUR_MAIN(1.0f);
		style->Colors[ImGuiCol_TitleBg] = IM_COLOUR_MAIN(1.0f);
		style->Colors[ImGuiCol_TitleBgActive] = IM_COLOUR_MAIN(1.0f);
		style->Colors[ImGuiCol_MenuBarBg] = IM_COLOUR_MAIN(0.5f);
		style->Colors[ImGuiCol_ScrollbarBg] = IM_COLOUR_MAIN(0.5f);
		style->Colors[ImGuiCol_ScrollbarGrab] = IM_COLOUR_MAIN(1.0f);
		style->Colors[ImGuiCol_ScrollbarGrabHovered] = IM_COLOUR_HOVERED(1.0f);
		style->Colors[ImGuiCol_ScrollbarGrabActive] = IM_COLOUR_HOVERED(0.8f);
		style->Colors[ImGuiCol_CheckMark] = IM_COLOUR_MAIN(1.0f);
		style->Colors[ImGuiCol_SliderGrab] = IM_COLOUR_MAIN(1.0f);
		style->Colors[ImGuiCol_SliderGrabActive] = IM_COLOUR_MAIN(1.0f);
		style->Colors[ImGuiCol_Button] = IM_COLOUR_BUTTON(0.9f);
		style->Colors[ImGuiCol_ButtonActive] = IM_COLOUR_MAIN(1.0f);
		style->Colors[ImGuiCol_ButtonHovered] = IM_COLOUR_HOVERED(0.9f);
		style->Colors[ImGuiCol_Header] = IM_COLOUR_MAIN(0.8f);
		style->Colors[ImGuiCol_HeaderHovered] = IM_COLOUR_HOVERED(0.9f);
		style->Colors[ImGuiCol_HeaderActive] = IM_COLOUR_MAIN(1.0f);
		style->Colors[ImGuiCol_Separator] = IM_COLOUR_MAIN(1.0f);
		style->Colors[ImGuiCol_SeparatorHovered] = IM_COLOUR_HOVERED(1.0f);
		style->Colors[ImGuiCol_SeparatorActive] = IM_COLOUR_MAIN(1.0f);
	}
}

void StockAid::SetupData()
{
	if (stocks_.empty())
	{
		for (const auto& entry : std::filesystem::directory_iterator("./data/"))
		{
			stocks_.push_back(entry.path().filename().string());
		}
	}

	if (algorithms_.empty())
	{
		for (const auto& entry : std::filesystem::directory_iterator("./Models/Python/"))
		{
			// Don't show folders starting with _ allowing extra files needed to be kept in the same directory.
			if (entry.path().filename().string().starts_with("_")) continue;

			if (is_directory(entry.path()))
			{
				algorithms_.push_back(entry.path().filename().string());
			}
		}
	}

	// If we don't have a stock selected, select the first in the list.
	if (selectedStock_.empty()) selectedStock_ = stocks_[0];
}

void StockAid::LoadStockGraphs()
{
	if (vulkanContext_.GetLogicalDevice().waitForFences(1, &renderer_.GetInFlightFence(), VK_TRUE, 0) !=
		vk::Result::eSuccess)
	{
		return;
	}

	vulkanContext_.GetLogicalDevice().waitIdle();

	if (changedStock_)
	{
		changedStock_ = false;

		delete stockGraph_;
		stockGraph_ = nullptr;

		delete volumeGraph_;
		volumeGraph_ = nullptr;

		delete predictionGraph_;
		predictionGraph_ = nullptr;

		selectedStockData_ = LoadData(selectedStock_);
	}

	startDateString_ = selectedStockData_.dates[static_cast<int>(static_cast<float>(selectedStockData_.dates.size() - 1) * (startDate_ / 100.0f))];
	endDateString_ = selectedStockData_.dates[static_cast<int>(static_cast<float>(selectedStockData_.dates.size() - 1) * (endDate_ / 100.0f))];

	UpdateStockGraph();
	UpdateVolumeGraph();
}

void StockAid::LoadAlgorithmGraphs()
{
	if ( vulkanContext_.GetLogicalDevice().waitForFences(1, &renderer_.GetInFlightFence(), VK_TRUE, 0) !=
		vk::Result::eSuccess )
	{
		return;
	}

	vulkanContext_.GetLogicalDevice().waitIdle();

	if ( changedAlgorithm_ )
	{
		changedAlgorithm_ = false;

		delete lossGraph_;
		lossGraph_ = nullptr;

		delete accuracyGraph_;
		accuracyGraph_ = nullptr;

		delete rewardGraph_;
		rewardGraph_ = nullptr;
	}

	UpdateLossGraph();
	UpdateAccuracyGraph();
	UpdateRewardGraph();
}

void StockAid::UpdateLossGraph()
{
	if (!lossGraph_)
	{
		CreateLossGraph();
	}

	lossGraph_->SetRange(
		dv_math::Vec2<int>(
			0,
			 lossGraph_->GetPlot("loss") ? lossGraph_->GetPlot("loss")->GetXValues().back() : 0
		)
	);

	lossGraph_->Draw();
}

void StockAid::CreateLossGraph()
{
	lossGraph_ = new AGraph(this);

	lossGraph_->GetTransform().scale = { 0.25f, 0.45f - (headerHeight_ / static_cast<float>(GetWindowSize().y)) };
	lossGraph_->GetTransform().rotation = 0.0f;
	lossGraph_->GetTransform().position = glm::vec2(0.225f, 0.5f);

	const std::vector<float> lossValues = pythonClass_.GetLossList();

	if (lossValues.empty())
	{
		return;
	}

	std::vector<int> epochs(lossValues.size());

	for (int i = 0; i < epochs.size(); i++)
	{
		epochs[i] = i + 1;
	}

	DataTable lossTable;
	lossTable.values["loss"] = lossValues;

	lossGraph_->AddPlot("Loss Graph", epochs, lossTable, "loss", { 1, 0, 1 }, LINE);
}

void StockAid::UpdateAccuracyGraph()
{
	if ( !accuracyGraph_ )
	{
		CreateAccuracyGraph();
	}

	accuracyGraph_->SetRange(
		dv_math::Vec2<int>(
			0,
			accuracyGraph_->GetPlot("loss") ? accuracyGraph_->GetPlot("loss")->GetXValues().back() : 0
			)
	);

	accuracyGraph_->Draw();
}

void StockAid::CreateAccuracyGraph()
{
	accuracyGraph_ = new AGraph(this);

	accuracyGraph_->GetTransform().scale = { 0.25f, 0.45f - (headerHeight_ / static_cast<float>(GetWindowSize().y)) };
	accuracyGraph_->GetTransform().rotation = 0.0f;
	accuracyGraph_->GetTransform().position = glm::vec2(0.5f, 0.5f);

	const std::vector<float> accuracyValues = pythonClass_.GetAccuracyList();
	LOG_ERROR("ACCURACY: %u", accuracyValues.size());

	if ( accuracyValues.empty() )
	{
		return;
	}

	std::vector<int> epochs(accuracyValues.size());

	for ( int i = 0; i < epochs.size(); i++ )
	{
		epochs[i] = i + 1;
	}

	DataTable accuracyTable;
	accuracyTable.values["loss"] = accuracyValues;

	accuracyGraph_->AddPlot("Loss Graph", epochs, accuracyTable, "loss", { 1, 0, 1 }, LINE);
}

void StockAid::UpdateRewardGraph()
{
	if ( !rewardGraph_ )
	{
		CreateRewardGraph();
	}

	rewardGraph_->SetRange(
		dv_math::Vec2<int>(
			0,
			rewardGraph_->GetPlot("loss") ? rewardGraph_->GetPlot("loss")->GetXValues().back() : 0
			)
	);

	rewardGraph_->Draw();
}

void StockAid::CreateRewardGraph()
{
	rewardGraph_ = new AGraph(this);

	rewardGraph_->GetTransform().scale = { 0.25f, 0.45f - (headerHeight_ / static_cast<float>(GetWindowSize().y)) };
	rewardGraph_->GetTransform().rotation = 0.0f;
	rewardGraph_->GetTransform().position = glm::vec2(0.225f, 0.05f);

	const std::vector<float> rewardValues = pythonClass_.GetRewardList();

	if ( rewardValues.empty() )
	{
		return;
	}

	std::vector<int> epochs(rewardValues.size());

	for ( int i = 0; i < epochs.size(); i++ )
	{
		epochs[i] = i + 1;
	}

	DataTable rewardTable;
	rewardTable.values["loss"] = rewardValues;

	rewardGraph_->AddPlot("Loss Graph", epochs, rewardTable, "loss", { 1, 0, 1 }, LINE);
}

void StockAid::CalculateSharpeRatio()
{
	auto preds =  pythonClass_.Predict(selectedStock_, startDateString_, endDateString_);

	// int indexOfFirstDAy
	// for (int pred : preds)
	// {
	// }
}

void StockAid::UpdateStockGraph()
{	
	if (!stockGraph_)
	{
		CreateStockGraph();
	}

	if (!xValsModel_.empty())
	{
		DataTable predictionTable;
		predictionTable.values["Prediction"] = yValsModel_;
		if (!stockGraph_->GetPlot("Prediction"))
		{
			stockGraph_->AddPlot("Prediction", xValsModel_, predictionTable, "Prediction", predictionPlotColour_, LINE);
			stockGraph_->GetPlot("Prediction")->SetToShow(numberOfPredictionsMade_);
		}
		else
		{
			stockGraph_->GetPlot("Prediction")->UpdateValues(xValsModel_, predictionTable);
			stockGraph_->GetPlot("Prediction")->SetToShow(numberOfPredictionsMade_);
		}
	}

	stockGraph_->SetRange(dv_math::Vec2<int>(
			static_cast<int>(ConvertDateStringToTimeSinceEpoch(startDateString_)),
			static_cast<int>(ConvertDateStringToTimeSinceEpoch(endDateString_))
		)
	);


	Plot60DayAverage();
	PlotLinearRegression();

	stockGraph_->Draw();
}

void StockAid::CreateStockGraph()
{
	delete stockGraph_;
	stockGraph_ = new AGraph(this);

	stockGraph_->AddPlot(selectedStock_, "High", stockPlotColour_, LINE);

	stockGraph_->GetTransform().scale = {0.5f, 0.65f - (headerHeight_ / static_cast<float>(GetWindowSize().y))};
	stockGraph_->GetTransform().rotation = 0.0f;
	stockGraph_->GetTransform().position = glm::vec2(0.25f, 0.3f);
}

void StockAid::UpdateVolumeGraph()
{
	if (!volumeGraph_)
	{
		CreateVolumeGraph();
	}

	volumeGraph_->SetRange(dv_math::Vec2{
			static_cast<int>(ConvertDateStringToTimeSinceEpoch(startDateString_)),
			static_cast<int>(ConvertDateStringToTimeSinceEpoch(endDateString_))
		}
	);

	volumeGraph_->Draw();
}

void StockAid::CreateVolumeGraph()
{
	volumeGraph_ = new AGraph(this);
	volumeGraph_->SetGraphType(BAR);

	volumeGraph_->AddPlot(selectedStock_, "Volume", volumePlotColour_, BAR);

	volumeGraph_->GetTransform().scale = { 0.5f, -0.15f };
	volumeGraph_->GetTransform().rotation = 0.0f;
	volumeGraph_->GetTransform().position = glm::vec2(0.25f, 0.25f);
}

void StockAid::OnUpdate(float deltaTime)
{
	// auto start = std::chrono::high_resolution_clock::now();
	// auto end = std::chrono::high_resolution_clock::now();
	// float waitTime = deltaTime + static_cast<float>(std::chrono::duration<double>((end - start)).count());
	// do {
	// 	end = std::chrono::high_resolution_clock::now();
	// 	waitTime = deltaTime + static_cast<float>(std::chrono::duration<double>((end - start)).count());
	// 	std::this_thread::yield();
	// 	std::this_thread::sleep_until(std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(1));
	// } while ( waitTime < 0.016f );

	if ( vulkanContext_.GetLogicalDevice().waitForFences(1, &renderer_.GetInFlightFence(), VK_TRUE, 0) != vk::Result::eSuccess )
	{
		return;
	}

	vulkanContext_.GetLogicalDevice().waitIdle();

	headerWidth_ = static_cast<float>(GetWindowSize().x);
	UpdateUI();
	UpdateMouseOverGraphPosition();
}

void StockAid::UpdateUI()
{
	if ( graphNeedsRefreshing_ )
	{
		LoadStockGraphs();
		stockGraph_->RedrawPlots();
		graphNeedsRefreshing_ = false;
		LoadAlgorithmGraphs();
	}

	if (isUpdating_) MakePredictions();

	// ImGui Frame Setup
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ShowHeader();

	mainAreaWidth_ = static_cast<float>(GetWindowSize().x);
	mainAreaHeight_ = static_cast<float>(GetWindowSize().y) - headerHeight_;

	if ( selectedPage_ == STOCKS ) ShowStockPage();
	if ( selectedPage_ == ALGORITHMS ) ShowAlgorithmPage();

#ifndef DV_RELEASE
	ImGui::Begin("Output", nullptr, ImGuiWindowFlags_NoTitleBar);
	{
		ShowDebugInformation();
	}
	ImGui::End();
#endif

	ImGui::Render();
	renderer_.SetDrawData(ImGui::GetDrawData());
}

void StockAid::Plot60DayAverage()
{
	while ( stockGraph_->GetPlot("60 Day Average") )
	{
		int index = 0;

		for ( const auto& plot : stockGraph_->GetPlots() )
		{
			if ( plot->GetName() == "60 Day Average" )
			{
				break;
			}
			index++;
		}

		stockGraph_->RemovePlot(index);
	}

	const auto values = selectedStockData_.values["High"];
	std::vector<float> averages(values.size());

	float sum = 0;

	for (int i = 0; i < 60; i++)
	{
		sum += values[i];
		averages[i] = (sum / (i + 1));
	}

	for (int i = 60; i < values.size(); ++i)
	{
		sum += values[i];
		sum -= values[i - 60];

		averages[i] = (sum / 60);
	}

	static DataTable averagesDataTable;
	averagesDataTable.values["Average"] = averages;
	averagesDataTable.dates = selectedStockData_.dates;
	averagesDataTable.datesSinceEpoch = selectedStockData_.datesSinceEpoch;

	const std::vector<int> xVals(averagesDataTable.datesSinceEpoch.begin(), averagesDataTable.datesSinceEpoch.end());

	stockGraph_->AddPlot("60 Day Average", xVals, averagesDataTable, "Average", {1.0f, 0.0f, 0.0f}, LINE);
}

void StockAid::PlotLinearRegression()
{
	while ( stockGraph_->GetPlot("Regression") )
	{
		int index = 0;

		for (const auto& plot : stockGraph_->GetPlots())
		{
			if (plot->GetName() == "Regression")
			{
				break;
			}
			index++;
		}

		stockGraph_->RemovePlot(index);
	}

	const std::vector<float> xValues(selectedStockData_.datesSinceEpoch.begin() + (std::ranges::find(selectedStockData_.dates, startDateString_) - selectedStockData_.dates.begin()),
									 selectedStockData_.datesSinceEpoch.begin() + (std::ranges::find(selectedStockData_.dates, endDateString_) + 1 - selectedStockData_.dates.begin()));

	const std::vector<int> xPlotValues(selectedStockData_.datesSinceEpoch.begin() + (std::ranges::find(selectedStockData_.dates, startDateString_) - selectedStockData_.dates.begin()),
									 selectedStockData_.datesSinceEpoch.begin() + (std::ranges::find(selectedStockData_.dates, endDateString_) + 1 - selectedStockData_.dates.begin()));

	const std::vector<float> yValues(selectedStockData_.values["High"].begin() + (std::ranges::find(selectedStockData_.dates, startDateString_) - selectedStockData_.dates.begin()), 
					  selectedStockData_.values["High"].begin() + (std::ranges::find(selectedStockData_.dates, endDateString_) + 1 - selectedStockData_.dates.begin()));

	float meanX = (float)(std::accumulate(xValues.begin(), xValues.end(), 0.0) / (double)xValues.size());
	float meanY = (float)(std::accumulate(yValues.begin(), yValues.end(), 0.0) / (double)yValues.size());

	float covariance = 0.0f;
	for (int i = 0; i < xValues.size(); i++)
	{
		covariance += (xValues[i] - meanX) * (yValues[i] - meanY);
	}

	float variance = 0.0f;
	for (int i = 0; i < xValues.size(); i++)
	{
		variance += std::pow((xValues[i] - meanX), 2);
	}

	float m = covariance / variance;

	float c = meanY - m * meanX;

	std::vector<float> linearRegressedValues(xValues.size());

	for (int i = 0; i < xValues.size(); i++)
	{
		linearRegressedValues[i] = m * xValues[i] + c;
	}

	DataTable linearRegressionTable;
	linearRegressionTable.values["Regression"] = {linearRegressedValues.front(), linearRegressedValues.back()};

	stockGraph_->AddPlot("Regression", {xPlotValues.front(), xPlotValues.back()}, linearRegressionTable, "Regression", {0.0f, 1.0f, 0.0f}, LINE);
}

void StockAid::ShowHeader()
{
	PushHeaderStyle();

	ImGui::SetNextWindowPos({ 0, 0 });
	ImGui::SetNextWindowSize({ headerWidth_, headerHeight_ }, ImGuiCond_Always);
	ImGui::Begin("Page Header", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
	{
		bool coloursPushed = false;

		float sizeMultiplier = 1.0f;

		if ( selectedPage_ == STOCKS )
		{
			// Invisible button to push button to the center.
			ImGui::Button("##Hidden", { (headerWidth_ / 4.0f), headerHeight_ });
			ImGui::SameLine();

			sizeMultiplier = 2.0f;
			coloursPushed = true;
			PushHeaderButtonStyle();
		}

		if ( stockPageHovered_ )
		{
			ImGui::PushStyleColor(ImGuiCol_Text, { 0.0f, 0.0f, 0.0f, 0.7f });
		}

		if ( ImGui::Button("STOCKS", { sizeMultiplier * (headerWidth_ / 4.0f), headerHeight_ }) )
		{
			selectedPage_ = STOCKS;
		}

		if (stockPageHovered_) ImGui::PopStyleColor(1);

		stockPageHovered_ = ImGui::IsItemHovered();

		if ( coloursPushed )
		{
			coloursPushed = false;
			PopHeaderButtonStyle();
		}

		ImGui::SameLine();

		sizeMultiplier = 1.0f;

		if ( selectedPage_ == ALGORITHMS )
		{
			sizeMultiplier = 2.0f;
			coloursPushed = true;
			PushHeaderButtonStyle();
		}

		if ( algorithmPageHovered_ )
		{
			ImGui::PushStyleColor(ImGuiCol_Text, { 0.0f, 0.0f, 0.0f, 0.7f });
		}
		
		if ( ImGui::Button("ALGORITHMS", { sizeMultiplier * (headerWidth_ / 4.0f), headerHeight_ }) )
		{
			selectedPage_ = ALGORITHMS;
		}

		if ( algorithmPageHovered_ ) ImGui::PopStyleColor(1);

		if ( selectedPage_ == ALGORITHMS )
		{
			ImGui::SameLine();
			ImGui::Button("##Hidden2", { headerWidth_ / 4.0f, headerHeight_ });
		}

		algorithmPageHovered_ = ImGui::IsItemHovered();

		if ( coloursPushed )
		{
			PopHeaderButtonStyle();
		}
	}
	ImGui::End();

	PopHeaderStyle();
}

void StockAid::PushHeaderStyle() const
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0.0f, 0.0f });
	ImGui::PushStyleColor(ImGuiCol_Button, IM_COLOUR_BACKGROUND(1.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COLOUR_BACKGROUND(1.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COLOUR_BACKGROUND(1.0f));
	ImGui::PushStyleColor(ImGuiCol_Text, { 0.0f, 0.0f, 0.0f, 0.3f });
	ImGui::PushFont(headerFont_);
}

void StockAid::PushHeaderButtonStyle() const
{
	ImGui::PushStyleColor(ImGuiCol_Text, { 0.0f, 0.0f, 0.0f, 1.0f });
	ImGui::PushFont(titleFont_);
}

void StockAid::PopHeaderStyle()
{
	ImGui::PopStyleVar(2);
	ImGui::PopStyleColor(4);
	ImGui::PopFont();
}

void StockAid::PopHeaderButtonStyle()
{
	ImGui::PopStyleColor(1);
	ImGui::PopFont();
}

void StockAid::ShowStockGraphLabels()
{
	if ( !stockGraph_ || stockGraph_->GetPlots().empty() ) return;

	float screenHeight = static_cast<float>(GetWindowSize().y);

	const float graphMaxX = mainAreaWidth_ * (stockGraph_->GetTransform().position.x + stockGraph_->GetTransform().scale.x);
	const float graphMinX = mainAreaWidth_ * (stockGraph_->GetTransform().position.x);
	const float graphMaxY = screenHeight - screenHeight * (stockGraph_->GetTransform().position.y + stockGraph_->GetTransform().scale.y);
	const float graphMinY = screenHeight - screenHeight * (stockGraph_->GetTransform().position.y);
	const float volumeMinX = mainAreaWidth_ * (volumeGraph_->GetTransform().position.x);
	const float volumeMaxY = screenHeight - screenHeight * (volumeGraph_->GetTransform().position.y + volumeGraph_->GetTransform().scale.y);
	const float volumeMinY = screenHeight - screenHeight * (volumeGraph_->GetTransform().position.y);

	auto firstIter = std::ranges::find(selectedStockData_.datesSinceEpoch, stockGraph_->GetRange().x);

	if ( firstIter == selectedStockData_.datesSinceEpoch.end() )
	{
		firstIter = selectedStockData_.datesSinceEpoch.begin();
	}

	auto lastIter = std::ranges::find(selectedStockData_.datesSinceEpoch, stockGraph_->GetRange().y);

	if ( lastIter == selectedStockData_.datesSinceEpoch.end() )
	{
		lastIter = selectedStockData_.datesSinceEpoch.end();
	}

	const float stockVolumeSpacing = min(
		(screenHeight - screenHeight * std::abs(volumeGraph_->GetTransform().position.y) - graphMinY) / 2.0f,
		(screenHeight - screenHeight * (volumeGraph_->GetTransform().position.y + volumeGraph_->GetTransform().scale.y) - graphMinY) / 2.0f
	);

	ImGui::SetNextWindowSize({ 120, 0 });
	ImGui::SetNextWindowPos({ graphMinX, graphMaxY }, ImGuiCond_Always, { 1.0f, 0.5f });
	ImGui::Begin("Labels Stock Top Y", nullptr, ImGuiWindowFlags_NoDecoration |  ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMouseInputs);
	{
		const std::string maxYString = std::format("{:.2f}",
												   *std::max_element(stockGraph_->GetPlots()[0]->GetYValues().values[stockGraph_->GetPlots()[0]->GetPlotHeader()].begin() + (firstIter - selectedStockData_.datesSinceEpoch.begin()),
																	 stockGraph_->GetPlots()[0]->GetYValues().values[stockGraph_->GetPlots()[0]->GetPlotHeader()].begin() + (lastIter - selectedStockData_.datesSinceEpoch.begin()))
		);

		std::stringstream labelStringStream;
		labelStringStream << std::right << std::setw(12) << maxYString;

		ImGui::Text(labelStringStream.str().c_str());
	}
	ImGui::End();

	ImGui::SetNextWindowSize({ 120, 0 });
	ImGui::SetNextWindowPos({ graphMinX, graphMinY }, ImGuiCond_Always, { 1.0f, 0.5f });
	ImGui::Begin("Labels Stock Bot Y", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMouseInputs);
	{
		std::string minYString = std::format("{:.2f}",
												   *std::min_element(stockGraph_->GetPlots()[0]->GetYValues().values[stockGraph_->GetPlots()[0]->GetPlotHeader()].begin() + (firstIter - selectedStockData_.datesSinceEpoch.begin()),
																	 stockGraph_->GetPlots()[0]->GetYValues().values[stockGraph_->GetPlots()[0]->GetPlotHeader()].begin() + (lastIter - selectedStockData_.datesSinceEpoch.begin()))
		);

		if ( stockGraph_->GetPlots()[0]->GetGraphType() == CANDLE )
		{
			minYString = std::format("{:.2f}",
									 *std::min_element(stockGraph_->GetPlots()[0]->GetYValues().values["Low"].begin() + (firstIter - selectedStockData_.datesSinceEpoch.begin()),
													   stockGraph_->GetPlots()[0]->GetYValues().values["Low"].begin() + (lastIter - selectedStockData_.datesSinceEpoch.begin()))
			);
		}

		std::stringstream labelStringStream;
		labelStringStream << std::right << std::setw(12) << minYString;

		ImGui::Text(labelStringStream.str().c_str());
	}
	ImGui::End();

	ImGui::SetNextWindowSize({ 120, 0 });
	ImGui::SetNextWindowPos({ volumeMinX, volumeMaxY }, ImGuiCond_Always, { 1.0f, 0.5f });
	ImGui::Begin("Labels Volume Top Y", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMouseInputs);
	{
		const std::string maxYString = std::format("{}",
												   *std::max_element(volumeGraph_->GetPlots()[0]->GetYValues().values[stockGraph_->GetPlots()[0]->GetPlotHeader()].begin() + (firstIter - selectedStockData_.datesSinceEpoch.begin()),
																	 volumeGraph_->GetPlots()[0]->GetYValues().values[stockGraph_->GetPlots()[0]->GetPlotHeader()].begin() + (lastIter - selectedStockData_.datesSinceEpoch.begin()))
		);

		std::stringstream labelStringStream;
		labelStringStream << std::right << std::setw(12) << maxYString;

		ImGui::Text(labelStringStream.str().c_str());
	}
	ImGui::End();

	ImGui::SetNextWindowSize({ 120, 0 });
	ImGui::SetNextWindowPos({ volumeMinX, volumeMinY }, ImGuiCond_Always, { 1.0f, 0.5f });
	ImGui::Begin("Labels Volume Bot Y", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMouseInputs);
	{
		const std::string minYString = std::format("{}",
		                                           *std::min_element(volumeGraph_->GetPlots()[0]->GetYValues().values[stockGraph_->GetPlots()[0]->GetPlotHeader()].begin() + (firstIter - selectedStockData_.datesSinceEpoch.begin()),
		                                                             volumeGraph_->GetPlots()[0]->GetYValues().values[stockGraph_->GetPlots()[0]->GetPlotHeader()].begin() + (lastIter - selectedStockData_.datesSinceEpoch.begin()))
		);

		std::stringstream labelStringStream;
		labelStringStream << std::setw(12) << std::right << minYString;

		ImGui::Text(labelStringStream.str().c_str());
	}
	ImGui::End();

	ImGui::SetNextWindowSize({ 120, 0 });
	ImGui::SetNextWindowPos({ graphMinX, graphMinY + stockVolumeSpacing }, ImGuiCond_Always, { 0.5f, 0.5f });
	ImGui::Begin("Labels Left X", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMouseInputs);
	{
		const auto iter = std::ranges::find(selectedStockData_.datesSinceEpoch, stockGraph_->GetRange().x);
		if (iter != selectedStockData_.datesSinceEpoch.end())
		{
			const auto index = iter - selectedStockData_.datesSinceEpoch.begin();
			ImGui::Text("%s", selectedStockData_.dates[index].c_str());
		}
	}
	ImGui::End();

	ImGui::SetNextWindowSize({ 120, 0 });
	ImGui::SetNextWindowPos({ graphMaxX, graphMinY + stockVolumeSpacing }, ImGuiCond_Always, { 0.5f, 0.5f });
	ImGui::Begin("Labels Right X", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMouseInputs);
	{
		const auto iter = std::ranges::find(selectedStockData_.datesSinceEpoch, stockGraph_->GetRange().y);
		if ( iter != selectedStockData_.datesSinceEpoch.end() )
		{
			const auto index = iter - selectedStockData_.datesSinceEpoch.begin();
			ImGui::Text("%s", selectedStockData_.dates[index].c_str());
		}
	}
	ImGui::End();
}

void StockAid::ShowStockPage()
{
	stockGraph_->Enable();
	volumeGraph_->Enable();

	lossGraph_->Disable();
	accuracyGraph_->Disable();
	rewardGraph_->Disable();
	
	ImGui::SetNextWindowPos({ 0, headerHeight_ }, ImGuiCond_Always);
	ImGui::SetNextWindowSize({ mainAreaWidth_, mainAreaHeight_ }, ImGuiCond_Always);
	ImGui::Begin("Main Area", nullptr, kUiFlags | ImGuiWindowFlags_NoMouseInputs);
	{
		ShowStockLeftPanel();
		
		ShowStockRightPanel();

		ShowStockGraphLabels();

		ShowStockGraphButtons();
	}
	ImGui::End();
}

void StockAid::ShowStockLeftPanel()
{
	ImGui::SetNextWindowPos({ 0, headerHeight_ }, ImGuiCond_Always);
	ImGui::SetNextWindowSize({ mainAreaWidth_ * 0.2f, mainAreaHeight_ }, ImGuiCond_Always);
	ImGui::Begin("Stock Left Info Panel", nullptr, kUiFlags);
	{
		ImGui::BeginChild("Stock Selection Menu", { mainAreaWidth_ * 0.185f, mainAreaHeight_ * 0.5f });
		{
			if ( GenerateListBox("Stocks", { -FLT_MIN, -FLT_MIN }, stocks_, selectedStock_) )
			{
				changedStock_ = true;
				graphNeedsRefreshing_ = true;
			}
		}
		ImGui::EndChild();

		UISpacing();

		ImGui::BeginChild("Stock Actions", { mainAreaWidth_ * 0.2f, mainAreaHeight_ * 0.4f });
		{
			TextAsHeader("Actions");
			CreateStockGraphStatisticsSelection();
		}
		ImGui::EndChild();
	}
	ImGui::End();
}

void StockAid::CreateStockGraphStatisticsSelection()
{
	static bool show60DayAverage;
	static bool showLinearRegression;

	if ( show60DayAverage )
	{
		if ( stockGraph_->GetPlot("60 Day Average") )
		{
			stockGraph_->GetPlot("60 Day Average")->SetState(Component::ACTIVE);
		}
	} else
	{
		if ( stockGraph_->GetPlot("60 Day Average") )
		{
			stockGraph_->GetPlot("60 Day Average")->SetState(Component::SLEEP);
		}
	}

	ImGui::Checkbox("60 Day Average", &show60DayAverage);

	if ( showLinearRegression )
	{
		if ( stockGraph_->GetPlot("Regression") )
		{
			stockGraph_->GetPlot("Regression")->SetState(Component::ACTIVE);
		}
	} else
	{
		if ( stockGraph_->GetPlot("Regression") )
		{
			stockGraph_->GetPlot("Regression")->SetState(Component::SLEEP);
		}
	}

	ImGui::Checkbox("linear Regression", &showLinearRegression);


	if ( ImGui::Button("Run Backtesting", { mainAreaWidth_ * 0.15f, 30 }) )
	{
		isUpdating_ = true;
		newPortfolioSim_ = true;
	}
}

void StockAid::ShowStockRightPanel()
{
	ImGui::SetNextWindowPos({ mainAreaWidth_ * 0.8f, headerHeight_ }, ImGuiCond_Always);
	ImGui::Begin("Right Info Panel", nullptr, kUiFlags);
	{
		ImGui::BeginChild("Information Panel", { mainAreaWidth_ * 0.2f, mainAreaHeight_ * 0.5f });
		{
			TextAsHeader("Statistics");
			if ( MouseHoveringGraph() )
			{
				ShowInformationAtMousePosition();
			}
			else
			{
				ImGui::TextWrapped("Hover over the graph to see values");
				stockGraph_->HideSelectionMarker();
			}
		}
		ImGui::EndChild();
	}
	ImGui::End();
}

void StockAid::ShowStockGraphButtons()
{
	const float windowHeight = static_cast<float>(GetWindowSize().y);
	const float headerToTopOfGraphSpacing = (windowHeight - windowHeight * (stockGraph_->GetTransform().position.y + stockGraph_->GetTransform().scale.y)) - headerHeight_;

	ImGui::SetNextWindowPos({ mainAreaWidth_ * 0.5f, headerHeight_ - 5 + max(0, headerToTopOfGraphSpacing - 50)}, ImGuiCond_Always, {0.5f, 0.0f});
	ImGui::Begin("Graph Buttons", nullptr, kUiFlags);
	{
		bool pressed = false;

		auto datesVector = selectedStockData_.datesSinceEpoch;

		uint64_t lastDay = datesVector.back();

		// MAX BUTTON
		{ 
			if ( startDate_ < 0.05f )
			{
				ImGui::PushStyleColor(ImGuiCol_Button, IM_COLOUR_MAIN(1.0f));
				ImGui::PushStyleColor(ImGuiCol_Text, { 1.0f, 1.0f, 1.0f, 1.0f });
				pressed = true;
			}

			if ( ImGui::Button("Max", ImVec2(80, 30)) )
			{
				startDate_ = 0;
				endDate_ = 100;
				graphNeedsRefreshing_ = true;
			}

			if ( pressed )
			{
				ImGui::PopStyleColor(2);
			}
		}

		ImGui::SameLine();

		// YEAR BUTTON
		{	
			auto isLessThanYearAgo = [lastDay](const uint64_t i)
			{
				return i > (lastDay - 365);
			};

			const auto firstDateAfterYearIterator = std::ranges::find_if(datesVector.begin(), datesVector.end(),
																		 isLessThanYearAgo);
			const int yearBackIndex = static_cast<int>(firstDateAfterYearIterator - datesVector.begin());

			const float yearDate = (static_cast<float>(yearBackIndex) / static_cast<float>(datesVector.size()) * 100);

			pressed = false;
			if ( startDate_ > yearDate - 0.05f and startDate_ < yearDate + 0.05f )
			{
				ImGui::PushStyleColor(ImGuiCol_Button, IM_COLOUR_MAIN(1.0f));
				ImGui::PushStyleColor(ImGuiCol_Text, { 1.0f, 1.0f, 1.0f, 1.0f });

				pressed = true;
			}

			if ( ImGui::Button("Year", ImVec2(80, 30)) )
			{
				startDate_ = yearDate;
				endDate_ = 100.0f;
				graphNeedsRefreshing_ = true;
			}

			if ( pressed )
			{
				ImGui::PopStyleColor(2);
			}
		}

		ImGui::SameLine();

		// MONTH BUTTON
		{
			auto isLessThanMonthAgo = [lastDay](const uint64_t i)
			{
				return i > (lastDay - 30);
			};

			const auto firstDateAfterMonthIterator = std::ranges::find_if(datesVector.begin(), datesVector.end(),
																		  isLessThanMonthAgo);
			const int monthBackIndex = static_cast<int>(firstDateAfterMonthIterator - datesVector.begin());

			const float monthDate = (static_cast<float>(monthBackIndex) / static_cast<float>(datesVector.size()) * 100);

			pressed = false;
			if ( startDate_ > monthDate - 0.05f and startDate_ < monthDate + 0.05f )
			{
				ImGui::PushStyleColor(ImGuiCol_Button, IM_COLOUR_MAIN(1.0f));
				ImGui::PushStyleColor(ImGuiCol_Text, { 1.0f, 1.0f, 1.0f, 1.0f });

				pressed = true;
			}

			if ( ImGui::Button("Month", ImVec2(80, 30)) )
			{
				startDate_ = monthDate;
				endDate_ = 100;
				graphNeedsRefreshing_ = true;
			}

			if ( pressed )
			{
				ImGui::PopStyleColor(2);
			}
		}

		ImGui::SameLine();

		// WEEK BUTTON
		{
			auto isLessThanWeekAgo = [lastDay](const uint64_t i)
			{
				return i > (lastDay - 7);
			};

			const auto firstDateAfterWeekIterator = std::ranges::find_if(datesVector.begin(), datesVector.end(),
																		 isLessThanWeekAgo);
			const int weekBackIndex = static_cast<int>(firstDateAfterWeekIterator - datesVector.begin());

			const float weekDate = (static_cast<float>(weekBackIndex) / static_cast<float>(datesVector.size()) * 100);

			pressed = false;
			if ( startDate_ > weekDate - 0.05f and startDate_ < weekDate + 0.05f )
			{
				ImGui::PushStyleColor(ImGuiCol_Button, IM_COLOUR_MAIN(1.0f));
				ImGui::PushStyleColor(ImGuiCol_Text, { 1.0f, 1.0f, 1.0f, 1.0f });

				pressed = true;
			}

			if ( ImGui::Button("Week", ImVec2(80, 30)) )
			{
				startDate_ = weekDate;
				endDate_ = 100;
				graphNeedsRefreshing_ = true;
			}

			if ( pressed )
			{
				ImGui::PopStyleColor(2);
			}
		}

		ImGui::SameLine();

		ImGui::Text("Current Stock: %s", selectedStock_.c_str());

		ImGui::SameLine();

		// LINE BUTTON
		{
			pressed = false;
			if ( stockGraph_ and stockGraph_->GetPlots()[0]->GetGraphType() == LINE )
			{
				ImGui::PushStyleColor(ImGuiCol_Button, IM_COLOUR_MAIN(1.0f));
				ImGui::PushStyleColor(ImGuiCol_Text, { 1.0f, 1.0f, 1.0f, 1.0f });

				pressed = true;
			}

			if ( ImGui::Button("Line", ImVec2(80, 30)) )
			{
				if ( stockGraph_ && stockGraph_->GetPlots().size() > 0)
				{
					stockGraph_->GetPlots()[0]->SetGraphType(LINE);
					graphNeedsRefreshing_ = true;
				}
			}

			if ( pressed )
			{
				ImGui::PopStyleColor(2);
			}
		}

		ImGui::SameLine();

		// BAR BUTTON
		{
			pressed = false;
			if ( stockGraph_ and stockGraph_->GetPlots()[0]->GetGraphType() == BAR )
			{
				ImGui::PushStyleColor(ImGuiCol_Button, IM_COLOUR_MAIN(1.0f));
				ImGui::PushStyleColor(ImGuiCol_Text, { 1.0f, 1.0f, 1.0f, 1.0f });

				pressed = true;
			}

			if ( ImGui::Button("Bar", ImVec2(80, 30)) )
			{
				if (stockGraph_ && stockGraph_->GetPlots().size() > 0 )
				{
					stockGraph_->GetPlots()[0]->SetGraphType(BAR);
					graphNeedsRefreshing_ = true;
				}
			}

			if ( pressed )
			{
				ImGui::PopStyleColor(2);
			}
		}

		ImGui::SameLine();

		// CANDLE BUTTON
		{
			pressed = false;
			if ( stockGraph_ and stockGraph_->GetPlots()[0]->GetGraphType() == CANDLE )
			{
				ImGui::PushStyleColor(ImGuiCol_Button, IM_COLOUR_MAIN(1.0f));
				ImGui::PushStyleColor(ImGuiCol_Text, { 1.0f, 1.0f, 1.0f, 1.0f });

				pressed = true;
			}

			if ( ImGui::Button("Candle", ImVec2(80, 30)) )
			{
				if ( stockGraph_ && stockGraph_->GetPlots().size() > 0 )
				{
					stockGraph_->GetPlots()[0]->SetGraphType(CANDLE);
					graphNeedsRefreshing_ = true;
				}
			}

			if ( pressed )
			{
				ImGui::PopStyleColor(2);
			}
		}
	}
	ImGui::End();
}

void StockAid::ShowAlgorithmPage()
{
	stockGraph_->Disable();
	volumeGraph_->Disable();

	lossGraph_->Enable();
	accuracyGraph_->Enable();
	rewardGraph_->Enable();
	
	ImGui::SetNextWindowPos({ 0, headerHeight_ }, ImGuiCond_Always);
	ImGui::SetNextWindowSize({ mainAreaWidth_, mainAreaHeight_ }, ImGuiCond_Always);
	ImGui::Begin("Main Algorithm Area", nullptr, kUiFlags | ImGuiWindowFlags_NoMouseInputs);
	{
		ShowAlgorithmLeftPanel();

		ShowAlgorithmRightPanel();
	}
	ImGui::End();
}

void StockAid::ShowAlgorithmLeftPanel()
{
	ImGui::SetNextWindowPos({ 0, headerHeight_ }, ImGuiCond_Always);
	ImGui::SetNextWindowSize({ mainAreaWidth_ * 0.2f, mainAreaHeight_ }, ImGuiCond_Always);
	ImGui::Begin("Left Algorithm Info Panel", nullptr, kUiFlags);
	{
		ImGui::BeginChild("Algorithm Selection Menu", { mainAreaWidth_ * 0.185f, mainAreaHeight_ * 0.5f });
		{
			if ( GenerateListBox("Algorithms", { mainAreaWidth_ * 0.15f, 200 }, algorithms_, selectedAlgorithm_, true) )
			{
				graphNeedsRefreshing_ = true;
				changedAlgorithm_ = true;
				pythonClass_.LoadNewModule(selectedAlgorithm_);
			}
		}
		ImGui::EndChild();

		ImGui::BeginChild("Algorithm Interaction Menu", { mainAreaWidth_ * 0.185f, mainAreaHeight_ * 0.4f });
		{
			CreatePythonInteractionElements();
		}
		ImGui::EndChild();
	}
	ImGui::End();
}

void StockAid::ShowAlgorithmRightPanel()
{
	ImGui::SetNextWindowPos({ mainAreaWidth_ * 0.8f, headerHeight_ }, ImGuiCond_Always);
	ImGui::Begin("Right Algorithm Info Panel", nullptr, kUiFlags);
	{
		ImGui::BeginChild("Algorithm Information Panel", { mainAreaWidth_ * 0.2f, mainAreaHeight_ * 0.5f });
		{
			TextAsHeader("Statistics");
		}
		ImGui::EndChild();
	}
	ImGui::End();
}

void StockAid::UpdateMouseOverGraphPosition()
{
	const glm::vec2 stockGraphPos = stockGraph_->GetTransform().position;
	const glm::vec2 stockGraphScale = stockGraph_->GetTransform().scale;

	graphSize_ = glm::vec2(GetWindowSize().x, GetWindowSize().y) * stockGraphScale;

	const glm::vec2 bottomLeftPixel = glm::vec2(GetWindowSize().x, GetWindowSize().y) * stockGraphPos;

	mousePos_ = ImGui::GetMousePos();
	mouseOverGraphPosition_ = {
		(mousePos_.x - bottomLeftPixel.x) / graphSize_.x,
		(static_cast<float>(GetWindowSize().y) - mousePos_.y - bottomLeftPixel.y) / graphSize_.y
	};
}

bool StockAid::MouseHoveringGraph() const
{
	return mouseOverGraphPosition_.x >= 0 && mouseOverGraphPosition_.x < 1.0f && mouseOverGraphPosition_.y >= 0 &&
		mouseOverGraphPosition_.y < 1.0f;
}

void StockAid::ShowInformationAtMousePosition()
{
	const auto firstVal = std::ranges::find(selectedStockData_.datesSinceEpoch, stockGraph_->GetRange().x);
	const auto lastVal = std::ranges::find(selectedStockData_.datesSinceEpoch, stockGraph_->GetRange().y) + 1;

	const float totalValuesInSelectedRange = static_cast<float>(lastVal - firstVal);

	const float indexOfFirstValueInSelection = static_cast<float>(firstVal - selectedStockData_.datesSinceEpoch.begin());

	int closestIndexOfCurrentlySelected = 0;

	// Change which data point is closest based on type of graph, as bar plot extends value forward, and line is centered.
	if (stockGraph_->GetPlots()[0]->GetGraphType() == BAR or stockGraph_->GetPlots()[0]->GetGraphType() == CANDLE )
	{
		closestIndexOfCurrentlySelected = static_cast<int>(std::floor(indexOfFirstValueInSelection + ((totalValuesInSelectedRange) * mouseOverGraphPosition_.x)));
	}
	else if ( stockGraph_->GetPlots()[0]->GetGraphType() == LINE)
	{
		closestIndexOfCurrentlySelected = static_cast<int>(std::round(indexOfFirstValueInSelection + ((totalValuesInSelectedRange - 1) * mouseOverGraphPosition_.x)));
	}

	stockGraph_->ShowSelectionMarker(closestIndexOfCurrentlySelected);

	ImGui::TextWrapped("Graph Values for index %i", closestIndexOfCurrentlySelected);
	const std::string date = selectedStockData_.dates[closestIndexOfCurrentlySelected];
	const float open = selectedStockData_.values["Open"][closestIndexOfCurrentlySelected];
	const float close = selectedStockData_.values["Close"][closestIndexOfCurrentlySelected];
	const float high = selectedStockData_.values["High"][closestIndexOfCurrentlySelected];
	const float low = selectedStockData_.values["Low"][closestIndexOfCurrentlySelected];
	const int volume = static_cast<int>(selectedStockData_.values["Volume"][closestIndexOfCurrentlySelected]);

	ImGui::TextWrapped("Date %s", date.c_str());
	ImGui::TextWrapped("Open %.2f", static_cast<double>(open));
	ImGui::TextWrapped("Close %.2f", static_cast<double>(close));
	ImGui::TextWrapped("High %.2f", static_cast<double>(high));
	ImGui::TextWrapped("Low %.2f", static_cast<double>(low));
	ImGui::TextWrapped("Volume %i", volume);
}

void StockAid::ShowDebugInformation() const
{
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", static_cast<double>(1000.0f / ImGui::GetIO().Framerate),
	            static_cast<double>(ImGui::GetIO().Framerate));
	ImGui::Text("Mouse Position x:%f y:%f)", static_cast<double>(mousePos_.x), static_cast<double>(mousePos_.y));
	ImGui::Text("Mouse Position On Graph x:%.4f y:%.4f)", static_cast<double>(mouseOverGraphPosition_.x),
	            static_cast<double>(mouseOverGraphPosition_.y));
}

void StockAid::TextAsHeader(const std::string& text) const
{
	ImGui::PushFont(headerFont_);
	ImGui::Text(text.c_str());
	ImGui::PopFont();
}

bool StockAid::GenerateListBox(const std::string& header, const ImVec2 size, const std::vector<std::string>& values, std::string& selection, const bool showToolTips) const
{
	TextAsHeader(header);
	bool valueChanged = false;
	if (ImGui::BeginListBox(std::string("##" + header).c_str(), size))
	{
		for (const auto& value : values)
		{
			if (ImGui::Selectable(value.c_str(), value == selection))
			{
				valueChanged = true;
				selection = value;
			}

			if (!showToolTips ) continue;

			std::string description = ReadDescriptionFile(value);
			if (description.empty()) continue;
			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip(description.c_str());
			}
		}
		ImGui::EndListBox();
	}
	return valueChanged;
}

void StockAid::UISpacing()
{
	ImGui::NewLine();
	ImGui::Separator();
	ImGui::NewLine();
}



void StockAid::CreatePythonInteractionElements()
{
	ImGui::BeginDisabled(training_);
	if (ImGui::Button("Reload Module", {mainAreaWidth_ * 0.15f, 30}))
	{
		pythonClass_.ReloadModule(selectedAlgorithm_);
	}

	if (ImGui::Button("Setup", { mainAreaWidth_ * 0.15f, 30}))
	{
		pythonClass_.Initialise();
	}

	static int epochs;
	ImGui::SetNextItemWidth(mainAreaWidth_ * 0.1f);
	ImGui::DragInt("Epochs", &epochs, 1, 0, 1000);

	if (ImGui::Button("Train", { mainAreaWidth_ * 0.15f, 30}))
	{
		trainThread_ = pythonClass_.Train(epochs);
	}

	static std::string loadFileName;
	loadFileName.resize(80);
	ImGui::InputText(":Load", loadFileName.data(), 80);

	if (ImGui::Button("load Weights", { mainAreaWidth_ * 0.15f, 30}))
	{
		pythonClass_.LoadWeights(loadFileName);
		changedAlgorithm_ = true;
		graphNeedsRefreshing_ = true;
	}

	static std::string saveFileName;
	saveFileName.resize(80);
	ImGui::InputText(":Save", saveFileName.data(), 80);

	if (ImGui::Button("Save Weights", { mainAreaWidth_ * 0.15f, 30}))
	{
		pythonClass_.SaveWeights(saveFileName);
	}

	if (ImGui::Button("Evaluate", { mainAreaWidth_ * 0.15f, 30}))
	{
		pythonClass_.Evaluate({1});
	}
	ImGui::EndDisabled();
}

void StockAid::MakePredictions()
{
	static bool isBuying = false;
	static float buyAmount = 0;
	static std::vector<uint64_t> startingDates = {};
	static int startingDate = 0;
	static int i = 0;
	static auto firstDateInSelection = std::ranges::find(selectedStockData_.dates, startDateString_);
	static auto lastDateInSelection = std::ranges::find(selectedStockData_.dates, endDateString_) + 1;
	static int daysToPredict = static_cast<int>(lastDateInSelection - firstDateInSelection);
	static std::vector<int> predictions;

	if (newPortfolioSim_)
	{
		delete predictionGraph_;
		predictionGraph_ = nullptr;
		if ( stockGraph_->GetPlot("Prediction")) stockGraph_->RemovePlot("Prediction");

		newPortfolioSim_ = false;
		i = 0;
		firstDateInSelection = std::ranges::find(selectedStockData_.dates, startDateString_);
		lastDateInSelection = std::ranges::find(selectedStockData_.dates, endDateString_) + 1;
		daysToPredict = static_cast<int>(lastDateInSelection - firstDateInSelection);
		isBuying = false;
		xValsModel_.clear();
		xValsModel_.resize(daysToPredict);
		yValsModel_.clear();
		yValsModel_.resize(daysToPredict);
		startingDate = static_cast<int>(firstDateInSelection - selectedStockData_.dates.begin());
		startingDates = selectedStockData_.datesSinceEpoch;

		predictions = pythonClass_.Predict(selectedStock_, startDateString_, endDateString_);
	}


	if (i < daysToPredict)
	{
		xValsModel_[i] = static_cast<int>(startingDates[startingDate + i]);

		//For the first 15 iterations just use the 15th (index 14 starting from selection start) value, as we assume stock predictor needs initial data.
		if (i < 15)
		{
			yValsModel_[i] = (selectedStockData_.values["High"][startingDate + 14]);
		}
		else
		{
			const float differenceFromDayBeforeValue = (selectedStockData_.values["High"][startingDate + i] - selectedStockData_.values["High"][startingDate + i - 1]);

			yValsModel_[i] = (yValsModel_[i - 1] + (buyAmount * differenceFromDayBeforeValue));

			const std::vector<float>::const_iterator last = selectedStockData_.values["High"].begin() + (startingDate + i);
			const auto first = last - 15;

			const std::vector newVec(first, last);
			int prediction = -1;
			if ( predictions.size() >= i )
			{
				prediction = predictions[i];
			}

			// If failed to make predictions just buy (For testing)
			if ( prediction == -1 ) prediction = 0;

			if (prediction == 0)
			{
				buyAmount++;
				if (buyAmount > 1) buyAmount = 1;
				isBuying = true;
			}
			else if (prediction == 2)
			{
				buyAmount = 0;
				if (buyAmount < 0) buyAmount = 0;
				isBuying = false;
			}
		}
		graphNeedsRefreshing_ = true;
		i++;
		numberOfPredictionsMade_ = i;
	}
	else
	{
		DataTable predictionTable;
		predictionTable.values["Prediction"] = yValsModel_;

		if (!predictionGraph_)
		{
			predictionGraph_ = new AGraph(this);
			predictionGraph_->AddPlot("PredictionGraph", xValsModel_, predictionTable, "Prediction", predictionPlotColour_, LINE);
			predictionGraph_->GetTransform().position = {0.8f, 0.2f};
			predictionGraph_->GetTransform().scale = {0.15f, 0.25f};
		}
		else
		{
			predictionGraph_->GetPlots()[0]->UpdateValues(xValsModel_, predictionTable);
		}

		const std::string startDateString = selectedStockData_.dates[static_cast<int>(selectedStockData_.dates.size() * (startDate_ / 100.0f))];
		const std::string endDateString = selectedStockData_.dates[static_cast<int>(selectedStockData_.dates.size() * (endDate_ / 100.0f)) - 1];

		predictionGraph_->SetRange(dv_math::Vec2{ static_cast<int>(selectedStockData_.datesSinceEpoch[static_cast<int>(selectedStockData_.dates.size() * (startDate_ /
			100.0f))]), (int)selectedStockData_.datesSinceEpoch[static_cast<int>(selectedStockData_.dates.size() * (endDate_ / 100.0f)) - 1] });
		predictionGraph_->Draw();

		isUpdating_ = false;
	}
}

[[nodiscard]] std::string StockAid::ReadDescriptionFile(const std::string& name) const
{
	const std::filesystem::path descriptionPath = "./Models/Python/" + name + "/Description.txt";
	std::ifstream file(descriptionPath);

	if (!file.is_open())
	{
		return "";
	}

	std::stringstream fileContents;
	std::string line;

	while (std::getline(file, line))
	{
		fileContents << line << "\n";
	}

	file.close();

	return fileContents.str();
}

Application* CreateApplication()
{
	return new StockAid();
}
