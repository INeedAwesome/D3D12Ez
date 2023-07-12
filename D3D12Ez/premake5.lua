project "D3D12Ez"
   kind "ConsoleApp"
   language "C++"
   cppdialect "C++20"
   staticruntime "off"

--   pchheader "stdafx.h"
--   pchsource "../%{prj.name}/src/stdafx.cpp"

   files { "src/**.h", "src/**.cpp" }
   
   targetdir ("$(SolutionDir)bin\\" .. outputdir .. "-%{prj.name}")
   objdir ("$(SolutionDir)bin-int\\" .. outputdir .. "-%{prj.name}")
   
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

   filter "system:windows"
      systemversion "latest"
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