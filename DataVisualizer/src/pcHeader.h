#pragma once

// DEFINES
#define VK_USE_PLATFORM_WIN32_KHR
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

// INCLUDES
#define PY_SSIZE_T_CLEAN
#undef Py_REF_DEBUG
#include <Python.h>
#include "glm.hpp"
#include "ext/matrix_transform.hpp"
#include "ext/matrix_clip_space.hpp"
#include "vulkan/vulkan.hpp"
#include "windowsx.h"
#include <fstream>
#include <iostream>

// CONSTEXPR
constexpr auto kEngineName = "DataVisualizer";
constexpr auto kApplicationVersion = 1;
constexpr auto kEngineVersion = 1;

// ENUMS
enum LogType
{
	kLError = 4,
	kLWarn = 14,
	kLInfo = 10,
	kLVulk = 13,
	kLFuncStart = 11,
	kLFuncEnd = 12
};

// LOGGING TODO::MOVE TO CLASS
#define CONSOLE_COLOR(ConsoleColors) (SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), (ConsoleColors)))

inline void StartLogMessage(const LogType type, const int lineNumber, const char* fileName)
{
	CONSOLE_COLOR(type);
	if (type == kLFuncStart || type == kLFuncEnd)
	{
		CONSOLE_COLOR(kLInfo);
	}

	static int tabs = 0;
	if (tabs > 0)
	{
		int i = 0;
		if (type == kLFuncEnd) i = 1;
		for (; i < tabs; i++)
		{
			std::cout << "| ";
		}
	}
	switch (type)
	{
	case kLError:
		printf("[ERRO] file: %s    line: %i\n message: ", fileName, lineNumber);
		break;
	case kLWarn:
		printf("[WARN] file: %s    line: %i\n message: ", fileName, lineNumber);
		break;
	case kLInfo:
		printf("[INFO] ");
		break;
	case kLVulk:
		printf("[VULK] ");
		break;
	case kLFuncStart:
		tabs++;
		printf("[STRT] ");
		break;
	case kLFuncEnd:
		tabs--;
		printf("[END_] ");
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

#define LOG_VULK(...) StartLogMessage(kLVulk, __LINE__, __FILE__); EndLogMessage(__VA_ARGS__)
#define LOG_INFO(...) StartLogMessage(kLInfo, __LINE__, __FILE__); EndLogMessage(__VA_ARGS__)
#define LOG_WARN(...) StartLogMessage(kLWarn, __LINE__, __FILE__); EndLogMessage(__VA_ARGS__)
#define LOG_ERROR(...) StartLogMessage(kLError, __LINE__, __FILE__); EndLogMessage(__VA_ARGS__)

#define LOG_FUNC_START() StartLogMessage(kLFuncStart, __LINE__, __FILE__); EndLogMessage("%s", __FUNCTION__)
#define LOG_FUNC_END() StartLogMessage(kLFuncEnd, __LINE__, __FILE__); EndLogMessage("%s", __FUNCTION__)


#endif



struct ShaderInfo
{
	vk::ShaderStageFlagBits type{};
	const char* fileName{};
};

struct GraphicsPipelineCreateInfo
{
	vk::Device logicalDevice{};
	std::vector<ShaderInfo> shaderInfos{};
	vk::PipelineLayout pipelineLayout{};
	vk::RenderPass renderPass{};
	vk::PrimitiveTopology primitiveTopology{};
	vk::PolygonMode polygonMode{};
	vk::CullModeFlagBits cullMode{};
	std::vector<vk::Viewport> viewports{};
	std::vector<vk::Rect2D> scissors{};
	std::vector<vk::DynamicState> dynamicStates{};
};

namespace dv_math
{
	// STRUCTS
	template <typename T = float>
	struct Vec2
	{
		T x{};
		T y{};
	};

	template <typename T = float>
	struct Vec3
	{
		T x{};
		T y{};
		T z{};
	};

	struct PushConstant
	{
		glm::vec4 color{ 1.0f };
		glm::mat4 mvp{};
	};

	struct UniformBufferObject
	{
		glm::mat4 mvp;
	};

	struct Vertex
	{
		Vec2<float> position;
	};

	struct Transform
	{
		glm::vec2 position{ 0.0f };
		float depth{ 1.0f };
		float rotation{ 0.0f };
		glm::vec2 scale{ 1.0f, 1.0f };
	};
}