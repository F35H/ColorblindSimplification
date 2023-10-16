#pragma once
#pragma link(D3D12)

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#if defined(min)
#undef min
#endif

#if defined(max)
#undef max
#endif

extern "C" { __declspec(dllexport) extern const UINT D3D12SDKVersion = 4; }

extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = u8".\\D3D12\\"; }

#include <wrl.h>
#include <d3dx12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <pathcch.h>

#include <algorithm>
#include <cassert>
#include <chrono>
#include <vector>
#include <Array>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <queue>

using Microsoft::WRL::ComPtr;
using namespace DirectX;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

static struct ErrorHandler {
  ErrorHandler() {
    
    AllocConsole();

    FILE* fp = nullptr;
    freopen_s(&fp, "CONOUT$", "w", stderr);

    if (GetStdHandle(STD_ERROR_HANDLE) != INVALID_HANDLE_VALUE)
      if (freopen_s(&fp, "CONOUT$", "w", stderr) != 0)
        throw std::runtime_error("Console Unavailable, Try Again");
      else
        setvbuf(stderr, NULL, _IONBF, 0);

    std::ios::sync_with_stdio(true);

    std::cerr.clear();

    ComPtr<ID3D12Debug> debugInterface;
    D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface));
    debugInterface->EnableDebugLayer();

  }; //ErrorHandler Ctor

  static void ConfirmSuccess(HRESULT errorCode, std::string activity) {
    if FAILED(errorCode) { std::cerr << activity << ErrorString(errorCode) << std::endl; std::terminate(); }
    else { std::cerr <<  activity << ErrorString(errorCode) << "\n"; };
  }; //ConfirmSuccess

  static void ConfirmSuccess(ATOM errorCode, std::string activity) {
    if (errorCode == 0) { std::cerr << activity << " Failure!" << std::endl; std::terminate(); }
    else { std::cerr << activity << " Success" << "\n"; };
  }; //ConfirmSuccess

  static void ConfirmSuccess(HWND hwnd, std::string activity) {
    HRESULT errorCode = HRESULT_FROM_WIN32(GetLastError());
    if (hwnd == NULL) { std::cerr << activity << ErrorString(errorCode) << std::endl; std::terminate(); }
    else { std::cerr << activity << ErrorString(errorCode) << "\n"; };
  }; //ConfirmSuccess

  static void ConfirmSuccess(HANDLE fence, std::string activity) {
    HRESULT errorCode = HRESULT_FROM_WIN32(GetLastError());
    if (!fence) { std::cerr << activity << ErrorString(errorCode) << std::endl; std::terminate(); }
    else { std::cerr << activity << ErrorString(errorCode) << "\n"; };
  }; //ConfirmSuccess

  static void ConfirmSuccess(bool error, std::string activity) {
    if (error) { std::cerr << activity << " Failure!" << std::endl; std::terminate(); }
    else { std::cerr << activity << " Success" << "\n"; }
  }; //ConfirmSuccesss

  static void Log(std::string activity) {
    std::cerr << activity << " Success" << "\n";
  }; //Log

  private:
    static std::string ErrorString(HRESULT errorCode) {
      switch (errorCode) {
      case E_FAIL:
      default:
        return " Unknown Error! " + std::to_string(errorCode);
      case S_OK:
        return " Success! ";
      case E_ABORT:
        return " Operation Aborted! ";
      case E_ACCESSDENIED:
        return " ACCESS DENIED! ";
      case E_HANDLE:
        return " HANDLE IS NOT VALID! ";
      case E_INVALIDARG:
        return " ARGUMENTS INVALID! ";
      case E_NOINTERFACE:
        return " INTERFACE NOT SUPPORTED! ";
      case E_NOTIMPL:
        return " NOT IMPLEMENTED! ";
      case E_OUTOFMEMORY:
        return " OUT OF MEMORY! ";
      case E_POINTER:
        return " POINTER NOT VALID! ";
      case E_UNEXPECTED:
        return " UNEXPECTED FAILURE! ";
      }; //switch errorCode
    }; //ErrorString
}; //ErrorHandler

