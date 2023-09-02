#include <iostream>

#include "Support/WinInclude.h"
#include "Support/ComPointer.h"
#include "Support/ImageLoader.h"
#include "Support/Window.h"
#include "Support/Shader.h"

#include "Debug/DXDebugLayer.h" 
#include "D3D/DXContext.h"
#include "Debug/Timer.h"


void pukeColor(float* color)
{
	static int pukeState = 0;
	color[pukeState] += 0.01f;
	if (color[pukeState] > 1.0f)
	{
		pukeState++;
		if (pukeState == 3)
		{
			color[0] = .0f;
			color[1] = .0f;
			color[2] = .0f;
			pukeState = 0;
		}
	}
}

#ifdef EZ_DIST
int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
#endif //EZ_DIST
#ifndef EZ_DIST
int main(int argc, char* argv[])
#endif //EZ_DIST
{
	Timer init("DirectX & Window Init");

	std::cout << "Hello World!" << std::endl;
	DXDebugLayer::Get().Init();

	if (!DXContext::Get().Init())
		return 1;

	if (!DXWindow::Get().Init())
		return 2;

	DXWindow::Get().SetFullscreen(true);

	init.StopAndPrintTime();

	Timer dxInit("DirectX Pipeline Init");

	// ====== HEAP PROPERTIES ON GPU ======

	D3D12_HEAP_PROPERTIES hpUpload{};
	{
		hpUpload.Type = D3D12_HEAP_TYPE_UPLOAD;
		hpUpload.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		hpUpload.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		hpUpload.CreationNodeMask = 0;
		hpUpload.VisibleNodeMask = 0;
	}

	D3D12_HEAP_PROPERTIES hpDefault{};
	{
		hpDefault.Type = D3D12_HEAP_TYPE_DEFAULT;
		hpDefault.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		hpDefault.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		hpDefault.CreationNodeMask = 0;
		hpDefault.VisibleNodeMask = 0;
	}

	// ====== VERTEX DATA ======

	const char* hello = "Hello World!";

	struct Vertex
	{
		float x;
		float y;
	};

	Vertex vertices[] =
	{
		{-1.0f, -1.0f},
		{0.0f, 1.0f},
		{1.0f, -1.0f}
	};

	D3D12_INPUT_ELEMENT_DESC vertexLayout[] =
	{
		{ "Position", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 } // 2 dimensional array (vertex (x, y))
	};

	// ====== TEXTURE DATA ======

	ImageLoader::ImageData textureData;
	ImageLoader::LoadImageFromDisk("./app/auge_512_512_BGRA_32BPP.png", textureData);
	uint32_t textureStride = (((textureData.BitsPerPixel + 7) / 8));
	uint32_t textureSize = textureData.Height * textureData.Width * textureStride;

	// ====== UPLOAD & VERTEX DESCRIPTORS ======

	D3D12_RESOURCE_DESC uploadResourceDesc{};
	{
		uploadResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		uploadResourceDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		uploadResourceDesc.Width = textureSize + 1024;
		uploadResourceDesc.Height = 1;
		uploadResourceDesc.DepthOrArraySize = 1;
		uploadResourceDesc.MipLevels = 1;
		uploadResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
		uploadResourceDesc.SampleDesc.Count = 1;
		uploadResourceDesc.SampleDesc.Quality = 0;
		uploadResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		uploadResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	}
	D3D12_RESOURCE_DESC vertexResourceDesc{};
	{
		vertexResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		vertexResourceDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		vertexResourceDesc.Width = 1024;
		vertexResourceDesc.Height = 1;
		vertexResourceDesc.DepthOrArraySize = 1;
		vertexResourceDesc.MipLevels = 1;
		vertexResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
		vertexResourceDesc.SampleDesc.Count = 1;
		vertexResourceDesc.SampleDesc.Quality = 0;
		vertexResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		vertexResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	}

	// ====== UPLOAD & VERTEX BUFFER ======

	ComPointer<ID3D12Resource2> uploadBuffer, vertexBuffer;

	// Create resources on GPU heap
	DXContext::Get().GetDevice()->CreateCommittedResource(
		&hpUpload,
		D3D12_HEAP_FLAG_NONE, &uploadResourceDesc, D3D12_RESOURCE_STATE_COMMON, nullptr,
		IID_PPV_ARGS(&uploadBuffer)
	);

	DXContext::Get().GetDevice()->CreateCommittedResource(
		&hpDefault,
		D3D12_HEAP_FLAG_NONE, &vertexResourceDesc, D3D12_RESOURCE_STATE_COMMON, nullptr,
		IID_PPV_ARGS(&vertexBuffer)
	);

	// ====== TEXTURE ======

	D3D12_RESOURCE_DESC textureResourceDesc{};
	{
		textureResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		textureResourceDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		textureResourceDesc.Width = textureData.Width;
		textureResourceDesc.Height = textureData.Height;
		textureResourceDesc.DepthOrArraySize = 1;
		textureResourceDesc.MipLevels = 1;
		textureResourceDesc.Format = textureData.GiPixelFormat;
		textureResourceDesc.SampleDesc.Count = 1;
		textureResourceDesc.SampleDesc.Quality = 0;
		textureResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		textureResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	}

	ComPointer<ID3D12Resource2> texture;

	DXContext::Get().GetDevice()->CreateCommittedResource(
		&hpDefault,
		D3D12_HEAP_FLAG_NONE, &textureResourceDesc, D3D12_RESOURCE_STATE_COMMON, nullptr,
		IID_PPV_ARGS(&texture)
	);
	
	// ====== COPY CPU resource to GPU resource
	D3D12_BOX textureSizeAsBox;
	textureSizeAsBox.left = textureSizeAsBox.top = textureSizeAsBox.front = 0;
	textureSizeAsBox.right = textureData.Width;
	textureSizeAsBox.bottom = textureData.Height;
	textureSizeAsBox.back = 1;
	D3D12_TEXTURE_COPY_LOCATION txtcSrc, txtcDst;
	txtcSrc.pResource = uploadBuffer;
	txtcSrc.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
	txtcSrc.PlacedFootprint.Offset = 0;
	txtcSrc.PlacedFootprint.Footprint.Width = textureData.Width;
	txtcSrc.PlacedFootprint.Footprint.Height = textureData.Height;
	txtcSrc.PlacedFootprint.Footprint.Depth = 1;
	txtcSrc.PlacedFootprint.Footprint.RowPitch = 2048;
	txtcSrc.PlacedFootprint.Footprint.Format = textureData.GiPixelFormat;

	txtcDst.pResource = texture;
	txtcDst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	txtcDst.SubresourceIndex = 0;

	// copy void* to cpu resource
	char* uploadBufferAddress;
	D3D12_RANGE uploadRange;
	uploadRange.Begin = 0;
	uploadRange.End = 1024 + textureSize;
	uploadBuffer->Map(0, &uploadRange, (void**)&uploadBufferAddress);
	memcpy(&uploadBufferAddress[0], textureData.Data.data(), textureSize);
	memcpy(&uploadBufferAddress[textureSize], vertices, sizeof(vertices));
	uploadBuffer->Unmap(0, &uploadRange);

	auto* commandList = DXContext::Get().InitCommandList();
	commandList->CopyBufferRegion(vertexBuffer, 0, uploadBuffer, textureSize, 1024);
	commandList->CopyTextureRegion(&txtcDst, 0, 0, 0, &txtcSrc, &textureSizeAsBox);
	DXContext::Get().ExecuteCommandList();

	// ====== SHADERS ======

	Shader rootSignatureShader("RootSignature.cso");
	Shader vertexShader("VertexShader.cso");
	Shader pixelShader("PixelShader.cso");

	// ====== CREATE ROOT SIGNATURE

	ComPointer<ID3D12RootSignature> rootSignature;
	DXContext::Get().GetDevice()->CreateRootSignature(
		0,
		rootSignatureShader.GetBuffer(),
		rootSignatureShader.GetSize(),
		IID_PPV_ARGS(&rootSignature)
	);

	// ====== PIPELINE STATE ======

	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	{
		// ROOT SIGNATURE FOR SHADERS
		graphicsPipelineStateDesc.pRootSignature = rootSignature;

		// SHADERS
		graphicsPipelineStateDesc.VS.pShaderBytecode = vertexShader.GetBuffer();
		graphicsPipelineStateDesc.VS.BytecodeLength = vertexShader.GetSize();

		graphicsPipelineStateDesc.PS.pShaderBytecode = pixelShader.GetBuffer();
		graphicsPipelineStateDesc.PS.BytecodeLength = pixelShader.GetSize();

		graphicsPipelineStateDesc.DS = { nullptr, 0 };
		graphicsPipelineStateDesc.HS = { nullptr, 0 };
		graphicsPipelineStateDesc.GS = { nullptr, 0 };

		// STREAM OUTPUT (streaming output buffer)
		graphicsPipelineStateDesc.StreamOutput.NumEntries = 0;
		graphicsPipelineStateDesc.StreamOutput.pSODeclaration = nullptr;
		graphicsPipelineStateDesc.StreamOutput.NumEntries = 0;
		graphicsPipelineStateDesc.StreamOutput.pBufferStrides = nullptr;
		graphicsPipelineStateDesc.StreamOutput.NumStrides = 0;
		graphicsPipelineStateDesc.StreamOutput.RasterizedStream = 0;

		// BLENDING (how output of fragment shader is written and blended into the render target)
		graphicsPipelineStateDesc.BlendState.AlphaToCoverageEnable = FALSE;
		graphicsPipelineStateDesc.BlendState.IndependentBlendEnable = FALSE;
		graphicsPipelineStateDesc.BlendState.RenderTarget[0].BlendEnable = FALSE;
		graphicsPipelineStateDesc.BlendState.RenderTarget[0].LogicOpEnable = FALSE;
		graphicsPipelineStateDesc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_ZERO;
		graphicsPipelineStateDesc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
		graphicsPipelineStateDesc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		graphicsPipelineStateDesc.BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ZERO;
		graphicsPipelineStateDesc.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
		graphicsPipelineStateDesc.BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
		graphicsPipelineStateDesc.BlendState.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
		graphicsPipelineStateDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

		// SAMPLE MASK (sample mask for the blend state)
		graphicsPipelineStateDesc.SampleMask = 0xFFFFFFFF;

		// RASTERIZER (scene/world geometry to pixels on screen)
		graphicsPipelineStateDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
		graphicsPipelineStateDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
		graphicsPipelineStateDesc.RasterizerState.FrontCounterClockwise = FALSE;
		graphicsPipelineStateDesc.RasterizerState.DepthBias = 0;
		graphicsPipelineStateDesc.RasterizerState.DepthBiasClamp = 0.0f;
		graphicsPipelineStateDesc.RasterizerState.SlopeScaledDepthBias = 0.0f;
		graphicsPipelineStateDesc.RasterizerState.DepthClipEnable = FALSE;
		graphicsPipelineStateDesc.RasterizerState.MultisampleEnable = FALSE;
		graphicsPipelineStateDesc.RasterizerState.AntialiasedLineEnable = FALSE;
		graphicsPipelineStateDesc.RasterizerState.ForcedSampleCount = 0;

		// DEPTH STENCIL STATE
		graphicsPipelineStateDesc.DepthStencilState.DepthEnable = FALSE;
		graphicsPipelineStateDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
		graphicsPipelineStateDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
		graphicsPipelineStateDesc.DepthStencilState.StencilEnable = FALSE;
		graphicsPipelineStateDesc.DepthStencilState.StencilReadMask = 0;
		graphicsPipelineStateDesc.DepthStencilState.StencilWriteMask = 0;
			// front & back face
		graphicsPipelineStateDesc.DepthStencilState.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
		graphicsPipelineStateDesc.DepthStencilState.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
		graphicsPipelineStateDesc.DepthStencilState.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
		graphicsPipelineStateDesc.DepthStencilState.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
		graphicsPipelineStateDesc.DepthStencilState.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
		graphicsPipelineStateDesc.DepthStencilState.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
		graphicsPipelineStateDesc.DepthStencilState.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
		graphicsPipelineStateDesc.DepthStencilState.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

		// INPUT LAYOUT
		graphicsPipelineStateDesc.InputLayout.NumElements = _countof(vertexLayout);
		graphicsPipelineStateDesc.InputLayout.pInputElementDescs = vertexLayout;

		graphicsPipelineStateDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;

		graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

		// RENDER TARGETS SPECIFICATION
		graphicsPipelineStateDesc.NumRenderTargets = 1;

		graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

		graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;

		// SAMPLE DESCRIPTION (multisampling parameters)
		graphicsPipelineStateDesc.SampleDesc.Count = 1;
		graphicsPipelineStateDesc.SampleDesc.Quality = 0;
		
		// single GPU operation
		graphicsPipelineStateDesc.NodeMask = 0;

		// A cached pipeline state object
		graphicsPipelineStateDesc.CachedPSO = { nullptr, 0 };

		// flags
		graphicsPipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	}

	ComPointer<ID3D12PipelineState> pipelineStateObject;
	DXContext::Get().GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&pipelineStateObject));


	// ====== VERTEX BUFFER VIEW ======
	D3D12_VERTEX_BUFFER_VIEW vbv{};
	{
		vbv.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
		vbv.SizeInBytes = sizeof(Vertex) * _countof(vertices);
		vbv.StrideInBytes = sizeof(Vertex);
	}

	dxInit.StopAndPrintTime();

	while (!DXWindow::Get().ShouldClose())
	{

		DXWindow::Get().Update();
		if (DXWindow::Get().ShouldResize())
		{
			DXContext::Get().Flush(DXWindow::GetFrameCount());
			DXWindow::Get().Resize();
		}

		// begin drawing
		commandList = DXContext::Get().InitCommandList();
		DXWindow::Get().BeginFrame(commandList);

		// PSO
		commandList->SetPipelineState(pipelineStateObject);
		commandList->SetGraphicsRootSignature(rootSignature);

		// IA (input assembler)
		commandList->IASetVertexBuffers(0, 1, &vbv);
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST); 

		// RS
		D3D12_VIEWPORT viewport{};
		{
			viewport.TopLeftX = 0;
			viewport.TopLeftY = 0;
			viewport.Width = DXWindow::Get().GetWidth();
			viewport.Height = DXWindow::Get().GetHeight();
			viewport.MinDepth = 1.0f;
			viewport.MaxDepth = 0.0f;
		}
		RECT scissorRect{};
		{
			scissorRect.left = 0;
			scissorRect.top = 0;
			scissorRect.right = DXWindow::Get().GetWidth();
			scissorRect.bottom = DXWindow::Get().GetHeight();
		}
		commandList->RSSetViewports(1, &viewport);
		commandList->RSSetScissorRects(1, &scissorRect);

		// Root arguments
		static float color[] = {1, 0, 1};
		pukeColor(color);
		commandList->SetGraphicsRoot32BitConstants(0, 3, color, 0);

		// draw
		commandList->DrawInstanced(_countof(vertices), 1, 0, 0);

		// end draw
		DXWindow::Get().EndFrame(commandList);
		DXContext::Get().ExecuteCommandList();

		// presenting it to the screen 
		DXWindow::Get().Present();
	}

	// flushing
	DXContext::Get().Flush(DXWindow::GetFrameCount());

	// close

	vertexBuffer.Release();
	uploadBuffer.Release();
	pipelineStateObject.Release();
	rootSignature.Release();

	DXWindow::Get().Shutdown();
	DXContext::Get().Shutdown();
	DXDebugLayer::Get().Shutdown();


	return 0;
}