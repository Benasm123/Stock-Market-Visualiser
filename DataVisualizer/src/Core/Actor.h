#pragma once
#include "pcHeader.h"

class actor
{
public:
	enum state
	{
		active,
		paused,
		dead
	};

	explicit actor(class application* application);
	virtual ~actor();

	void update(float delta_time);

	void add_component(class component* component);
	void remove_component(class component* component);

private:
	virtual void update_actor(float delta_time);
	void update_components(float delta_time) const;

public:
	[[nodiscard]] const state& get_state() const { return state_; }
	[[nodiscard]] PMATH::transform& get_transform() { return transform_; }
	[[nodiscard]] class application* get_application() const { return application_; }

private:
	state state_{active};
	std::vector<class component*> components_{};
	class application* application_{};

	PMATH::transform transform_{};
};

