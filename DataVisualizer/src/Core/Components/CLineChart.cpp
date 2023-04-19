#include "pcHeader.h"
#include "CLineChart.h"
#include "Core/Actor.h"
#include "Core/Application.h"
#include <Core/DataUtilities.h>


LineComponent::LineComponent(Actor* actor, std::string name)
	: Component(actor)
	, name_(std::move(name))
{
	app_ = actor->GetApplication();
}

LineComponent::~LineComponent()
{
	app_->GetRenderer().UnMapMemory(vertexBufferMemory_);
	app_->GetRenderer().FreeMemory(vertexBufferMemory_);
	app_->GetRenderer().DestroyBuffer(vertexBuffer_);

	vertexBuffer_ = VK_NULL_HANDLE;
	vertexBufferMemory_ = VK_NULL_HANDLE;
}

void LineComponent::Init(const std::vector<dv_math::Vertex>& points)
{
	points_.clear();
	points_ = points;
	numberOfPoints_ = points_.size();

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

void LineComponent::Init(const std::vector<int>& xValues, DataTable& yValuesDataTable, const std::string& plotHeader, const float minX, const float maxX, const float minY, const float maxY)
{
	const std::vector<float> yValues = yValuesDataTable.values[plotHeader];

	points_.clear();

	auto minPos = std::ranges::find(xValues, minX);

	if ( minPos == xValues.end())
	{
		minPos = xValues.begin();
	}

	const auto minIndex = minPos - std::begin(xValues);

	const auto maxIndex = (std::ranges::find(xValues, maxX)) - std::begin(xValues);

	for ( uint32_t index = 0; index < xValues.size(); index++ )
	{
		points_.push_back({
			(static_cast<float>(index - minIndex) / static_cast<float>(maxIndex - minIndex)),
			(yValues[index] - minY) / (maxY - minY)
		});
	}

	numberOfPoints_ = points_.size();

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

void LineComponent::Update(const float deltaTime)
{
	if ( initialised_ && !points_.empty() && state_ == ACTIVE)
		app_->GetRenderer().Draw(this);
}