struct DXConfig {
  static const std::array<const float, 4> bckgndcolor;
  static const LONG windowWidth;
  static const LONG windowHeight;
  static const uint8_t numFrames;
  static float FOV;
  static int VSync;
  static int screenTear;
}; //DXConfig

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

namespace DXBuffer {
  using namespace DirectX;
  
  struct Vertex
  {
    XMFLOAT3 Position;
    XMFLOAT3 Color;
  }; //Vertex

  enum BufferType {
    VerticeBuffer = 0,
    IndiceBuffer
  }; //BufferType

  struct BufferView : D3D12_VERTEX_BUFFER_VIEW, D3D12_INDEX_BUFFER_VIEW, D3D12_DEPTH_STENCIL_VIEW_DESC {
    D3D12_GPU_VIRTUAL_ADDRESS* BufferLocation;
    UINT*                      SizeInBytes;
    UINT*                      StrideInBytes;
    DXGI_FORMAT*               Format;

    BufferView(BufferType type) {
      switch (type) {
      case VerticeBuffer:
        this->BufferView::BufferLocation = &this->D3D12_VERTEX_BUFFER_VIEW::BufferLocation;
        this->BufferView::SizeInBytes = &this->D3D12_VERTEX_BUFFER_VIEW::SizeInBytes;
        this->BufferView::StrideInBytes = &this->D3D12_VERTEX_BUFFER_VIEW::StrideInBytes;
        return;
      case IndiceBuffer:
        this->BufferView::BufferLocation = &this->D3D12_INDEX_BUFFER_VIEW::BufferLocation;
        this->BufferView::SizeInBytes = &this->D3D12_INDEX_BUFFER_VIEW::SizeInBytes;
        this->BufferView::Format = &this->D3D12_INDEX_BUFFER_VIEW::Format;
        return;
      }; //switch
    }; //BufferView
  };
}; //DXBuffer

namespace DXGame {

}; //DXGame

namespace DXCommon {
struct DXWindow {
  HWND hwnd;
  RECT windowRect;
  int windowX = 0;
  int windowY = 0;

  DXWindow(HINSTANCE hInst, const wchar_t* className) {

    //Creating Window Info Class
    WNDCLASSEXW windowClass = {};

    windowClass.cbSize = sizeof(WNDCLASSEXW);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = &WndProc;
    windowClass.cbClsExtra = 0;
    windowClass.cbWndExtra = 0;
    windowClass.hInstance = hInst;
    windowClass.hIcon = ::LoadIcon(hInst, NULL);
    windowClass.hCursor = ::LoadCursor(NULL, IDC_ARROW);
    windowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    windowClass.lpszMenuName = NULL;
    windowClass.lpszClassName = className;
    windowClass.hIconSm = ::LoadIcon(hInst, NULL);

    ATOM atom = ::RegisterClassExW(&windowClass);
    ErrorHandler::ConfirmSuccess(atom, "Registering Window Class");


    //Creating Window
    int screenWidth = ::GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = ::GetSystemMetrics(SM_CYSCREEN);

    windowRect = { 0, 0, DXConfig::windowWidth, DXConfig::windowHeight };
    ::AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

    int windowWidth = windowRect.right - windowRect.left;
    int windowHeight = windowRect.bottom - windowRect.top;

    windowX = std::max<int>(0, (screenWidth - windowWidth) / 2);
    windowY = std::max<int>(0, (screenHeight - windowHeight) / 2);

    hwnd = ::CreateWindowExW(
      NULL,
      className,
      className,
      WS_OVERLAPPEDWINDOW,
      windowX,
      windowY,
      windowWidth,
      windowHeight,
      NULL,
      NULL,
      hInst,
      nullptr
    ); //CreateWindowExW

    ErrorHandler::ConfirmSuccess(hwnd, "Creating Window");

    ::GetWindowRect(hwnd, &windowRect);

  }; //DXWindow
}; //DXWindow
struct DXHardware {
  ComPtr<IDXGIAdapter4> adaptorFour;
  ComPtr<IDXGIAdapter1> adaptorOne;
  ComPtr<ID3D12Device2> device;
  ComPtr<ID3D12InfoQueue> debugQueue;
  ComPtr<IDXGIFactory4> factoryFour;
  ComPtr<IDXGIFactory5> factoryFive;

