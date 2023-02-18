#pragma once

#include <CDX12/Common/Timer.h>

#include <CDX12/DescripitorHeap/CPUDescriptorHeap.h>
#include <CDX12/DescripitorHeap/DescriptorHeapWrapper.h>
#include <CDX12/DescripitorHeap/GPUDescriptorHeap.h>

#include <CDX12/CmdQueue.h>
#include <CDX12/DXUtil.h>
#include <CDX12/DescriptorHeapMngr.h>
#include <CDX12/Device.h>
#include <CDX12/FrameResourceMngr.h>
#include <CDX12/GeneralDesc.h>
#include <CDX12/Metalib.h>
#include <CDX12/Resource/ResourceStateTracker.h>
#include <CDX12/Resource/Texture.h>

namespace Chen::CDX12 {
    class DX12App {
    protected:
        DX12App(HINSTANCE hInstance);
        DX12App(const DX12App& rhs)            = delete;
        DX12App& operator=(const DX12App& rhs) = delete;
        virtual ~DX12App();

        void SetNumCpuCSU(uint32_t num) { numCpuCSU = num; }
        void SetNumCpuRTV(uint32_t num) { numCpuRTV = num; }
        void SetNumCpuDSV(uint32_t num) { numCpuDSV = num; }
        void SetNumGpuCSU_Static(uint32_t num) { numGpuCSU_static = num; }
        void SetNumGpuCSU_Dynamic(uint32_t num) { numGpuCSU_dynamic = num; }

    public:
        static DX12App* GetAppInstance();

        HINSTANCE AppInst() const;
        HWND      MainWnd() const;
        float     AspectRatio() const;

        virtual int Run();

        virtual bool    Initialize();
        virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
        void            FlushCommandQueue();

        void SetCustomWindowText(LPCWSTR text);

    protected:
        virtual void CreateRtvAndDsvDescriptorHeaps();

        virtual void OnResize();
        virtual void Tick(const Timer& gt) = 0;

        // Convenience overrides for handling mouse input.
        virtual void OnMouseDown(WPARAM btnState, int x, int y) {}
        virtual void OnMouseUp(WPARAM btnState, int x, int y) {}
        virtual void OnMouseMove(WPARAM btnState, int x, int y) {}

    protected:
        bool InitMainWindow();
        bool InitDirect3D();
        void CreateCommandObjects();
        void CreateSwapChain();

        ID3D12Resource*             CurrentBackBuffer() const;
        D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView() const;
        D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView() const;

        void CalculateFrameStats();

        void LogAdapters();
        void LogAdapterOutputs(IDXGIAdapter* adapter);
        void LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format);

    protected:
        static DX12App* mApp;

        int m_frame_count = 3; // 3 frameResources

        HINSTANCE mhAppInst        = nullptr; // application instance handle
        HWND      mhMainWnd        = nullptr; // main window handle
        bool      mAppPaused       = false;   // is the application paused?
        bool      mMinimized       = false;   // is the application minimized?
        bool      mMaximized       = false;   // is the application maximized?
        bool      mResizing        = false;   // are the resize bars being dragged?
        bool      mFullscreenState = false;

        Timer                                 m_timer;
        Device                                m_device;
        Microsoft::WRL::ComPtr<IDXGIFactory4> m_dxgiFactory;

        static const int                        s_swapchain_buffer_count = 3; // triple buffering
        int                                     m_curr_back_buffer       = 0;
        Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swapchain;

        std::unique_ptr<Texture> m_swapchain_buffer[s_swapchain_buffer_count];
        std::unique_ptr<Texture> m_depthstencil_buffer[s_swapchain_buffer_count];

        UINT64                              m_current_fence = 0;
        Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;

        CmdQueue                                       m_cmdqueue;
        GCmdList                                       m_cmdlist;
        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_direct_cmdlist_alloc;

        std::unique_ptr<FrameResourceMngr> m_frameresource_mngr;

        ResourceStateTracker m_state_tracker;

        // DescriptorHeapMngr::GetInstance();  // this is the DescriptorHeap Manager For the App;
        // RenderResourceMngr::GetInstance();

        DescriptorHeapAllocation m_rtvCpuDH;
        DescriptorHeapAllocation m_dsvCpuDH;
        DescriptorHeapAllocation m_csuCpuDH;
        DescriptorHeapAllocation m_csuGpuDH;

        D3D12_VIEWPORT m_screen_viewport;
        D3D12_RECT     m_scissorRect;

        POINT m_lastmouse_pos;

        std::wstring    m_title        = L"DX12App";
        D3D_DRIVER_TYPE md3dDriverType = D3D_DRIVER_TYPE_HARDWARE;

        DXGI_FORMAT m_backbuffer_format   = DXGI_FORMAT_R8G8B8A8_UNORM;
        DXGI_FORMAT m_depthstencil_format = DXGI_FORMAT_D24_UNORM_S8_UINT;

        int m_client_width  = 1000;
        int m_client_height = 800;

        // Initial Total Descriptor_Num
        uint32_t numCpuRTV         = 168;
        uint32_t numCpuDSV         = 168;
        uint32_t numCpuCSU         = 648;
        uint32_t numGpuCSU_static  = 648;
        uint32_t numGpuCSU_dynamic = 648;
    };
} // namespace Chen::CDX12
