#pragma once
#include "plot.h"
#include "Core/Actor.h"
#include "Core/Components/CLineChart.h"
#include "Core/Components/CBarChart.h"

class graph : public Actor
{
public:
	enum GraphType
	{
		Line,
		Bar
	};

	graph(Application* app, const std::vector<plot>& plots, GraphType type);
	~graph() override;

	void UpdateActor(float delta_time) override;

	void add_plot(const plot& plot, glm::vec3 color={ 0.86f, 0.43f, 0.1f });

	void show();

	[[nodiscard]] std::vector<float> get_values() const { return plots_[0].get_y_values(); }
	[[nodiscard]] std::vector<plot> get_plots() const { return plots_; }

private:
	// Need to store a vector of plots.
	std::vector<plot> plots_;
	std::vector<LineComponent*> plot_lines_;

	LineComponent graph_border_;

	GraphType type{};

	float max_x_{};
	float min_x_{};
	float max_y_{};
	float min_y_{};
};

