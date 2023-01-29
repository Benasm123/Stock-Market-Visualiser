#include "pcHeader.h"
#include "CLineChart.h"
#include "Core/Actor.h"
#include "Core/Application.h"
#include <Core/DataUtilities.h>


LineComponent::LineComponent(Actor* actor)
	: Component(actor)
{
	app_ = actor->GetApplication();
}

LineComponent::~LineComponent()
{
	app_->GetRenderer().DestroyBuffer(vertexBuffer_);
	app_->GetRenderer().FreeMemory(vertexBufferMemory_);

	vertexBuffer_ = VK_NULL_HANDLE;
	vertexBufferMemory_ = VK_NULL_HANDLE;
}

void LineComponent::Init(const std::vector<PMATH::vertex>& points)
{
	points_ = points;

	vertexBuffer_ = app_->GetRenderer().CreateVertexBuffer(points_);
	vertexBufferMemory_ = app_->GetRenderer().BindVertexBufferMemory(vertexBuffer_, points_);

	initialised_ = true;
}

void LineComponent::Init(const char* csvFileName, const std::string& valueTitle, float minVal, float maxVal)
{
	const std::vector<float> values = LoadData(csvFileName, valueTitle);

	if (minVal == 0.0f && maxVal == 0.0f)
	{
		minVal = FLT_MAX;
		for (const auto& value : values)
		{
			maxVal = max(value, maxVal);
			minVal = min(value, minVal);
		}
	}

	maxVal = maxVal - minVal;

	uint32_t index = 0;
	for (const auto value : values)
	{
		points_.push_back({ 
			static_cast<float>(index) / static_cast<float>(values.size()),
			(value - minVal) / maxVal
		});
		index++;
	}

	vertexBuffer_ = app_->GetRenderer().CreateVertexBuffer(points_);
	vertexBufferMemory_ = app_->GetRenderer().BindVertexBufferMemory(vertexBuffer_, points_);

	initialised_ = true;
}

void LineComponent::Init(const std::vector<int>& xValues, const std::vector<float>& yValues, const float minX, const float maxX, const float minY, const float maxY)
{
	for (uint32_t index = 0; index < xValues.size(); index++)
	{
		points_.push_back({
			(static_cast<float>(xValues[index]) - minX) / (maxX - minX),
			(yValues[index] - minY) / (maxY - minY)
		});
	}

	vertexBuffer_ = app_->GetRenderer().CreateVertexBuffer(points_);
	vertexBufferMemory_ = app_->GetRenderer().BindVertexBufferMemory(vertexBuffer_, points_);

	initialised_ = true;
}

void LineComponent::Update(const float deltaTime)
{
	if ( initialised_ && !points_.empty())
		app_->GetRenderer().Draw(this);
}
