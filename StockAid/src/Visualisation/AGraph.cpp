#include "AGraph.h"

#include "Core/Application.h"
#include "Core/Components/CLineChart.h"

AGraph::AGraph(Application* app, std::string xPlot)
	: Actor(app)
	  , xPlot_(std::move(xPlot))
	  , graphPlots_({})
{
	delete graphAxis_;
	graphAxis_ = new LineComponent(this, "GraphAxis");

	delete graphGrid_;
	graphGrid_ = new LineComponent(this, "GraphGrid");
}

void AGraph::ShowSelectionMarker(const int selected)
{
	selected_ = selected;

	if ( graphPlots_.empty() ) return;

	if ( !selectedMarker_ ) selectedMarker_ = new LineComponent(this, "GraphMarker");

	selectedMarker_->SetState(Component::ACTIVE);

	const dv_math::Vertex location = graphPlots_[0]->GetLocationOf(selected_);

	const std::vector<dv_math::Vertex> points = {
		{ location.position.x - (0.005f), location.position.y - (0.005f * (GetTransform().scale.y / GetTransform().scale.x) * 1.5f) },
		{ location.position.x + (0.005f), location.position.y + (0.005f * (GetTransform().scale.y / GetTransform().scale.x) * 1.5f) },
		{ location.position.x, location.position.y},
		{ location.position.x - (0.005f), location.position.y + (0.005f * (GetTransform().scale.y / GetTransform().scale.x) * 1.5f) },
		{ location.position.x + (0.005f), location.position.y - (0.005f * (GetTransform().scale.y / GetTransform().scale.x) * 1.5f) },
	};

	selectedMarker_->SetColor({ 0.2f, 0.2f, 0.2f });
	selectedMarker_->Init(points);
}

void AGraph::HideSelectionMarker() const
{
	if ( selectedMarker_ ) selectedMarker_->SetState(Component::SLEEP);
}

void AGraph::DrawAxis()
{
	const std::vector<dv_math::Vertex> points = {
		{-0.005f / std::abs(GetTransform().scale.x), 1.0f},
		{0.0f, 1.0f},
		{0.0f, 0.0f},
		{-0.005f / std::abs(GetTransform().scale.x), 0.0f},
		{0.0f, 0.0f},
		{0.0f, -0.005f / std::abs(GetTransform().scale.y)},
		{0.0f, 0.0f},
		{1.0f, 0.0f},
		{1.0f, -0.005f / std::abs(GetTransform().scale.y)}
	};

	graphAxis_->SetColor(axisColour_);
	graphAxis_->Init(points);
}

void AGraph::DrawGrid()
{
	float hGridSpacing = 0.25f;
	float vGridSpacing = hGridSpacing * (GetTransform().scale.x / GetTransform().scale.y);
	if (!graphPlots_.empty())
	{
		hGridSpacing = 1.0f / max(static_cast<float>(std::floor(range_.y - range_.x)) / 1000.0f, 2.0f);
		vGridSpacing = 1.0f / min(max(static_cast<float>(std::floor(maxValue_ - minValue_)) / 100.0f, 2.0f), 10.0f);
	}

	std::vector<dv_math::Vertex> gridPoints = {
		{0.0f, 1.0f}, {1.0f, 1.0f}, {1.0f, 0.0f}
	};

	for (float horizontal = 1.0f; horizontal > 0.0f - (hGridSpacing / 2.0f); horizontal -= hGridSpacing)
	{
		const float point = std::clamp(horizontal, 0.0f, 1.0f);
		const dv_math::Vertex last = gridPoints.back();
		gridPoints.push_back({point, last.position.y});
		gridPoints.push_back({point, std::round(std::abs(last.position.y - 1.0f))});
	}

	dv_math::Vertex last = gridPoints.back();
	gridPoints.push_back({last.position.x, last.position.y});

	for (float vert = 0.0f; vert < 1.0f + (vGridSpacing / 2.0f); vert += vGridSpacing)
	{
		const float point = std::clamp(vert, 0.0f, 1.0f);
		last = gridPoints.back();
		gridPoints.push_back({last.position.x, point});
		gridPoints.push_back({std::round(std::abs(last.position.x - 1.0f)), point});
	}

	graphGrid_->SetOrder(-2);
	graphGrid_->SetColor(gridColour_);
	graphGrid_->Init(gridPoints);
}

void AGraph::RemovePlot(const int index)
{
	if ( index >= graphPlots_.size() ) return;

	toPlot_.erase(toPlot_.begin() + index);
	
	graphPlots_[index]->Shutdown();
	delete graphPlots_[index];
	graphPlots_.erase(graphPlots_.begin() + index);
}

void AGraph::RemovePlot(const std::string& name)
{
	int index = 0;
	for (const auto plot : graphPlots_)
	{
		if (plot->GetName() == name)
		{
			plot->Shutdown();
			delete plot;
			toPlot_.erase(toPlot_.begin() + index);
			graphPlots_.erase(graphPlots_.begin() + index);
		}
		index++;
	}

}

