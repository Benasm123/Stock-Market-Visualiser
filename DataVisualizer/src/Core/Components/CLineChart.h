#pragma once
#include "pcHeader.h"
#include "Core/Component.h"

class LineComponent : public Component
{
public:
	LineComponent(Actor* actor);
	~LineComponent() override;

public:
	void Init(const std::vector<PMATH::vertex>& points);
	void Init(const char* csvFileName, const std::string& valueTitle, float minVal = 0.0f, float maxVal = 0.0f);
	virtual void Init(const std::vector<int>& xValues, const std::vector<float>& yValues, float minX, float maxX, float minY, float maxY);

	void Update(float deltaTime) override;

public:
	std::vector<PMATH::vertex>& GetPoints() { return points_; }
	vk::Buffer& GetVertexBuffer() { return vertexBuffer_; }
	glm::vec3& GetColor() { return color_; }

	void SetColor(const glm::vec3 color) { color_ = color; }

protected:
	std::vector<PMATH::vertex> points_{};

	vk::Buffer vertexBuffer_{};
	vk::DeviceMemory vertexBufferMemory_{};

	glm::vec3 color_{1.0f};

	bool initialised_{false};

	Application* app_{};
};
