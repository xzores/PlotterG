--premake5.lua

workspace "PlotterG"
   configurations { "Debug","Release"}
   architecture ("x86_64")
   flags { "MultiProcessorCompile" }

   project "PlotterG"
      kind "ConsoleApp"
      language "C++"
      location "%{prj.name}"
      output_dir = "%{prj.name}/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
      targetdir ("bin/" .. output_dir)
      objdir ("bin-int/" .. output_dir)

      includedirs {
         "Libs/serial/include",
         "Libs/glfw/include",
         "Libs/glew/include"
      }
            
      libdirs {
         "Libs/serial/link",
         "Libs/glfw/link",
         "Libs/glew/link"
      }

      links {
         "opengl32",
         "glew32s",
         "glfw3",
         "serial",
         --"msvcrt",
         --"msvcmrt"
      }

      files {
         "%{prj.name}/**.h",
         "%{prj.name}/**.hpp",
         "%{prj.name}/**.c",
         "%{prj.name}/**.cpp",
         "Libs/serialib/src/**.cpp"
      }

      filter "system:windows"
         cppdialect "C++17"
         --staticruntime "On"
         systemversion "latest"
         defines { "WINDOWS" }
   
