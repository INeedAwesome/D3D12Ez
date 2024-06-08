project "D3D12Ez"
   kind "ConsoleApp"
   language "C++"
   cppdialect "C++20"
   staticruntime "off"
   systemversion "latest"
  
   files 
   { 
      "src/**.h", 
      "src/**.cpp", 
      "src/**.hlsli", 
      "src/**.hlsl" 
   }
   
   targetdir ("../bin/" .. outputdir .. "-%{prj.name}")
   objdir ("../bin-int/" .. outputdir .. "-%{prj.name}")

--   pchheader "stdafx.h"
--   pchsource "../%{prj.name}/src/stdafx.cpp"

   files { "src/**.h", "src/**.cpp", "src/**.hlsli", "src/**.hlsl" }
   
   targetdir ("../bin/" .. outputdir .. "-%{prj.name}")
   objdir ("../bin-int/" .. outputdir .. "-%{prj.name}")

   debugdir "../"
   
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
   
   defines { "EZ_PLATFORM_WINDOWS" }

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
      defines { "EZ_DIST" }
      runtime "Release"
      optimize "On"
      symbols "Off"