  DXHardware() {

    //Creating Hardware Adaptor
    ComPtr<IDXGIFactory4> dxgiFactory;
    UINT createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;

    ErrorHandler::ConfirmSuccess(
      CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory)
    ), "Creating Hardware Adaptor");

    //Querying Devices
    SIZE_T maxDedicatedVideoMemory = 0;
    for (UINT i = 0; dxgiFactory->EnumAdapters1(i, &adaptorOne) != DXGI_ERROR_NOT_FOUND; ++i)
    {
      DXGI_ADAPTER_DESC1 dxgiAdapterDesc1;
      adaptorOne->GetDesc1(&dxgiAdapterDesc1);

      if ((dxgiAdapterDesc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0 &&
        SUCCEEDED(D3D12CreateDevice(adaptorOne.Get(),
          D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr)) &&
        dxgiAdapterDesc1.DedicatedVideoMemory > maxDedicatedVideoMemory)
      {
        maxDedicatedVideoMemory = dxgiAdapterDesc1.DedicatedVideoMemory;
        ErrorHandler::ConfirmSuccess(adaptorOne.As(&adaptorFour), "Conforming Device");
      } //checking for flags
    } // for all adaptors


    //Creating Device
    ErrorHandler::ConfirmSuccess(
      D3D12CreateDevice(adaptorFour.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device)),
      "Creating DirectX Device" ); 


    //Creating Debug Info
    ErrorHandler::ConfirmSuccess(
      device.As(&debugQueue), 
      "Creating Debug Queue");

    debugQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
    debugQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
    debugQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);

    D3D12_MESSAGE_SEVERITY Severities[] = { D3D12_MESSAGE_SEVERITY_INFO }; 

    D3D12_MESSAGE_ID DenyIds[] = {
      D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,
      D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,
      D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,
    }; //DenyIds

    D3D12_INFO_QUEUE_FILTER NewFilter = {};
    NewFilter.DenyList.NumSeverities = _countof(Severities);
    NewFilter.DenyList.pSeverityList = Severities;
    NewFilter.DenyList.NumIDs = _countof(DenyIds);
    NewFilter.DenyList.pIDList = DenyIds;

    ErrorHandler::ConfirmSuccess(
      debugQueue->PushStorageFilter(&NewFilter), 
      "Creating Debug Filter");

    //Set DPI awareness for Win10
    SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    //Checking for Tearing Support
    DXConfig::screenTear = false;

    ErrorHandler::ConfirmSuccess(
      CreateDXGIFactory1(
        IID_PPV_ARGS(&factoryFour)),
        "Creating Factory for Tearing Support");

    ErrorHandler::ConfirmSuccess(
      factoryFour.As(&factoryFive),
      "Casting Factory for Tearing Support");

    if (!factoryFive->CheckFeatureSupport(
      DXGI_FEATURE_PRESENT_ALLOW_TEARING,
      &DXConfig::screenTear,
      sizeof(DXConfig::screenTear))) { DXConfig::screenTear = true; }
  } //DXHardware Ctor
}; //HardWare
class DXUniformDataFactory {
  struct Buffer {
    ComPtr<ID3D12Resource> buffer;
    DXBuffer::BufferView* bufferInfo;
    DXBuffer::BufferType bufferType;

    Buffer(DXBuffer::BufferType type) {
      bufferType = type;
      bufferInfo = new DXBuffer::BufferView(type);
    }; //Buffer

