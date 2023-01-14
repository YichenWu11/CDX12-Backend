#include <CDX12/Common/DX12App.h>
#include <WindowsX.h>

using namespace Chen::CDX12;
using Microsoft::WRL::ComPtr;
using namespace std;
using namespace DirectX;

LRESULT CALLBACK
MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    return DX12App::GetAppInstance()->MsgProc(hwnd, msg, wParam, lParam);
}

DX12App* DX12App::mApp = nullptr;

DX12App* DX12App::GetAppInstance() {
    return mApp;
}

DX12App::DX12App(HINSTANCE hInstance) :
    mhAppInst(hInstance) {
    // Singleton
    assert(mApp == nullptr);
    mApp = this;
}

DX12App::~DX12App() {
    if (mDevice.Get() != nullptr)
        FlushCommandQueue();

    if (!rtvCpuDH.IsNull())
        DescriptorHeapMngr::GetInstance().GetRTVCpuDH()->Free(std::move(rtvCpuDH));
    if (!dsvCpuDH.IsNull())
        DescriptorHeapMngr::GetInstance().GetDSVCpuDH()->Free(std::move(dsvCpuDH));
    if (!csuCpuDH.IsNull())
        DescriptorHeapMngr::GetInstance().GetCSUCpuDH()->Free(std::move(csuCpuDH));
    if (!csuGpuDH.IsNull())
        DescriptorHeapMngr::GetInstance().GetCSUGpuDH()->Free(std::move(csuGpuDH));

    mApp = nullptr;
}

HINSTANCE DX12App::AppInst() const {
    return mhAppInst;
}

HWND DX12App::MainWnd() const {
    return mhMainWnd;
}

float DX12App::AspectRatio() const {
    return static_cast<float>(mClientWidth) / (mClientHeight);
}

int DX12App::Run() {
    MSG msg = {0};

    mTimer.Reset();

    while (msg.message != WM_QUIT) {
        // If there are Window messages then process them.
        if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        // Otherwise, do animation/game stuff.
        else {
            mTimer.Tick();

            if (!mAppPaused) {
                CalculateFrameStats();
                LogicTick(mTimer);
                RenderTick(mTimer);
            }
            else {
                Sleep(100);
            }
        }
    }
    return (int)msg.wParam;
}

bool DX12App::Initialize() {
    if (!InitMainWindow())
        return false;

    if (!InitDirect3D())
        return false;

    // Do the initial resize code.
    OnResize();

    FlushCommandQueue();

    return true;
}

void DX12App::CreateRtvAndDsvDescriptorHeaps() {
    rtvCpuDH = DescriptorHeapMngr::GetInstance().GetRTVCpuDH()->Allocate(10);

    dsvCpuDH = DescriptorHeapMngr::GetInstance().GetDSVCpuDH()->Allocate(10);
}

