#pragma once
#include "GraphPlot.h"
#include "pcHeader.h"
#include "Core/Actor.h"
#include "../data_manager.h"
#include "Core/Components/line_component.h"

struct PlotInfo
{
	std::string stockName;
	std::string y_plot;
	PMATH::vec3<float> colour;
	GraphType type;
};

class AGraph : public actor
{
public:
	AGraph(application* app, std::string x_plot="Date");

	void AddPlot(const std::string stockName, const std::string y_plot, const PMATH::vec3<float> colour, GraphType type);
	void AddPlot(std::vector<int> xValues, std::vector<float> yValues, PMATH::vec3<float> colour, GraphType type);

	void Draw();

	void SetRange(PMATH::vec2<int> range);
	void SetRange(const std::string& start, std::string end);

private:
	line_component graphAxis_;

	std::vector<PlotInfo> toPlot_;
	std::string x_plot_;

	std::vector<GraphPlot> graphPlots_;
	PMATH::vec2<int> range_;
};