    void FillBuffer(ComPtr<ID3D12GraphicsCommandList2> cmdList, ComPtr<ID3D12Device2> device, std::vector<void*> vertices) {
      void* buffData = vertices.data();
      UINT buffCount = vertices.size();
      UINT buffSize = sizeof(buffData);
      ComPtr<ID3D12Resource> stageBuffer;

      ErrorHandler::ConfirmSuccess(
        device->CreateCommittedResource(
          &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
          D3D12_HEAP_FLAG_NONE,
          &CD3DX12_RESOURCE_DESC::Buffer(buffSize, D3D12_RESOURCE_FLAGS()),
          D3D12_RESOURCE_STATE_COPY_DEST,
          nullptr,
          IID_PPV_ARGS(&buffer)),
        "Allocating Vertex Buffer");

      ErrorHandler::ConfirmSuccess(
        device->CreateCommittedResource(
          &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
          D3D12_HEAP_FLAG_NONE,
          &CD3DX12_RESOURCE_DESC::Buffer(buffSize),
          D3D12_RESOURCE_STATE_GENERIC_READ,
          nullptr,
          IID_PPV_ARGS(&stageBuffer)),
        "Allocating Stage Buffer");

      D3D12_SUBRESOURCE_DATA subData = {};
      subData.pData = buffData;
      subData.RowPitch = buffSize;
      subData.SlicePitch = subData.RowPitch;

      UpdateSubresources(cmdList.Get(),
        buffer.Get(), stageBuffer.Get(),
        0, 0, 1, &subData);

      *bufferInfo->BufferLocation = buffer->GetGPUVirtualAddress();
      *bufferInfo->SizeInBytes = buffSize;
      *bufferInfo->StrideInBytes = sizeof(DXBuffer::Vertex);;
      *bufferInfo->Format = DXGI_FORMAT_R16_UINT;
    }; //FillBuffer

    void FillBuffer(ComPtr<ID3D12GraphicsCommandList2> cmdList, ComPtr<ID3D12Device2> device, int width, int height) {
      D3D12_CLEAR_VALUE clearValue = {};
      clearValue.Format = DXGI_FORMAT_D32_FLOAT;
      clearValue.DepthStencil = { 1.0f, 0 };
      
      ErrorHandler::ConfirmSuccess(
        device->CreateCommittedResource(
          &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
          D3D12_HEAP_FLAG_NONE,
          &CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, width, height,
            1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
          D3D12_RESOURCE_STATE_DEPTH_WRITE,
          &clearValue,
          IID_PPV_ARGS(&buffer)),
      "Allocating Depth Buffer");
      
      *bufferInfo->Format = DXGI_FORMAT_D32_FLOAT;
      *bufferInfo->ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
      *bufferInfo->Texture2D.MipSlice = 0; 
      *bufferInfo->Flags = D3D12_DSV_FLAG_NONE;
    }; //FillBuffer
  }; //Buffer

  struct Pipeline {
    struct StreamClass {
      CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE root;
      CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT input;
      CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY topology;
      CD3DX12_PIPELINE_STATE_STREAM_VS vertShader;
      CD3DX12_PIPELINE_STATE_STREAM_PS fragShader;
      CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT dsv;
      CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS rtv;
    }; //StreamClass

    ComPtr<ID3DBlob> vertShader;
    ComPtr<ID3DBlob> fragShader;
    ComPtr<ID3DBlob> rootSignatureBlob;
    ComPtr<ID3DBlob> errorLog;
    ComPtr<ID3D12RootSignature> rootSignature;
    ComPtr<ID3D12PipelineState> state;
    StreamClass* stream;

    Pipeline(ComPtr<ID3D12Device2> device, LPCWSTR vertFile, LPCWSTR fragFile) {
      //Read Shaders to Blobs
      ErrorHandler::ConfirmSuccess(
        D3DReadFileToBlob(vertFile, &vertShader),
        "Allocating Vertex Shader");

      ErrorHandler::ConfirmSuccess(
        D3DReadFileToBlob(fragFile, &fragShader),
        "Allocating Frag Shader");


      //Set Layout for Buffers
      D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
      }; //inputLayout


