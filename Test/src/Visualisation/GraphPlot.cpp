#include "GraphPlot.h"

#include "Core/Components/CBarChart.h"
#include "Core/Components/CLineChart.h"

GraphPlot::GraphPlot(Actor* owner, std::vector<int> xValues, std::vector<float> yValues, const PMATH::vec3<float> colour)
	: Component(owner)
	, xValues_(xValues)
	, yValues_(yValues)
	, colour_(colour)
{
}

void GraphPlot::Draw(const float minY, const float maxY, const float minX, const float maxX) const
{
	LineComponent* line = nullptr;

	if (graphType_ == LINE)
	{
		line = new LineComponent(owner_);
	} else
	{
		line = new BarComponent(owner_);
	}

	line->Init(xValues_, yValues_, minX, maxX, minY, maxY);
	line->SetColor(glm::vec3{ colour_.x, colour_.y, colour_.z });
}
