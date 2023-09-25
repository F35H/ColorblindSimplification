#pragma once

#include <iostream>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#if defined(min)
#undef min
#endif

#if defined(max)
#undef max
#endif

#if defined(CreateWindow)
#undef CreateWindow
#endif

#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <d3dx12.h>

#include <algorithm>
#include <cassert>
#include <chrono>

struct ErrorHandler {
  static void ConfirmSuccess(HRESULT errorCode, std::string activity) {
    if FAILED(errorCode) throw std::runtime_error(activity);
    else { std::cout << activity << " | Success" << "\n"; };
  }; //ConfirmSuccess

  static void ConfirmSuccess(ATOM errorCode, std::string activity) {
    if (errorCode < 0) throw std::runtime_error(activity);
    else { std::cout << activity << " | Success" << "\n"; };
  }; //ConfirmSuccess

  static void ConfirmSuccess(HWND hwnd, std::string activity) {
    if (!hwnd) throw std::runtime_error(activity);
    else { std::cout << activity << " | Success" << "\n"; };
  }; //ConfirmSuccess
}; //ErrorHandler


struct DXWindow {
  HWND hwnd;
  RECT windowRect;
  int windowX;
  int windowY;

  DXWindow(HINSTANCE hInst, const wchar_t* className) {

    //Creating Window Info Class
    WNDCLASSEXW windowClass = {};

    windowClass.cbSize = sizeof(WNDCLASSEX);
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

    static ATOM atom = ::RegisterClassExW(&windowClass);
    ErrorHandler::ConfirmSuccess(atom, "Registering Window Class");


    //Creating Window
    int screenWidth = ::GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = ::GetSystemMetrics(SM_CYSCREEN);

    windowRect = { 0, 0, DXConfig::windowWidth, DXConfig::windowHeight };
    ::AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

    int windowWidth = windowRect.right - windowRect.left;
    int windowHeight = windowRect.bottom - windowRect.top;

    int windowX = std::max<int>(0, (screenWidth - windowWidth) / 2);
    int windowY = std::max<int>(0, (screenHeight - windowHeight) / 2);

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
  }; //DXWindow
}; //DXWindow


struct DXHardware {
  ComPtr<IDXGIAdapter4> adaptorFour;
  ComPtr<IDXGIAdapter1> adaptorOne;
  ComPtr<ID3D12Device2> device;

  DXHardware() {

    //Creating Hardware Adaptor
    ComPtr<IDXGIFactory4> dxgiFactory;
    UINT createFactoryFlags = 0;

    if (_DEBUG) {
      createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
    } //if DEBUG

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


    ErrorHandler::ConfirmSuccess(
      D3D12CreateDevice(adaptorFour.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device)),
      "Creating DirectX Device" ); //ErrorHandler 
  } //DXHardware Ctor
}; //HardWare


struct DXSwapChain {
  ComPtr<IDXGISwapChain4> swapChain;
  ComPtr<ID3D12Resource> backBuffers[DXConfig::numFrames];
  UINT backBufferIndex;

  DXSwapChain(ComPtr<ID3D12Device2> device, ComPtr<ID3D12DescriptorHeap> uniformDesc) {
    ComPtr<IDXGISwapChain4> dxgiSwapChain4;
    ComPtr<IDXGIFactory4> dxgiFactory4;
    UINT createFactoryFlags = DXGI_SWAP_EFFECT_FLIP_DISCARD;

    ErrorHandler::ConfirmSuccess(
      CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory4)),
      "Create SwapChain");


    auto rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(uniformDesc->GetCPUDescriptorHandleForHeapStart());

    for (int i = 0; i < DXConfig::numFrames; ++i) {
      ComPtr<ID3D12Resource> backBuffer;
      ErrorHandler::ConfirmSuccess(
        swapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer)),
        "Getting Buffer From SwapChain");

      device->CreateRenderTargetView(backBuffer.Get(), nullptr, rtvHandle);

      backBuffers[i] = backBuffer;

      rtvHandle.Offset(rtvDescriptorSize);
    } //for backBuffer in frames
  }; //DXSwapChain
}; //SwapChain


struct DXCmdChain {
  ComPtr<ID3D12CommandQueue> cmdQueue;
  ComPtr<ID3D12GraphicsCommandList> cmdLists;
  ComPtr<ID3D12CommandAllocator> cmdAccel[DXConfig::numFrames];

  DXCmdChain(ComPtr<ID3D12Device2> device) {
    D3D12_COMMAND_QUEUE_DESC desc = {};
    desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    desc.NodeMask = 0;

    ErrorHandler::ConfirmSuccess(
      device->CreateCommandQueue(&desc, IID_PPV_ARGS(&cmdQueue)),
      "Create Command Chain");
  }; //DXCmdChain Ctor
}; //DXCmdChain


struct DXUniformData {
  ComPtr<ID3D12DescriptorHeap> uniformDesc;
  UINT uniformDescSize;

  DXUniformData(ComPtr<ID3D12Device2> device, DXSwapChain* swapChain) {
    auto trueSwapChain = swapChain->swapChain;


    //Create Unifrom Description
    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.NumDescriptors = 1;
    desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

    ErrorHandler::ConfirmSuccess(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&uniformDesc)),
      "Created Descriptor Allocator");
  }; //DXUniformData
}; //DXUniformData

struct DXSyncObjectFactory {

}; //DXSyncObjectFactory

struct DXConfig {
  static const uint32_t windowWidth;
  static const uint32_t windowHeight;
  static const uint8_t numFrames;
  static const bool warpDrive;
  static const bool VSync;
  static const bool screenTear;
  static const bool fillscreen;
}; //DXConfig


LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

using namespace Microsoft::WRL;