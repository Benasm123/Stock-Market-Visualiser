#include "graph.h"

graph::graph(application* app, const std::vector<plot>& plots)
	: actor(app)
	, graph_border_(this)
{
	const std::vector<vertex> points = {
		{0.0f, 1.0f},
		{0.0f, 0.0f},
		{1.0f, 0.0f},
	};
	graph_border_.init(points);

	for (auto& plot : plots)
	{
		add_plot(plot);
	}
}

graph::~graph()
{

}

void graph::update_actor(float delta_time)
{
}

void graph::add_plot(const plot& plot, glm::vec3 color)
{
	if (plots_.empty())
	{
		max_x_ = plot.get_max_x();
		min_x_ = plot.get_min_x();
		max_y_ = plot.get_max_y();
		min_y_ = plot.get_min_y();
	}
	else
	{
		max_x_ = max(max_x_, plot.get_max_x());
		min_x_ = min(min_x_, plot.get_min_x());
		max_y_ = max(max_y_, plot.get_max_y());
		min_y_ = min(min_y_, plot.get_min_y());
	}

	plots_.push_back(plot);
}

void graph::show()
{
	for (auto& plot : plots_)
	{
		plot_lines_.push_back(new line_component(this));
		plot_lines_.back()->init(plot.get_x_values(), plot.get_y_values(), min_x_, max_x_, min_y_, max_y_);
		plot_lines_.back()->set_color(plot.get_color());
	}
}
