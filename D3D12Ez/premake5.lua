project "D3D12Ez"
   kind "ConsoleApp"
   language "C++"
   cppdialect "C++20"
   targetdir "bin/%{cfg.buildcfg}"
   staticruntime "off"
   dependson { "StartupScreen" }

--   pchheader "stdafx.h"
--   pchsource "../%{prj.name}/src/stdafx.cpp"

   files { "src/**.h", "src/**.cpp" }
   
   targetdir ("../bin/" .. outputdir .. "-%{prj.name}")
   objdir ("../bin-int/" .. outputdir .. "-%{prj.name}")
   
   includedirs
   {
      "src/", 
      "../StartupScreen/src"
   }

   libdirs 
   {
      "$(SolutionDir)bin/" .. outputdir .. "-StartupScreen/"
   }

   links 
   { 
      "d3d12.lib", 
      "dxgi.lib", 
      "dxguid.lib",
      "StartupScreen.lib"
   }

   postbuildcommands 
   { 
      "{COPY} $(SolutionDir)bin/" .. outputdir .. "-StartupScreen/StartupScreen.dll $(SolutionDir)bin/" .. outputdir .. "-D3D12Ez"
   }

   filter "system:windows"
      systemversion "latest"
      defines { "EZ_PLATFORM_WINDOWS" }
   filter "system:linux"
      systemversion "latest"
      defines { "EZ_PLATFORM_LINUX" }

   filter "configurations:Debug"
      defines { "EZ_DEBUG" }
      runtime "Debug"
      symbols "On"

   filter "configurations:Release"
      defines { "EZ_RELEASE" }
      runtime "Release"
      optimize "On"
      symbols "Off"

   filter "configurations:Dist"
      kind "WindowedApp"
      defines { "ST_DIST" }
      runtime "Release"
      optimize "On"
      symbols "Off"