
#include "ColorBlindSimplification.h"

const uint32_t DXConfig::windowHeight = 800;
const uint32_t DXConfig::windowWidth = 800;
const uint8_t DXConfig::numFrames = 3;
const bool DXConfig::warpDrive = false;
const bool DXConfig::VSync = true;
const bool DXConfig::screenTear = false;
const bool DXConfig::fillscreen = false;

int main()
{
  ComPtr<ID3D12Fence> fence;
  uint64_t fenceSemaphore = 0;
  uint64_t frame[DXConfig::numFrames];
  HANDLE g_FenceEvent;

  bool isInitialized = false;


  if (_DEBUG) {
    ComPtr<ID3D12Debug> debugInterface;
    ErrorHandler::ConfirmSuccess(
      D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface)
      ), "Setting Up Debug Interface");
    debugInterface->EnableDebugLayer();
  } //If Debug

}
