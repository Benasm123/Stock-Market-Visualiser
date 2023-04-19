#pragma once
#include "GraphPlot.h"
#include "pcHeader.h"
#include "Core/Actor.h"
#include "Core/DataUtilities.h"

class LineComponent;

struct PlotInfo
{
	std::string stockName;
	std::string yPlot;
	dv_math::Vec3<float> colour;
	GraphType type;
};

class AGraph : public Actor
{
public:
	AGraph(Application* app, std::string xPlot = "Date");
	void ShowSelectionMarker(int selected);
	void HideSelectionMarker() const;
	void DrawAxis();
	void DrawGrid();
	void RemovePlot(int index);
	void RemovePlot(const std::string& name);

	void AddPlot(const std::string& stockName, const std::string& yPlot, const dv_math::Vec3<float>& colour,
	             GraphType type);
	void AddPlot(const std::string& name, const std::vector<int>& xValues, DataTable& yValues, const std::string& plotHeader, const
	             dv_math::Vec3<float>
	             colour, const GraphType type);

	void Draw();
	void RedrawPlots() const;

	void SetRange(dv_math::Vec2<int> range);
	void SetRange(const std::string& start, const std::string& end);

	void SetGraphType(const GraphType type) { graphType_ = type; }
	[[nodiscard]] GraphType GetGraphType() const { return graphType_; }

	std::vector<GraphPlot*> GetPlots() { return graphPlots_; }

	GraphPlot* GetPlot(const std::string& plotName) const;

	[[nodiscard]] dv_math::Vec2<int> GetRange() const { return range_; }

	void SetAxisColour(const glm::vec3 newColour) { axisColour_ = newColour; }
	void SetGridColour(const glm::vec3 newColour) { gridColour_ = newColour; }

	void Disable();
	void Enable();

private:
	std::string plotHeader_;

	LineComponent* graphAxis_{};
	LineComponent* graphGrid_{};
	LineComponent* selectedMarker_{};

	std::vector<PlotInfo> toPlot_;
	std::string xPlot_;

	std::vector<GraphPlot*> graphPlots_;
	dv_math::Vec2<int> range_;

	GraphType graphType_ = LINE;

	float maxValue_{};
	float minValue_{};

	glm::vec3 axisColour_{0.0f};
	glm::vec3 gridColour_{0.9f};

	bool isEnabled_ = true;
	int selected_ = 0;
};