void DX12App::OnResize() {
    assert(mDevice.Get());
    assert(mSwapChain);
    assert(mDirectCmdListAlloc);

    // Flush before changing any resources.
    FlushCommandQueue();

    ThrowIfFailed(mCmdList->Reset(mDirectCmdListAlloc.Get(), nullptr));

    // Release the previous resources we will be recreating.
    for (int i = 0; i < SwapChainBufferCount; ++i)
        mSwapChainBuffer[i].Reset();
    mDepthStencilBuffer.Reset();

    // Resize the swap chain.
    ThrowIfFailed(mSwapChain->ResizeBuffers(
        SwapChainBufferCount,
        mClientWidth, mClientHeight,
        mBackBufferFormat,
        DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

    mCurrBackBuffer = 0;

    for (UINT i = 0; i < SwapChainBufferCount; ++i) {
        ThrowIfFailed(mSwapChain->GetBuffer(i, IID_PPV_ARGS(&mSwapChainBuffer[i])));
        mDevice->CreateRenderTargetView(mSwapChainBuffer[i].Get(), nullptr, rtvCpuDH.GetCpuHandle(i));
    }

    // Create the depth/stencil buffer and view.
    D3D12_RESOURCE_DESC depthStencilDesc =
        Desc::RSRC::Basic(
            D3D12_RESOURCE_DIMENSION_TEXTURE2D,
            mClientWidth,
            mClientHeight,
            DXGI_FORMAT_R24G8_TYPELESS,
            D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

    D3D12_CLEAR_VALUE optClear;
    optClear.Format               = mDepthStencilFormat;
    optClear.DepthStencil.Depth   = 1.0f;
    optClear.DepthStencil.Stencil = 0;
    ThrowIfFailed(mDevice->CreateCommittedResource(
        get_rvalue_ptr(CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT)),
        D3D12_HEAP_FLAG_NONE,
        &depthStencilDesc,
        D3D12_RESOURCE_STATE_COMMON,
        &optClear,
        IID_PPV_ARGS(mDepthStencilBuffer.GetAddressOf())));

    // Create descriptor to mip level 0 of entire resource using the format of the resource.
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = Desc::DSV::Basic(mDepthStencilFormat);

    mDevice->CreateDepthStencilView(mDepthStencilBuffer.Get(), &dsvDesc, DepthStencilView());

    // Transition the resource from its initial state to be used as a depth buffer.
    mCmdList.ResourceBarrierTransition(
        mDepthStencilBuffer.Get(),
        D3D12_RESOURCE_STATE_COMMON,
        D3D12_RESOURCE_STATE_DEPTH_WRITE);

    ThrowIfFailed(mCmdList->Close());
    mCmdQueue.Execute(mCmdList.Get());

    // Flush before changing any resources.
    FlushCommandQueue();

    // Update the viewport transform to cover the client area.
    // mScreenViewport.TopLeftX = mClientWidth * 0.1615f;
    // mScreenViewport.TopLeftY = 0.0f;
    // mScreenViewport.Width = static_cast<float>(mClientWidth/1.653f);
    // mScreenViewport.Height = static_cast<float>(mClientHeight/1.335f);
    // mScreenViewport.MinDepth = 0.0f;
    // mScreenViewport.MaxDepth = 1.0f;

    mScreenViewport.TopLeftX = 0.0f;
    mScreenViewport.TopLeftY = 0.0f;
    mScreenViewport.Width    = static_cast<float>(mClientWidth);
    mScreenViewport.Height   = static_cast<float>(mClientHeight);
    mScreenViewport.MinDepth = 0.0f;
    mScreenViewport.MaxDepth = 1.0f;

    mScissorRect = {0, 0, mClientWidth, mClientHeight};
}

LRESULT DX12App::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
            // WM_ACTIVATE is sent when the window is activated or deactivated.
            // We pause the game when the window is deactivated and unpause it
            // when it becomes active.
        case WM_ACTIVATE:
            if (LOWORD(wParam) == WA_INACTIVE) {
                mAppPaused = true;
                mTimer.Stop();
            }
            else {
                mAppPaused = false;
                mTimer.Start();
            }
            return 0;

            // WM_SIZE is sent when the user resizes the window.
        case WM_SIZE:
            // Save the new client area dimensions.
            mClientWidth  = LOWORD(lParam);
            mClientHeight = HIWORD(lParam);
            if (mDevice.Get()) {
                if (wParam == SIZE_MINIMIZED) {
                    mAppPaused = true;
                    mMinimized = true;
                    mMaximized = false;
                }
                else if (wParam == SIZE_MAXIMIZED) {
                    mAppPaused = false;
                    mMinimized = false;
                    mMaximized = true;
                    OnResize();
                }
                else if (wParam == SIZE_RESTORED) {
                    // Restoring from minimized state?
                    if (mMinimized) {
                        mAppPaused = false;
                        mMinimized = false;
                        OnResize();
                    }

                    // Restoring from maximized state?
                    else if (mMaximized) {
                        mAppPaused = false;
                        mMaximized = false;
                        OnResize();
                    }
                    else if (mResizing) {
                    }
                    else // API call such as SetWindowPos or mSwapChain->SetFullscreenState.
                    {
                        OnResize();
                    }
                }
            }
            return 0;

            // WM_EXITSIZEMOVE is sent when the user grabs the resize bars.
        case WM_ENTERSIZEMOVE:
            mAppPaused = true;
            mResizing  = true;
            mTimer.Stop();
            return 0;

            // WM_EXITSIZEMOVE is sent when the user releases the resize bars.
            // Here we reset everything based on the new window dimensions.
        case WM_EXITSIZEMOVE:
            mAppPaused = false;
            mResizing  = false;
            mTimer.Start();
            OnResize();
            return 0;

            // WM_DESTROY is sent when the window is being destroyed.
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

            // The WM_MENUCHAR message is sent when a menu is active and the user presses
            // a key that does not correspond to any mnemonic or accelerator key.
        case WM_MENUCHAR:
            // Don't beep when we alt-enter.
            return MAKELRESULT(0, MNC_CLOSE);

            // Catch this message so to prevent the window from becoming too small.
        case WM_GETMINMAXINFO:
            ((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
            ((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
            return 0;

        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONDOWN:
            OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            return 0;
        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP:
            OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            return 0;
        case WM_MOUSEMOVE:
            OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            return 0;
        case WM_KEYUP:
            if (wParam == VK_ESCAPE) {
                PostQuitMessage(0);
            }
            return 0;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam); // Default
}

bool DX12App::InitMainWindow() {
    WNDCLASS wc;
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = MainWndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = mhAppInst;
    wc.hIcon         = LoadIcon(0, IDI_APPLICATION);
    wc.hCursor       = LoadCursor(0, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
    wc.lpszMenuName  = 0;
    wc.lpszClassName = L"MainWnd";

    if (!RegisterClass(&wc)) {
        MessageBox(0, L"RegisterClass Failed.", 0, 0);
        return false;
    }

    // Compute window rectangle dimensions based on requested client area dimensions.
    RECT R = {0, 0, mClientWidth, mClientHeight};
    AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
    int width  = R.right - R.left;
    int height = R.bottom - R.top;

    mhMainWnd = CreateWindow(L"MainWnd", mMainWndCaption.c_str(),
                             WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, mhAppInst, 0);
    if (!mhMainWnd) {
        MessageBox(0, L"CreateWindow Failed.", 0, 0);
        return false;
    }

    ShowWindow(mhMainWnd, SW_SHOWMAXIMIZED);
    UpdateWindow(mhMainWnd);

    return true;
}

bool DX12App::InitDirect3D() {
#if defined(DEBUG) || defined(_DEBUG)
    {
        ComPtr<ID3D12Debug> debugController;
        ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
        debugController->EnableDebugLayer();
    }
#endif

    // create dxgi
    ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&mdxgiFactory)));

    // Try to create hardware device.
    HRESULT hardwareResult = mDevice.Create(nullptr, D3D_FEATURE_LEVEL_11_0);

    // Fallback to WARP device.
    if (FAILED(hardwareResult)) {
        ComPtr<IDXGIAdapter> pWarpAdapter;
        ThrowIfFailed(mdxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&pWarpAdapter)));

        ThrowIfFailed(mDevice.Create(pWarpAdapter.Get(), D3D_FEATURE_LEVEL_11_0));
    }

    D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
    msQualityLevels.Format           = mBackBufferFormat;
    msQualityLevels.SampleCount      = 4;
    msQualityLevels.Flags            = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
    msQualityLevels.NumQualityLevels = 0;
    ThrowIfFailed(mDevice->CheckFeatureSupport(
        D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
        &msQualityLevels,
        sizeof(msQualityLevels)));

    ThrowIfFailed(mDevice->CreateFence(mCurrentFence, D3D12_FENCE_FLAG_NONE,
                                       IID_PPV_ARGS(&mFence)));

    DescriptorHeapMngr::GetInstance().Init(
        mDevice.Get(),
        numCpuCSU,
        numCpuRTV,
        numCpuDSV,
        numGpuCSU_static,
        numGpuCSU_dynamic);

    mFrameResourceMngr = std::make_unique<FrameResourceMngr>(gNumFrameResource, mDevice.Get());

#ifdef _DEBUG
    LogAdapters();
#endif
    CreateCommandObjects();
    CreateSwapChain();
    CreateRtvAndDsvDescriptorHeaps();

    return true;
}

