#pragma once

int main()
{
	auto* app = create_application();
	if (app->init())
	{
		app->run();
	}
	app->shutdown();
	delete app;
}
