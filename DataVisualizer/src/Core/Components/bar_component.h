#pragma once
#include "Core/Component.h"
#include "Core/Components/line_component.h"

class bar_component : public line_component
{
public:
	bar_component(class actor* owner);

	void init(const std::vector<int>& x_values, const std::vector<float>& y_values, float min_x, float max_x, float min_y, float max_y) override;

private:
	std::vector<PMATH::vertex> values_;
};

