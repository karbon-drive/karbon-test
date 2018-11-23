
#include <karbon/drive.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <vector>

#if KD_WINDOWS
#include "renderer_dx12.hpp"
#elif KD_LINUX
#include "renderer_vk1.hpp"
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
        int i;
};

spinning_cube_ctx spin_ctx{};




KD_API KD_EXPORT void
kd_setup() {
        #if KD_WINDOWS
        renderer_dx12_create();
        #elif KD_LINUX
        renderer_vk1_create();
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
        #if KD_WINDOWS
        renderer_dx12_render();
        #elif KD_LINUX
        renderer_vk1_render();
        #endif
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
