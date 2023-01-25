#include "AGraph.h"

#include "Core/Application.h"
#include "Core/Components/line_component.h"

AGraph::AGraph(application* app, const std::string x_plot)
	: actor(app)
	, graphAxis_(this)
	, x_plot_(x_plot)
	, graphPlots_({})
{
	const std::vector<PMATH::vertex> points = {
		{0.0f, 1.0f},
		{0.0f, 0.0f},
		{1.0f, 0.0f},
	};
	graphAxis_.set_color(glm::vec3{ 0.86f, 1.43f, 1.1f });
	graphAxis_.init(points);
}

void AGraph::AddPlot(const std::string stockName, const std::string y_plot, const PMATH::vec3<float> colour, GraphType type)
{
	PlotInfo plotInfo{};
	plotInfo.stockName = stockName;
	plotInfo.y_plot = y_plot;
	plotInfo.colour = colour;
	plotInfo.type = type;
	toPlot_.push_back(plotInfo);
}

void AGraph::AddPlot(std::vector<int> xValues, std::vector<float> yValues, const PMATH::vec3<float> colour, GraphType type)
{
	graphPlots_.emplace_back(this, xValues, yValues, colour);
	graphPlots_.back().setGraphType(type);
}

void AGraph::Draw()
{
	float max = 0;

	for (auto [stockName, y_plot, colour, type] : toPlot_)
	{
		data_table data = load_data(stockName);
		 
		std::vector<uint64_t> dates = data.dates_int();
		auto start = std::ranges::find(dates, range_.x);
		auto end = std::ranges::find(dates, range_.y);

		if (start == dates.end())
		{
			start = dates.begin();
		}

		const int start_ind = static_cast<int>(start - dates.begin());
		const int end_ind = static_cast<int>(start_ind + (end - start));

		std::vector<int> x_vals(dates.begin() + start_ind, dates.begin() + end_ind);

		std::vector<float> y_vals(data.values[y_plot].begin() + start_ind, data.values[y_plot].begin() + end_ind);

		for (auto& val : y_vals)
		{
			max = max(max, val);
		}

		std::vector<int> datesx = {};

		for (auto x : dates)
		{
			datesx.push_back((int)x);
		}

		graphPlots_.emplace_back(this, x_vals, y_vals, colour);
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
