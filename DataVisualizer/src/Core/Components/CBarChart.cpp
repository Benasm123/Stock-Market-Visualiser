#include "pcHeader.h"
#include "CBarChart.h"
#include "Core/Application.h"

BarComponent::BarComponent(Actor* actor, const std::string& name)
	: LineComponent(actor, name)
{
}

void BarComponent::Init(const std::vector<int>& xValues, DataTable& yValuesDataTable, const std::string& plotHeader, const float minX, const
                        float maxX, const float minY, const float maxY)
{
	const std::vector<float> yValues = yValuesDataTable.values[plotHeader];

	values_.clear();

	const auto start = std::ranges::find(xValues, minX);
	const auto end = std::ranges::find(xValues, maxX) + 1;
	const int elements = end - start;

	const float barWidth = 1.0f / static_cast<float>(elements);
	
	for ( uint32_t index = 0; index < xValues.size(); index++ )
	{
		values_.push_back({
			(static_cast<float>(xValues[index]) - minX) / (maxX - minX),
			(yValues[index] - minY) / (maxY - minY)
		});
	}
	
	// Calculate how many points the bars need, with spacing need an extra to split the bars.
	constexpr int verticesPerBar = 3;

	points_.clear();
	points_.reserve((values_.size() * verticesPerBar) + 1);
	points_.push_back({ values_[0].position.x, 0.0f});

	int i = 0;
	float lastPos = values_[0].position.x;
	for (const auto& [position] : values_ )
	{
		const auto [x, y] = position;
		
		// Top Left point of bar
		points_.push_back({ lastPos, y});
		lastPos = (static_cast<float>(i - (start - xValues.begin())) + 1.0f) * barWidth;
		// Top Right point of bar
		points_.push_back({ lastPos, y });
		// Bottom Right point of bar
		points_.push_back({ lastPos, 0.0f });
		
		i++;
	}

	numberOfPoints_ = static_cast<int>(points_.size());

	if (initialised_)
	{
		app_->GetRenderer().WriteToVertexBufferMemory(vertexBuffer_, dataPointer_, points_);
		return;
	}

	vertexBuffer_ = app_->GetRenderer().CreateVertexBuffer(points_);
	vertexBufferMemory_ = app_->GetRenderer().BindVertexBufferMemory(vertexBuffer_, points_);
	dataPointer_ = app_->GetRenderer().MapMemory(vertexBuffer_, vertexBufferMemory_);
	app_->GetRenderer().WriteToVertexBufferMemory(vertexBuffer_, dataPointer_, points_);

	initialised_ = true;
}
