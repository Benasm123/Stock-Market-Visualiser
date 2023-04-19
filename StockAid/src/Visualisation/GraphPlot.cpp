#include "GraphPlot.h"

#include "Core/Application.h"
#include "Core/Components/CBarChart.h"
#include "Core/Components/CLineChart.h"
#include "Core/Components/BCandleChart.h"

GraphPlot::GraphPlot(Actor* owner, const std::string& name, const std::vector<int>& xValues, const DataTable& yValues, const std::string& feature, const dv_math::Vec3<float> colour)
	: Component(owner)
	, name_(name)
	, plotHeader_(feature)
	, colour_(colour)
	, plot_(nullptr)
{
	UpdateValues(xValues, yValues);
}

GraphPlot::~GraphPlot()
{
}

void GraphPlot::Shutdown()
{
	delete plot_;
}

void GraphPlot::UpdateValues(const std::vector<int>& xValues, const DataTable& yValues, const int toShow)
{
	xValues_ = xValues;
	yValues_ = yValues;
	toShow_ = toShow;
	if (toShow_ == -1)
	{
		toShow_ = xValues.size();
		offset_ = 0;
	}
}

void GraphPlot::Draw(const std::string& name, const float minY, const float maxY, const float minX, const float maxX)
{
	if (resetPlotB_)
	{
		owner_->GetApplication()->GetRenderer().GetVulkanContext()->GetLogicalDevice().waitIdle();
		resetPlotB_ = false;
		delete plot_;

		if (graphType_ == LINE)
		{
			plot_ = new LineComponent(owner_, name);
		}
		else if (graphType_ == BAR )
		{
			plot_ = new BarComponent(owner_, name);
		}
		else
		{
			plot_ = new BCandleChart(owner_, name);
		}
	}

	auto firstValIndex = std::ranges::find(xValues_, minX);

	if (firstValIndex == xValues_.end())
	{
		firstValIndex = xValues_.begin();
	}

	auto lastValueIter = std::ranges::find(xValues_, maxX) + 1;
	if (maxX == 0)
	{
		lastValueIter = xValues_.end();
	}

	const int offsetTemp = max(offset_, (int)(firstValIndex - xValues_.begin()));
	int toShowTemp = min(toShow_, (lastValueIter) - xValues_.begin()) - offsetTemp;
	toShowTemp = min(toShowTemp, xValues_.size());

	plot_->Init(xValues_, yValues_, plotHeader_, minX, maxX, minY, maxY);
	plot_->SetNumberOfPoints(toShowTemp);
	plot_->SetStartPoints(offsetTemp);
	plot_->SetColor(glm::vec3{colour_.x, colour_.y, colour_.z});
}

void GraphPlot::Update(const float deltaTime)
{
	Component::Update(deltaTime);
	plot_->SetState(state_);
}

dv_math::Vertex GraphPlot::GetLocationOf(const int selected) const
{
	return plot_ ? plot_->GetPoint(selected) : dv_math::Vertex{ { 0, 0 } };
}
