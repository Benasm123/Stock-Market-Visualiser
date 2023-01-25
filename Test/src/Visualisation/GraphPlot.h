#pragma once
#include "pcHeader.h"
#include "Core/Component.h"

enum GraphType
{
	BAR,
	LINE
};

class GraphPlot : public component
{
public:
	GraphPlot(class actor* owner, std::vector<int> xValues, std::vector<float> yValues, const PMATH::vec3<float> colour);

	void Draw(float minY, float maxY, float minX, float maxX) const;

	void setGraphType(const GraphType type) { graphType_ = type; }

private:
	std::vector<int> xValues_;
	std::vector<float> yValues_;
	PMATH::vec3<float> colour_;

	GraphType graphType_ = LINE;
};

