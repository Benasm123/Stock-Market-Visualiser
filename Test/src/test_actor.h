#pragma once
#include "plot.h"
#include "Core/Actor.h"
#include "Core/Components/line_component.h"

class test_actor : public actor
{
public:
	test_actor(application* app, const std::vector<vertex>& points);
	test_actor(application* app, const std::vector<const char*>& data, const std::vector<const char*>& titles, const std::vector<glm::vec3>
	           & colors);
	~test_actor() override = default;

	void update_actor(float delta_time) override;

	void add_plot(plot plot);

private:
	// Need pointers as this is variable size
	std::vector<line_component*> lines_;
};
