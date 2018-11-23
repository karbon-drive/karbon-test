
#include <karbon/drive.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <vector>

#if KD_WINDOWS
#include <d3d12.h>
#include <dxgi1_4.h>
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#endif


void
alloc(void **out_addr, int *bytes) {
        struct kd_allocator_frame_desc allocator;
        allocator.type_id = KD_STRUCT_ALLOCATOR_FRAME_DESC;
        allocator.ext = 0;
        
        struct kd_alloc_desc desc;
        desc.type_id = KD_STRUCT_ALLOC_DESC;
        desc.ext = 0;
        desc.allocator_desc = &allocator;
        
        kd_result res = kd_alloc(&desc, out_addr, bytes);
        assert(res == KD_RESULT_OK);
}


struct spinning_cube_ctx {
        #if KD_WINDOWS
        ID3D12Device *d3d_device;
        ID3D12CommandQueue *cmd_queue;
        ID3D12CommandAllocator* cmd_alloc;
        ID3D12GraphicsCommandList* cmd_list;

        ID3D12Resource *backbuffer[2];
        UINT backbuffer_index;
        #endif
};

spinning_cube_ctx spin_ctx{};




KD_API KD_EXPORT void
kd_setup() {
        #if KD_WINDOWS
        ID3D12Device *dev = nullptr;

        D3D_FEATURE_LEVEL fl = D3D_FEATURE_LEVEL_12_1;
        void **pp_dev = (void**)&dev;
        auto ref_id = __uuidof(ID3D12Device);
        auto res = D3D12CreateDevice(nullptr, fl, ref_id, pp_dev);
        assert(!FAILED(res));

        D3D12_COMMAND_QUEUE_DESC cmd_queue_desc{};
        cmd_queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        cmd_queue_desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
        cmd_queue_desc.NodeMask = 0;
        cmd_queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

        ID3D12CommandQueue *cmd_queue = nullptr;
        void **pp_cmd = (void**)&cmd_queue;
        ref_id = __uuidof(ID3D12CommandQueue);
        res = dev->CreateCommandQueue(&cmd_queue_desc, ref_id, pp_cmd);
        assert(!FAILED(res));
                
        IDXGIFactory4 *factory = nullptr;
        void **pp_fac = (void**)&factory;
        ref_id = __uuidof(IDXGIFactory4);
        res = CreateDXGIFactory1(ref_id, pp_fac);
        assert(!FAILED(res));

        IDXGIAdapter *adapter = nullptr;
        res = factory->EnumAdapters(0, &adapter);
        assert(!FAILED(res));

        IDXGIOutput *adapter_output = nullptr;
        res = adapter->EnumOutputs(0, &adapter_output);
        assert(!FAILED(res));

        DXGI_FORMAT e_fmt = DXGI_FORMAT_R8G8B8A8_UNORM;
        UINT flags = DXGI_ENUM_MODES_INTERLACED;
        UINT num_modes;
        res = adapter_output->GetDisplayModeList(e_fmt, flags, &num_modes, nullptr);
        assert(!FAILED(res));

        auto list = std::vector<DXGI_MODE_DESC>(num_modes);
        assert(!FAILED(!list.empty()));
        res = adapter_output->GetDisplayModeList(e_fmt, flags, &num_modes, list.data());
        assert(!FAILED(res));

        /* pick first atm */
        DXGI_ADAPTER_DESC adapter_desc{};
        res = adapter->GetDesc(&adapter_desc);
        assert(!FAILED(res));

        int card_mem = (int)(adapter_desc.DedicatedVideoMemory / 1024 / 1024);

        DXGI_MODE_DESC mode_desc{};
        for (auto l : list) {
                if (l.Height == 480) {
                        if (l.Width == 720) {
                                mode_desc = l;
                                break;
                        }
                }
        }

        /* swap chain */
        DXGI_SWAP_CHAIN_DESC swap_desc{};
        swap_desc.BufferCount = 2;
        swap_desc.BufferDesc.Width = 720;
        swap_desc.BufferDesc.Height = 480;
        swap_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swap_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swap_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

        HWND hwnd = GetActiveWindow();

        swap_desc.OutputWindow = hwnd;
        swap_desc.Windowed = true;

        swap_desc.BufferDesc.RefreshRate.Numerator = mode_desc.RefreshRate.Numerator;
        swap_desc.BufferDesc.RefreshRate.Denominator = mode_desc.RefreshRate.Denominator;
        swap_desc.SampleDesc.Count = 1;
        swap_desc.SampleDesc.Quality = 0;
        swap_desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
        swap_desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
        swap_desc.Flags = 0;

        IDXGISwapChain *swap_chain_ = nullptr;
        res = factory->CreateSwapChain(cmd_queue, &swap_desc, &swap_chain_);
        assert(!FAILED(res));

        IDXGISwapChain3 *swap_chain = nullptr;
        void ** pp_swap = (void**)&swap_chain;
        ref_id = __uuidof(IDXGISwapChain3);

        res = swap_chain_->QueryInterface(ref_id, pp_swap);

        factory->Release();
        factory = 0;

        D3D12_DESCRIPTOR_HEAP_DESC rt_heap_desc{};
        rt_heap_desc.NumDescriptors = 2;
        rt_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        rt_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

        ID3D12DescriptorHeap *desc_heap = nullptr;

        ref_id = __uuidof(ID3D12DescriptorHeap);
        void **pp_rtvh = (void**)&desc_heap;

        res = dev->CreateDescriptorHeap(&rt_heap_desc, ref_id, pp_rtvh);
        assert(!FAILED(res));

        auto heap_start = desc_heap->GetCPUDescriptorHandleForHeapStart();
        auto rtv_desc_size = dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        ref_id = __uuidof(ID3D12Resource);
        ID3D12Resource *back_buffer_rt[2]{};
        void **pp_backbuffer_rt = (void**)&back_buffer_rt[0];
        res = swap_chain->GetBuffer(0, ref_id, pp_backbuffer_rt);
        assert(!FAILED(res));

        dev->CreateRenderTargetView(back_buffer_rt[0], NULL, heap_start);

        heap_start.ptr += rtv_desc_size;

        pp_backbuffer_rt = (void**)&back_buffer_rt[1];
        res = swap_chain->GetBuffer(1, ref_id, pp_backbuffer_rt);
        assert(!FAILED(res));

        auto index = swap_chain->GetCurrentBackBufferIndex();

        spin_ctx.backbuffer[0] = back_buffer_rt[0];
        spin_ctx.backbuffer[1] = back_buffer_rt[1];
        spin_ctx.backbuffer_index = index;

        ID3D12CommandAllocator* cmd_alloc = nullptr;
        auto alloc_type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        ref_id = __uuidof(ID3D12CommandAllocator);
        void **pp_cmd_alloc = (void**)&cmd_alloc;
        res = dev->CreateCommandAllocator(alloc_type, ref_id, pp_cmd_alloc);
        assert(!FAILED(res));

        spin_ctx.cmd_alloc = cmd_alloc;

        auto cmd_type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        ref_id = __uuidof(ID3D12GraphicsCommandList);
        ID3D12GraphicsCommandList* cmd_list = nullptr;
        void **pp_cmd_list = (void**)&cmd_list;
        res = dev->CreateCommandList(0, cmd_type, cmd_alloc, NULL, ref_id, pp_cmd_list);
        assert(!FAILED(res));
        cmd_list->Close();

        spin_ctx.cmd_list = cmd_list;

        ID3D12Fence* fence = nullptr;;
        auto fflag = D3D12_FENCE_FLAG_NONE;
        ref_id = __uuidof(ID3D12Fence);
        void **pp_fence = (void**)&fence;
        res = dev->CreateFence(0, fflag, ref_id, pp_fence);
        assert(!FAILED(res));

        HANDLE fence_evt = 0;

        fence_evt = CreateEventEx(NULL, FALSE, FALSE, EVENT_ALL_ACCESS);
        assert(fence_evt);

        unsigned long long fence_value;


        /* http://www.rastertek.com/dx12tut03.html */

        /*
        IDXGIAdapter* adapter;
        IDXGIOutput* adapterOutput;
        unsigned int numModes, i, numerator, denominator, renderTargetViewDescriptorSize;
        unsigned long long stringLength;
        DXGI_MODE_DESC* displayModeList;
        DXGI_ADAPTER_DESC adapterDesc;
        int error;
        DXGI_SWAP_CHAIN_DESC swapChainDesc;
        IDXGISwapChain* swapChain;
        D3D12_DESCRIPTOR_HEAP_DESC renderTargetViewHeapDesc;
        D3D12_CPU_DESCRIPTOR_HANDLE renderTargetViewHandle;

        bool m_vsync_enabled;
        ID3D12Device* m_device;
        ID3D12CommandQueue* m_commandQueue;
        char m_videoCardDescription[128];
        IDXGISwapChain3* m_swapChain;
        ID3D12DescriptorHeap* m_renderTargetViewHeap;
        ID3D12Resource* m_backBufferRenderTarget[2];
        unsigned int m_bufferIndex;
        ID3D12CommandAllocator* m_commandAllocator;
        ID3D12GraphicsCommandList* m_commandList;
        ID3D12PipelineState* m_pipelineState;
        ID3D12Fence* m_fence;
        HANDLE m_fenceEvent;
        unsigned long long m_fenceValue;
        */

        /* save d3d items */
        spin_ctx.d3d_device = dev;
        spin_ctx.cmd_queue = cmd_queue;
        #endif
}

