
#include "ColorBlindSimplification.h"

const LONG DXConfig::windowHeight = 800;
const LONG DXConfig::windowWidth = 800;
const uint8_t DXConfig::numFrames = 3;
//bool DXCommon::warpDrive = false;
int DXConfig::VSync = 1;
int DXGame::screenTear = 0;
const std::array<const float,4> DXConfig::bckgndcolor = { 0.0f, 0.0f, 0.0f, 1.0f };

std::vector<DXBuffer::Vertex> vertices = {
    { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f) }, 
    { XMFLOAT3(-1.0f,  1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) }, 
    { XMFLOAT3(1.0f,  1.0f, -1.0f), XMFLOAT3(1.0f, 1.0f, 0.0f) }, 
    { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) }, 
    { XMFLOAT3(-1.0f, -1.0f,  1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) }, 
    { XMFLOAT3(-1.0f,  1.0f,  1.0f), XMFLOAT3(0.0f, 1.0f, 1.0f) }, 
    { XMFLOAT3(1.0f,  1.0f,  1.0f), XMFLOAT3(1.0f, 1.0f, 1.0f) }, 
    { XMFLOAT3(1.0f, -1.0f,  1.0f), XMFLOAT3(1.0f, 0.0f, 1.0f) }  
}; //vertices

std::vector<WORD> indices = {
    0, 1, 2, 0, 2, 3,
    4, 6, 5, 4, 7, 6,
    4, 5, 1, 4, 1, 0,
    3, 2, 6, 3, 6, 7,
    1, 5, 6, 1, 6, 2,
    4, 0, 3, 4, 3, 7
}; //WordVector

int CALLBACK wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nCmdShow) {
  using namespace DXGame;
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
  dxCmdChain = new DXCmdChain(dxHardware->device);

  dxUniform = new DXUniformDataFactory();
  dxUniform->CreateBuffer(DXBuffer::VerticeBuffer);
  dxUniform->CreateBuffer(DXBuffer::IndiceBuffer);
  dxUniform->CreateBuffer(DXBuffer::DepthBuffer);

  dxUniform->verticeBuffers[0].FillBuffer(dxCmdChain->cmdList, dxHardware->device, vertices);
  dxUniform->indiceBuffers[0].FillBuffer(dxCmdChain->cmdList, dxHardware->device, indices);
  dxUniform->depthBuffers[0].FillBuffer(dxCmdChain->cmdList, dxHardware->device, DXConfig::windowWidth, DXConfig::windowHeight);

  dxUniform->CreatePipeline(dxHardware->device, L"VerticeShader.hlsl", L"FragShader.hlsl");
  dxUniform->CreateDescriptors(dxHardware->device);

  dxSwapChain = new DXSwapChain(dxHardware->device, dxWindow, dxUniform, dxCmdChain);

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

  dxSyncObjects->TriggerSemaphore(i, dxCmdChain->cmdQueues[D3D12_COMMAND_LIST_TYPE_DIRECT]);
  dxSyncObjects->ReturnSemaphore(i, dxCmdChain->cmdQueues[D3D12_COMMAND_LIST_TYPE_DIRECT]);

  MSG msg = {};
  for (;msg.message != WM_QUIT;) {
    if (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
      ::TranslateMessage(&msg);
      ::DispatchMessage(&msg);
    }; //if 
  }; //forloop

  for (;i<dxSyncObjects->fences.size();++i) {
    dxSyncObjects->TriggerSemaphore(i,dxCmdChain->cmdQueues[D3D12_COMMAND_LIST_TYPE_DIRECT]);
    dxSyncObjects->ReturnSemaphore(i, dxCmdChain->cmdQueues[D3D12_COMMAND_LIST_TYPE_DIRECT]);
    ::CloseHandle(dxSyncObjects->fenceEvents[i]);
  }; //fori>=0
}; //wWinMain