      //Produce Root Signature
      D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
      featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

      if (device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData)))
        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;

      D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

      CD3DX12_ROOT_PARAMETER1 rootParameters[1];
      rootParameters[0].InitAsConstants(sizeof(XMMATRIX) / 4, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);

      CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription;
      rootSignatureDescription.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr, rootSignatureFlags);

      ErrorHandler::ConfirmSuccess(
        D3DX12SerializeVersionedRootSignature(&rootSignatureDescription,
          featureData.HighestVersion, &rootSignatureBlob, &errorLog),
        "Serializin Root Signature");

      ErrorHandler::ConfirmSuccess(
        device->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(),
          rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature)),
        "Creating Root Signature");


      //Creating Pipeline
      D3D12_RT_FORMAT_ARRAY rtvFormats = {};
      rtvFormats.NumRenderTargets = 1;
      rtvFormats.RTFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

      stream->root = rootSignature.Get();
      stream->input = { inputLayout, _countof(inputLayout) };
      stream->topology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
      stream->vertShader = CD3DX12_SHADER_BYTECODE(vertShader.Get());
      stream->fragShader = CD3DX12_SHADER_BYTECODE(fragShader.Get());
      stream->dsv = DXGI_FORMAT_D32_FLOAT;
      stream->rtv = rtvFormats;

      D3D12_PIPELINE_STATE_STREAM_DESC streamDesc = { sizeof(StreamClass), &stream };
      ErrorHandler::ConfirmSuccess(
        device->CreatePipelineState(&streamDesc, IID_PPV_ARGS(&state)),
        "Creating Pipeline");
    }; //PipelineStates
  }; //PipelineStates

  struct DescHeap {
    ComPtr<ID3D12DescriptorHeap> descHeap;
    UINT descSize;
    D3D12_DESCRIPTOR_HEAP_TYPE descType;

    DescHeap(ComPtr<ID3D12Device2> device, int type) {
      D3D12_DESCRIPTOR_HEAP_DESC desc = {};
      desc.NumDescriptors = DXConfig::numFrames;
      desc.Type = static_cast<D3D12_DESCRIPTOR_HEAP_TYPE>(type);
      desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

      ErrorHandler::ConfirmSuccess(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&descHeap)),
      "Allocating Descriptor Heap" + type);

      descType = desc.Type;
    }; //DescHeap

    CD3DX12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(ComPtr<ID3D12Device2> device) {
      CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(descHeap->GetCPUDescriptorHandleForHeapStart());
      descSize = device->GetDescriptorHandleIncrementSize(descType);
    }; //UpdateDescSize
  }; //DescHeap

  public:
  std::vector<Pipeline> pipelineVector;
  std::vector<Buffer>   indiceBuffers;
  std::vector<Buffer>   verticeBuffers;
  std::vector<DescHeap> descHeaps;

  DXUniformDataFactory() = default;

  int CreateBuffer(DXBuffer::BufferType type) {
    switch (type) {
    case DXBuffer::VerticeBuffer:
      verticeBuffers.emplace_back(new Buffer(DXBuffer::VerticeBuffer));
      return verticeBuffers.size() - 1;
    case DXBuffer::IndiceBuffer:
      indiceBuffers.emplace_back(new Buffer(DXBuffer::IndiceBuffer));
      return verticeBuffers.size() - 1;
    case DXBuffer::DepthBuffer:
      indiceBuffers.emplace_back(new Buffer(DXBuffer::IndiceBuffer));
      return verticeBuffers.size() - 1;
    }; //Type
  }; //CreateBuffer

  void CreatePipeline(ComPtr<ID3D12Device2> device, LPCWSTR vertFile, LPCWSTR fragFile) {
    pipelineVector.emplace_back(new Pipeline(device, vertFile, fragFile));
  }; //CreatePipeline

  void CreateDescriptors(ComPtr<ID3D12Device2> device) {
    for (int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i) descHeaps.emplace_back(new DescHeap(device, i));
  }; //CreateDescriptors
}; //DXUniformData
struct DXCmdChain {
  ComPtr<ID3D12GraphicsCommandList2> cmdList;
  std::vector< ComPtr<ID3D12CommandQueue>> cmdQueues;
  std::vector<ComPtr<ID3D12CommandAllocator>> cmdAllocs;

