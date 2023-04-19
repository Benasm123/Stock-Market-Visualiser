#pragma once
#include "Core/Component.h"
#include "Core/Components/CLineChart.h"

class BCandleChart : public LineComponent
{
public:
	BCandleChart(Actor* actor, const std::string& name);
	void Init(const std::vector<int>& xValues, DataTable& yValuesDataTable, const std::string& plotHeader, const float minX, const
	          float maxX, const float minY, const float maxY) override;
	void SetNumberOfPoints(const int number) override { numberOfPoints_ = (number * 15); }
	void SetStartPoints(const int number) override { startPoint_ = (number * 15); }
	dv_math::Vertex GetPoint(const int index) override { return{ points_[index * 15 + 2].position.x , points_[index * 15 + 2].position.y }; }

private:
	std::vector<dv_math::Vertex> values_;
};

