#pragma once
#include "GraphPlot.h"
#include "pcHeader.h"
#include "Core/Actor.h"
#include "Core/DataUtilities.h"
#include "Core/Components/CLineChart.h"

struct PlotInfo
{
	std::string stockName;
	std::string y_plot;
	PMATH::vec3<float> colour;
	GRAPH_TYPE type;
};

class AGraph : public Actor
{
public:
	AGraph(Application* app, std::string x_plot="Date");
	~AGraph() override;

	void AddPlot(const std::string& stockName, const std::string& yPlot, const PMATH::vec3<float>& colour, GRAPH_TYPE type);
	void AddPlot(const std::vector<int>& xValues, const std::vector<float>& yValues, PMATH::vec3<float> colour, GRAPH_TYPE type);

	void Draw();

	void SetRange(PMATH::vec2<int> range);
	void SetRange(const std::string& start, std::string end);

private:
	LineComponent graphAxis_;

	std::vector<PlotInfo> toPlot_;
	std::string xPlot_;

	std::vector<GraphPlot> graphPlots_;
	PMATH::vec2<int> range_;
};

