#include "pcHeader.h"
#include "python_caller.h"

#include <filesystem>
#include <iostream>
#include <map>

python_caller::python_caller(const std::string& name)
{
	std::filesystem::path cwd = std::filesystem::current_path() / "Models" / "Python" / "";
	std::cout << "FILE: " << cwd.string() << "\n";
	// return;
	Py_Initialize();
	PyObject* sysPath = PySys_GetObject((char*)"path");
	PyList_Append(sysPath, PyUnicode_FromString(cwd.string().c_str()));
	return;
	PyConfig config;
	PyConfig_InitPythonConfig(&config);

	PyConfig_Read(&config);

	config.module_search_paths_set = 1;

	const char* c = ("D:/PracticeCode/C++/DataVisualizer/Test/Models/Python/" + name + "/").c_str();
	
	std::wstring w;
	std::copy_n(c, strlen(c), back_inserter(w));
	const WCHAR* pwcs_name = w.c_str();
	
	PyWideStringList_Insert(&config.module_search_paths, 0, pwcs_name);
	
	const char* c2 = ("D:/PracticeCode/C++/DataVisualizer/Test/Models/Python/");
	
	std::wstring w2;
	std::copy_n(c2, strlen(c2), back_inserter(w2));
	const WCHAR* pwcs_name2 = w2.c_str();
	
	PyWideStringList_Insert(&config.module_search_paths, 1, pwcs_name2);

	PyStatus status = Py_InitializeFromConfig(&config);
	PyConfig_Clear(&config);
	if ( PyStatus_Exception(status) ) {
		Py_ExitStatusException(status);
	}

	p_name_ = PyUnicode_FromString("main");
	
	if (!p_name_)
	{
		LOG_ERROR("COULDNT FIND PYTHON PROGRAM");
		return;
	}

	p_module_ = PyImport_ImportModule("main");
	if ( !p_module_ )
	{
		LOG_ERROR("COULDNT MODULE");
		return;
	}

	p_dict_ = PyModule_GetDict(p_module_);
	if ( !p_dict_ )
	{
		LOG_ERROR("COULDNT DICT");
		return;
	}

	p_class_ = PyDict_GetItemString(p_dict_, "StockPredictor");
	if ( !p_class_ )
	{
		LOG_ERROR("COULDNT CLASS");
		return;
	}

	Py_DECREF(p_dict_);

	p_object_ = PyObject_CallObject(p_class_, nullptr);

	Py_DECREF(p_class_);
	Py_DECREF(p_name_);
}

python_caller::~python_caller()
{
	Py_Finalize();
}

void python_caller::loadNewModule(const std::string& name)
{
	if (p_object_)
	{
		Py_DecRef(p_object_);
	}

	PyObject* sysPath = PySys_GetObject((char*)"path");
	PyList_Append(sysPath, PyUnicode_FromString(("D:/PracticeCode/C++/DataVisualizer/Test/Models/Python/" + name + "/").c_str()));
	
	p_module_ = PyImport_ImportModule(name.c_str());
	if ( !p_module_ )
	{
		LOG_ERROR("COULDNT MODULE");
		return;
	}

	p_dict_ = PyModule_GetDict(p_module_);
	if ( !p_dict_ )
	{
		LOG_ERROR("COULDNT DICT");
		return;
	}

	p_class_ = PyDict_GetItemString(p_dict_, "StockPredictor");
	if ( !p_class_ )
	{
		LOG_ERROR("COULDNT CLASS");
		return;
	}

	Py_DECREF(p_dict_);

	p_object_ = PyObject_CallObject(p_class_, nullptr);

	Py_DECREF(p_class_);
}

bool python_caller::SaveWeights(const std::string& saveName)
{
	if ( p_module_ )
	{
		p_value_ = PyObject_CallMethod(p_object_, "SaveModel", "s", saveName.c_str());
		if ( p_value_ )
		{
			const bool returnValue = PyLong_AsLong(p_value_) == 1l;
			return returnValue;
		}

		LOG_WARN("CANT FIND FUNCTION");
		PyErr_Print();
		return false;
	}
	LOG_WARN("CANT FIND MODULE");
	PyErr_Print();
	return false;
}