inline void RenderLoop() {
  using namespace DXGame;
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




  //Set Matrices
  float angle = static_cast<float>(elapsedTime * 90.0);
  const XMVECTOR rotationAxis = XMVectorSet(0, 1, 1, 0);
  modelMat = XMMatrixRotationAxis(rotationAxis, XMConvertToRadians(angle));

  const XMVECTOR eyePosition = XMVectorSet(0, 0, -10, 1);
  const XMVECTOR focusPoint = XMVectorSet(0, 0, 0, 1);
  const XMVECTOR upDirection = XMVectorSet(0, 1, 0, 0);
  viewMat = XMMatrixLookAtLH(eyePosition, focusPoint, upDirection);

  float aspectRatio = dxWindow->windowX / dxWindow->windowY;
  projMat = XMMatrixPerspectiveFovLH(XMConvertToRadians(DXConfig::FOV), aspectRatio, 0.1f, 100.0f);

  
  //Begin Render
  dxSyncObjects->barriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(
    dxSwapChain->backBuffers[dxSwapChain->backBufferIndex].Get(),
    D3D12_RESOURCE_STATE_PRESENT, 
    D3D12_RESOURCE_STATE_RENDER_TARGET
  ); //ClrScrn
  
  dxCmdChain->cmdList->ResourceBarrier(1, &dxSyncObjects->barriers[0]);
  dxCmdChain->cmdList->ClearRenderTargetView(
    dxUniform->descHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_RTV].GetCPUHandle(dxHardware->device),
    DXConfig::bckgndcolor.data(),
    0,
    nullptr);

  dxCmdChain->cmdList->ClearDepthStencilView(
    dxUniform->descHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_DSV].GetCPUHandle(dxHardware->device), 
    D3D12_CLEAR_FLAG_DEPTH, 
    0, 0, 0, nullptr);

  dxCmdChain->cmdList->SetPipelineState(dxUniform->pipelineVector[0].state.Get());
  dxCmdChain->cmdList->SetGraphicsRootSignature(dxUniform->pipelineVector[0].rootSignature.Get());

  dxCmdChain->cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  dxCmdChain->cmdList->IASetVertexBuffers(0, 1, dxUniform->verticeBuffers[0].bufferInfo);
  dxCmdChain->cmdList->IASetIndexBuffer(dxUniform->indiceBuffers[0].bufferInfo);

  dxCmdChain->cmdList->RSSetViewports(1, &dxSwapChain->viewPort);
  dxCmdChain->cmdList->RSSetScissorRects(1, &dxSwapChain->scissorRect);

  dxCmdChain->cmdList->OMSetRenderTargets(
    1, 
    &dxUniform->descHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_RTV].GetCPUHandle(dxHardware->device),
    FALSE, 
    &dxUniform->descHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_DSV].GetCPUHandle(dxHardware->device));


  XMMATRIX mvpMatrix = XMMatrixMultiply(modelMat, viewMat);
  mvpMatrix = XMMatrixMultiply(mvpMatrix, projMat);
  dxCmdChain->cmdList->SetGraphicsRoot32BitConstants(0, sizeof(XMMATRIX) / 4, &mvpMatrix, 0);

  dxCmdChain->cmdList->DrawIndexedInstanced(indices.size(), 1, 0, 0, 0);


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

  dxCmdChain->cmdQueues[D3D12_COMMAND_LIST_TYPE_DIRECT]->ExecuteCommandLists(_countof(commandLists), commandLists);
  dxSyncObjects->TriggerSemaphore(dxSwapChain->backBufferIndex, dxCmdChain->cmdQueues[D3D12_COMMAND_LIST_TYPE_DIRECT]);

  auto presentFlags = screenTear && !DXConfig::VSync ? DXGI_PRESENT_ALLOW_TEARING : 0;
  ErrorHandler::ConfirmSuccess(
    dxSwapChain->swapChain->Present(DXConfig::VSync, presentFlags),
    "Presenting Framebuffer to SwapChain");

  dxSwapChain->backBufferIndex = dxSwapChain->swapChain->GetCurrentBackBufferIndex();

  dxSyncObjects->ReturnSemaphore(dxSwapChain->backBufferIndex, dxCmdChain->cmdQueues[D3D12_COMMAND_LIST_TYPE_DIRECT]);
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
        dxSyncObjects->TriggerSemaphore(i, dxCmdChain->cmdQueues[D3D12_COMMAND_LIST_TYPE_DIRECT]);
        dxSyncObjects->ReturnSemaphore(i, dxCmdChain->cmdQueues[D3D12_COMMAND_LIST_TYPE_DIRECT]);
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

    dxSwapChain->viewPort = 
      CD3DX12_VIEWPORT(
        0.0f, 0.0f, 
        dxWindow->windowX, 
        dxWindow->windowY);

    dxUniform->depthBuffers[0].FillBuffer(dxCmdChain->cmdList, dxHardware->device, dxWindow->windowX, dxWindow->windowY);
    dxHardware->device->CreateDepthStencilView(dxUniform->depthBuffers[0].buffer.Get(), dxUniform->depthBuffers[0].bufferInfo,
      dxUniform->descHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_DSV].descHeap->GetCPUDescriptorHandleForHeapStart());


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
