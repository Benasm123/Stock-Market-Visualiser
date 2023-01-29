#pragma once
#include "plot.h"
#include "Core/Actor.h"
#include "Core/Components/CLineChart.h"

class test_actor : public Actor
{
public:
	test_actor(Application* app, const std::vector<PMATH::vertex>& points);
	test_actor(Application* app, const std::vector<const char*>& data, const std::vector<const char*>& titles, const std::vector<glm::vec3>
	           & colors);
	~test_actor() override = default;

	void UpdateActor(float delta_time) override;

	void add_plot(plot plot);

private:
	// Need pointers as this is variable size
	std::vector<LineComponent*> lines_;
};
