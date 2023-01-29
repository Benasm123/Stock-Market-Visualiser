#pragma once
#include "Renderer/Renderer.h"
#include "Renderer/VulkanContext.h"
#include "Renderer/Window.h"

class Application
{
public:
	Application();
	virtual ~Application();

	bool Init();
	void Run();
	void Shutdown();

	void AddActor(class Actor* actor);
	void RemoveActor(class Actor* actor);

private:
	void ProcessInput(float deltaTime);
	void Update(float deltaTime);
	void GenerateOutput(float deltaTime);

protected:
	virtual void OnUpdate(float deltaTime) = 0;
	virtual void OnStartup() = 0;
	virtual void OnShutdown() = 0;

public:
	[[nodiscard]] Renderer& GetRenderer() { return renderer_; }
	[[nodiscard]] PMATH::vec2<uint32_t> GetWindowSize() { return { vulkanContext_.get_window().get_width(), vulkanContext_.get_window().get_height() }; }

protected:
	bool isRunning_{ true };
	bool updatingActors_{ false };

	std::vector<Actor*> actors_;
	std::vector<Actor*> pendingActors_;

	VulkanContext vulkanContext_;
	Renderer renderer_;
};

Application* CreateApplication();

