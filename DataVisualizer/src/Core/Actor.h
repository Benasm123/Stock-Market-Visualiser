#pragma once
#include "pcHeader.h"

class Actor
{
public:
	enum STATE
	{
		ACTIVE,
		PAUSED,
		DEAD
	};

	explicit Actor(class Application* application);
	virtual ~Actor();

	void Update(float deltaTime);

	void AddComponent(class Component* component);
	void RemoveComponent(class Component* component);

private:
	virtual void UpdateActor(float deltaTime);
	void UpdateComponents(float deltaTime) const;

public:
	[[nodiscard]] const STATE& GetState() const { return state_; }
	[[nodiscard]] PMATH::transform& GetTransform() { return transform_; }
	[[nodiscard]] class Application* GetApplication() const { return application_; }

private:
	STATE state_{ACTIVE};
	std::vector<class Component*> components_{};
	class Application* application_{};

	PMATH::transform transform_{};
};

