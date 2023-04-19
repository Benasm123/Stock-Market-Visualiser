#pragma once
#include "pcHeader.h"

class Component;
class Application;

class Actor
{
public:
	enum STATE
	{
		ACTIVE,
		PAUSED,
		DEAD
	};

	explicit Actor(Application* application);
	virtual ~Actor();

	void Update(float deltaTime);

	void AddComponent(Component* component);
	void RemoveComponent(Component* component);

private:
	virtual void UpdateActor(float deltaTime);
	void UpdateComponents(float deltaTime) const;

public:
	[[nodiscard]] const STATE& GetState() const { return state_; }
	[[nodiscard]] dv_math::Transform& GetTransform() { return transform_; }
	[[nodiscard]] Application* GetApplication() const { return application_; }

private:
	STATE state_{ACTIVE};
	std::vector<Component*> components_{};
	Application* application_{};

	dv_math::Transform transform_{};
};

