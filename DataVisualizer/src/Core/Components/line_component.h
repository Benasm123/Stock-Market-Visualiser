#pragma once
#include "pcHeader.h"
#include "Core/Component.h"

class line_component : public component
{
public:
	line_component(class actor* owner);
	~line_component() override;

	void init(std::vector<vertex> points);
	void init(const char* csv_file_name, const std::string& value_title, double min_val = 0.0, double max_val=0.0);
	void init(const std::vector<float>& x_values, const std::vector<float>& y_values, float min_x, float max_x, float min_y, float max_y);

	void update(float delta_time) override;

public:
	std::vector<vertex>& get_points() { return points_; }
	vk::Buffer& get_vertex_buffer() { return vertex_buffer_; }
	glm::vec3& get_color() { return color_; }

	void set_color(const glm::vec3 color) { color_ = color; }

private:
	std::vector<vertex> points_;

	vk::Buffer vertex_buffer_;
	vk::DeviceMemory vertex_buffer_memory_;

	glm::vec3 color_{1.0f};
};
