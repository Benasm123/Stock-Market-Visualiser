#pragma once
#include "Actor.h"

class Component
{
public:
	enum State {ACTIVE, SLEEP, DEAD};
	explicit Component(Actor* owner, int updateOrder = 100);
	virtual ~Component();
	
	virtual void Update(float deltaTime);

	void SetState(const State state) { state_ = state; }

	[[nodiscard]] int GetUpdateOrder() const { return updateOrder_; }
	[[nodiscard]] Actor& GetOwner() const { return *owner_; }
	[[nodiscard]] dv_math::Transform& GetTransform() const { return owner_->GetTransform(); }

protected:
	Actor* owner_;
	uint8_t updateOrder_;
	State state_ = ACTIVE;
};
