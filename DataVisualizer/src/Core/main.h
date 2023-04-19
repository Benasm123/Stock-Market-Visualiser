#pragma once
#include "Application.h"
#include "pcHeader.h"
#include "StockAid.h"

// ReSharper disable once CppNonInlineFunctionDefinitionInHeaderFile
int main()
{
	LOG_FUNC_START();
	auto* app = CreateApplication();

	if (app->Init())
	{
		app->Run();
	}

	app->Shutdown();

	delete app;
	LOG_FUNC_END();
}
