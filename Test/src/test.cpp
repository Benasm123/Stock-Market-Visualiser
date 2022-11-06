#pragma once
#include <chrono>
#include <iostream>

#include "data_manager.h"
#include "graph.h"
#include "test_actor.h"
#include "Core/Application.h"
#include "Core/main.h"

class test final : public application
{
protected:

	graph* data_graph = nullptr;
	float rotation_speed = 360.0f;

	void on_startup() override
	{
		data_graph = new graph(this, {});
		data_table data = load_data("./data/AAPL-Year.csv");
		data_table data2 = load_data("./data/TSLA-Year.csv");

		std::vector<float> x_vals = {};
		for (int i = 0; i < data["Date"].size(); i++)
		{
			x_vals.push_back(i);
		}

		std::vector<float> x_2vals = {};
		for (int i = 0; i < data2["Date"].size(); i++)
		{
			x_2vals.push_back(i + (data["Date"].size() - data2["Date"].size()));
		}

		const auto low_plot = plot(x_vals, data["High"], { 1.0f, 0.0f, 0.0f });
		const auto high_plot = plot(x_2vals, data2["High"], { 0.0f, 1.0f, 0.0f });

		data_graph->add_plot(high_plot);
		data_graph->add_plot(low_plot);

		data_graph->get_transform().scale = 1.5f;
		data_graph->get_transform().rotation = 0.0f;
		data_graph->get_transform().position = glm::vec2(0.25f, 0.25f);

		data_graph->show();

		auto data_2 = new graph(this, {high_plot});
		
		data_2->get_transform().position = glm::vec2(2.0f, 0.25f);
		data_2->get_transform().scale = 0.5f;
		
		data_2->show();

		std::string date_time_format = "%Y-%m-%d";

		std::istringstream string_date{"2020-1-28"};

		std::chrono::year_month_day date{};

		string_date >> std::chrono::parse(date_time_format, date);

		using namespace std::chrono;
		using namespace std;
		auto first = 2012y/1/24;

		// std::cout << "Year: " << date.year() << "\n";
		// std::cout << "Month: " << date.month() << "\n";
		// std::cout << "Day: " << date.day() << "\n";
		//
		// std::cout << "Difference: " << std::chrono::sys_days{ date } - std::chrono::sys_days{first} << std::endl;
	}

	void on_update(float delta_time) override
	{
		// data_graph->get_transform().rotation += rotation_speed * delta_time;
		// if (data_graph->get_transform().rotation > 360.0f || data_graph->get_transform().rotation < -0.0f) 
		// {
		// 	rotation_speed *= -1;
		// }
		// data_graph->get_transform().position.x = (data_graph->get_transform().rotation / 360.0f) + 0.5f;
		// data_graph->get_transform().scale += 0.1f * delta_time;
	}

	void on_shutdown() override
	{
		
	}
	
};

application* create_application()
{
	return new test();
}
