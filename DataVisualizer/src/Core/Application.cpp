#include "pcHeader.h"
#include "Application.h"

#include "Actor.h"
#include "backends/imgui_impl_vulkan.h"
#include "backends/imgui_impl_win32.h"

application::application()
= default;

application::~application()
= default;

bool application::init()
{
	bool successful_init = true;

	successful_init &= vulkan_context_.init();
	successful_init &= renderer_.init(&vulkan_context_);

	on_startup();

	LOG_INFO("Initialized Application");
	return successful_init;
}


void application::run()
{
	LOG_INFO("Started Running Application");

	//TODO::Add a way to calculate delta time and pass through.
	double delta_time = 0;

	auto last_frame = std::chrono::steady_clock::now();
	auto this_frame = std::chrono::steady_clock::now();
	//TODO::Add a way to calculate delta time and pass through.

	while (is_running_)
	{
		this_frame = std::chrono::steady_clock::now();
		delta_time = std::chrono::duration<double>((this_frame - last_frame)).count();
		last_frame = this_frame;

		process_input(delta_time);
		update(delta_time);
		generate_output(delta_time);
	}
	LOG_INFO("Ended Running Application");
}

void application::shutdown()
{
	on_shutdown();

	vulkan_context_.get_logical_device().waitIdle();

	while (!actors_.empty())
	{
		delete actors_.back();
	}

	renderer_.shutdown();
	vulkan_context_.shutdown();

	LOG_INFO("Shutdown Application");
}

void application::add_actor(actor* actor)
{
	if (updating_actors_)
	{
		pending_actors_.emplace_back(actor);
		return;
	}
	actors_.emplace_back(actor);
}

void application::remove_actor(actor* actor)
{
	auto iterator = std::ranges::find(pending_actors_, actor);
	if (iterator != pending_actors_.end())
	{
		std::iter_swap(iterator, pending_actors_.end() - 1);
		pending_actors_.pop_back();
		return;
	}

	iterator = std::ranges::find(actors_, actor);
	if (iterator != actors_.end())
	{
		std::iter_swap(iterator, actors_.end() - 1);
		actors_.pop_back();
		return;
	}

	LOG_WARN("Could not find an actor you were trying to remove.");
}


// ReSharper disable once CppMemberFunctionMayBeStatic
// Have no event system yet to use this.
//Process all user inputs here.
void application::process_input(float delta_time)
{
}

//Process all application loop actions here.
void application::update(float delta_time)
{
	is_running_ &= vulkan_context_.update();

	on_update(delta_time);

	for (const auto& actor : actors_)
	{
		actor->update(delta_time);
	}
}

//Process all rendering and output here.
void application::generate_output(float delta_time)
{
	is_running_ &= renderer_.update();
}