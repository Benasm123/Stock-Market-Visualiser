#include "pcHeader.h"
#include "python_caller.h"

python_caller::python_caller(const std::string& name)
{
	Py_Initialize();

	p_name_ = PyUnicode_DecodeFSDefault(name.c_str());
	
	if (!p_name_)
	{
		LOG_ERROR("COULDNT FIND PYTHON PROGRAM");
		exit(1);
	}
	
	p_module_ = PyImport_Import(p_name_);
	Py_DECREF(p_name_);
}


long python_caller::call_function(const std::string function_name)
{
	if (p_module_)
	{
		p_func_ = PyObject_GetAttrString(p_module_, function_name.c_str());

		if (p_func_ && PyCallable_Check(p_func_))
		{
			p_value_ = PyObject_CallObject(p_func_, nullptr);
			if (p_value_)
			{
				printf("Result of call: %ld\n", PyLong_AsLong(p_value_));
				return PyLong_AsLong(p_value_);
			}
		}
		LOG_WARN("CANT FIND FUNCTION");
		PyErr_Print();
		return 0;
	}
	LOG_WARN("CANT FIND MODULE");
	PyErr_Print();
	return 0;
}