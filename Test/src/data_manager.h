#pragma once
#include <map>
#include <ranges>
#include <string>
#include <vector>

#include "pcHeader.h"

typedef std::map<std::string, std::vector<float>> data_table;

inline data_table load_data(const std::string& csv_file_name)
{
	data_table read_data;
	std::map<int, std::string> index_data;

	std::ifstream values_file(csv_file_name);

	std::string line;
	
	if (getline(values_file, line))
	{
		int index = 0;
		while (!line.empty())
		{
			const auto title_end_index = line.find(',');
			std::string title = line.substr(0, title_end_index);
			
			read_data[title] = {};
			index_data[index] = title;

			if (title_end_index == std::string::npos)
			{
				line.clear();
			}

			line.erase(0, title_end_index + 1);
			index++;
		}
	}

	while (getline(values_file, line))
	{
		int current_index = 0;
		size_t pos = 0;
		while ((pos = line.find(',')) != std::string::npos)
		{
			read_data[index_data[current_index]].push_back(std::stof(line.substr(0, pos)));
			line.erase(0, pos + 1);
			current_index++;
		}
		read_data[index_data[current_index]].push_back(std::stof(line.substr(0, pos)));
	}

	return read_data;
}
