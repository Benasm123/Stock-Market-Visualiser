#pragma once
#include "pcHeader.h"
#include "Core/Component.h"
#include "Core/DataUtilities.h"

class LineComponent;

enum GraphType
{
	BAR,
	LINE,
	CANDLE
};

class GraphPlot : public Component
{
public:
	GraphPlot(class Actor* owner, const std::string& name, const std::vector<int>& xValues, const DataTable& yValues, const std::string& feature, dv_math::Vec3<float> colour);
	~GraphPlot() override;

	void Shutdown();

	void UpdateValues(const std::vector<int>& xValues, const DataTable& yValues, int toShow = -1);

	void Draw(const std::string& name, float minY, float maxY, float minX, float maxX);
	void ResetPlot() { resetPlotB_ = true; }

	void SetGraphType(const GraphType type) { graphType_ = type; }
	GraphType GetGraphType() const { return graphType_; }

	DataTable& GetYValues() { return yValues_; }
	std::string GetPlotHeader() { return plotHeader_; }
	std::vector<int>& GetXValues() { return xValues_; }

	void SetToShow(const int num) { toShow_ = num; }
	[[nodiscard]] int GetToShow() const { return toShow_; }
	void SetOffset(const int num) { offset_ = num; }

	void Update(float deltaTime) override;

	[[nodiscard]] dv_math::Vertex GetLocationOf(int selected) const;

	std::string& GetName() { return name_; }

private:
	std::string name_;
	std::string plotHeader_;

	std::vector<int> xValues_;
	DataTable yValues_;
	int toShow_;
	int offset_;
	dv_math::Vec3<float> colour_;

	GraphType graphType_ = LINE;

	LineComponent* plot_;

	bool resetPlotB_ = true;
};
