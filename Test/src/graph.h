#pragma once
#include "plot.h"
#include "Core/Actor.h"
#include "Core/Components/line_component.h"

class graph : public actor
{
public:
	graph(application* app, const std::vector<plot>& plots);
	~graph() override;

	void update_actor(float delta_time) override;

	void add_plot(const plot& plot, glm::vec3 color={1.0f, 1.0f, 1.0f});

	void show();

private:
	// Need to store a vector of plots.
	std::vector<plot> plots_;
	std::vector<line_component*> plot_lines_;

	line_component graph_border_;

	float max_x_{};
	float min_x_{};
	float max_y_{};
	float min_y_{};
};

