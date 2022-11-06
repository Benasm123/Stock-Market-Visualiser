#pragma once
#include "pcHeader.h"

class line_mesh
{
public:
	line_mesh() = default;
	~line_mesh() = default;

	bool init(class renderer* renderer, const std::vector<vertex>& points);
	void shutdown();

private:

public:
	vk::Buffer& get_vertex_buffer() { return vertex_buffer_; }
	std::vector<vertex>& get_line_points() { return line_points_; }

private:
	class renderer* renderer_{};

	std::vector<vertex> line_points_;
	vk::Buffer vertex_buffer_;
	vk::DeviceMemory vertex_memory_;
};

