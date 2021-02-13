project "demo2d"
    targetname "dm2d"
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
    targetname "dmtext"
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
		"../libs/stb", "../src"
	}
	files {
		"demotext/main.c"
	}
	buildoptions {
		"-m64",
		"-Wno-unused-parameter",
		"-Wno-unused-function",
		"-Wno-missing-field-initializers",
		"-Wno-sign-compare"
    }
    libdirs { "../build" }
	links {"lgl"}
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
    targetname "dmht"
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
	links {"lgl"}
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
    targetname "dmsandsim"
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


project "demorayui"
    targetname "dmrayui"
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
		"demorayui/main.c"
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

project "demotexturing"
    targetname "dmtextures"
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
		"demotexture/main.c"
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

project "demomicroui"
    targetname "dmmui"
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
		"demomicroui/main.c",
		"demomicroui/microui.c"
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
