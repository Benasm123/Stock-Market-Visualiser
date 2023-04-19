#include "pcHeader.h"
#include "PythonUtilities.h"

#include <filesystem>
#include <iostream>
#include <map>

// Initialise a python module using a config to load all needed file paths.
PythonCaller::PythonCaller()
{
	LOG_FUNC_START();

	if (Py_IsInitialized())
	{
		return;
	}

	const std::filesystem::path currentPath = std::filesystem::absolute(".");
	const std::filesystem::path pythonPath = currentPath.parent_path() / "ext" / "Python39_64";
	const std::filesystem::path pythonDLLPath = pythonPath / "DLLs";
	const std::filesystem::path pythonLibPath = pythonPath / "Lib";
	const std::filesystem::path moduleFilePath = std::filesystem::current_path() / "Models" / "Python";
	const std::filesystem::path sitePackagesPath = pythonLibPath / "site-packages";
	
	const std::wstring p = currentPath.wstring() + L";" + pythonPath.wstring() + L";" + pythonDLLPath.wstring() + L";" + pythonLibPath.wstring()+ L";" + moduleFilePath.wstring() + L";" + sitePackagesPath.wstring();
	Py_SetPath(p.c_str());

	Py_Initialize();

	PyEval_SaveThread();

	LOG_FUNC_END();
}

PythonCaller::~PythonCaller()
{

	save_ = PyGILState_Ensure();
	Py_Finalize();

}

// Load a new python module/file.
void PythonCaller::LoadNewModule(const std::string name)
{
	LOG_FUNC_START();

	LOG_INFO("Loading python module: %s", name.c_str());

	// if haven't already loaded this module Load the folder into the python module path to make it visible to python.
	if (!loadedModules_.contains(name))
	{
		save_ = PyGILState_Ensure();

		PyObject* sysPath = PySys_GetObject("path");
		const std::filesystem::path moduleFilePath = std::filesystem::current_path() / "Models" / "Python" / name;
		PyList_Append(sysPath, PyUnicode_FromString(moduleFilePath.string().c_str()));
		loadedModules_[name] = PyImport_ImportModule(name.c_str());
		PyErr_Print();
		Py_CLEAR(sysPath);

		PyGILState_Release(save_);

		if ( !loadedModules_[name] )
		{
			LOG_ERROR("Failed Loading python module! %s", name.c_str());
		}

	}

	// Check if this module is already loaded, if so we do not need to reload it, just need to get it and set as module.
	selectedModule_ = name;

	LoadObject(name);

	LOG_FUNC_END();
}

void PythonCaller::LoadObject(const std::string name)
{
	LOG_FUNC_START();

	if (!loadedObjects_.contains(name))
	{
		save_ = PyGILState_Ensure();

		PyObject* pObjectClass = PyObject_GetAttrString(loadedModules_[selectedModule_], "StockPredictor");
		loadedObjects_[name] = PyObject_CallObject(pObjectClass, nullptr);
		PyErr_Print();
		Py_CLEAR(pObjectClass);

		PyGILState_Release(save_);

		if ( !loadedModules_[name] )
		{
			LOG_ERROR("Failed Loading python module! %s", name.c_str());
		}
	}


	LOG_FUNC_END();
}

void PythonCaller::ReloadModule(const std::string& name)
{
	if (!loadedModules_.contains(name))
	{
		LOG_WARN("Trying to reload a module which is not loaded! this should not happen and is likely a bug!");
	}

	PyImport_ReloadModule(loadedModules_[name]);
	PyErr_Print();
}

// Helper function to call the python interface with needed inputs.
bool PythonCaller::SaveWeights(const std::string& saveName)
{
	if ( loadedModules_.contains(selectedModule_) && loadedObjects_.contains(selectedModule_) )
	{
		save_ = PyGILState_Ensure();
		PyObject* pValue = PyObject_CallMethod(loadedObjects_[selectedModule_], "SaveModel", "s", saveName.c_str());
		PyErr_Print();

		if (pValue)
		{
			const bool returnValue = PyLong_AsLong(pValue) == 1l;
			Py_CLEAR(pValue);
			PyGILState_Release(save_);
			return returnValue;
		}

		Py_CLEAR(pValue);

		LOG_WARN("CANT FIND FUNCTION");
		PyErr_Print();
		PyGILState_Release(save_);
		return false;
	}
	LOG_WARN("CANT FIND MODULE");
	PyErr_Print();
	return false;
}

