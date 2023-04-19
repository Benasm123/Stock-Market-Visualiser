#include "pcHeader.h"
#include "BCandleChart.h"
#include "Core/Application.h"

BCandleChart::BCandleChart(Actor* actor, const std::string& name)
	: LineComponent(actor, name)
{
}

void BCandleChart::Init(const std::vector<int>& xValues, DataTable& yValuesDataTable, const std::string& plotHeader, const float minX, const
                        float maxX, const float minY, const float maxY)
{
	values_.clear();

	auto highValues = yValuesDataTable.values["High"];
	auto lowValues = yValuesDataTable.values["Low"];
	auto openValues = yValuesDataTable.values["Open"];
	auto closeValues = yValuesDataTable.values["Close"];

	std::vector<float> highValuesNorm = yValuesDataTable.values["High"];
	std::vector<float> lowValuesNorm = yValuesDataTable.values["Low"];
	std::vector<float> openValuesNorm = yValuesDataTable.values["Open"];
	std::vector<float> closeValuesNorm = yValuesDataTable.values["Close"];

	const auto start = std::ranges::find(xValues, minX);
	const auto end = std::ranges::find(xValues, maxX) + 1;
	const int elements = end - start;
	
	const float candleWidth = 1.0f / static_cast<float>(elements);

	float updatedMinY = minY;
	
	for ( uint32_t index = 0; index < xValues.size(); index++ )
	{
		const float xValue = (static_cast<float>(xValues[index]) - minX) / (maxX - minX);

		if (xValue >= 0.0f && xValue <= 1.0f)
		{
			updatedMinY = min(minY, lowValues[index]);
		}

		values_.push_back({
			xValue,
			(highValues[index] - updatedMinY) / (maxY - updatedMinY)});

		highValuesNorm[index] = (highValues[index] - updatedMinY) / (maxY - updatedMinY);
		lowValuesNorm[index] = (lowValues[index] - updatedMinY) / (maxY - updatedMinY);
		openValuesNorm[index] = (openValues[index] - updatedMinY) / (maxY - updatedMinY);
		closeValuesNorm[index] = (closeValues[index] - updatedMinY) / (maxY - updatedMinY);
	}
	
	// Calculate how many points the bars need, with spacing need an extra to split the bars.
	constexpr int verticesPerBar = 15;
	
	points_.clear();
	points_.reserve((values_.size() * verticesPerBar) + 1);
	
	int i = 0;
	float lastPos = values_[0].position.x;
	for ( const auto& [position] : values_ )
	{
		const auto [x, y] = position;

		const float leftX = lastPos + candleWidth * 0.1f;
		const float rightX = leftX + (candleWidth * 0.8f);
		const float middleX = leftX + ((rightX - leftX) / 2.0f);

		// Top bar of candle
		points_.push_back({ leftX, highValuesNorm[i] });
		points_.push_back({ rightX, highValuesNorm[i]});
		points_.push_back({ middleX, highValuesNorm[i] });

		// Middle Bar of Candle
		points_.push_back({ middleX, max(openValuesNorm[i], closeValuesNorm[i])});
		points_.push_back({ leftX, max(openValuesNorm[i], closeValuesNorm[i]) });
		points_.push_back({ leftX, min(openValuesNorm[i], closeValuesNorm[i]) });
		points_.push_back({ rightX, min(openValuesNorm[i], closeValuesNorm[i]) });
		points_.push_back({ rightX, max(openValuesNorm[i], closeValuesNorm[i]) });
		points_.push_back({ leftX, max(openValuesNorm[i], closeValuesNorm[i]) });
		points_.push_back({ leftX, min(openValuesNorm[i], closeValuesNorm[i]) });
		points_.push_back({ middleX, min(openValuesNorm[i], closeValuesNorm[i]) });

		//Bottom bar of candle
		points_.push_back({ middleX, lowValuesNorm[i] });
		points_.push_back({ leftX, lowValuesNorm[i] });
		points_.push_back({ rightX, lowValuesNorm[i] });

		points_.push_back({ -1, 0.5 });

		lastPos = (static_cast<float>(i - (start - xValues.begin())) + 1.0f) * candleWidth;
		i++;
	}
	
	numberOfPoints_ = static_cast<int>(points_.size());
	
	if ( initialised_ )
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
