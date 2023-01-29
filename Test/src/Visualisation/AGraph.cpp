#include "AGraph.h"

#include "Core/Application.h"
#include "Core/Components/CLineChart.h"

AGraph::AGraph(Application* app, const std::string x_plot)
	: Actor(app)
	, graphAxis_(this)
	, xPlot_(x_plot)
	, graphPlots_({})
{
	const std::vector<PMATH::vertex> points = {
		{0.0f, 1.0f},
		{0.0f, 0.0f},
		{1.0f, 0.0f},
	};
	graphAxis_.SetColor(glm::vec3{ 0.86f, 1.43f, 1.1f });
	graphAxis_.Init(points);
}

AGraph::~AGraph()
{
}

void AGraph::AddPlot(const std::string& stockName, const std::string& yPlot, const PMATH::vec3<float>& colour, const GRAPH_TYPE type)
{
	PlotInfo plotInfo{};
	plotInfo.stockName = stockName;
	plotInfo.y_plot = yPlot;
	plotInfo.colour = colour;
	plotInfo.type = type;
	toPlot_.push_back(plotInfo);
}

void AGraph::AddPlot(const std::vector<int>& xValues, const std::vector<float>& yValues, const PMATH::vec3<float> colour, const GRAPH_TYPE type)
{
	graphPlots_.emplace_back(this, xValues, yValues, colour);
	graphPlots_.back().setGraphType(type);
}

void AGraph::Draw()
{
	float max = 0;

	for (auto [stockName, y_plot, colour, type] : toPlot_)
	{
		DataTable data = LoadData(stockName);
		 
		std::vector<uint64_t> dates = data.DatesInt();
		auto start = std::ranges::find(dates, range_.x);
		auto end = std::ranges::find(dates, range_.y);

		if (start == dates.end())
		{
			start = dates.begin();
		}

		const int startInd = static_cast<int>(start - dates.begin());
		const int endInd = static_cast<int>(startInd + (end - start));

		std::vector<int> xVals(dates.begin() + startInd, dates.begin() + endInd);

		std::vector<float> yVals(data.values[y_plot].begin() + startInd, data.values[y_plot].begin() + endInd);

		for (auto& val : yVals)
		{
			max = max(max, val);
		}
		
		graphPlots_.emplace_back(this, xVals, yVals, colour);
		graphPlots_.back().setGraphType(type);
	}

	for (auto& plot : graphPlots_)
	{
		plot.Draw(0, max, range_.x, range_.y);
	}
}

void AGraph::SetRange(const PMATH::vec2<int> range)
{
	range_ = range;
}

void AGraph::SetRange(const std::string& start, const std::string end)
{
	const std::string date_time_format = "%Y-%m-%d";

	std::chrono::year_month_day startDate{};
	std::istringstream startDateStream(start);

	std::chrono::year_month_day endDate{};
	std::istringstream endDateStream(end);

	startDateStream >> std::chrono::parse(date_time_format, startDate);
	endDateStream >> std::chrono::parse(date_time_format, endDate);

	range_ = { std::chrono::sys_days{ startDate }.time_since_epoch().count() , std::chrono::sys_days{ endDate }.time_since_epoch().count() };
}
