#pragma once

// DEFINES
#define VK_USE_PLATFORM_WIN32_KHR
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

// INCLUDES
#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "glm.hpp"
#include "ext/matrix_transform.hpp"
#include "ext/matrix_clip_space.hpp"
#include "vulkan/vulkan.hpp"
#include "windowsx.h"
#include <fstream>
#include <iostream>

// CONSTEXPR
constexpr auto ENGINE_NAME = "DataVisualizer";
constexpr auto APPLICATION_VERSION = 1;
constexpr auto ENGINE_VERSION = 1;

// ENUMS
enum log_type
{
	l_error = 4,
	l_warn = 14,
	l_info = 10,
	l_vulk = 13,
	l_func_start = 11,
	l_func_end = 12
};

// LOGGING TODO::MOVE TO CLASS
#define CONSOLE_COLOR(ConsoleColors) (SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), (ConsoleColors)))

inline void StartLogMessage(const log_type type, const int line_number, const char* file_name)
{
	CONSOLE_COLOR(type);
	if (type == l_func_start || type == l_func_end)
	{
		CONSOLE_COLOR(l_info);
	}

	static int tabs = 0;
	if (tabs > 0)
	{
		int i = 0;
		if (type == l_func_end) i = 1;
		for (i; i < tabs; i++)
		{
			std::cout << "|";
			std::cout << "   ";
		}
	}
	switch (type)
	{
	case l_error:
		printf("[ERRO] file: %s    line: %i\n message: ", file_name, line_number);
		break;
	case l_warn:
		printf("[WARN] file: %s    line: %i\n message: ", file_name, line_number);
		break;
	case l_info:
		printf("[INFO] ");
		break;
	case l_vulk:
		printf("[VULK] ");
		break;
	case l_func_start:
		tabs++;
		printf("[STRT] ");
		break;
	case l_func_end:
		tabs--;
		// std::cout << std::string(tabs * 4, ' ');
		printf("[END_]\t");
	}
}

inline void EndLogMessage(const char* vars...)
{
	printf(vars);
	printf("\n");

	CONSOLE_COLOR(7);
}

// MACROS
#ifdef DV_RELEASE

#define LOG_VULK(...)
#define LOG_INFO(...)
#define LOG_WARN(...)
#define LOG_ERROR(...)

#define LOG_FUNC_START()
#define LOG_FUNC_END()

#else

#define LOG_VULK(...) StartLogMessage(l_vulk, __LINE__, __FILE__); EndLogMessage(__VA_ARGS__)
#define LOG_INFO(...) StartLogMessage(l_info, __LINE__, __FILE__); EndLogMessage(__VA_ARGS__)
#define LOG_WARN(...) StartLogMessage(l_warn, __LINE__, __FILE__); EndLogMessage(__VA_ARGS__)
#define LOG_ERROR(...) StartLogMessage(l_error, __LINE__, __FILE__); EndLogMessage(__VA_ARGS__)

#define LOG_FUNC_START() StartLogMessage(l_func_start, __LINE__, __FILE__); EndLogMessage("%s", __FUNCTION__)
#define LOG_FUNC_END() StartLogMessage(l_func_end, __LINE__, __FILE__); EndLogMessage("%s", __FUNCTION__)


#endif



struct shader_info
{
	vk::ShaderStageFlagBits type{};
	const char* file_name{};
};

struct graphics_pipeline_create_info
{
	vk::Device logical_device{};
	std::vector<shader_info> shader_infos{};
	vk::PipelineLayout pipeline_layout{};
	vk::RenderPass render_pass{};
	vk::PrimitiveTopology primitive_topology{};
	vk::PolygonMode polygon_mode{};
	vk::CullModeFlagBits cull_mode{};
	std::vector<vk::Viewport> viewports{};
	std::vector<vk::Rect2D> scissors{};
	std::vector<vk::DynamicState> dynamic_states{};
};

namespace PMATH
{
	// STRUCTS
	template <typename T = float>
	struct vec2
	{
		T x{};
		T y{};
	};

	template <typename T = float>
	struct vec3
	{
		T x{};
		T y{};
		T z{};
	};

	struct push_constant
	{
		glm::vec4 color{ 1.0f };
		glm::mat4 mvp{};
	};

	struct uniform_buffer_object
	{
		glm::mat4 mvp;
	};

	struct vertex
	{
		vec2<float> position;
	};

	struct transform
	{
		glm::vec2 position{ 0.0f };
		float depth{ 1.0f };
		float rotation{ 0.0f };
		glm::vec2 scale{ 1.0f, 1.0f };
	};
}