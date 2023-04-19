#include "pcHeader.h"
#include "Application.h"

#include "Actor.h"

Application::Application()
= default;

Application::~Application()
= default;

bool Application::Init()
{
	LOG_FUNC_START();
	if (!vulkanContext_.Init()) return false;
	if (!renderer_.Init(&vulkanContext_)) return false;

	OnStartup();

	LOG_FUNC_END();
	return true;
}


void Application::Run()
{
	LOG_FUNC_START();
	
	float deltaTime = 0;

	auto lastFrame = std::chrono::steady_clock::now();
	auto thisFrame = std::chrono::steady_clock::now();

	while (isRunning_)
	{
		thisFrame = std::chrono::steady_clock::now();
		deltaTime = static_cast<float>(std::chrono::duration<double>((thisFrame - lastFrame)).count());
		lastFrame = thisFrame;

		ProcessInput(deltaTime);
		Update(deltaTime);
		GenerateOutput(deltaTime);
	}

	LOG_FUNC_END();
}

void Application::Shutdown()
{
	LOG_FUNC_START();
	OnShutdown();

	vulkanContext_.GetLogicalDevice().waitIdle();

	while (!actors_.empty())
	{
		delete actors_.back();
	}

	renderer_.Shutdown();
	vulkanContext_.Shutdown();

	LOG_FUNC_END();
}

void Application::AddActor(Actor* actor)
{
	if (updatingActors_)
	{
		pendingActors_.emplace_back(actor);
		return;
	}
	actors_.emplace_back(actor);
}

void Application::RemoveActor(Actor* actor)
{
	auto iterator = std::ranges::find(pendingActors_, actor);
	if (iterator != pendingActors_.end())
	{
		std::iter_swap(iterator, pendingActors_.end() - 1);
		pendingActors_.pop_back();
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
void Application::ProcessInput(float deltaTime)
{
}

//Process all Application loop actions here.
void Application::Update(const float deltaTime)
{
	isRunning_ &= vulkanContext_.Update();

	OnUpdate(deltaTime);

	for (const auto& actor : actors_)
	{
		actor->Update(deltaTime);
	}
}

//Process all rendering and output here.
void Application::GenerateOutput(float deltaTime)
{
	isRunning_ &= renderer_.Update();
}