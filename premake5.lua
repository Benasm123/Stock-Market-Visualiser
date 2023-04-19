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
PythonInclude = "ext/Python39_64/include"
imguiInclude = "ext/imgui"
VulkanLib = "DataVisualizer/ext/Vulkan/Lib"
PythonLib = "ext/Python39_64/libs"
GLMInclude = "DataVisualizer/ext/glm"
SDLInclude = "ext/SDL"

project "ImGui"
	kind "StaticLib"
	language "C++"
    staticruntime "off"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files {
		"ext/imgui/imconfig.h",
		"ext/imgui/imgui.h",
		"ext/imgui/imgui.cpp",
		"ext/imgui/imgui_draw.cpp",
		"ext/imgui/imgui_internal.h",
		"ext/imgui/imgui_tables.cpp",
		"ext/imgui/imgui_widgets.cpp",
		"ext/imgui/imstb_rectpack.h",
		"ext/imgui/imstb_textedit.h",
		"ext/imgui/imstb_truetype.h",
		"ext/imgui/imgui_demo.cpp",
		"ext/imgui/backends/imgui_impl_win32.h",
		"ext/imgui/backends/imgui_impl_win32.cpp",
		"ext/imgui/backends/imgui_impl_vulkan.h",
		"ext/imgui/backends/imgui_impl_vulkan.cpp",
		"ext/imgui/backends/vulkan/**"
	}
	
	includedirs {
		"DataVisualizer/src",
		VulkanInclude,
		GLMInclude,
		imguiInclude,
		SDLInclude
	}

	filter "system:windows"
		systemversion "latest"

		defines {
			"USING_PLATFORM_WINDOWS"
		}

	filter "configurations:TEST"
		defines "DV_TEST"
		runtime "Release"
		optimize "On"

	filter "configurations:RELEASE"
		defines "DV_RELEASE"
		runtime "Release"
		optimize "On"

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
		GLMInclude,
		imguiInclude,
		SDLInclude,
		PythonInclude
	}

	libdirs {
		VulkanLib,
		PythonLib
	}

	links {
		"vulkan-1",
		"python39"
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

	

project "StockAid"
	location "StockAid"
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
		GLMInclude,
		imguiInclude,
		SDLInclude,
		PythonInclude
	}

	libdirs {
		PythonLib
	}

	links {
		"DataVisualizer",
		"ImGui",
		"python39"
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