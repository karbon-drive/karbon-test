
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

        swap_desc.BufferDesc.RefreshRate.Numerator = 0; //list[0].RefreshRate.Numerator;
        swap_desc.BufferDesc.RefreshRate.Denominator = 1; //list[0].RefreshRate.Denominator;

        swap_desc.SampleDesc.Count = 1;
        swap_desc.SampleDesc.Quality = 1;

        swap_desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
        swap_desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

        swap_desc.Flags = 0;

        IDXGISwapChain *swap_chain = nullptr;
        res = factory->CreateSwapChain(cmd_queue, &swap_desc, &swap_chain);
        assert(!FAILED(res));

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
        int i;
        (void)i;
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
