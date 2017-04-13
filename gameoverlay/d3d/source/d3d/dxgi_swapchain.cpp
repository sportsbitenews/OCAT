// Copyright 2016 Patrick Mours.All rights reserved.
//
// https://github.com/crosire/gameoverlay
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met :
//
// 1. Redistributions of source code must retain the above copyright notice, this
// list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and / or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY COPYRIGHT HOLDER ``AS IS'' AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.IN NO EVENT
// SHALL COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES(INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT(INCLUDING NEGLIGENCE
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.

#include "dxgi_swapchain.hpp"
#include <assert.h>
#include <wrl.h>

#include "MessageLog.h"

using namespace Microsoft::WRL;
extern bool g_uwpApp;

DXGISwapChain::DXGISwapChain(ID3D11Device *device, IDXGISwapChain *swapChain)
    : direct3DDevice_{static_cast<IUnknown *>(device)},
      swapChain_{swapChain},
      d3dVersion_{D3DVersion_11}
{
  g_messageLog.Log(MessageLog::LOG_INFO, "DXGISwapChain", "calling constructor D3D11");
  d3d11Renderer_ = std::make_unique<gameoverlay::d3d11_renderer>(device, swapChain);
}

DXGISwapChain::DXGISwapChain(ID3D11Device *device, IDXGISwapChain1 *swapChain)
    : DXGISwapChain(device, static_cast<IDXGISwapChain *>(swapChain))
{
  g_messageLog.Log(MessageLog::LOG_INFO, "DXGISwapChain",
                   "calling constructor D3D11 IDXGISwapchain1");
  swapChainVersion_ = SWAPCHAIN_1;
}

DXGISwapChain::DXGISwapChain(ID3D12CommandQueue *commandQueue, IDXGISwapChain *swapChain)
    : direct3DDevice_{static_cast<IUnknown *>(commandQueue)},
      swapChain_{swapChain},
      d3dVersion_{D3DVersion_12},
      swapChainVersion_{SWAPCHAIN_3}
{
  g_messageLog.Log(MessageLog::LOG_INFO, "DXGISwapChain", "calling constructor D3D12");
  d3d12Renderer_ = std::make_unique<gameoverlay::d3d12_renderer>(
      commandQueue, static_cast<IDXGISwapChain3 *>(swapChain));
}

// IUnknown
HRESULT STDMETHODCALLTYPE DXGISwapChain::QueryInterface(REFIID riid, void **ppvObj)
{
  if (ppvObj == nullptr) {
    return E_POINTER;
  }

  bool swapChainUpdated = false;
#pragma region Update to IDXGISwapChain1 interface
  if (riid == __uuidof(IDXGISwapChain1) && swapChainVersion_ < SWAPCHAIN_1) {
    IDXGISwapChain1 *swapchain1 = nullptr;

    HRESULT hr = swapChain_->QueryInterface(&swapchain1);
    if (FAILED(hr)) {
      g_messageLog.Log(MessageLog::LOG_ERROR, "DXGISwapChain",
                       "QueryInterface IDXGISwapChain1 failed", hr);
      return E_NOINTERFACE;
    }

    swapChain_->Release();

    swapChain_ = swapchain1;
    swapChainVersion_ = SWAPCHAIN_1;
    swapChainUpdated = true;
  }
#pragma endregion
#pragma region Update to IDXGISwapChain2 interface
  if (riid == __uuidof(IDXGISwapChain2) && swapChainVersion_ < SWAPCHAIN_2) {
    IDXGISwapChain2 *swapchain2 = nullptr;

    HRESULT hr = swapChain_->QueryInterface(&swapchain2);
    if (FAILED(hr)) {
      g_messageLog.Log(MessageLog::LOG_ERROR, "DXGISwapChain",
                       "QueryInterface IDXGISwapChain2 failed", hr);
      return E_NOINTERFACE;
    }

    swapChain_->Release();

    swapChain_ = swapchain2;
    swapChainVersion_ = SWAPCHAIN_2;
    swapChainUpdated = true;
  }
#pragma endregion
#pragma region Update to IDXGISwapChain3 interface
  if (riid == __uuidof(IDXGISwapChain3) && swapChainVersion_ < SWAPCHAIN_3) {
    IDXGISwapChain3 *swapchain3 = nullptr;

    HRESULT hr = swapChain_->QueryInterface(&swapchain3);
    if (FAILED(hr)) {
      g_messageLog.Log(MessageLog::LOG_ERROR, "DXGISwapChain",
                       "QueryInterface IDXGISwapChain3 failed", hr);
      return E_NOINTERFACE;
    }

    swapChain_->Release();

    swapChain_ = swapchain3;
    swapChainVersion_ = SWAPCHAIN_3;
    swapChainUpdated = true;
  }
#pragma endregion
#pragma region Update to IDXGISwapChain4 interface
  if (riid == __uuidof(IDXGISwapChain4) && swapChainVersion_ < SWAPCHAIN_4) {
    IDXGISwapChain4 *swapchain4 = nullptr;

    HRESULT hr = swapChain_->QueryInterface(&swapchain4);
    if (FAILED(hr)) {
      g_messageLog.Log(MessageLog::LOG_ERROR, "DXGISwapChain",
                       "QueryInterface IDXGISwapChain3 failed", hr);
      return E_NOINTERFACE;
    }

    swapChain_->Release();

    swapChain_ = swapchain4;
    swapChainVersion_ = SWAPCHAIN_4;
    swapChainUpdated = true;
  }
#pragma endregion

  if (swapChainUpdated || g_uwpApp) {
    AddRef();
    *ppvObj = this;
    return S_OK;
  }
  else {
    return swapChain_->QueryInterface(riid, ppvObj);
  }
}

