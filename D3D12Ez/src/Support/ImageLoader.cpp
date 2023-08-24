#include "ImageLoader.h"

#include "ComPointer.h"

#include <algorithm>

const std::vector<ImageLoader::GUID_to_DXGI> ImageLoader::s_lookupTable =
{
	{ GUID_WICPixelFormat32bppBGRA, DXGI_FORMAT_B8G8R8A8_UNORM },
	{ GUID_WICPixelFormat32bppRGBA, DXGI_FORMAT_R8G8B8A8_UNORM }
};

bool ImageLoader::LoadImageFromDisk(const std::filesystem::path& imagePath, ImageData& data)
{
	ComPointer<IWICImagingFactory> wicFactory;
	if (FAILED(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&wicFactory))))
		return false;

	// Load Image
	ComPointer <IWICStream > wicFileStream;
	if (FAILED(wicFactory->CreateStream(&wicFileStream)))
		return false;

	if (FAILED(wicFileStream->InitializeFromFilename(imagePath.wstring().c_str(), GENERIC_READ)))
		return false;

	ComPointer<IWICBitmapDecoder> wicDecoder;
	if (FAILED(wicFactory->CreateDecoderFromStream(wicFileStream, nullptr, WICDecodeMetadataCacheOnDemand, &wicDecoder)))
		return false;

	ComPointer<IWICBitmapFrameDecode> wicFrameDecoder;
	if (FAILED(wicDecoder->GetFrame(0, &wicFrameDecoder)))
		return false;

	// Trivial Image MetaData
	if (FAILED(wicFrameDecoder->GetSize(&data.Width, &data.Height)))
		return false;
	if (FAILED(wicFrameDecoder->GetPixelFormat(&data.WicPixelFormat)))
		return false;

	// metadata of pixel format

	ComPointer<IWICComponentInfo> wicComponentInfo;
	if (FAILED(wicFactory->CreateComponentInfo(data.WicPixelFormat, &wicComponentInfo)))
		return false;

	ComPointer<IWICPixelFormatInfo> wicPixelFormatInfo;
	if (FAILED(wicComponentInfo->QueryInterface(&wicPixelFormatInfo)))
		return false;


	// Get Data to struct

	if (FAILED(wicPixelFormatInfo->GetBitsPerPixel(&data.BitsPerPixel))) return false;
	if (FAILED(wicPixelFormatInfo->GetChannelCount(&data.ChannelCount))) return false;

	// DXGI PIXEL FORMAT
	auto findIt = std::find_if(s_lookupTable.begin(), s_lookupTable.end(), [&](const GUID_to_DXGI& entry)
		{
			return memcmp(&entry.Wic, &data.WicPixelFormat, sizeof(GUID)) == 0;
		}
	);
	if (findIt == s_lookupTable.end())
	{
		return false;
	}
	data.GiPixelFormat = findIt->Gi;

	// Image loading
	uint32_t stride = ((data.BitsPerPixel+7) / 8) * data.Width;
	uint32_t size = stride * data.Height;
	data.Data.resize(size);

	WICRect copyRect{};
	copyRect.X = 0;
	copyRect.Y = 0;
	copyRect.Width = data.Width;
	copyRect.Height = data.Height;

	
	if (FAILED(wicFrameDecoder->CopyPixels(&copyRect, stride, size, (BYTE*)data.Data.data()))) 
		return false;

	return true;
}
