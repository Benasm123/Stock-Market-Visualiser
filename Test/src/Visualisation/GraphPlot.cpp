#include "GraphPlot.h"

#include "Core/Components/bar_component.h"
#include "Core/Components/line_component.h"

GraphPlot::GraphPlot(actor* owner, std::vector<int> xValues, std::vector<float> yValues, const PMATH::vec3<float> colour)
	: component(owner)
	, xValues_(xValues)
	, yValues_(yValues)
	, colour_(colour)
{
}

void GraphPlot::Draw(const float minY, const float maxY, const float minX, const float maxX) const
{
	line_component* line = nullptr;

	if (graphType_ == LINE)
	{
		line = new line_component(owner_);
	} else
	{
		line = new bar_component(owner_);
	}

	line->init(xValues_, yValues_, minX, maxX, minY, maxY);
	line->set_color(glm::vec3{ colour_.x, colour_.y, colour_.z });
}
