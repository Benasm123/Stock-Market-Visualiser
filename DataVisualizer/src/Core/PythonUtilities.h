#pragma once
#include "pcHeader.h"
#include <map>
#include <unordered_map>

class PythonCaller
{
public:
	PythonCaller();
	~PythonCaller();
	void LoadNewModule(const std::string name);
	void LoadObject(const std::string name);
	void ReloadModule(const std::string& name);

	bool SaveWeights(const std::string& saveName);
	bool LoadWeights(const std::string& loadName);
	bool Initialise();
	std::thread* Train(int epochs);
	std::map<std::string, float> GetHyperParameters();
	float Evaluate(std::vector<float> vals);
	std::vector<int> Predict(const std::string& stockName, const std::string& startDate, const std::string& endDate);
	std::vector<float> GetAccuracyList();
	std::vector<float> GetLossList();
	std::vector<float> GetRewardList();
	std::vector<float> ListToVectorFromFunction(const std::string& functionName);

private:
	// std::vector<std::string> loadedModules_{};
	std::unordered_map<std::string, PyObject*> loadedObjects_{};
	std::unordered_map<std::string, PyObject*> loadedModules_{};
	
	std::string selectedModule_{};
	PyGILState_STATE save_{};
};

