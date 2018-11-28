#if defined(_WIN32)

#include <assert.h>
#include <vector>
#include <d3d12.h>
#include <dxgi1_4.h>
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

struct rdr_ctx {
        ID3D12Device *d3d_device;
        ID3D12CommandQueue *cmd_queue;
        ID3D12CommandAllocator* cmd_alloc;
        ID3D12GraphicsCommandList* cmd_list;
        ID3D12DescriptorHeap *desc_heap;
        IDXGISwapChain3 *swap_chain;

        ID3D12Resource *backbuffer[2];
        UINT backbuffer_index;

        unsigned long long fence_value;
        ID3D12Fence* fence;
        HANDLE fence_evt;

} rdr_ctx;


int
renderer_dx12_create() {
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

        rdr_ctx.swap_chain = swap_chain;

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

        rdr_ctx.desc_heap = desc_heap;

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

        rdr_ctx.backbuffer[0] = back_buffer_rt[0];
        rdr_ctx.backbuffer[1] = back_buffer_rt[1];
        rdr_ctx.backbuffer_index = index;

        ID3D12CommandAllocator* cmd_alloc = nullptr;
        auto alloc_type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        ref_id = __uuidof(ID3D12CommandAllocator);
        void **pp_cmd_alloc = (void**)&cmd_alloc;
        res = dev->CreateCommandAllocator(alloc_type, ref_id, pp_cmd_alloc);
        assert(!FAILED(res));

        rdr_ctx.cmd_alloc = cmd_alloc;

        auto cmd_type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        ref_id = __uuidof(ID3D12GraphicsCommandList);
        ID3D12GraphicsCommandList* cmd_list = nullptr;
        void **pp_cmd_list = (void**)&cmd_list;
        res = dev->CreateCommandList(0, cmd_type, cmd_alloc, NULL, ref_id, pp_cmd_list);
        assert(!FAILED(res));
        cmd_list->Close();

        rdr_ctx.cmd_list = cmd_list;

        ID3D12Fence* fence = nullptr;;
        auto fflag = D3D12_FENCE_FLAG_NONE;
        ref_id = __uuidof(ID3D12Fence);
        void **pp_fence = (void**)&fence;
        res = dev->CreateFence(0, fflag, ref_id, pp_fence);
        assert(!FAILED(res));

        HANDLE fence_evt = 0;

        fence_evt = CreateEventEx(NULL, FALSE, FALSE, EVENT_ALL_ACCESS);
        assert(fence_evt);

        rdr_ctx.fence_evt = fence_evt;
        rdr_ctx.fence_value = 1;
        rdr_ctx.fence = fence;

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
        rdr_ctx.d3d_device = dev;
        rdr_ctx.cmd_queue = cmd_queue;

        /* mesh */
        float cube_verts[] = {
                /* pos f3 - color f3 */
                -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
                -0.5f, -0.5f, +0.5f, 0.0f, 0.0f, 1.0f,
                -0.5f, +0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
                -0.5f, +0.5f, +0.5f, 0.0f, 1.0f, 1.0f,
                +0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 1.0f,
                +0.5f, +0.5f, -0.5f, 1.0f, 1.0f, 0.0f,
                +0.5f, +0.5f, +0.5f, 1.0f, 1.0f, 1.0f,
        };

        unsigned short cube_index[] = {
                0, 2, 1,
                1, 2, 3,

                4, 5, 6,
                5, 7, 6,

                0, 1, 5,
                0, 5, 4,

                2, 6, 7,
                2, 7, 3,

                0, 4, 5,
                0, 6, 2,

                1, 3, 7,
                1, 7, 5
        };

        D3D12_HEAP_PROPERTIES heap_props;
        heap_props.Type = D3D12_HEAP_TYPE_DEFAULT;


        return 1;
}


int
renderer_dx12_render() {
        HRESULT result;
        D3D12_RESOURCE_BARRIER barrier;
        D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle;
        unsigned int rtv_desc_size;
        float color[4];
        ID3D12CommandList* pp_cmd_list[1];
        unsigned long long fence_wait_for;
        ID3D12PipelineState* pipeline = 0;

        result = rdr_ctx.cmd_alloc->Reset();
        assert(!FAILED(result));

        result = rdr_ctx.cmd_list->Reset(rdr_ctx.cmd_alloc, pipeline);
        assert(!FAILED(result));

        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        UINT index = rdr_ctx.backbuffer_index;
        barrier.Transition.pResource = rdr_ctx.backbuffer[index];

        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        rdr_ctx.cmd_list->ResourceBarrier(1, &barrier);

        D3D12_CPU_DESCRIPTOR_HANDLE heap_handle = rdr_ctx.desc_heap->GetCPUDescriptorHandleForHeapStart();
        rtv_desc_size = rdr_ctx.d3d_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        if (rdr_ctx.backbuffer_index == 1) {
                heap_handle.ptr += rtv_desc_size;
        }

        rdr_ctx.cmd_list->OMSetRenderTargets(1, &heap_handle, FALSE, NULL);

        color[0] = (float)(rand() % 255) / 255.0f;
        color[1] = (float)(rand() % 255) / 255.0f;
        color[2] = (float)(rand() % 255) / 255.0f;
        color[3] = 1.0f;
        rdr_ctx.cmd_list->ClearRenderTargetView(heap_handle, color, 0, NULL);

        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
        
        rdr_ctx.cmd_list->ResourceBarrier(1, &barrier);

        result = rdr_ctx.cmd_list->Close();
        assert(!FAILED(result));

        pp_cmd_list[0] = rdr_ctx.cmd_list;

        rdr_ctx.cmd_queue->ExecuteCommandLists(1, pp_cmd_list);

        fence_wait_for = rdr_ctx.fence_value;
        result = rdr_ctx.cmd_queue->Signal(rdr_ctx.fence, fence_wait_for);
        assert(!FAILED(result));
        rdr_ctx.fence_value += 1;

        auto comp_value = rdr_ctx.fence->GetCompletedValue();

        //if (comp_value < fence_wait_for) {
                result = rdr_ctx.fence->SetEventOnCompletion(fence_wait_for, rdr_ctx.fence_evt);
                assert(!FAILED(result));
        //}
        WaitForSingleObject(rdr_ctx.fence_evt, INFINITE);

        rdr_ctx.backbuffer_index == 0 ? rdr_ctx.backbuffer_index = 1 : rdr_ctx.backbuffer_index = 0;

        rdr_ctx.swap_chain->Present(0, 0);

        /* http://www.rastertek.com/dx12tut03.html */
        return 1;
}


int
renderer_dx12_destroy() {
        return 0;
}


#endif