// Helper function to call the python interface with needed inputs.
bool PythonCaller::LoadWeights(const std::string& loadName)
{
	if ( loadedModules_.contains(selectedModule_) && loadedObjects_.contains(selectedModule_) )
	{
		save_ = PyGILState_Ensure();

		PyObject_CallMethod(loadedObjects_[selectedModule_], "LoadModel", "s", loadName.c_str());
		PyErr_Print();

		// if (pValue)
		{
			// const bool returnValue = PyLong_AsLong(pValue) == 1l;
			// Py_CLEAR(pValue);
			PyGILState_Release(save_);
			return true;
		}

		// Py_CLEAR(pValue);

		LOG_WARN("CANT FIND FUNCTION");
		PyErr_Print();
		PyGILState_Release(save_);
		return false;
	}
	LOG_WARN("CANT FIND MODULE");
	PyErr_Print();
	return false;
}

// Helper function to call the python interface with needed inputs.
bool PythonCaller::Initialise()
{
	if ( loadedModules_.contains(selectedModule_) && loadedObjects_.contains(selectedModule_) )
	{
		save_ = PyGILState_Ensure();
		PyObject* pValue = PyObject_CallMethod(loadedObjects_[selectedModule_], "Initialise", "");

		if (pValue)
		{
			const bool returnValue = PyLong_AsLong(pValue) == 1l;
			Py_CLEAR(pValue);
			PyGILState_Release(save_);
			return returnValue;
		}

		Py_CLEAR(pValue);

		LOG_WARN("CANT FIND FUNCTION");
		PyErr_Print();
		PyGILState_Release(save_);
		return false;
	}
	LOG_WARN("CANT FIND MODULE");
	PyErr_Print();
	return false;
}

// Helper function to call the python interface with needed inputs.
std::thread* PythonCaller::Train(const int epochs)
{
	if ( loadedModules_.contains(selectedModule_) && loadedObjects_.contains(selectedModule_))
	{
		auto function = [this](const std::string& object, const int epoch) -> void
		{
			save_ = PyGILState_Ensure();
			// Py_BEGIN_ALLOW_THREADS
			PyObject_CallMethod(loadedObjects_[object], "Train", "i", epoch);
			// Py_END_ALLOW_THREADS
			LOG_INFO("EXIT EARLY");
			PyErr_Print();
			PyGILState_Release(save_);
		};

		std::thread* thread = new std::thread(function, selectedModule_, epochs);

		return thread;
	}

	return nullptr;
}

// Helper function to call the python interface with needed inputs.
std::map<std::string, float> PythonCaller::GetHyperParameters()
{
	if ( loadedModules_.contains(selectedModule_) && loadedObjects_.contains(selectedModule_) )
	{
		save_ = PyGILState_Ensure();
		PyObject* pValue = PyObject_CallMethod(loadedObjects_[selectedModule_], "Initialise", "");

		if (pValue)
		{
			std::map<std::string, float> returnValue{};
			Py_CLEAR(pValue);
			PyGILState_Release(save_);
			return returnValue;
		}

		Py_CLEAR(pValue);

		LOG_WARN("CANT FIND FUNCTION");
		PyErr_Print();
		PyGILState_Release(save_);
		return {};
	}
	LOG_WARN("CANT FIND MODULE");
	PyErr_Print();
	return {};
}

// Helper function to call the python interface with needed inputs.
float PythonCaller::Evaluate(std::vector<float> vals)
{
	if ( loadedModules_.contains(selectedModule_) && loadedObjects_.contains(selectedModule_) )
	{
		save_ = PyGILState_Ensure();
		PyObject* pValue = PyObject_CallMethod(loadedObjects_[selectedModule_], "EvaluateModel", ("(" + std::string(vals.size(), 'f') + ")").c_str(), vals.data());

		if ( pValue )
		{
			const float returnValue = static_cast<float>(PyFloat_AsDouble(pValue));
			Py_CLEAR(pValue);
			PyGILState_Release(save_);
			return returnValue;
		}

		Py_CLEAR(pValue);

		LOG_WARN("CANT FIND FUNCTION");
		PyErr_Print();
		PyGILState_Release(save_);
		return 0.0f;
	}
	LOG_WARN("CANT FIND MODULE");
	PyErr_Print();
	return 0.0f;
}

