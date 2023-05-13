project "D3D12Ez"
   kind "ConsoleApp"
   language "C++"
   cppdialect "C++20"
   targetdir "bin/%{cfg.buildcfg}"
   staticruntime "off"

--   pchheader "stdafx.h"
--   pchsource "../%{prj.name}/src/stdafx.cpp"

   files { "src/**.h", "src/**.cpp" }

   includedirs
   {
      "src/"
   }

   links 
   { 
      "d3d12.lib", 
      "dxgi.lib", 
      "dxguid.lib" 
   }

   targetdir ("../bin/" .. outputdir .. "-%{prj.name}")
   objdir ("../bin-int/" .. outputdir .. "-%{prj.name}")

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