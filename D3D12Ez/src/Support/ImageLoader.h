#pragma once

#include "WinInclude.h"

#include <vector>
#include <filesystem>

class ImageLoader 
{
public:
	struct ImageData
	{
		std::vector<char> Data;
		uint32_t Width;
		uint32_t Height;
		uint32_t BitsPerPixel;
		uint32_t ChannelCount;
		
		GUID WicPixelFormat;
		DXGI_FORMAT GiPixelFormat;
	};

	static bool LoadImageFromDisk(const std::filesystem::path& imagePath, ImageData& data);

private:
	struct GUID_to_DXGI 
	{
		GUID Wic;
		DXGI_FORMAT Gi;
	};

	static const std::vector<GUID_to_DXGI> s_lookupTable;

private:
	ImageLoader() = default;
	ImageLoader(const ImageLoader&) = default;
	ImageLoader& operator=(const ImageLoader&) = default;
};