// Helper function to call the python interface with needed inputs.
std::vector<int> PythonCaller::Predict(const std::string& stockName, const std::string& startDate, const std::string& endDate)
{
	static bool onlyOnce = true;
	if ( loadedModules_.contains(selectedModule_) and loadedObjects_.contains(selectedModule_) )
	{
		save_ = PyGILState_Ensure();

		PyObject* pyStockName = PyUnicode_FromString(stockName.c_str());
		PyObject* pyStartDate = PyUnicode_FromString(startDate.c_str());
		PyObject* pyEndDate = PyUnicode_FromString(endDate.c_str());

		PyObject* pXVec = PyTuple_Pack(3, pyStockName, pyStartDate, pyEndDate);

		PyErr_Print();
		PyObject* name = PyUnicode_FromString("MakePrediction");
		PyErr_Print();
		PyObject* pListValue = PyObject_CallMethodOneArg(loadedObjects_[selectedModule_], name, pXVec);
		PyErr_Print();

		Py_CLEAR(name);
		Py_CLEAR(pyStockName);
		Py_CLEAR(pyStartDate);
		Py_CLEAR(pyEndDate);
		Py_CLEAR(pXVec);

		if (!pListValue)
		{
			PyErr_Print();
			PyGILState_Release(save_);
			return {};
		}

		if (PyList_Check(pListValue))
		{
			std::vector<int> vals(PyList_Size(pListValue));
			for (int i = 0; i < PyList_Size(pListValue); ++i)
			{
				PyObject* val = PyList_GetItem(pListValue, i);

				vals[i] = _PyLong_AsInt(val);

				Py_CLEAR(val);
			}
			PyGILState_Release(save_);
			return vals;
		}

		if ( pListValue )
		{
			const int returnValue = _PyLong_AsInt(pListValue);
			Py_CLEAR(pListValue);
			PyGILState_Release(save_);
			return {returnValue};
		}

		Py_CLEAR(pListValue);

		if ( onlyOnce )
		{
			LOG_WARN("CANT FIND FUNCTION");
			PyErr_Print();
			onlyOnce = false;
		}
		PyGILState_Release(save_);
		return {};
	}

	if (onlyOnce)
	{
		LOG_WARN("CANT FIND MODULE");
		PyErr_Print();
		onlyOnce = false;
	}
	return {};
}

std::vector<float> PythonCaller::GetAccuracyList()
{
	return ListToVectorFromFunction("GetAccuracyValues");
}

std::vector<float> PythonCaller::GetLossList()
{
	return ListToVectorFromFunction("GetLossValues");
}

std::vector<float> PythonCaller::GetRewardList()
{
	return ListToVectorFromFunction("GetRewardValues");
}

std::vector<float> PythonCaller::ListToVectorFromFunction(const std::string& functionName)
{
	if ( !loadedObjects_.contains(selectedModule_) or !loadedModules_.contains(selectedModule_) )
	{
		LOG_ERROR("No python object or module loaded! Load an algorithm before getting values!");
		return {};
	}

	save_ = PyGILState_Ensure();

	PyObject* name = PyUnicode_FromString(functionName.c_str());

	PyObject* pListValue = PyObject_CallMethodNoArgs(loadedObjects_[selectedModule_], name);

	Py_CLEAR(name);

	if ( !PyList_Check(pListValue) )
	{
		LOG_ERROR("Expected return type from python function: List. Got non list type!");
		PyGILState_Release(save_);
		return {};
	}

	std::vector<float> values(PyList_Size(pListValue));
	for ( int i = 0; i < PyList_Size(pListValue); ++i )
	{
		PyObject* value = PyList_GetItem(pListValue, i);

		values[i] = static_cast<float>(PyFloat_AsDouble(value));

		Py_CLEAR(value);
	}

	PyGILState_Release(save_);
	return values;
}