  DXCmdChain(ComPtr<ID3D12Device2> device) {
    
    //CommandQueue
    cmdQueues.resize(D3D12_COMMAND_LIST_TYPE_VIDEO_ENCODE);

    int i = 0;
    for (;i <= D3D12_COMMAND_LIST_TYPE_VIDEO_ENCODE;++i) {
      D3D12_COMMAND_QUEUE_DESC desc = {};
      desc.Type = static_cast<D3D12_COMMAND_LIST_TYPE>(i);
      desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
      desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
      desc.NodeMask = 0;

      ErrorHandler::ConfirmSuccess(
        device->CreateCommandQueue(&desc, IID_PPV_ARGS(&cmdQueues[i])),
        "Create Command Queue" + i);
    }; //CommandQueues

    cmdAllocs.resize(DXConfig::numFrames);

    //Command Allocators
    for (int i = DXConfig::numFrames-1; i > -1; --i) {
      ErrorHandler::ConfirmSuccess(
        device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&cmdAllocs[i])),
        "Creating Command Allocator");

      if (i < 1) {
        ErrorHandler::ConfirmSuccess(
          device->CreateCommandList(0, 
            D3D12_COMMAND_LIST_TYPE_DIRECT, 
            cmdAllocs[i].Get(), 
            nullptr, 
            IID_PPV_ARGS(&cmdList)),
          "Creating Command List");
        ErrorHandler::ConfirmSuccess(
          cmdList->Close(),
          "Checking If CmdList was Recorded Prior");
      }; //if i < 1
    }; //cmdAllocs
  }; //DXCmdChain Ctor
}; //DXCmdChain
struct DXSwapChain {
  ComPtr<IDXGISwapChain4> swapChain;
  std::vector<ComPtr<ID3D12Resource>> backBuffers;
  UINT backBufferIndex;
  D3D12_VIEWPORT viewPort;
  D3D12_RECT scissorRect;


  DXSwapChain(ComPtr<ID3D12Device2> device, DXWindow* window, DXUniformDataFactory* uniform, DXCmdChain* cmdChain) {
    backBuffers.resize(DXConfig::numFrames);
    ComPtr<IDXGIFactory4> dxgiFactory4;
    UINT factoryFlags = DXGI_CREATE_FACTORY_DEBUG;
    backBufferIndex = 0;

    ErrorHandler::ConfirmSuccess(
      CreateDXGIFactory2(factoryFlags, IID_PPV_ARGS(&dxgiFactory4)),
      "Creating Factory for SwapChain");

    DXGI_SWAP_CHAIN_DESC1 swapChainParams = {};
    swapChainParams.Width = window->windowX;
    swapChainParams.Height = window->windowY;
    swapChainParams.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainParams.Stereo = FALSE;
    swapChainParams.SampleDesc = { 1, 0 };
    swapChainParams.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainParams.BufferCount = DXConfig::numFrames;
    swapChainParams.Scaling = DXGI_SCALING_STRETCH;
    swapChainParams.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainParams.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    swapChainParams.Flags = DXConfig::screenTear ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

    ComPtr<IDXGISwapChain1> swapChain1;
    dxgiFactory4->CreateSwapChainForHwnd(
      cmdChain->cmdQueues[D3D12_COMMAND_LIST_TYPE_DIRECT].Get(),
      window->hwnd,
      &swapChainParams,
      nullptr,
      nullptr,
      &swapChain1);

    dxgiFactory4->MakeWindowAssociation(window->hwnd, DXGI_MWA_NO_ALT_ENTER);
      
    swapChain1.As(&swapChain);
    backBufferIndex = swapChain->GetCurrentBackBufferIndex();

    RefreshData(device, uniform);

    ErrorHandler::Log("Creating SwapChain");
  }; //DXSwapChain

