#pragma once
#include "Core/Component.h"
#include "Core/Components/CLineChart.h"

class BarComponent : public LineComponent
{
public:
	BarComponent(class Actor* actor);

	void Init(const std::vector<int>& xValues, const std::vector<float>& yValues, float minX, float maxX, float minY, float maxY) override;

private:
	std::vector<PMATH::vertex> values_;
};

