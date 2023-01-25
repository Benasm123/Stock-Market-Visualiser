#pragma once
#include <iosfwd>
#include <iosfwd>
#include <map>
#include <vector>
#include <vector>

class python_caller
{
public:
	python_caller(const std::string& name);
	~python_caller();
	void loadNewModule(const std::string& name);

	bool SaveWeights(const std::string& saveName);
	bool LoadWeights(const std::string& loadName);
	bool Initialise();
	float Train(int epochs);
	std::map<std::string, float> GetHyperParameters();
	float Evaluate(std::vector<float> vals);
	int Predict(const std::vector<float>& vals);


private:
	PyObject* p_name_{};
	PyObject* p_module_{};
	PyObject* p_dict_{};
	PyObject* p_func_{};
	PyObject* p_value_{};
	PyObject* p_class_{};
	PyObject* p_object_{};
};

