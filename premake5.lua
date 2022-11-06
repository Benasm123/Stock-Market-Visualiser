workspace "DataVisualizer"
	architecture "x64"

	configurations {
		"DEBUG",
		"TEST",
		"RELEASE"
	}

	flags {
		"MultiProcessorCompile"
	}


outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

VulkanInclude = "DataVisualizer/ext/Vulkan/Include"
VulkanLib = "DataVisualizer/ext/Vulkan/Lib"
GLMInclude = "DataVisualizer/ext/glm"

project "DataVisualizer"
	location "DataVisualizer"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "pcHeader.h"
	pchsource "DataVisualizer/src/pcHeader.cpp"

	files {
		"%{prj.name}//**.h",
		"%{prj.name}/src/**.hpp",
		"%{prj.name}/src/**.cpp"
	}

	includedirs {
		"DataVisualizer/src",
		VulkanInclude,
		GLMInclude
	}

	libdirs {
		VulkanLib
	}

	links {
		"vulkan-1"
	}

	filter "system:windows"
		systemversion "latest"

		defines {
			"USING_PLATFORM_WINDOWS"
		}

	filter "configurations:DEBUG"
		defines "DV_DEBUG"
		runtime "Debug"
		symbols "On"

	filter "configurations:TEST"
		defines "DV_TEST"
		runtime "Release"
		optimize "On"

	filter "configurations:RELEASE"
		defines "DV_RELEASE"
		runtime "Release"
		optimize "On"

	

project "Test"
	location "Test"
	kind "ConsoleApp"
	language "C++"
	cppdialect "c++20"
	staticruntime "off"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files {
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.hpp",
		"%{prj.name}/src/**.cpp"
	}

	includedirs {
		"DataVisualizer/src",
		VulkanInclude,
		GLMInclude
	}

	links {
		"DataVisualizer"
	}

	filter "system:windows"
		systemversion "latest"

	filter "configurations:DEBUG"
		defines "DV_DEBUG"
		runtime "Debug"
		symbols "On"

	filter "configurations:TEST"
		defines "DV_TEST"
		runtime "Release"
		optimize "On"

	filter "configurations:RELEASE"
		defines "DV_RELEASE"
		runtime "Release"
		optimize "On"