  void RefreshData(ComPtr<ID3D12Device2> device, DXUniformDataFactory* uniform) {
    CD3DX12_CPU_DESCRIPTOR_HANDLE handle = uniform->descHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_RTV].GetCPUHandle(device);
    int size = uniform->descHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_RTV].descSize;

    for (int i = 0; i < DXConfig::numFrames; ++i) {
      backBuffers[i] = nullptr;
      ErrorHandler::ConfirmSuccess(
        swapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffers[i])),
        "Getting Buffer From SwapChain");


      device->CreateRenderTargetView(backBuffers[i].Get(), nullptr, handle);
      handle.Offset(1,size);

    } //for backBuffer in frames
  }; //RefreshData

}; //SwapChain
class DXSyncObjectFactory {
public:
  std::vector < ComPtr<ID3D12Fence> > fences;
  std::vector <HANDLE>  fenceEvents;
  std::vector <uint64_t> fenceSemaphore;
  std::vector <CD3DX12_RESOURCE_BARRIER> barriers;
  enum SyncType {
    Fence = 0,
    Barrier
  }; //SyncType

  void CreateSyncObject(SyncType type, ComPtr<ID3D12Device2> device) {
    switch (type) {
    case SyncType::Fence: {
      fences.resize(fences.size() + 1);
      fenceEvents.resize(fenceEvents.size() + 1);
      fenceSemaphore.emplace_back(0);

      ErrorHandler::ConfirmSuccess(
        device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fences[fences.size() - 1])),
        "Creating GPU Fence");

      fenceEvents[fenceEvents.size() - 1] = ::CreateEvent(NULL, FALSE, FALSE, NULL);
      ErrorHandler::ConfirmSuccess(fenceEvents[fenceEvents.size() - 1], "Creating Fence Event");

    } //SyncType
    return;
    case SyncType::Barrier: {
      CD3DX12_RESOURCE_BARRIER barrier;
      barriers.emplace_back(barrier);
    }; //Barrier
    return;
    }; //switch type
  }; //DXSyncObjectFactory Ctor

  void TriggerSemaphore(int fenceIdx, ComPtr<ID3D12CommandQueue> cmdQueue) {
    ++fenceSemaphore[fenceIdx];
    ErrorHandler::ConfirmSuccess(
      cmdQueue->Signal(fences[fenceIdx].Get(), fenceSemaphore[fenceIdx]),
      "Incrementing Fence Semaphore");
  }; //SignalFence

  void ReturnSemaphore(int fenceIdx, ComPtr<ID3D12CommandQueue> cmdQueue) {
    if (fences[fenceIdx]->GetCompletedValue() < fenceSemaphore[fenceIdx]) {
      ErrorHandler::ConfirmSuccess(
        fences[fenceIdx]->SetEventOnCompletion(fenceSemaphore[fenceIdx], fenceEvents[fenceIdx]),
        "Finishing Semaphore");
      ::WaitForSingleObject(fenceEvents[fenceIdx], static_cast<DWORD>(std::chrono::milliseconds::max().count()));
    } //If semaphore uncomplete
  }; //ReturnSingleton
}; //DXSyncObjectFactory

static DXWindow* dxWindow;
static DXHardware* dxHardware;
static DXSwapChain* dxSwapChain;
static DXCmdChain* dxCmdChain;
static DXUniformDataFactory* dxUniform;
static DXSyncObjectFactory* dxSyncObjects;

static size_t fps;
static double elapsedTime;
static std::chrono::high_resolution_clock frameTimer;
static std::chrono::steady_clock::time_point startTime;
static std::chrono::duration<long long, std::nano> deltaTime;
}; //DXCommon




using namespace Microsoft::WRL;