ULONG STDMETHODCALLTYPE DXGISwapChain::AddRef()
{
  refCount_++;

  return swapChain_->AddRef();
}
ULONG STDMETHODCALLTYPE DXGISwapChain::Release()
{
  if (--refCount_ == 0) {
    direct3DDevice_->Release();
  }

  ULONG ref = swapChain_->Release();
  if (refCount_ == 0 && ref != 0) {
    ref = 0;
  }

  if (ref == 0) {
    assert(refCount_ <= 0);

    delete this;
  }

  return ref;
}

// IDXGIObject
HRESULT STDMETHODCALLTYPE DXGISwapChain::SetPrivateData(REFGUID Name, UINT DataSize,
                                                        const void *pData)
{
  return swapChain_->SetPrivateData(Name, DataSize, pData);
}
HRESULT STDMETHODCALLTYPE DXGISwapChain::SetPrivateDataInterface(REFGUID Name,
                                                                 const IUnknown *pUnknown)
{
  return swapChain_->SetPrivateDataInterface(Name, pUnknown);
}
HRESULT STDMETHODCALLTYPE DXGISwapChain::GetPrivateData(REFGUID Name, UINT *pDataSize, void *pData)
{
  return swapChain_->GetPrivateData(Name, pDataSize, pData);
}
HRESULT STDMETHODCALLTYPE DXGISwapChain::GetParent(REFIID riid, void **ppParent)
{
  return swapChain_->GetParent(riid, ppParent);
}

// IDXGIDeviceSubObject
HRESULT STDMETHODCALLTYPE DXGISwapChain::GetDevice(REFIID riid, void **ppDevice)
{
  if (ppDevice == nullptr) {
    return DXGI_ERROR_INVALID_CALL;
  }

  return direct3DDevice_->QueryInterface(riid, ppDevice);
}

// IDXGISwapChain
HRESULT STDMETHODCALLTYPE DXGISwapChain::Present(UINT SyncInterval, UINT Flags)
{
  // skip presents that are discarded
  if (Flags != DXGI_PRESENT_TEST) {
    switch (d3dVersion_) {
      case D3DVersion_11:
        d3d11Renderer_->on_present();
        break;
      case D3DVersion_12:
        d3d12Renderer_->on_present();
        break;
    }
  }

  return swapChain_->Present(SyncInterval, Flags);
}
HRESULT STDMETHODCALLTYPE DXGISwapChain::GetBuffer(UINT Buffer, REFIID riid, void **ppSurface)
{
  return swapChain_->GetBuffer(Buffer, riid, ppSurface);
}
HRESULT STDMETHODCALLTYPE DXGISwapChain::SetFullscreenState(BOOL Fullscreen, IDXGIOutput *pTarget)
{
  return swapChain_->SetFullscreenState(Fullscreen, pTarget);
}
HRESULT STDMETHODCALLTYPE DXGISwapChain::GetFullscreenState(BOOL *pFullscreen,
                                                            IDXGIOutput **ppTarget)
{
  return swapChain_->GetFullscreenState(pFullscreen, ppTarget);
}
HRESULT STDMETHODCALLTYPE DXGISwapChain::GetDesc(DXGI_SWAP_CHAIN_DESC *pDesc)
{
  return swapChain_->GetDesc(pDesc);
}
HRESULT STDMETHODCALLTYPE DXGISwapChain::ResizeBuffers(UINT BufferCount, UINT Width, UINT Height,
                                                       DXGI_FORMAT NewFormat, UINT SwapChainFlags)
{
  d3d11Renderer_.reset();
  d3d12Renderer_.reset();

  const HRESULT hr =
      swapChain_->ResizeBuffers(BufferCount, Width, Height, NewFormat, SwapChainFlags);
  if (FAILED(hr)) {
    g_messageLog.Log(MessageLog::LOG_ERROR, "DXGISwapChain", "Resize Buffers failed, HRESULT", hr);
  }

  if (SUCCEEDED(hr) || hr == DXGI_ERROR_INVALID_CALL) {
    switch (d3dVersion_) {
      case D3DVersion_11:
        d3d11Renderer_ = std::make_unique<gameoverlay::d3d11_renderer>(
            static_cast<ID3D11Device *>(direct3DDevice_), swapChain_);
        break;
      case D3DVersion_12:
        d3d12Renderer_ = std::make_unique<gameoverlay::d3d12_renderer>(
            static_cast<ID3D12CommandQueue *>(direct3DDevice_),
            static_cast<IDXGISwapChain3 *>(swapChain_));
        break;
    }
  }

  return hr;
}
HRESULT STDMETHODCALLTYPE DXGISwapChain::ResizeTarget(const DXGI_MODE_DESC *pNewTargetParameters)
{
  return swapChain_->ResizeTarget(pNewTargetParameters);
}
HRESULT STDMETHODCALLTYPE DXGISwapChain::GetContainingOutput(IDXGIOutput **ppOutput)
{
  return swapChain_->GetContainingOutput(ppOutput);
}
HRESULT STDMETHODCALLTYPE DXGISwapChain::GetFrameStatistics(DXGI_FRAME_STATISTICS *pStats)
{
  return swapChain_->GetFrameStatistics(pStats);
}
HRESULT STDMETHODCALLTYPE DXGISwapChain::GetLastPresentCount(UINT *pLastPresentCount)
{
  return swapChain_->GetLastPresentCount(pLastPresentCount);
}

