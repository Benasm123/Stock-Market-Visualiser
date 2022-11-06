#include "test_actor.h"

test_actor::test_actor(application* app, const std::vector<vertex>& points)
	: actor(app)
{
	lines_.push_back(new line_component(this));
	lines_[0]->init(points);
	lines_[0]->set_color({ 0, 1, 1 });
}

test_actor::test_actor(application* app, const std::vector<const char*>& data, const std::vector<const char*>& titles, const std::vector<glm::vec3>
                       & colors)
	: actor(app)
{
	
	for (int i = 0; i < data.size(); ++i)
	{
		lines_.push_back(new line_component(this));
		lines_[i]->init(data[i], titles[i], 0.0, 200.0);
		lines_[i]->set_color(colors[i]);
	}

}

void test_actor::update_actor(float delta_time)
{

}

void test_actor::add_plot(plot plot)
{
}
