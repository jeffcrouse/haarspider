solution "HaarSpider" 
	configurations { "Debug", "Release" }
	
-- A project defines one build target
project "haarspider"
	kind "ConsoleApp"
	language "C++"
	files { "src/*.h", "src/*.cpp" }
	links { 
		"pcrecpp", "curl", "opencv_core",  "opencv_highgui", 
		"opencv_imgproc", "opencv_objdetect"}
	libdirs { 
		"/opt/local/lib", 
		"/usr/local/lib", 
		os.findlib("opencv_core"),
	}
	includedirs { 
		"/opt/local/include",
		"/usr/local/include",
		"/usr/include"
	}
	
configuration "Debug"
	defines { "DEBUG" }
	flags { "Symbols" }
	
configuration "Release"
	defines { "NDEBUG" }
	flags { "Optimize" }  