// IDXGISwapChain1
HRESULT STDMETHODCALLTYPE DXGISwapChain::GetDesc1(DXGI_SWAP_CHAIN_DESC1 *pDesc)
{
  return static_cast<IDXGISwapChain1 *>(swapChain_)->GetDesc1(pDesc);
}
HRESULT STDMETHODCALLTYPE DXGISwapChain::GetFullscreenDesc(DXGI_SWAP_CHAIN_FULLSCREEN_DESC *pDesc)
{
  return static_cast<IDXGISwapChain1 *>(swapChain_)->GetFullscreenDesc(pDesc);
}
HRESULT STDMETHODCALLTYPE DXGISwapChain::GetHwnd(HWND *pHwnd)
{
  return static_cast<IDXGISwapChain1 *>(swapChain_)->GetHwnd(pHwnd);
}
HRESULT STDMETHODCALLTYPE DXGISwapChain::GetCoreWindow(REFIID refiid, void **ppUnk)
{
  return static_cast<IDXGISwapChain1 *>(swapChain_)->GetCoreWindow(refiid, ppUnk);
}
HRESULT STDMETHODCALLTYPE DXGISwapChain::Present1(UINT SyncInterval, UINT PresentFlags,
                                                  const DXGI_PRESENT_PARAMETERS *pPresentParameters)
{
  // skip presents that are discarded
  if (PresentFlags != DXGI_PRESENT_TEST) {
    switch (d3dVersion_) {
      case D3DVersion_11:
        d3d11Renderer_->on_present();
        break;
      case D3DVersion_12:
        d3d12Renderer_->on_present();
        break;
    }
  }

  return static_cast<IDXGISwapChain1 *>(swapChain_)
      ->Present1(SyncInterval, PresentFlags, pPresentParameters);
}
BOOL STDMETHODCALLTYPE DXGISwapChain::IsTemporaryMonoSupported()
{
  return static_cast<IDXGISwapChain1 *>(swapChain_)->IsTemporaryMonoSupported();
}
HRESULT STDMETHODCALLTYPE DXGISwapChain::GetRestrictToOutput(IDXGIOutput **ppRestrictToOutput)
{
  return static_cast<IDXGISwapChain1 *>(swapChain_)->GetRestrictToOutput(ppRestrictToOutput);
}
HRESULT STDMETHODCALLTYPE DXGISwapChain::SetBackgroundColor(const DXGI_RGBA *pColor)
{
  return static_cast<IDXGISwapChain1 *>(swapChain_)->SetBackgroundColor(pColor);
}
HRESULT STDMETHODCALLTYPE DXGISwapChain::GetBackgroundColor(DXGI_RGBA *pColor)
{
  return static_cast<IDXGISwapChain1 *>(swapChain_)->GetBackgroundColor(pColor);
}
HRESULT STDMETHODCALLTYPE DXGISwapChain::SetRotation(DXGI_MODE_ROTATION Rotation)
{
  return static_cast<IDXGISwapChain1 *>(swapChain_)->SetRotation(Rotation);
}
HRESULT STDMETHODCALLTYPE DXGISwapChain::GetRotation(DXGI_MODE_ROTATION *pRotation)
{
  return static_cast<IDXGISwapChain1 *>(swapChain_)->GetRotation(pRotation);
}

