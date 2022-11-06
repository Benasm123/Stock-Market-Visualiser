#pragma once
#include <vec3.hpp>
#include <vector>

class plot
{
public:
	plot(std::vector<float> x_values, std::vector<float> y_values, glm::vec3 color={1.0f, 1.0f, 1.0f});

	[[nodiscard]] float get_max_x() const { return max_x_; }
	[[nodiscard]] float get_min_x() const { return min_x_; }
	[[nodiscard]] float get_max_y() const { return max_y_; }
	[[nodiscard]] float get_min_y() const { return min_y_; }
	[[nodiscard]] std::vector<float> get_x_values() const { return x_values_; }
	[[nodiscard]] std::vector<float> get_y_values() const { return y_values_; }
	[[nodiscard]] glm::vec3 get_color() const { return color_; }

private:
	float max_x_{};
	float min_x_{};
	float max_y_{};
	float min_y_{};

	std::vector<float> x_values_;
	std::vector<float> y_values_;

	glm::vec3 color_;
};

