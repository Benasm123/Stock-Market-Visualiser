#pragma once
#include <chrono>
#include <map>
#include <ranges>
#include <string>
#include <vector>

#include "pcHeader.h"
#include <filesystem>
#include <vector>

typedef std::chrono::year_month_day Date;

inline Date ParseStringAsDate(const std::string& dateString)
{
	const std::string dateTimeFormat = "%Y-%m-%d";
	Date date{};
	std::istringstream dateStream(dateString);

	dateStream >> parse(dateTimeFormat, date);

	return date;
}

inline uint64_t ConvertDateToTimeSinceEpoch(const Date date)
{
	return std::chrono::sys_days{ date }.time_since_epoch().count();
}

inline uint64_t ConvertDateStringToTimeSinceEpoch(const std::string& dateString)
{
	const Date date = ParseStringAsDate(dateString);

	return ConvertDateToTimeSinceEpoch(date);
}

struct DataTable
{
	std::vector<std::string> dates{};
	std::map<std::string, std::vector<float>> values{};
	std::vector<uint64_t> datesSinceEpoch{};	
};

inline std::vector<std::string> ExtractColumnNames(std::ifstream& file, DataTable& dataTable)
{
	std::vector<std::string> columnNames{};

	std::string line;
	if ( getline(file, line) )
	{
		size_t columnEndIndex = -1;
		do
		{
			const auto temp = columnEndIndex + 1;
			columnEndIndex = line.find(',', columnEndIndex + 1);
			std::string columnName = line.substr(temp, columnEndIndex - temp);

			if ( columnName != "Date" )
			{
				dataTable.values[columnName] = {};
				columnNames.push_back(columnName);				
			}

		} while ( columnEndIndex != std::string::npos );
	}

	return columnNames;
}

inline void ExtractData( std::ifstream& valuesFile, const std::vector<std::string>& columnNames, DataTable& dataTable)
{
	std::string line;
	while ( getline(valuesFile, line) )
	{
		// If this line contains a null value, throw away the whole line and go next.
		if ( line.find("null") != std::string::npos )
		{
			continue;
		}

		//Find date first - Split from the values data, as this is a string, and the rest stored as floats.
		size_t columnEnd = line.find(',');

		// Just to be safe, if line is empty continue however this shouldn't happen.
		if ( columnEnd == std::string::npos ) continue;

		auto dateString = line.substr(0, columnEnd);

		// If the date is before the epoch (which is used for graphing and is the start of 1970) we throw away the value.
		Date date = ParseStringAsDate(dateString);
		if ( date.year() < std::chrono::year(1970) ) continue;

		dataTable.dates.push_back(dateString);
		dataTable.datesSinceEpoch.push_back(ConvertDateToTimeSinceEpoch(date));

		int currentIndex = 0;
		while ( columnEnd != std::string::npos )
		{
			columnEnd++;
			const auto temp = columnEnd;
			columnEnd = line.find(',', columnEnd);
			
			dataTable.values[columnNames[currentIndex]].push_back(std::stof(line.substr(temp, columnEnd - temp)));

			currentIndex++;
		} 
	}
}

inline DataTable LoadData(const std::string& csvFileName)
{
	LOG_FUNC_START();
	LOG_INFO("Loading From: %s", csvFileName.c_str());

	const std::string directoryPath = "./data/";
	std::ifstream valuesFile(directoryPath + csvFileName);

	DataTable dataTable;
	
	const std::vector<std::string> columnNames = ExtractColumnNames(valuesFile, dataTable);
	
	ExtractData(valuesFile, columnNames, dataTable);
	
	LOG_FUNC_END();
	return dataTable;
}