void DX12App::CreateCommandObjects() {
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type                     = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queueDesc.Flags                    = D3D12_COMMAND_QUEUE_FLAG_NONE;
    ThrowIfFailed(mDevice->CreateCommandQueue(
        &queueDesc,
        IID_PPV_ARGS(mCmdQueue.raw.GetAddressOf())));

    ThrowIfFailed(mDevice->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        IID_PPV_ARGS(mDirectCmdListAlloc.GetAddressOf())));

    ThrowIfFailed(mDevice->CreateCommandList(
        0,
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        mDirectCmdListAlloc.Get(),
        nullptr,
        IID_PPV_ARGS(mCmdList.raw.GetAddressOf())));

    // Start off in a closed state.  This is because the first time we refer
    // to the command list we will Reset it, and it needs to be closed before
    // calling Reset.
    mCmdList->Close();
}

void DX12App::CreateSwapChain() {
    mSwapChain.Reset();

    DXGI_SWAP_CHAIN_DESC sd;
    sd.BufferDesc.Width                   = mClientWidth;
    sd.BufferDesc.Height                  = mClientHeight;
    sd.BufferDesc.RefreshRate.Numerator   = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferDesc.Format                  = mBackBufferFormat;
    sd.BufferDesc.ScanlineOrdering        = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    sd.BufferDesc.Scaling                 = DXGI_MODE_SCALING_UNSPECIFIED;
    sd.SampleDesc.Count                   = 1;
    sd.SampleDesc.Quality                 = 0;
    sd.BufferUsage                        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.BufferCount                        = SwapChainBufferCount;
    sd.OutputWindow                       = mhMainWnd;
    sd.Windowed                           = true;
    sd.SwapEffect                         = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    sd.Flags                              = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    // Note: Swap chain uses queue to perform flush.
    ThrowIfFailed(mdxgiFactory->CreateSwapChain(
        mCmdQueue.Get(),
        &sd,
        mSwapChain.GetAddressOf()));
}

