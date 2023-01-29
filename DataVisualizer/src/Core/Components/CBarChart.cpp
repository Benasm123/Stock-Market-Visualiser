#include "pcHeader.h"
#include "CBarChart.h"
#include "Core/Application.h"

BarComponent::BarComponent(Actor* actor)
	: LineComponent(actor)
{
}

void BarComponent::Init(const std::vector<int>& xValues, const std::vector<float>& yValues, const float minX, const float maxX, const float minY, const float maxY)
{
	//TODO::Think about adding spacing between bars as an option. might look better for when there's less bars.
	const float spacing = 1.0f / static_cast<float>(xValues.size()) * 0.0f;
	const float barWidth = 1.0f / static_cast<float>(xValues.size()) - (spacing);
	
	for ( uint32_t index = 0; index < xValues.size(); index++ )
	{
		values_.push_back({
			(static_cast<float>((static_cast<float>(xValues[index]) - minX) / (maxX - minX)) * (barWidth + spacing)) - (barWidth + spacing),
			static_cast<float>((yValues[index] - minY) / (maxY - minY))
		});
	}
	
	// Calculate how many points the bars need, with spacing need an extra to split the bars.
	int verticesPerBar = 3;
	if (spacing > 0.01f)
	{
		verticesPerBar = 4;
	}

	points_.clear();
	points_.reserve((values_.size() * verticesPerBar) + 1);
	points_.push_back({ 0.0f, 0.0f});
	
	float lastPos = 0.0f;
	for (const auto& [position] : values_ )
	{
		const auto [x, y] = position;
		
		// Top Left point of bar
		points_.push_back({ lastPos, y});
		lastPos += barWidth;
		// Top Right point of bar
		points_.push_back({ lastPos, y });
		// Bottom Right point of bar
		points_.push_back({ lastPos, 0.0f });

		// If spacing is too small, no point adding the extra vertex as it will increase memory needed.
		if (spacing > 0.01f)
		{
			lastPos += spacing;
			// Start of next bar with appropriate spacing.
			points_.push_back({ lastPos, 0.0f });
		}
	}
	
	vertexBuffer_ = app_->GetRenderer().CreateVertexBuffer(points_);
	vertexBufferMemory_ = app_->GetRenderer().BindVertexBufferMemory(vertexBuffer_, points_);

	initialised_ = true;
}
