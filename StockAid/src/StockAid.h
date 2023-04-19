#pragma once
#include "pcHeader.h"
#include "Core/main.h"
#include "Core/Application.h"
#include <chrono>
#include <filesystem>
#include <functional>

#include "Core/DataUtilities.h"
#include "Core/PythonUtilities.h"
#include "Visualisation/GraphPlot.h"

class AGraph;

class StockAid final : public Application
{
public:
	void OnStartup() override;
	void OnUpdate(float deltaTime) override;
	void OnShutdown() override;

private:
	void SetupImGui();
	void SetupData();
	void LoadStockGraphs();
	void LoadAlgorithmGraphs();
	void UpdateLossGraph();
	void CreateLossGraph();
	void UpdateAccuracyGraph();
	void CreateAccuracyGraph();
	void UpdateRewardGraph();
	void CreateRewardGraph();
	void CalculateSharpeRatio();
	void UpdateStockGraph();
	void CreateStockGraph();
	void UpdateVolumeGraph();
	void CreateVolumeGraph();
	void UpdateUI();
	void Plot60DayAverage();
	void PlotLinearRegression();
	void ShowHeader();
	void PushHeaderStyle() const;
	void PushHeaderButtonStyle() const;
	static void PopHeaderStyle();
	static void PopHeaderButtonStyle();
	void ShowStockGraphLabels();
	void ShowStockPage();
	void ShowStockLeftPanel();
	void CreateStockGraphStatisticsSelection();
	void ShowStockRightPanel();
	void ShowStockGraphButtons();
	void ShowAlgorithmPage();
	void ShowAlgorithmLeftPanel();
	void ShowAlgorithmRightPanel();
	void UpdateMouseOverGraphPosition();
	bool MouseHoveringGraph() const;
	void ShowInformationAtMousePosition();
	void ShowDebugInformation() const;
	void TextAsHeader(const std::string& text) const;
	bool GenerateListBox(const std::string& header, ImVec2 size, const std::vector<std::string>& values,
						 std::string& selection, bool toolTips = false) const;
	static void UISpacing();
	void CreatePythonInteractionElements();
	void MakePredictions();
	[[nodiscard]] std::string ReadDescriptionFile(const std::string& name) const;

protected:
	// GRAPHS
	AGraph* stockGraph_ = nullptr;
	AGraph* volumeGraph_ = nullptr;

	AGraph* lossGraph_ = nullptr;
	AGraph* accuracyGraph_ = nullptr;
	AGraph* rewardGraph_ = nullptr;

	AGraph* predictionGraph_ = nullptr;

	ImFont* mainFont_{};
	ImFont* headerFont_{};
	ImFont* titleFont_{};

	// GRAPH VARIABLES
	glm::vec3 col_ = { 0.86f, 0.43f, 0.1f };
	std::string selectedStock_{};
	bool changedStock_ = true;
	bool changedAlgorithm_ = true;
	std::string selectedAlgorithm_{};
	float startDate_ = 0.0f;
	float endDate_ = 100.0f;

	std::string startDateString_;
	std::string endDateString_;

	std::vector<int> xValsModel_;
	std::vector<float> yValsModel_;
	DataTable selectedStockData_{};
	GraphType type_ = LINE;

	// APPLICATION VARIABLES
	bool isUpdating_ = false;
	bool newPortfolioSim_ = true;
	PythonCaller pythonClass_{};

	std::vector<std::string> stocks_ = {};
	std::vector<std::string> algorithms_ = {};

	bool graphNeedsRefreshing_ = true;

	// MOUSE VARIABLES
	ImVec2 mousePos_{};
	glm::vec2 graphSize_ = {};
	ImVec2 mouseOverGraphPosition_{};

	int numberOfPredictionsMade_{};

	dv_math::Vec3<float> stockPlotColour_{ 0.6f, 0.6f, 0.6f };
	dv_math::Vec3<float> predictionPlotColour_{ 0.2f, 0.2f, 0.8f };
	dv_math::Vec3<float> volumePlotColour_{ 0.6f, 0.6f, 0.6f };

	enum Pages { STOCKS, ALGORITHMS };

	Pages selectedPage_ = STOCKS;
	bool stockPageHovered_ = false;
	bool algorithmPageHovered_ = false;

	float headerWidth_ = static_cast<float>(GetWindowSize().x);
	float headerHeight_ = 75.0f;

	float mainAreaWidth_ = static_cast<float>(GetWindowSize().x);
	float mainAreaHeight_ = static_cast<float>(GetWindowSize().y) - headerHeight_;

	bool training_ = false;
	std::thread* trainThread_{};
};
