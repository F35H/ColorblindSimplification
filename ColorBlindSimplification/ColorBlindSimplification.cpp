
#include "ColorBlindSimplification.h"

const LONG DXConfig::windowHeight = 800;
const LONG DXConfig::windowWidth = 800;
const uint8_t DXConfig::numFrames = 3;
//bool DXCommon::warpDrive = false;
int DXConfig::VSync = 1;
int DXConfig::screenTear = 0;
//bool fillscreen = false;
const std::array<const float,4> DXConfig::bckgndcolor = { 0.0f, 0.0f, 0.0f, 1.0f };

int CALLBACK wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nCmdShow) {
  using namespace DXCommon;

  size_t i = 0;
  const wchar_t* windowClassName = L"DX12WindowClass";

  WCHAR path[MAX_PATH];
  HMODULE hModule = GetModuleHandleW(NULL);
  if (GetModuleFileNameW(hModule, path, MAX_PATH) > 0) {
    PathCchRemoveFileSpec(path, MAX_PATH);
    SetCurrentDirectoryW(path);
  } //Set Current Directory

  ErrorHandler::ConfirmSuccess(
    !DirectX::XMVerifyCPUSupport(),
    "Confirming Math Library");



  ErrorHandler::ErrorHandler();
  dxHardware = new DXHardware();
  dxWindow = new DXWindow(hInstance, L"DX12WindowClass");
  //Add Tearing Into SwapChain
  dxUniform = new DXUniformData(dxHardware->device);
  dxCmdChain = new DXCmdChain(dxHardware->device);
  dxCmdChain->CreateCmdList(D3D12_COMMAND_LIST_TYPE_COPY);
  dxSwapChain = new DXSwapChain(dxHardware->device, dxUniform, dxWindow, dxCmdChain);
  dxSyncObjects = new DXSyncObjectFactory();
  for (; i < DXConfig::numFrames; ++i) {
    dxSyncObjects->CreateSyncObject(DXSyncObjectFactory::Fence, dxHardware->device);
  }; //syncByFrame
  dxSyncObjects->CreateSyncObject(DXSyncObjectFactory::Barrier, dxHardware->device);
  dxSyncObjects->CreateSyncObject(DXSyncObjectFactory::Barrier, dxHardware->device);
  ::ShowWindow(dxWindow->hwnd, SW_SHOW);

  fps = 0;
  elapsedTime = 0.0;
  startTime = frameTimer.now();

  MSG msg = {};
  for (;msg.message != WM_QUIT;) {
    if (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
      ::TranslateMessage(&msg);
      ::DispatchMessage(&msg);
    }; //if 
  }; //forloop

  for (;i<dxSyncObjects->fences.size();++i) {
    dxSyncObjects->TriggerSemaphore(i,dxCmdChain->cmdQueue);
    dxSyncObjects->ReturnSemaphore(i, dxCmdChain->cmdQueue);
    ::CloseHandle(dxSyncObjects->fenceEvents[i]);
  }; //fori>=0
}; //wWinMain

