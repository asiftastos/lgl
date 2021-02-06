newaction {
    trigger     = "clean",
    description = "Clean the software",
    execute     = function()
		os.rmdir("build")
    end
}

solution "LearningGL"
	configurations {
		"Debug",
		"Release"
	}
	location ("build")

	project "lgl"
		targetname "lgl"
		language "C"
		kind "SharedLib"
		flags {
			"ExtraWarnings",
			"No64BitChecks",
			"StaticRuntime"
		}
		implibdir "build"

		includedirs {
			"libs/glad", "libs/glfw", "libs/cglm/include/cglm",
			"libs/stb", "src"
		}

		files {
			"libs/glad/glad.c", "libs/glfw/context.c", 
			"libs/glfw/init.c", "libs/glfw/input.c", "libs/glfw/monitor.c",
			"libs/glfw/vulkan.c", "libs/glfw/window.c", "libs/stb/stb_ds.c",
			"src/lgl/demo.c", "src/lgl/shader.c", "libs/stb/stb_rect_pack.c",
			"libs/stb/stb_truetype.c"
		}

		buildoptions {
			"-m64", 
			"-Wno-unused-parameter", 
			"-Wno-unused-function",
			"-Wno-missing-field-initializers",
			"-Wno-sign-compare"
		}

		configuration "windows"
			defines { "_WIN32", "_GLFW_WIN32"}

			files {
				"libs/glfw/win32_*.c", "libs/glfw/wgl_context.c", "libs/glfw/osmesa_context.c",
				"libs/glfw/egl_context.c"
			}

			links { "gdi32" }
		
		configuration "Debug"
			targetsuffix "d"
			defines     { "_DEBUG" }
			flags       { "Symbols" }

		configuration "Release"
			defines     { "NDEBUG" }
			flags       { "OptimizeSize" }
		
		configuration {}


	project "lglui"
		targetname "lglui"
		language "C"
		kind "SharedLib"
		flags {
			"ExtraWarnings",
			"No64BitChecks",
			"StaticRuntime"
		}
		implibdir "build"

		includedirs {
			"src/lglui",
			"libs/glad",
			"libs/glfw",
			"libs/stb", 
			"libs/nanovg"
		}

		files {
			"src/lglui/demoui.c",
			"libs/nanovg/nanovg.c"
		}

		buildoptions {
			"-m64",
			"-Wno-unused-parameter",
			"-Wno-unused-function",
			"-Wno-missing-field-initializers",
			"-Wno-sign-compare"
		}

		links {"lgl"}

		configuration "windows"
			defines { "_WIN32" }

		configuration "Debug"
			targetsuffix "d"
			defines     { "_DEBUG" }
			flags       { "Symbols" }

		configuration "Release"
			defines     { "NDEBUG" }
			flags       { "OptimizeSize" }
		
		configuration {}


dofile("src/demos.lua")
