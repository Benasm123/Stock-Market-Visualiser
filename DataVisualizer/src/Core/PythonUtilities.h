#pragma once
#include "pcHeader.h"
#include <map>

class PythonCaller
{
public:
	PythonCaller();
	~PythonCaller();
	void LoadNewModule(const std::string& name);

	bool SaveWeights(const std::string& saveName);
	bool LoadWeights(const std::string& loadName);
	bool Initialise();
	float Train(int epochs);
	std::map<std::string, float> GetHyperParameters();
	float Evaluate(std::vector<float> vals);
	int Predict(const std::vector<float>& vals);


private:
	std::vector<std::string> loadedModules_{};

	PyObject* pName_{};
	PyObject* pModule_{};
	PyObject* pDict_{};
	PyObject* pFunc_{};
	PyObject* pValue_{};
	PyObject* pClass_{};
	PyObject* pObject_{};
};

