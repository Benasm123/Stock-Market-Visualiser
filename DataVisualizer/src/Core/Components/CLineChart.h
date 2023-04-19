#pragma once
#include "pcHeader.h"
#include "Core/Component.h"
#include "Core/DataUtilities.h"

class LineComponent : public Component
{
public:
	LineComponent(Actor* actor, std::string name);
	~LineComponent() override;

public:
	void Init(const std::vector<dv_math::Vertex>& points);
	virtual void Init(const std::vector<int>& xValues, DataTable& yValuesDataTable, const std::string& plotHeader, const float minX, const
	                  float maxX, const float minY, const float maxY);

	void Update(float deltaTime) override;

public:
	virtual dv_math::Vertex GetPoint(int index) { return points_[index]; }
	vk::Buffer& GetVertexBuffer() { return vertexBuffer_; }
	glm::vec3& GetColor() { return color_; }
	[[nodiscard]] int GetNumberOfPoints() const { return numberOfPoints_; }
	[[nodiscard]] int GetStartPoints() const { return startPoint_; }
	virtual void SetNumberOfPoints(const int number) { numberOfPoints_ = number; }
	virtual void SetStartPoints(const int number) { startPoint_ = number; }

	void SetColor(const glm::vec3 color) { color_ = color; }

	void SetOrder(const int order) { order_ = order; }
	[[nodiscard]] int GetOrder() const { return order_; }

	std::string name_{};
protected:
	std::vector<dv_math::Vertex> points_{};

	vk::Buffer vertexBuffer_{};
	vk::DeviceMemory vertexBufferMemory_{};
	void* dataPointer_{};

	glm::vec3 color_{ 1.0f };

	bool initialised_{ false };

	Application* app_{};

	int startPoint_{};
	int numberOfPoints_{};

	int order_{1};
};
