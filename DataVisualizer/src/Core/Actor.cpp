#include "pcHeader.h"
#include "Actor.h"
#include "Application.h"
#include "Component.h"

actor::actor(application* application)
	:application_(application)
{
	application_->add_actor(this);
}

actor::~actor()
{
	application_->remove_actor(this);
	while(!components_.empty())
	{
		delete components_.back();
	}
}

void actor::update(const float delta_time)
{
	if (state_ == active)
	{
		update_actor(delta_time);
		update_components(delta_time);
	}
}

void actor::add_component(component* component)
{
	const int my_order = component->get_update_order();
	auto iterator = components_.begin();
	for (iterator = components_.begin(); iterator != components_.end(); ++iterator)
	{
		if (my_order < (*iterator)->get_update_order())
		{
			break;
		}
	}
	components_.insert(iterator, component);
}

void actor::remove_component(component* component)
{
	const auto iterator = std::ranges::find(components_, component);
	if (iterator != components_.end())
	{
		components_.erase(iterator);
		return;
	}
	LOG_WARN("Could not find a component you tried to delete.");
}

void actor::update_actor(float delta_time)
{
}

void actor::update_components(const float delta_time) const
{
	for (auto& component : components_)
	{
		component->update(delta_time);
	}
}
