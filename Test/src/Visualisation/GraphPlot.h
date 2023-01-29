#pragma once
#include "pcHeader.h"
#include "Core/Component.h"

enum GRAPH_TYPE
{
	BAR,
	LINE
};

class GraphPlot : public Component
{
public:
	GraphPlot(class Actor* owner, std::vector<int> xValues, std::vector<float> yValues, const PMATH::vec3<float> colour);

	void Draw(float minY, float maxY, float minX, float maxX) const;

	void setGraphType(const GRAPH_TYPE type) { graphType_ = type; }

private:
	std::vector<int> xValues_;
	std::vector<float> yValues_;
	PMATH::vec3<float> colour_;

	GRAPH_TYPE graphType_ = LINE;
};