inline void RenderLoop() {
  using namespace DXCommon;

  //Manage Time
  ++fps;
  deltaTime = frameTimer.now() - startTime;
  startTime = frameTimer.now();

  elapsedTime += deltaTime.count() * 1e-9;
  if (elapsedTime > 1.0)
  {
    char buffer[500];
    fps = static_cast<int>(fps / elapsedTime);
    std::cerr << "FPS:" << fps << "\n";

    fps = 0;
    elapsedTime = 0.0;
  }; //if elapsedTime

  auto& cmdAlloc = dxCmdChain->cmdAllocs[dxSwapChain->backBufferIndex];
  cmdAlloc->Reset();
  dxCmdChain->cmdList->Reset(cmdAlloc.Get(), nullptr);


  //Clear Screen
  dxSyncObjects->barriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(
    dxSwapChain->backBuffers[dxSwapChain->backBufferIndex].Get(),
    D3D12_RESOURCE_STATE_PRESENT, 
    D3D12_RESOURCE_STATE_RENDER_TARGET
  ); //ClrScrn
  
  dxCmdChain->cmdList->ResourceBarrier(1, &dxSyncObjects->barriers[0]);

  CD3DX12_CPU_DESCRIPTOR_HANDLE rtv(
    dxUniform->uniformDesc->GetCPUDescriptorHandleForHeapStart(),
    dxSwapChain->backBufferIndex, 
    dxUniform->uniformDescSize
  ); //DescHandle
  dxCmdChain->cmdList->ClearRenderTargetView(rtv,DXConfig::bckgndcolor.data(), 0, nullptr);

  //Present Screen
  dxSyncObjects->barriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(
    dxSwapChain->backBuffers[dxSwapChain->backBufferIndex].Get(),
    D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
  dxCmdChain->cmdList->ResourceBarrier(1, &dxSyncObjects->barriers[1]);

  ErrorHandler::ConfirmSuccess(
    dxCmdChain->cmdList->Close(),
    "Confirming CmdList is Empty");

  ID3D12CommandList* const commandLists[] = {
      dxCmdChain->cmdList.Get()
  }; //commandLists

  dxCmdChain->cmdQueue->ExecuteCommandLists(_countof(commandLists), commandLists);
  dxSyncObjects->TriggerSemaphore(dxSwapChain->backBufferIndex, dxCmdChain->cmdQueue);

  auto presentFlags = DXConfig::screenTear && !DXConfig::VSync ? DXGI_PRESENT_ALLOW_TEARING : 0;

  ErrorHandler::ConfirmSuccess(
    dxSwapChain->swapChain->Present(DXConfig::VSync, presentFlags),
    "Presenting Framebuffer to SwapChain");

  dxSwapChain->backBufferIndex = dxSwapChain->swapChain->GetCurrentBackBufferIndex();

  dxSyncObjects->ReturnSemaphore(dxSwapChain->backBufferIndex, dxCmdChain->cmdQueue);
}; //RenderLoop

inline void ResizeWindow() {
  using namespace DXCommon;
  
  RECT winRect = {};
  ::GetClientRect(dxWindow->hwnd, &winRect);

  int crntWidth = winRect.right - winRect.left;
  int crntHeight = winRect.bottom - winRect.top;

  if (crntWidth != dxWindow->windowX || crntHeight != dxWindow->windowX) {
    dxWindow->windowX = std::max(1, crntWidth);
    dxWindow->windowY = std::max(1, crntHeight);

    size_t i = 0;
    for (; i < DXConfig::numFrames; ++i) {
      if (i == 0) {
        dxSyncObjects->TriggerSemaphore(i, dxCmdChain->cmdQueue);
        dxSyncObjects->ReturnSemaphore(i, dxCmdChain->cmdQueue);
      }; //if i==0 
      
      dxSwapChain->backBuffers[i].Reset();
    }; //forloop

    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};

    ErrorHandler::ConfirmSuccess(
      dxSwapChain->swapChain->GetDesc(&swapChainDesc),
      "Transfering Descriptor Data");

    ErrorHandler::ConfirmSuccess(
      dxSwapChain->swapChain->ResizeBuffers(
        DXConfig::numFrames, 
        dxWindow->windowX,
        dxWindow->windowY,
        swapChainDesc.BufferDesc.Format,
        swapChainDesc.Flags),
      "Resizing SwapChain Buffer");

    dxSwapChain->backBufferIndex = dxSwapChain->swapChain->GetCurrentBackBufferIndex();
    dxSwapChain->RefreshData(dxHardware->device, dxUniform);
  }; //if width/height

}; //ResizeWindow

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
  switch (message) {
  case WM_PAINT:
    RenderLoop();
    break;
  case WM_SYSKEYDOWN:
  case WM_KEYDOWN:
    if (wParam == VK_ESCAPE)
      ::PostQuitMessage(0);
    break;
  case WM_SYSCHAR:
    break;
  case WM_SIZE:
    ResizeWindow();
    break;
  case WM_DESTROY:
    ::PostQuitMessage(0);
    break;
  default:
    return ::DefWindowProc(hwnd, message, wParam, lParam);
  }; //switch (message)
  return 0; 
}; //WndProc
