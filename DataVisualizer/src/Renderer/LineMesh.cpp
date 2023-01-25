#include "pcHeader.h"
#include "LineMesh.h"

#include "renderer.h"

bool line_mesh::init(renderer* renderer, const std::vector<PMATH::vertex>& points)
{
	renderer_ = renderer;

	line_points_ = points;
	
	vertex_buffer_ = renderer_->create_vertex_buffer(line_points_);
	vertex_memory_ = renderer_->bind_vertex_buffer_memory(vertex_buffer_, line_points_);

	return true;
}

void line_mesh::shutdown()
{
	renderer_->free_memory(vertex_memory_);
	renderer_->destroy_buffer(vertex_buffer_);
}