KD_API KD_EXPORT void
kd_shutdown() {
        int i;
        (void)i;
}


KD_API KD_EXPORT void
kd_early_think() {
        int i;
        (void)i;
}


KD_API KD_EXPORT void
kd_think() {
        int i;
        (void)i;
}


KD_API KD_EXPORT void
kd_late_think() {
        HRESULT result;
        D3D12_RESOURCE_BARRIER barrier;
        D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle;
        unsigned int rtv_desc_size;
        float color[4];
        ID3D12CommandList* pp_cmd_list[1];
        unsigned long long fence_wait_for;
        ID3D12PipelineState* pipeline = 0;

        result = spin_ctx.cmd_alloc->Reset();
        assert(!FAILED(result));

        result = spin_ctx.cmd_list->Reset(spin_ctx.cmd_alloc, pipeline);
        assert(!FAILED(result));

        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        UINT index = spin_ctx.backbuffer_index;
        barrier.Transition.pResource = spin_ctx.backbuffer[index];

        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        spin_ctx.cmd_list->ResourceBarrier(1, &barrier);

        /* http://www.rastertek.com/dx12tut03.html */
}


KD_API KD_EXPORT int
kd_project_entry()
{
        /* show vendor string */
        int size;
        kd_ctx_get_vendor_string(0, &size);
        
        char *str = 0;
        int bytes = 0;
        alloc((void**)&str, &bytes);
        if(bytes > size) {
                kd_ctx_get_vendor_string(&str, 0);
        }
        
        printf("Vendor String %s\n", str);
        
        return 1;
}


KD_API KD_EXPORT
int kd_project_details() {

        return 0;
}


KD_API KD_EXPORT
int
kd_render() {
        return 0;
}
