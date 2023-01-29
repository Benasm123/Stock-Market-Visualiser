#include "pcHeader.h"
#include "Actor.h"
#include "Application.h"
#include "Component.h"

Actor::Actor(Application* application)
	:application_(application)
{
	application_->AddActor(this);
}

Actor::~Actor()
{
	application_->RemoveActor(this);
	while(!components_.empty())
	{
		delete components_.back();
	}
}

void Actor::Update(const float deltaTime)
{
	if (state_ == ACTIVE)
	{
		UpdateActor(deltaTime);
		UpdateComponents(deltaTime);
	}
}

void Actor::AddComponent(Component* component)
{
	const int myOrder = component->GetUpdateOrder();
	auto iterator = components_.begin();
	for (iterator = components_.begin(); iterator != components_.end(); ++iterator)
	{
		if (myOrder < (*iterator)->GetUpdateOrder())
		{
			break;
		}
	}
	components_.insert(iterator, component);
}

void Actor::RemoveComponent(Component* component)
{
	const auto iterator = std::ranges::find(components_, component);
	if (iterator != components_.end())
	{
		components_.erase(iterator);
		return;
	}
	LOG_WARN("Could not find a component you tried to delete.");
}

void Actor::UpdateActor(float deltaTime)
{
}

void Actor::UpdateComponents(const float deltaTime) const
{
	for (auto& component : components_)
	{
		component->Update(deltaTime);
	}
}
