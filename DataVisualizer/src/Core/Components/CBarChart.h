#pragma once
#include "Core/Component.h"
#include "Core/Components/CLineChart.h"

class BarComponent : public LineComponent
{
public:
	BarComponent(Actor* actor, const std::string& name);
	void Init(const std::vector<int>& xValues, DataTable& yValuesDataTable, const std::string& plotHeader, const float minX, const
	          float maxX, const float minY, const float maxY) override;
	void SetNumberOfPoints(const int number) override { numberOfPoints_ = (number * 3) + 1; }
	void SetStartPoints(const int number) override { startPoint_ = (number * 3); }
	dv_math::Vertex GetPoint(const int index) override { return{ (points_[index * 3 + 1].position.x + points_[index * 3 + 2].position.x) / 2.0f, points_[index * 3 + 1].position.y}; }

private:
	std::vector<dv_math::Vertex> values_;
};