bool python_caller::LoadWeights(const std::string& loadName)
{
	if ( p_module_ )
	{
		p_value_ = PyObject_CallMethod(p_object_, "LoadModel", "s", loadName.c_str());
		if ( p_value_ )
		{
			const bool returnValue = PyLong_AsLong(p_value_) == 1l;
			return returnValue;
		}

		LOG_WARN("CANT FIND FUNCTION");
		PyErr_Print();
		return false;
	}
	LOG_WARN("CANT FIND MODULE");
	PyErr_Print();
	return false;
}

bool python_caller::Initialise()
{
	if ( p_module_ )
	{
		p_value_ = PyObject_CallMethod(p_object_, "Initialise", "");
		if ( p_value_ )
		{
			const bool returnValue = PyLong_AsLong(p_value_) == 1l;
			return returnValue;
		}

		LOG_WARN("CANT FIND FUNCTION");
		PyErr_Print();
		return false;
	}
	LOG_WARN("CANT FIND MODULE");
	PyErr_Print();
	return false;
}

float python_caller::Train(const int epochs)
{
	if ( p_module_ )
	{
		p_value_ = PyObject_CallMethod(p_object_, "Train", "i", epochs);
		if ( p_value_ )
		{
			const float returnValue = static_cast<float>(PyFloat_AsDouble(p_value_));
			return returnValue;
		}

		LOG_WARN("CANT FIND FUNCTION");
		PyErr_Print();
		return 0.0f;
	}
	LOG_WARN("CANT FIND MODULE");
	PyErr_Print();
	return 0.0f;
}

std::map<std::string, float> python_caller::GetHyperParameters()
{
	if ( p_module_ )
	{
		p_value_ = PyObject_CallMethod(p_object_, "Initialise", "");
		if ( p_value_ )
		{
			const std::map<std::string, float> returnValue{};
			return returnValue;
		}

		LOG_WARN("CANT FIND FUNCTION");
		PyErr_Print();
		return {};
	}
	LOG_WARN("CANT FIND MODULE");
	PyErr_Print();
	return {};
}

float python_caller::Evaluate(std::vector<float> vals)
{
	if ( p_module_ )
	{
		p_value_ = PyObject_CallMethod(p_object_, "EvaluateModel", ("(" + std::string(vals.size(), 'f') + ")").c_str(), vals.data());
		if ( p_value_ )
		{
			const float returnValue = static_cast<float>(PyFloat_AsDouble(p_value_));
			return returnValue;
		}

		LOG_WARN("CANT FIND FUNCTION");
		PyErr_Print();
		return 0.0f;
	}
	LOG_WARN("CANT FIND MODULE");
	PyErr_Print();
	return 0.0f;
}

int python_caller::Predict(const std::vector<float>& vals)
{
	if ( p_module_ )
	{
		PyObject* pXVec = PyTuple_New(vals.size());
		PyObject* test = nullptr;

		for (int i = 0; i < vals.size(); ++i ) {
			test = PyFloat_FromDouble((double)vals[i]);
			if ( !test ) {
				fprintf(stderr, "Cannot convert array value\n");
				return 1;
			}
			PyTuple_SetItem(pXVec, i, test);
		}
		
		PyObject* name = PyUnicode_FromString("MakePrediction");
		p_value_ = PyObject_CallMethodOneArg(p_object_, name, pXVec);

		Py_DECREF(name);
		Py_DECREF(test);

		if ( p_value_ )
		{
			const int returnValue = _PyLong_AsInt(p_value_);
			return returnValue;
		}

		LOG_WARN("CANT FIND FUNCTION");
		PyErr_Print();
		return 0;
	}

	LOG_WARN("CANT FIND MODULE");
	PyErr_Print();
	return 0;
}

