#pragma once
#include "Actor.h"

class component
{
public:
	explicit component(class actor* owner, int update_order = 100);
	virtual ~component();
	
	virtual void update(float delta_time);

	[[nodiscard]] int get_update_order() const { return update_order_; }
	[[nodiscard]] actor& get_owner() const { return *owner_; }
	[[nodiscard]] const PMATH::transform& get_transform() const { return owner_->get_transform(); }

protected:
	class actor* owner_;
	uint8_t update_order_;
};
