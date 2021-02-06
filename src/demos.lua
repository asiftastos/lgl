project "demo2d"
    targetname "demo2d"
    targetdir "../"
	language "C"
    kind "ConsoleApp"
	flags {
		"ExtraWarnings",
		"No64BitChecks",
		"StaticRuntime"
	}
	includedirs {
		"../libs/glad",
		"../libs/glfw",
		"../libs/cglm/include/cglm", 
		"../libs/stb",
		"../libs/nanovg",
		"../src"
	}
	files {
		"demo2d/main.c"
	}
	buildoptions {
		"-m64",
		"-Wno-unused-parameter",
		"-Wno-unused-function",
		"-Wno-missing-field-initializers",
		"-Wno-sign-compare"
    }
    libdirs { "../build" }
	links {"lgl", "lglui"}
	configuration "windows"
		defines { "_WIN32", "_GLFW_WIN32"}
		links { "gdi32" }
	
	configuration "Debug"
		targetsuffix "d"
		defines     { "_DEBUG" }
        flags       { "Symbols" }
	configuration "Release"
		defines     { "NDEBUG" }
        flags       { "OptimizeSize" }
	
	configuration {}

project "demotext"
    targetname "demotext"
    targetdir "../"
	language "C"
    kind "ConsoleApp"
	flags {
		"ExtraWarnings",
		"No64BitChecks",
		"StaticRuntime"
	}
	includedirs {
		"../libs/glad", "../libs/glfw", "../libs/cglm/include/cglm", 
		"../libs/stb", "../libs/nanovg", "../src"
	}
	files {
		"demotext/main.c"
	}
	buildoptions {
		"-m64",
    }
    libdirs { "../build" }
	links {"Common"}
	configuration "windows"
		defines { "_WIN32", "_GLFW_WIN32"}
		links { "gdi32" }
	
	configuration "Debug"
		targetsuffix "d"
		defines     { "_DEBUG" }
        flags       { "Symbols" }
	configuration "Release"
		defines     { "NDEBUG" }
        flags       { "OptimizeSize" }
	
	configuration {}

project "demohashtable"
    targetname "demoht"
    targetdir "../"
	language "C"
    kind "ConsoleApp"
	flags {
		"ExtraWarnings",
		"No64BitChecks",
		"StaticRuntime"
	}
	includedirs {
		"../libs/glad", "../libs/glfw", "../libs/cglm/include/cglm", 
		"../libs/stb", "../libs/nanovg", "../src"
	}
	files {
		"demohashtable/main.c"
	}
	buildoptions {
		"-m64", "-Wno-unused-function"
    }
    libdirs { "../build" }
	links {"Common"}
	configuration "windows"
		defines { "_WIN32", "_GLFW_WIN32"}
		links { "gdi32" }
	
	configuration "Debug"
		targetsuffix "d"
		defines     { "_DEBUG" }
        flags       { "Symbols" }
	configuration "Release"
		defines     { "NDEBUG" }
        flags       { "OptimizeSize" }
	
	configuration {}

project "demosandsim"
    targetname "demosm"
    targetdir "../"
	language "C"
    kind "ConsoleApp"
	flags {
		"ExtraWarnings",
		"No64BitChecks",
		"StaticRuntime"
	}
	includedirs {
		"../libs/glad", "../libs/glfw", "../libs/cglm/include/cglm", 
		"../libs/stb", "../libs/nanovg", "../src"
	}
	files {
		"demosandsim/main.c"
	}
	buildoptions {
		"-m64", "-Wno-unused-function", "-Wno-unused-parameter"
    }
	libdirs { "../build" }
	links {"Common"}
	configuration "windows"
		defines { "_WIN32", "_GLFW_WIN32"}
		links { "gdi32" }
	
	configuration "Debug"
		targetsuffix "d"
		defines     { "_DEBUG" }
        flags       { "Symbols" }
        
	configuration "Release"
		defines     { "NDEBUG" }
        flags       { "OptimizeSize" }
	
	configuration {}
