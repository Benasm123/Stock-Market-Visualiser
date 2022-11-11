#pragma once
#include "Renderer/renderer.h"
#include "Renderer/VulkanContext.h"
#include "Renderer/Window.h"

class application
{
public:
	application();
	virtual ~application();

	bool init();
	void run();
	void shutdown();

	void add_actor(class actor* actor);
	void remove_actor(class actor* actor);

private:
	void process_input(float delta_time);
	void update(float delta_time);
	void generate_output(float delta_time);

protected:
	virtual void on_update(float delta_time) = 0;
	virtual void on_ui(float delta_time) = 0;
	virtual void on_startup() = 0;
	virtual void on_shutdown() = 0;

public:
	[[nodiscard]] renderer& get_renderer() { return renderer_; }

protected:
	bool is_running_ = true;
	int test_rum_;
	bool updating_actors_{ false };

	std::vector<class actor*> actors_;
	std::vector<class actor*> pending_actors_;

	vulkan_context vulkan_context_;
	renderer renderer_;
};

application* create_application();

