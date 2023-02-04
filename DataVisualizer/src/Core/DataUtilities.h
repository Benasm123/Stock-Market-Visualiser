#pragma once
#include <chrono>
#include <map>
#include <ranges>
#include <string>
#include <vector>

#include "pcHeader.h"

struct DataTable
{
	std::vector<std::string> dates{};
	std::map<std::string, std::vector<float>> values{};
	std::vector<uint64_t> datesInts{};

	[[nodiscard]] std::vector<uint64_t> DatesInt()
	{
		if ( !datesInts.empty() ) return datesInts;

		for ( auto& dateString : dates )
		{
			std::string dateTimeFormat = "%Y-%m-%d";
			std::chrono::year_month_day date{};
			std::istringstream dateStream(dateString);

			dateStream >> std::chrono::parse(dateTimeFormat, date);

			const auto value = std::chrono::sys_days{ date }.time_since_epoch().count();

			datesInts.push_back(value);
		}

		return datesInts;
	}
};

inline DataTable LoadData(const std::string& csvFileName)
{
	LOG_FUNC_START();
	LOG_INFO("Loading From: %s", csvFileName.c_str());
	DataTable readData;
	std::map<int, std::string> indexData;

	std::ifstream valuesFile(csvFileName);

	std::string line;

	LOG_INFO("Getting Titles for data");
	if (getline(valuesFile, line))
	{
		int index = 0;
		while (!line.empty())
		{
			const auto titleEndIndex = line.find(',');
			std::string title = line.substr(0, titleEndIndex);


			if ( title != "Date" )
			{
				readData.values[title] = {};
				indexData[index] = title;

				if ( titleEndIndex == std::string::npos )
				{
					line.clear();
				}
			}

			line.erase(0, titleEndIndex + 1);
			index++;
		}
	}
	LOG_INFO("Finished Getting Titles for data");

	LOG_INFO("Getting Values for data");
	while (getline(valuesFile, line))
	{
		if (line.find("null") != std::string::npos)
		{
			continue;
		}
		int currentIndex = 0;
		size_t pos = 0;
		while ((pos = line.find(',')) != std::string::npos)
		{
			if (currentIndex == 0)
			{
				readData.dates.push_back(line.substr(0, pos));
			} else
			{
				readData.values[indexData[currentIndex - 1]].push_back(std::stof(line.substr(0, pos)));
			}

			line.erase(0, pos + 1);
			currentIndex++;
		}

		readData.values[indexData[currentIndex]].push_back(std::stof(line));

		if ( currentIndex == 0 )
		{
			readData.dates.push_back(line.substr(0, pos));
		}
		else
		{
			readData.values[indexData[currentIndex - 1]].push_back(std::stof(line.substr(0, pos)));
		}
	}

	LOG_FUNC_END();
	return readData;
}

inline std::vector<float> LoadData(const std::string& csvFileName, const std::string& valueTitle)
{
	LOG_FUNC_START();
	LOG_INFO("Loading From: %s Title: %s", csvFileName.c_str(), valueTitle.c_str());

	std::ifstream valuesFile(csvFileName);

	std::string line;

	std::vector<float> values;

	//TODO::Create a utility function that does this for a wanted title. Already have read file utility just make it take a title parameter.
	bool wantedTitleFound = false;
	uint32_t wantedIndex = 0;
	if (getline(valuesFile, line))
	{
		while (!line.empty())
		{
			const auto titleEndIndex = line.find(',');
			std::string title = line.substr(0, titleEndIndex);
			if (title == valueTitle)
			{
				wantedTitleFound = true;
				break;
			}

			line.erase(0, titleEndIndex + 1);
			wantedIndex++;
		}
	}
	if (!wantedTitleFound)
	{
		LOG_ERROR("Can not find the value title specified: %s", valueTitle.c_str());
		return {};
	}

	while (getline(valuesFile, line))
	{
		for (uint32_t i = 0; i < wantedIndex; ++i)
		{
			line.erase(0, line.find(',') + 1);
		}

		values.push_back(std::stof(line.substr(0, line.find(','))));
	}

	valuesFile.close();

	LOG_FUNC_END();
	return values;
}