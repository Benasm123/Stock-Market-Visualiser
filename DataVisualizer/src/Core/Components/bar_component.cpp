#include "pcHeader.h"
#include "bar_component.h"
#include "Core/Application.h"

bar_component::bar_component(actor* owner)
	: line_component(owner)
{
}

void bar_component::init(const std::vector<int>& x_values, const std::vector<float>& y_values, float min_x, float max_x, float min_y, float max_y)
{
	//TODO::Think about adding spacing between bars as an option. might look better for when there's less bars.
	// const float bar_width = 1.0f / static_cast<float>(x_values.size());
	const float bar_width = 1.0f / (max_x - min_x);
	constexpr float spacing = 0.0f;
	
	for ( uint32_t index = 0; index < x_values.size(); index++ )
	{
		values_.push_back({
			(static_cast<float>((x_values[index] - min_x) / (max_x - min_x)) * (bar_width)) - bar_width,
			static_cast<float>((y_values[index] - min_y) / (max_y - min_y))
		});
	}
	
	// (size * 3) + 1 accounts for all the needed vertices for a bar graph for each data point.
	// points_.reserve((values_.size() * 3) + 1);
	points_.clear();
	points_.push_back({ 0.0f, 0.0f});
	
	//TODO::When adding gaps need too add a break in vertex list as well as this will break with triangle rendering when i move to that
	int i = 0;
	for (const auto& [position] : values_ )
	{
		const auto [x, y] = position;

		static float last_pos{ x + (bar_width * static_cast<float>(i)) };

		if (i == 0)
		{
			last_pos = 0.0f;
		}
		points_.push_back({ last_pos, y});

		last_pos = x + (bar_width * static_cast<float>(i)) + bar_width;

		points_.push_back({ last_pos, y });
		points_.push_back({ last_pos, 0.0f });
	
		i++;

	}
	
	vertex_buffer_ = owner_->get_application()->get_renderer().create_vertex_buffer(points_);
	vertex_buffer_memory_ = owner_->get_application()->get_renderer().bind_vertex_buffer_memory(vertex_buffer_, points_);

	initialised = true;
}
