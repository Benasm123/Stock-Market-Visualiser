#pragma once
#include <chrono>
#include <map>
#include <ranges>
#include <string>
#include <vector>

#include "pcHeader.h"

struct data_table
{
	std::vector<std::string> dates{};
	std::map<std::string, std::vector<float>> values{};

	std::vector<uint64_t> dates_int() const
	{
		std::vector<uint64_t> vals{};

		for ( auto& dateString : dates )
		{
			std::string date_time_format = "%Y-%m-%d";
			std::chrono::year_month_day date{};
			std::istringstream dateStream(dateString);

			dateStream >> std::chrono::parse(date_time_format, date);

			auto value = std::chrono::sys_days{ date }.time_since_epoch().count();

			vals.push_back(value);
		}

		return vals;
	}
};

inline data_table load_data(const std::string& csv_file_name)
{
	LOG_INFO("Loading data for stock: %s", csv_file_name.c_str());
	data_table read_data;
	std::map<int, std::string> index_data;

	std::ifstream values_file(csv_file_name);

	std::string line;

	LOG_INFO("Getting Titles for data");
	if (getline(values_file, line))
	{
		int index = 0;
		while (!line.empty())
		{
			const auto title_end_index = line.find(',');
			std::string title = line.substr(0, title_end_index);


			if ( title != "Date" )
			{
				read_data.values[title] = {};
				index_data[index] = title;

				if ( title_end_index == std::string::npos )
				{
					line.clear();
				}
			}

			line.erase(0, title_end_index + 1);
			index++;
		}
	}
	LOG_INFO("Finished Getting Titles for data");

	LOG_INFO("Getting Values for data");
	while (getline(values_file, line))
	{
		if (line.find("null") != std::string::npos)
		{
			continue;
		}
		int current_index = 0;
		size_t pos = 0;
		while ((pos = line.find(',')) != std::string::npos)
		{
			if (current_index == 0)
			{
				read_data.dates.push_back(line.substr(0, pos));
			} else
			{
				read_data.values[index_data[current_index - 1]].push_back(std::stof(line.substr(0, pos)));
			}

			line.erase(0, pos + 1);
			current_index++;
		}

		read_data.values[index_data[current_index]].push_back(std::stof(line));

		if ( current_index == 0 )
		{
			read_data.dates.push_back(line.substr(0, pos));
		}
		else
		{
			read_data.values[index_data[current_index - 1]].push_back(std::stof(line.substr(0, pos)));
		}
	}
	LOG_INFO("Finished Getting Values for data");

	return read_data;
}
