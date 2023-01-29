#include "test_actor.h"

test_actor::test_actor(Application* app, const std::vector<PMATH::vertex>& points)
	: Actor(app)
{
	lines_.push_back(new LineComponent(this));
	lines_[0]->Init(points);
	lines_[0]->SetColor({ 0, 1, 1 });
}

test_actor::test_actor(Application* app, const std::vector<const char*>& data, const std::vector<const char*>& titles, const std::vector<glm::vec3>
                       & colors)
	: Actor(app)
{
	
	for (int i = 0; i < static_cast<int>(data.size()); ++i)
	{
		lines_.push_back(new LineComponent(this));
		lines_[i]->Init(data[i], titles[i], 0.0, 200.0);
		lines_[i]->SetColor(colors[i]);
	}

}

void test_actor::UpdateActor(float delta_time)
{

}

void test_actor::add_plot(plot plot)
{
}
