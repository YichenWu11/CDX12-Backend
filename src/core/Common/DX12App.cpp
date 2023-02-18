#include <CDX12/Common/DX12App.h>
#include <WindowsX.h>

using Microsoft::WRL::ComPtr;
using namespace std;
using namespace DirectX;

namespace Chen::CDX12 {
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
        if (m_device.Get() != nullptr)
            FlushCommandQueue();

        if (!m_rtvCpuDH.IsNull())
            DescriptorHeapMngr::GetInstance().GetRTVCpuDH()->Free(std::move(m_rtvCpuDH));
        if (!m_dsvCpuDH.IsNull())
            DescriptorHeapMngr::GetInstance().GetDSVCpuDH()->Free(std::move(m_dsvCpuDH));
        if (!m_csuCpuDH.IsNull())
            DescriptorHeapMngr::GetInstance().GetCSUCpuDH()->Free(std::move(m_csuCpuDH));
        if (!m_csuGpuDH.IsNull())
            DescriptorHeapMngr::GetInstance().GetCSUGpuDH()->Free(std::move(m_csuGpuDH));

        mApp = nullptr;
    }

    HINSTANCE DX12App::AppInst() const {
        return mhAppInst;
    }

    HWND DX12App::MainWnd() const {
        return mhMainWnd;
    }

    float DX12App::AspectRatio() const {
        return static_cast<float>(m_client_width) / (m_client_height);
    }

    void DX12App::SetCustomWindowText(LPCWSTR text) {
        std::wstring windowText = m_title + L": " + text;
        SetWindowText(mhMainWnd, windowText.c_str());
    }

    int DX12App::Run() {
        MSG msg = {0};

        m_timer.Reset();

        while (msg.message != WM_QUIT) {
            // If there are Window messages then process them.
            if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            // Otherwise, do animation/game stuff.
            else {
                m_timer.Tick();

                if (!mAppPaused) {
                    CalculateFrameStats();
                    Tick(m_timer);
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
        m_rtvCpuDH = DescriptorHeapMngr::GetInstance().GetRTVCpuDH()->Allocate(10);

        m_dsvCpuDH = DescriptorHeapMngr::GetInstance().GetDSVCpuDH()->Allocate(10);
    }

    void DX12App::OnResize() {
        assert(m_device.Get());
        assert(m_swapchain);
        assert(m_direct_cmdlist_alloc);

        // Flush before changing any resources.
        FlushCommandQueue();

        ThrowIfFailed(m_cmdlist->Reset(m_direct_cmdlist_alloc.Get(), nullptr));

        // Release the previous resources we will be recreating.
        for (int i = 0; i < s_swapchain_buffer_count; ++i) {
            m_swapchain_buffer[i].reset(nullptr);
            m_depthstencil_buffer[i].reset();
        }

        // Resize the swap chain.
        ThrowIfFailed(m_swapchain->ResizeBuffers(
            s_swapchain_buffer_count,
            m_client_width, m_client_height,
            m_backbuffer_format,
            DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

        m_curr_back_buffer = 0;

        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvCpuDH.GetCpuHandle());
        CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_dsvCpuDH.GetCpuHandle());

        // Create a RTV and DSV for each frame.
        for (uint32_t n = 0; n < m_frame_count; ++n) {
            m_swapchain_buffer[n]    = std::unique_ptr<Texture>(new Texture(m_device.Get(), m_swapchain.Get(), n));
            m_depthstencil_buffer[n] = std::unique_ptr<Texture>(
                new Texture(
                    m_device.Get(),
                    m_scissorRect.right,
                    m_scissorRect.bottom,
                    DXGI_FORMAT_D32_FLOAT,
                    TextureDimension::Tex2D,
                    1,
                    1,
                    Texture::TextureUsage::DepthStencil,
                    D3D12_RESOURCE_STATE_DEPTH_READ));

            m_device->CreateRenderTargetView(m_swapchain_buffer[n]->GetResource(), nullptr, rtvHandle);
            m_device->CreateDepthStencilView(m_depthstencil_buffer[n]->GetResource(), nullptr, dsvHandle);
            rtvHandle.Offset(1, m_rtvCpuDH.GetDescriptorSize());
            dsvHandle.Offset(1, m_dsvCpuDH.GetDescriptorSize());
        }

        ThrowIfFailed(m_cmdlist->Close());
        m_cmdqueue.Execute(m_cmdlist.Get());

        // Flush before changing any resources.
        FlushCommandQueue();

        m_screen_viewport.TopLeftX = 0.0f;
        m_screen_viewport.TopLeftY = 0.0f;
        m_screen_viewport.Width    = static_cast<float>(m_client_width);
        m_screen_viewport.Height   = static_cast<float>(m_client_height);
        m_screen_viewport.MinDepth = 0.0f;
        m_screen_viewport.MaxDepth = 1.0f;

        m_scissorRect = {0, 0, m_client_width, m_client_height};
    }

    LRESULT DX12App::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        switch (msg) {
                // WM_ACTIVATE is sent when the window is activated or deactivated.
                // We pause the game when the window is deactivated and unpause it
                // when it becomes active.
            case WM_ACTIVATE:
                if (LOWORD(wParam) == WA_INACTIVE) {
                    mAppPaused = true;
                    m_timer.Stop();
                }
                else {
                    mAppPaused = false;
                    m_timer.Start();
                }
                return 0;

                // WM_SIZE is sent when the user resizes the window.
            case WM_SIZE:
                // Save the new client area dimensions.
                m_client_width  = LOWORD(lParam);
                m_client_height = HIWORD(lParam);
                if (m_device.Get()) {
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
                        else // API call such as SetWindowPos or m_swapchain->SetFullscreenState.
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
                m_timer.Stop();
                return 0;

                // WM_EXITSIZEMOVE is sent when the user releases the resize bars.
                // Here we reset everything based on the new window dimensions.
            case WM_EXITSIZEMOVE:
                mAppPaused = false;
                mResizing  = false;
                m_timer.Start();
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
        RECT R = {0, 0, m_client_width, m_client_height};
        AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
        int width  = R.right - R.left;
        int height = R.bottom - R.top;

        mhMainWnd = CreateWindow(L"MainWnd", m_title.c_str(),
                                 WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, mhAppInst, 0);
        if (!mhMainWnd) {
            MessageBox(0, L"CreateWindow Failed.", 0, 0);
            return false;
        }

        // ShowWindow(mhMainWnd, SW_SHOWDEFAULT);
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
        ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&m_dxgiFactory)));

        // Try to create hardware device.
        HRESULT hardwareResult = m_device.Create(nullptr, D3D_FEATURE_LEVEL_11_0);

        // Fallback to WARP device.
        if (FAILED(hardwareResult)) {
            ComPtr<IDXGIAdapter> pWarpAdapter;
            ThrowIfFailed(m_dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&pWarpAdapter)));

            ThrowIfFailed(m_device.Create(pWarpAdapter.Get(), D3D_FEATURE_LEVEL_11_0));
        }

        D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
        msQualityLevels.Format           = m_backbuffer_format;
        msQualityLevels.SampleCount      = 4;
        msQualityLevels.Flags            = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
        msQualityLevels.NumQualityLevels = 0;
        ThrowIfFailed(m_device->CheckFeatureSupport(
            D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
            &msQualityLevels,
            sizeof(msQualityLevels)));

        ThrowIfFailed(m_device->CreateFence(m_current_fence, D3D12_FENCE_FLAG_NONE,
                                            IID_PPV_ARGS(&m_fence)));

        DescriptorHeapMngr::GetInstance().Init(
            m_device.Get(),
            numCpuCSU,
            numCpuRTV,
            numCpuDSV,
            numGpuCSU_static,
            numGpuCSU_dynamic);

        m_frameresource_mngr = std::make_unique<FrameResourceMngr>(m_frame_count, m_device.Get());

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
        ThrowIfFailed(m_device->CreateCommandQueue(
            &queueDesc,
            IID_PPV_ARGS(m_cmdqueue.raw.GetAddressOf())));

        ThrowIfFailed(m_device->CreateCommandAllocator(
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            IID_PPV_ARGS(m_direct_cmdlist_alloc.GetAddressOf())));

        ThrowIfFailed(m_device->CreateCommandList(
            0,
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            m_direct_cmdlist_alloc.Get(),
            nullptr,
            IID_PPV_ARGS(m_cmdlist.raw.GetAddressOf())));

        // Start off in a closed state.  This is because the first time we refer
        // to the command list we will Reset it, and it needs to be closed before
        // calling Reset.
        m_cmdlist->Close();
    }

    void DX12App::CreateSwapChain() {
        m_swapchain.Reset();

        // DXGI_SWAP_CHAIN_DESC sd;
        // sd.BufferDesc.Width                   = m_client_width;
        // sd.BufferDesc.Height                  = m_client_height;
        // sd.BufferDesc.RefreshRate.Numerator   = 60;
        // sd.BufferDesc.RefreshRate.Denominator = 1;
        // sd.BufferDesc.Format                  = m_backbuffer_format;
        // sd.BufferDesc.ScanlineOrdering        = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
        // sd.BufferDesc.Scaling                 = DXGI_MODE_SCALING_UNSPECIFIED;
        // sd.SampleDesc.Count                   = 1;
        // sd.SampleDesc.Quality                 = 0;
        // sd.BufferUsage                        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        // sd.BufferCount                        = s_swapchain_buffer_count;
        // sd.OutputWindow                       = mhMainWnd;
        // sd.Windowed                           = true;
        // sd.SwapEffect                         = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        // sd.Flags                              = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
        swapChainDesc.BufferCount           = m_frame_count;
        swapChainDesc.Width                 = m_client_width;
        swapChainDesc.Height                = m_client_height;
        swapChainDesc.Format                = m_backbuffer_format;
        swapChainDesc.BufferUsage           = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.SwapEffect            = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapChainDesc.SampleDesc.Count      = 1;

        ComPtr<IDXGISwapChain1> swapChain;
        ThrowIfFailed(m_dxgiFactory->CreateSwapChainForHwnd(
            m_cmdqueue.Get(), // Swap chain needs the queue so that it can force a flush on it.
            mhMainWnd,
            &swapChainDesc,
            nullptr,
            nullptr,
            &swapChain));

        // This sample does not support fullscreen transitions.
        ThrowIfFailed(m_dxgiFactory->MakeWindowAssociation(mhMainWnd, DXGI_MWA_NO_ALT_ENTER));

        ThrowIfFailed(swapChain.As(&m_swapchain));
    }

    void DX12App::FlushCommandQueue() {
        // Advance the fence value to mark commands up to this fence point.
        m_current_fence++;

        // Add an instruction to the command queue to set a new fence point.  Because we
        // are on the GPU timeline, the new fence point won't be set until the GPU finishes
        // processing all the commands prior to this Signal().
        ThrowIfFailed(m_cmdqueue->Signal(m_fence.Get(), m_current_fence));

        // Wait until the GPU has completed commands up to this fence point.
        if (m_fence->GetCompletedValue() < m_current_fence) {
            HANDLE eventHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);

            // Fire event when GPU hits current fence.
            ThrowIfFailed(m_fence->SetEventOnCompletion(m_current_fence, eventHandle));

            // Wait until the GPU hits current fence event is fired.
            WaitForSingleObject(eventHandle, INFINITE);
            CloseHandle(eventHandle);
        }
    }

    ID3D12Resource* DX12App::CurrentBackBuffer() const {
        return m_swapchain_buffer[m_curr_back_buffer]->GetResource();
    }

    D3D12_CPU_DESCRIPTOR_HANDLE DX12App::CurrentBackBufferView() const {
        return m_rtvCpuDH.GetCpuHandle(m_curr_back_buffer);
    }

    D3D12_CPU_DESCRIPTOR_HANDLE DX12App::DepthStencilView() const {
        return m_dsvCpuDH.GetCpuHandle(m_curr_back_buffer);
    }

    void DX12App::CalculateFrameStats() {
        static int   frameCnt    = 0;
        static float timeElapsed = 0.0f;

        frameCnt++;

        // Compute averages over one second period.
        if ((m_timer.TotalTime() - timeElapsed) >= 1.0f) {
            float fps  = (float)frameCnt; // fps = frameCnt / 1
            float mspf = 1000.0f / fps;

            wstring fpsStr  = to_wstring(fps);
            wstring mspfStr = to_wstring(mspf);

            wstring windowText = m_title + L"    fps: " + fpsStr + L"   mspf: " + mspfStr;

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
        while (m_dxgiFactory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND) {
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

            LogOutputDisplayModes(output, m_backbuffer_format);

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

} // namespace Chen::CDX12