void AGraph::AddPlot(const std::string& stockName, const std::string& yPlot, const dv_math::Vec3<float>& colour, const GraphType type)
{
	toPlot_.emplace_back(PlotInfo{ stockName, {}, {}, LINE });
	DataTable data = LoadData(stockName);

	std::vector<uint64_t> dates = data.datesSinceEpoch;

	auto start = std::ranges::find(dates, range_.x);
	auto end = std::ranges::find(dates, range_.y) + 1;
	if ( range_.y == 0 ) end = dates.end() - 1;

	if (start == dates.end())
	{
		start = dates.begin();
	}

	if (end == dates.end())
	{
		end = dates.end() - 1;
	}

	const int startInd = static_cast<int>(start - dates.begin());
	const int endInd = static_cast<int>(end - dates.begin());

	// const std::vector<int> xVals(dates.begin() + startInd, dates.begin() + endInd + 1);
	const std::vector<int> xVals(dates.begin(), dates.end());

	// const std::vector<float> yVals(data.values[yPlot].begin() + startInd, data.values[yPlot].begin() + endInd + 1);
	const std::vector<float> yVals(data.values[yPlot].begin(), data.values[yPlot].end());

	for (auto& val : yVals)
	{
		maxValue_ = max(maxValue_, val);
	}

	// graphPlots_.emplace_back(new GraphPlot(this, stockName, xVals, yVals, colour));
	graphPlots_.emplace_back(new GraphPlot(this, stockName, xVals, data, yPlot, colour));
	graphPlots_.back()->SetGraphType(type);

	if ( isEnabled_ )
	{
		graphPlots_.back()->SetState(Component::ACTIVE);
	}
	else
	{
		graphPlots_.back()->SetState(Component::SLEEP);
	}
}

void AGraph::AddPlot(const std::string& name, const std::vector<int>& xValues, DataTable& yValues, const std::string& plotHeader, const dv_math::Vec3<float> colour,
                     const GraphType type)
{
	plotHeader_ = plotHeader;
	toPlot_.emplace_back(PlotInfo{ name, {}, {}, LINE });

	graphPlots_.emplace_back(new GraphPlot(this, name, xValues, yValues, plotHeader, colour));
	graphPlots_.back()->SetGraphType(type);
	if ( isEnabled_ )
	{
		graphPlots_.back()->SetState(Component::ACTIVE);
	} else
	{
		graphPlots_.back()->SetState(Component::SLEEP);
	}
	maxValue_ = max(maxValue_, *std::ranges::max_element(graphPlots_.back()->GetYValues().values[plotHeader]));
}

void AGraph::Draw()
{
	maxValue_ = 0;
	minValue_ = INT_MAX;

	for (const auto& plot : graphPlots_)
	{
		if (plot->GetXValues().empty()) continue;
		auto plotValues = plot->GetYValues().values[plot->GetPlotHeader()];
		auto xValues = plot->GetXValues();

		auto findMinX = std::ranges::find(xValues, range_.x);
		if (findMinX == xValues.end())
		{
			findMinX = xValues.begin();
		}

		auto findMaxX = std::ranges::find(xValues, range_.y);
		if ( findMaxX == xValues.end() )
		{
			findMaxX = xValues.begin() + plot->GetToShow();
		}

		maxValue_ = max(
			maxValue_,
			*std::max_element(plotValues.begin() + (findMinX - xValues.begin()), plotValues.
				begin() + (findMaxX + 1 - xValues.begin())));

		if (plot->GetGraphType() == CANDLE)
		{
			// Make sure were getting minimum of lows not minimum of highs!
			plotValues = plot->GetYValues().values["Low"];
		}

		minValue_ = min(
			minValue_,
			*std::min_element(
				plotValues.begin() + (findMinX - xValues.begin()), plotValues.begin() + min(plot->GetToShow() ,(findMaxX + 1 - xValues.begin()))
			)
		);
	}
	DrawAxis();
	DrawGrid();
	for (auto plot = graphPlots_.rbegin(); plot != graphPlots_.rend(); ++plot)
	{
		if ( (*plot)->GetXValues().empty() ) continue;
		// plot->SetGraphType(graphType_);
		(*plot)->Draw((*plot)->GetName(), minValue_, maxValue_, static_cast<float>(range_.x), static_cast<float>(range_.y));
	}
}

void AGraph::RedrawPlots() const
{
	for (const auto& plot : graphPlots_)
	{
		plot->ResetPlot();
	}
}

void AGraph::SetRange(const dv_math::Vec2<int> range)
{
	range_ = range;
}

void AGraph::SetRange(const std::string& start, const std::string& end)
{
	const std::string dateTimeFormat = "%Y-%m-%d";

	std::chrono::year_month_day startDate{};
	std::istringstream startDateStream(start);

	std::chrono::year_month_day endDate{};
	std::istringstream endDateStream(end);

	startDateStream >> parse(dateTimeFormat, startDate);
	endDateStream >> parse(dateTimeFormat, endDate);

	range_ = {
		(std::chrono::sys_days{startDate}.time_since_epoch().count()),
		(std::chrono::sys_days{endDate}.time_since_epoch().count())
	};
}

GraphPlot* AGraph::GetPlot(const std::string& plotName) const
{
	for (const auto plot : graphPlots_)
	{
		if (plot->GetName() == plotName)
		{
			return plot;
		}
	}
	return nullptr;
}

void AGraph::Disable()
{
	for (const auto plot : graphPlots_)
	{
		plot->SetState(Component::SLEEP);
	}
	graphAxis_->SetState(Component::SLEEP);
	graphGrid_->SetState(Component::SLEEP);
	if ( selectedMarker_ ) selectedMarker_->SetState(Component::SLEEP);
	isEnabled_ = false;
}

void AGraph::Enable()
{
	for (const auto plot : graphPlots_ )
	{
		plot->SetState(Component::ACTIVE);
	}
	graphAxis_->SetState(Component::ACTIVE);
	graphGrid_->SetState(Component::ACTIVE);
	if (selectedMarker_) selectedMarker_->SetState(Component::ACTIVE);
	isEnabled_ = true;
}
