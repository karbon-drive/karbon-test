
#include <karbon/drive.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


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


KD_EXPORT int
kd_logic_tick() {
        return 1;
}


KD_EXPORT int
kd_project_entry()
{
        /* show vendor string */
        int size;
        kd_ctx_get_vendor_string(0, &size);
        
        char *str;
        
        kd_ctx_get_vendor_string(&str, 0);
        
        
        
        printf("%s\n", str);
        free(str);
        
//        kd_window_get(0);
//        kd_window_set(0);
//        void *addr;
//        int bytes;
//        kd_alloc(0, &addr, &bytes);
        
        return 1;
}


KD_EXPORT
int kd_project_details() {

        return 0;
}

