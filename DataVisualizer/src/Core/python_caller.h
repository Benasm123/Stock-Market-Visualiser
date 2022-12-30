#pragma once

class python_caller
{
public:
	python_caller(const std::string& name);
	
	long call_function(std::string);

private:
	PyObject* p_name_;
	PyObject* p_module_;
	PyObject* p_dict_;
	PyObject* p_func_;
	PyObject* p_value_;
};

