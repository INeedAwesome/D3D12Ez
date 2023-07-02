workspace "D3D12Ez"
   architecture "x64"
   configurations { "Debug", "Release", "Dist"}
   startproject "D3D12Ez"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

include "StartupScreen"
include "D3D12Ez"