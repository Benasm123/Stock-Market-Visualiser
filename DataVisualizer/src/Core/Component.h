#pragma once
#include "Actor.h"

class Component
{
public:
	explicit Component(class Actor* owner, int updateOrder = 100);
	virtual ~Component();
	
	virtual void Update(float deltaTime);

	[[nodiscard]] int GetUpdateOrder() const { return updateOrder_; }
	[[nodiscard]] Actor& GetOwner() const { return *owner_; }
	[[nodiscard]] const PMATH::transform& GetTransform() const { return owner_->GetTransform(); }

protected:
	class Actor* owner_;
	uint8_t updateOrder_;
};