void DX12App::FlushCommandQueue() {
    // Advance the fence value to mark commands up to this fence point.
    mCurrentFence++;

    // Add an instruction to the command queue to set a new fence point.  Because we
    // are on the GPU timeline, the new fence point won't be set until the GPU finishes
    // processing all the commands prior to this Signal().
    ThrowIfFailed(mCmdQueue->Signal(mFence.Get(), mCurrentFence));

    // Wait until the GPU has completed commands up to this fence point.
    if (mFence->GetCompletedValue() < mCurrentFence) {
        HANDLE eventHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);

        // Fire event when GPU hits current fence.
        ThrowIfFailed(mFence->SetEventOnCompletion(mCurrentFence, eventHandle));

        // Wait until the GPU hits current fence event is fired.
        WaitForSingleObject(eventHandle, INFINITE);
        CloseHandle(eventHandle);
    }
}

ID3D12Resource* DX12App::CurrentBackBuffer() const {
    return mSwapChainBuffer[mCurrBackBuffer].Get();
}

D3D12_CPU_DESCRIPTOR_HANDLE DX12App::CurrentBackBufferView() const {
    return rtvCpuDH.GetCpuHandle(mCurrBackBuffer);
}

D3D12_CPU_DESCRIPTOR_HANDLE DX12App::DepthStencilView() const {
    return dsvCpuDH.GetCpuHandle(0);
}

void DX12App::CalculateFrameStats() {
    static int   frameCnt    = 0;
    static float timeElapsed = 0.0f;

    frameCnt++;

    // Compute averages over one second period.
    if ((mTimer.TotalTime() - timeElapsed) >= 1.0f) {
        float fps  = (float)frameCnt; // fps = frameCnt / 1
        float mspf = 1000.0f / fps;

        wstring fpsStr  = to_wstring(fps);
        wstring mspfStr = to_wstring(mspf);

        wstring windowText = mMainWndCaption + L"    fps: " + fpsStr + L"   mspf: " + mspfStr;

        // SetWindowText(mhMainWnd, windowText.c_str());

        // Reset for next average.
        frameCnt = 0;
        timeElapsed += 1.0f;
    }
}

void DX12App::LogAdapters() {
    UINT                       i       = 0;
    IDXGIAdapter*              adapter = nullptr;
    std::vector<IDXGIAdapter*> adapterList;
    while (mdxgiFactory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND) {
        DXGI_ADAPTER_DESC desc;
        adapter->GetDesc(&desc);

        std::wstring text = L"***Adapter: ";
        text += desc.Description;
        text += L"\n";

        OutputDebugString(text.c_str());

        adapterList.push_back(adapter);

        ++i;
    }

    for (size_t i = 0; i < adapterList.size(); ++i) {
        LogAdapterOutputs(adapterList[i]);
        ReleaseCom(adapterList[i]);
    }
}

void DX12App::LogAdapterOutputs(IDXGIAdapter* adapter) {
    UINT         i      = 0;
    IDXGIOutput* output = nullptr;
    while (adapter->EnumOutputs(i, &output) != DXGI_ERROR_NOT_FOUND) {
        DXGI_OUTPUT_DESC desc;
        output->GetDesc(&desc);

        std::wstring text = L"***Output: ";
        text += desc.DeviceName;
        text += L"\n";
        OutputDebugString(text.c_str());

        LogOutputDisplayModes(output, mBackBufferFormat);

        ReleaseCom(output);

        ++i;
    }
}

void DX12App::LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format) {
    UINT count = 0;
    UINT flags = 0;

    // Call with nullptr to get list count.
    output->GetDisplayModeList(format, flags, &count, nullptr);

    std::vector<DXGI_MODE_DESC> modeList(count);
    output->GetDisplayModeList(format, flags, &count, &modeList[0]);

    for (auto& x : modeList) {
        UINT         n = x.RefreshRate.Numerator;
        UINT         d = x.RefreshRate.Denominator;
        std::wstring text =
            L"Width = " + std::to_wstring(x.Width) + L" " + L"Height = " + std::to_wstring(x.Height) + L" " + L"Refresh = " + std::to_wstring(n) + L"/" + std::to_wstring(d) + L"\n";

        ::OutputDebugString(text.c_str());
    }
}
