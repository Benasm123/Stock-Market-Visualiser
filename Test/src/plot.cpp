#include "plot.h"

#include <algorithm>
#include <utility>

plot::plot(std::vector<float> x_values, std::vector<float> y_values, glm::vec3 color)
{
	x_values_ = std::move(x_values);
	y_values_ = std::move(y_values);
	
	max_x_ = *std::ranges::max_element(x_values_);
	min_x_ = *std::ranges::min_element(x_values_);
	max_y_ = *std::ranges::max_element(y_values_);
	min_y_ = *std::ranges::min_element(y_values_);

	color_ = color;
}