// IDXGISwapChain2
HRESULT STDMETHODCALLTYPE DXGISwapChain::SetSourceSize(UINT Width, UINT Height)
{
  return static_cast<IDXGISwapChain2 *>(swapChain_)->SetSourceSize(Width, Height);
}
HRESULT STDMETHODCALLTYPE DXGISwapChain::GetSourceSize(UINT *pWidth, UINT *pHeight)
{
  return static_cast<IDXGISwapChain2 *>(swapChain_)->GetSourceSize(pWidth, pHeight);
}
HRESULT STDMETHODCALLTYPE DXGISwapChain::SetMaximumFrameLatency(UINT MaxLatency)
{
  return static_cast<IDXGISwapChain2 *>(swapChain_)->SetMaximumFrameLatency(MaxLatency);
}
HRESULT STDMETHODCALLTYPE DXGISwapChain::GetMaximumFrameLatency(UINT *pMaxLatency)
{
  return static_cast<IDXGISwapChain2 *>(swapChain_)->GetMaximumFrameLatency(pMaxLatency);
}
HANDLE STDMETHODCALLTYPE DXGISwapChain::GetFrameLatencyWaitableObject()
{
  return static_cast<IDXGISwapChain2 *>(swapChain_)->GetFrameLatencyWaitableObject();
}
HRESULT STDMETHODCALLTYPE DXGISwapChain::SetMatrixTransform(const DXGI_MATRIX_3X2_F *pMatrix)
{
  return static_cast<IDXGISwapChain2 *>(swapChain_)->SetMatrixTransform(pMatrix);
}
HRESULT STDMETHODCALLTYPE DXGISwapChain::GetMatrixTransform(DXGI_MATRIX_3X2_F *pMatrix)
{
  return static_cast<IDXGISwapChain2 *>(swapChain_)->GetMatrixTransform(pMatrix);
}

// IDXGISwapChain3
UINT STDMETHODCALLTYPE DXGISwapChain::GetCurrentBackBufferIndex()
{
  return static_cast<IDXGISwapChain3 *>(swapChain_)->GetCurrentBackBufferIndex();
}
HRESULT STDMETHODCALLTYPE DXGISwapChain::CheckColorSpaceSupport(DXGI_COLOR_SPACE_TYPE ColorSpace,
                                                                UINT *pColorSpaceSupport)
{
  return static_cast<IDXGISwapChain3 *>(swapChain_)
      ->CheckColorSpaceSupport(ColorSpace, pColorSpaceSupport);
}
HRESULT STDMETHODCALLTYPE DXGISwapChain::SetColorSpace1(DXGI_COLOR_SPACE_TYPE ColorSpace)
{
  return static_cast<IDXGISwapChain3 *>(swapChain_)->SetColorSpace1(ColorSpace);
}
HRESULT STDMETHODCALLTYPE DXGISwapChain::ResizeBuffers1(UINT BufferCount, UINT Width, UINT Height,
                                                        DXGI_FORMAT Format, UINT SwapChainFlags,
                                                        const UINT *pCreationNodeMask,
                                                        IUnknown *const *ppPresentQueue)
{
  d3d11Renderer_.reset();
  d3d12Renderer_.reset();

  const HRESULT hr = static_cast<IDXGISwapChain3 *>(swapChain_)
                         ->ResizeBuffers1(BufferCount, Width, Height, Format, SwapChainFlags,
                                          pCreationNodeMask, ppPresentQueue);
  if (FAILED(hr)) {
    g_messageLog.Log(MessageLog::LOG_ERROR, "DXGISwapChain", "Resize buffers 1 failed, HRESULT",
                     hr);
  }

  if (SUCCEEDED(hr) || hr == DXGI_ERROR_INVALID_CALL) {
    switch (d3dVersion_) {
      case D3DVersion_11:
        d3d11Renderer_ = std::make_unique<gameoverlay::d3d11_renderer>(
            static_cast<ID3D11Device *>(direct3DDevice_), swapChain_);
        break;
      case D3DVersion_12:
        d3d12Renderer_ = std::make_unique<gameoverlay::d3d12_renderer>(
            static_cast<ID3D12CommandQueue *>(direct3DDevice_),
            static_cast<IDXGISwapChain3 *>(swapChain_));
        break;
    }
  }

  return hr;
}

// IDXGISwapChain4
HRESULT STDMETHODCALLTYPE DXGISwapChain::SetHDRMetaData(DXGI_HDR_METADATA_TYPE Type, UINT Size,
                                                        _In_reads_opt_(Size) void *pMetaData)
{
  return static_cast<IDXGISwapChain4 *>(swapChain_)->SetHDRMetaData(Type, Size, pMetaData);
}