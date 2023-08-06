#include <iostream>

#include "Support/WinInclude.h"
#include "Support/ComPointer.h"
#include "Support/Window.h"
#include "Support/Shader.h"

#include "Debug/DXDebugLayer.h" 
#include "D3D/DXContext.h"
#include "Debug/Timer.h"

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

	// ====== UPLOAD & VERTEX BUFFER ======
	D3D12_RESOURCE_DESC rDesc{};
	{
		rDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		rDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		rDesc.Width = 1024;
		rDesc.Height = 1;
		rDesc.DepthOrArraySize = 1;
		rDesc.MipLevels = 1;
		rDesc.Format = DXGI_FORMAT_UNKNOWN;
		rDesc.SampleDesc.Count = 1;
		rDesc.SampleDesc.Quality = 0;
		rDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		rDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	}

	ComPointer<ID3D12Resource> uploadBuffer, vertexBuffer;

	// Create resources on GPU heap
	DXContext::Get().GetDevice()->CreateCommittedResource(
		&hpUpload,
		D3D12_HEAP_FLAG_NONE, &rDesc, D3D12_RESOURCE_STATE_COMMON, nullptr,
		IID_PPV_ARGS(&uploadBuffer)
	);

	DXContext::Get().GetDevice()->CreateCommittedResource(
		&hpDefault,
		D3D12_HEAP_FLAG_NONE, &rDesc, D3D12_RESOURCE_STATE_COMMON, nullptr,
		IID_PPV_ARGS(&vertexBuffer)
	);

	// copy void* to cpu resource
	void* uploadBufferAddress;
	D3D12_RANGE uploadRange;
	{
		uploadRange.Begin = 0;
		uploadRange.End = 1024 - 1;
	}
	uploadBuffer->Map(0, &uploadRange, &uploadBufferAddress);
	memcpy(uploadBufferAddress, vertices, sizeof(vertices));
	uploadBuffer->Unmap(0, &uploadRange);

	// copy CPU resource to GPU resource
	auto* commandList = DXContext::Get().InitCommandList();
	commandList->CopyBufferRegion(vertexBuffer, 0, uploadBuffer, 0, 1024);
	DXContext::Get().ExecuteCommandList();

	// ====== SHADERS ======

	Shader vertexShader("VertexShader.cso");
	Shader pixelShader("PixelShader.cso");

	// ====== PIPELINE STATE ======

	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	{
		graphicsPipelineStateDesc.pRootSignature; 
		graphicsPipelineStateDesc.VS.pShaderBytecode = vertexShader.GetBuffer();
		graphicsPipelineStateDesc.VS.BytecodeLength = vertexShader.GetSize();
		graphicsPipelineStateDesc.PS.pShaderBytecode = pixelShader.GetBuffer();;
		graphicsPipelineStateDesc.PS.BytecodeLength = pixelShader.GetSize();;
		graphicsPipelineStateDesc.DS;
		graphicsPipelineStateDesc.HS;
		graphicsPipelineStateDesc.GS;
		graphicsPipelineStateDesc.StreamOutput;
		graphicsPipelineStateDesc.BlendState;
		graphicsPipelineStateDesc.SampleMask;
		graphicsPipelineStateDesc.RasterizerState;
		graphicsPipelineStateDesc.DepthStencilState;
		graphicsPipelineStateDesc.InputLayout.NumElements = _countof(vertexLayout);
		graphicsPipelineStateDesc.InputLayout.pInputElementDescs = vertexLayout;
		graphicsPipelineStateDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
		graphicsPipelineStateDesc.PrimitiveTopologyType;
		graphicsPipelineStateDesc.NumRenderTargets;
		graphicsPipelineStateDesc.RTVFormats[8];
		graphicsPipelineStateDesc.DSVFormat;
		graphicsPipelineStateDesc.SampleDesc;
		graphicsPipelineStateDesc.NodeMask;
		graphicsPipelineStateDesc.CachedPSO;
		graphicsPipelineStateDesc.Flags;
	}
	//DXContext::Get().GetDevice()->CreatePipelineState()


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

		// IA (input assembler)
		commandList->IASetVertexBuffers(0, 1, &vbv);
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // just a list of triangles

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

	DXWindow::Get().Shutdown();
	DXContext::Get().Shutdown();
	DXDebugLayer::Get().Shutdown();
	

	return 0;
}