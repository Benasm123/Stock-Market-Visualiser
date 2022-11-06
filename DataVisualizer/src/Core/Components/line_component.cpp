#include "pcHeader.h"
#include "line_component.h"
#include "Core/Actor.h"
#include "Core/Application.h"


line_component::line_component(class actor* owner)
	: component(owner, 100)
{
}

line_component::~line_component()
{
	owner_->get_application()->get_renderer().destroy_buffer(vertex_buffer_);
	owner_->get_application()->get_renderer().free_memory(vertex_buffer_memory_);
}

void line_component::init(std::vector<vertex> points)
{
	points_ = std::move(points);
	vertex_buffer_ = owner_->get_application()->get_renderer().create_vertex_buffer(points_);
	vertex_buffer_memory_ = owner_->get_application()->get_renderer().bind_vertex_buffer_memory(vertex_buffer_, points_);
}

void line_component::init(const char* csv_file_name, const std::string& value_title, double min_val, double max_val)
{
	std::ifstream values_file(csv_file_name);

	std::string line;

	std::vector<double> values;

	bool wanted_title_found = false;
	uint32_t wanted_index = 0;
	if (getline(values_file, line))
	{
		while (!line.empty())
		{
			const auto title_end_index = line.find(',');
			std::string title = line.substr(0, title_end_index);
			if (title == value_title)
			{
				wanted_title_found = true;
				break;
			}

			line.erase(0, title_end_index + 1);
			wanted_index++;
		}
	}
	if (!wanted_title_found)
	{
		LOG_ERROR("Can not find the value title specified: %s", value_title.c_str());
		return;
	}

	while (getline(values_file, line))
	{
		for (uint32_t i = 0; i < wanted_index; ++i)
		{
			line.erase(0, line.find(',') + 1);
		}

		values.push_back(std::stod(line.substr(0, line.find(','))));
	}

	if (min_val == 0.0 && max_val == 0.0)
	{
		min_val = DBL_MAX;
		for (auto value : values)
		{
			max_val = max(value, max_val);
			min_val = min(value, min_val);
		}
	}

	assert(max_val >= 0.001);

	max_val = max_val - min_val;

	const double multiplier_value = 1.0 / max_val;
	const double multiplier_index = 1.0 / static_cast<double>(values.size());

	uint32_t index = 0;
	for (const auto value : values)
	{
		points_.push_back({ 
			static_cast<float>(index / static_cast<double>(values.size())),
			static_cast<float>((value - min_val) / max_val)
		});
		index++;
	}

	values_file.close();

	vertex_buffer_ = owner_->get_application()->get_renderer().create_vertex_buffer(points_);
	vertex_buffer_memory_ = owner_->get_application()->get_renderer().bind_vertex_buffer_memory(vertex_buffer_, points_);
}

void line_component::init(const std::vector<float>& x_values, const std::vector<float>& y_values, float min_x, float max_x, float min_y, float max_y)
{
	for (uint32_t index = 0; index < x_values.size(); index++)
	{
		points_.push_back({
			static_cast<float>((x_values[index] - min_x) / (max_x - min_x)),
			static_cast<float>((y_values[index] - min_y) / (max_y - min_y))
		});
	}

	vertex_buffer_ = owner_->get_application()->get_renderer().create_vertex_buffer(points_);
	vertex_buffer_memory_ = owner_->get_application()->get_renderer().bind_vertex_buffer_memory(vertex_buffer_, points_);
}

void line_component::update(const float delta_time)
{
	component::update(delta_time);
	owner_->get_application()->get_renderer().draw(this);
}
