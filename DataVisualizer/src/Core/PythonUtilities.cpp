#include "pcHeader.h"
#include "PythonUtilities.h"

#include <filesystem>
#include <iostream>
#include <map>

// Initialise a python module using a config to load all needed file paths.
PythonCaller::PythonCaller()
{
	LOG_FUNC_START();

	// Create a config and set it to look for added module paths.
	PyConfig config;
	PyConfig_InitPythonConfig(&config);
	PyConfig_Read(&config);
	config.module_search_paths_set = 1;

	// Add all required Module paths including python paths, and also the base class files.
	std::filesystem::path algorithmPythonFolderPath = std::filesystem::current_path() / "Models" / "Python";
	PyWideStringList_Append(&config.module_search_paths, algorithmPythonFolderPath.wstring().c_str());
	
	std::filesystem::path pythonLibFolderPath = std::filesystem::current_path().parent_path() / "ext" / "Python39_64" / "Lib";	
	PyWideStringList_Append(&config.module_search_paths, pythonLibFolderPath.wstring().c_str());
	
	std::filesystem::path pythonDLLFolderPath = std::filesystem::current_path().parent_path() / "ext" / "Python39_64" / "DLLs";
	PyWideStringList_Append(&config.module_search_paths, pythonDLLFolderPath.wstring().c_str());

	// Initialise with out config. 
	PyStatus status = Py_InitializeFromConfig(&config);

	if (PyStatus_Exception(status)) {
		Py_ExitStatusException(status);
	}

	PyConfig_Clear(&config);

	LOG_FUNC_END();
}

PythonCaller::~PythonCaller()
{
	Py_Finalize();
}

// Load a new python module/file.
void PythonCaller::LoadNewModule(const std::string& name)
{
	LOG_FUNC_START();
	// If we already have a module loaded we want to decrement references to it so it is destroyed.
	if (pObject_)
	{
		Py_DECREF(pObject_);
	}

	// Load the folder into the python module path to make it visible to python.
	PyObject* sysPath = PySys_GetObject("path");
	if (std::ranges::find(loadedModules_, name) == loadedModules_.end())
	{
		const std::filesystem::path moduleFilePath = std::filesystem::current_path() / "Models" / "Python" / name;
		PyList_Append(sysPath, PyUnicode_FromString(moduleFilePath.string().c_str()));
		loadedModules_.push_back(name);
	}

	// Check if this module is already loaded, if so we do not need to reload it, just need to get it and set as module.
	pModule_ = PyImport_GetModule(PyUnicode_FromString(name.c_str()));
	if (!pModule_)
	{
		pModule_ = PyImport_ImportModule(name.c_str());
		if (!pModule_)
		{
			LOG_ERROR("COULDNT LOAD MODULE: %s", name.c_str());
			return;
		}
	}

	// Get modules dict to be able to get the objects we want to call.
	pDict_ = PyModule_GetDict(pModule_);
	if ( !pDict_ )
	{
		LOG_ERROR("COULDNT LOAD MODULE DICT FOR MODULE: %s", name.c_str());
		return;
	}

	// Get the class for the stock predictor using the dict.
	pClass_ = PyDict_GetItemString(pDict_, "StockPredictor");
	if ( !pClass_ )
	{
		LOG_ERROR("COULDNT FIND CLASS MAKE SURE TO NAME IT 'StockPredictor'");
		return;
	}

	// Create the object of type stock predictor.
	pObject_ = PyObject_CallObject(pClass_, nullptr);

	LOG_FUNC_END();
}

// Helper function to call the python interface with needed inputs.
bool PythonCaller::SaveWeights(const std::string& saveName)
{
	LOG_FUNC_START();
	if ( pModule_ )
	{
		pValue_ = PyObject_CallMethod(pObject_, "SaveModel", "s", saveName.c_str());
		if ( pValue_ )
		{
			const bool returnValue = PyLong_AsLong(pValue_) == 1l;
			Py_DECREF(pValue_);
			LOG_FUNC_END();
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

// Helper function to call the python interface with needed inputs.
bool PythonCaller::LoadWeights(const std::string& loadName)
{
	LOG_FUNC_START();
	if ( pModule_ )
	{
		pValue_ = PyObject_CallMethod(pObject_, "LoadModel", "s", loadName.c_str());
		if ( pValue_ )
		{
			const bool returnValue = PyLong_AsLong(pValue_) == 1l;
			Py_DECREF(pValue_);
			LOG_FUNC_END();
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

// Helper function to call the python interface with needed inputs.
bool PythonCaller::Initialise()
{
	LOG_FUNC_START();
	if ( pModule_ )
	{
		pValue_ = PyObject_CallMethod(pObject_, "Initialise", "");
		if ( pValue_ )
		{
			const bool returnValue = PyLong_AsLong(pValue_) == 1l;
			Py_DECREF(pValue_);
			LOG_FUNC_END();
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

// Helper function to call the python interface with needed inputs.
float PythonCaller::Train(const int epochs)
{
	LOG_FUNC_START();
	if ( pModule_ )
	{
		pValue_ = PyObject_CallMethod(pObject_, "Train", "i", epochs);
		if ( pValue_ )
		{
			const float returnValue = static_cast<float>(PyFloat_AsDouble(pValue_));
			Py_DECREF(pValue_);
			LOG_FUNC_END();
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

// Helper function to call the python interface with needed inputs.
std::map<std::string, float> PythonCaller::GetHyperParameters()
{
	LOG_FUNC_START();
	if ( pModule_ )
	{
		pValue_ = PyObject_CallMethod(pObject_, "Initialise", "");
		if ( pValue_ )
		{
			const std::map<std::string, float> returnValue{};
			Py_DECREF(pValue_);
			LOG_FUNC_END();
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

// Helper function to call the python interface with needed inputs.
float PythonCaller::Evaluate(std::vector<float> vals)
{
	LOG_FUNC_START();
	if ( pModule_ )
	{
		pValue_ = PyObject_CallMethod(pObject_, "EvaluateModel", ("(" + std::string(vals.size(), 'f') + ")").c_str(), vals.data());
		if ( pValue_ )
		{
			const float returnValue = static_cast<float>(PyFloat_AsDouble(pValue_));
			Py_DECREF(pValue_);
			LOG_FUNC_END();
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

// Helper function to call the python interface with needed inputs.
int PythonCaller::Predict(const std::vector<float>& vals)
{
	LOG_FUNC_START();
	if ( pModule_ )
	{
		PyObject* pXVec = PyTuple_New(vals.size());
		PyObject* test = nullptr;

		for (int i = 0; i < vals.size(); ++i ) {
			test = PyFloat_FromDouble((double)vals[i]);
			if ( !test ) {
				fprintf(stderr, "Cannot convert array value\n");
				LOG_FUNC_END();
				return 1;
			}
			PyTuple_SetItem(pXVec, i, test);
		}
		
		PyObject* name = PyUnicode_FromString("MakePrediction");
		pValue_ = PyObject_CallMethodOneArg(pObject_, name, pXVec);

		Py_DECREF(name);
		Py_DECREF(test);
		Py_DECREF(pXVec);

		if ( pValue_ )
		{
			const int returnValue = _PyLong_AsInt(pValue_);
			Py_DECREF(pValue_);
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

