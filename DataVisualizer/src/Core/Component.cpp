#include "pcHeader.h"
#include "Component.h"

#include "Actor.h"

Component::Component(Actor* owner, int updateOrder)
	:owner_(owner)
	,updateOrder_(static_cast<uint8_t>(updateOrder))
{
	if (updateOrder > 100)
	{
		LOG_WARN("Component update order should not be more than 100");
	}
	owner->AddComponent(this);
}

Component::~Component()
{
	owner_->RemoveComponent(this);
}

void Component::Update(float deltaTime)
{
}
