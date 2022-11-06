#include "pcHeader.h"
#include "Component.h"

#include "Actor.h"

component::component(actor* owner, int update_order)
	:owner_(owner)
	,update_order_(static_cast<uint8_t>(update_order))
{
	if (update_order > 100)
	{
		LOG_WARN("Component update order should not be more than 100");
	}
	owner->add_component(this);
}

component::~component()
{
	owner_->remove_component(this);
}

void component::update(float delta_time)
{
}
