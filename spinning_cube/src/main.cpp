#include <karbon/drive.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <vector>


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
        uint32_t chunk_id;
};

spinning_cube_ctx spin_ctx{};


KD_API KD_EXPORT void
kd_setup() {
        /* buffers */
        float vertices[] = {
                /* pos f3 - color f3 */
                -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
                -0.5f, -0.5f, +0.5f, 0.0f, 0.0f, 1.0f,
                -0.5f, +0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
                -0.5f, +0.5f, +0.5f, 0.0f, 1.0f, 1.0f,
                +0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 1.0f,
                +0.5f, +0.5f, -0.5f, 1.0f, 1.0f, 0.0f,
                +0.5f, +0.5f, +0.5f, 1.0f, 1.0f, 1.0f,
        };

        uint16_t index[] = {
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

        uint8_t *buffers[]{ (uint8_t*)vertices, (uint8_t*)index };

        /* mesh */
        kd_mesh_data mesh_data[1] {};
        mesh_data[0].geometry.buffer = 1;
        mesh_data[0].geometry.buffer_offset = 0;
        mesh_data[0].geometry.buffer_size = sizeof(vertices);

        mesh_data[0].index.buffer = 2;
        mesh_data[0].index.buffer_offset = 0;
        mesh_data[0].index.buffer_size = sizeof(index);

        /* chunk desc */
        kd_chunk_desc ch_desc{};
        ch_desc.type_id = KD_STRUCT_CHUNK_DESC;
        ch_desc.mesh_data = mesh_data;
        ch_desc.mesh_count = sizeof(mesh_data) / sizeof(mesh_data[0]);
        ch_desc.buffers = &buffers[0];
        ch_desc.buffer_count = sizeof(buffers) / sizeof(buffers[0]);

        auto res = kd_chunk_add(&ch_desc, &spin_ctx.chunk_id);
        (void)res;
        //assert(res == KD_RESULT_OK);
        
        auto ok = kd_log(KD_LOG_INFO, "setup");
        assert(ok == KD_RESULT_OK);
}

KD_API KD_EXPORT void
kd_shutdown() {
        int i;
        (void)i;
}


KD_API KD_EXPORT void
kd_early_think() {
        auto ok = kd_log(KD_LOG_INFO, "early think");
        assert(ok == KD_RESULT_OK);
}


KD_API KD_EXPORT void
kd_think() {
        auto ok = kd_log(KD_LOG_INFO, "think");
        assert(ok == KD_RESULT_OK);
}


KD_API KD_EXPORT void
kd_late_think() {
        auto ok = kd_log(KD_LOG_INFO, "late think");
        assert(ok == KD_RESULT_OK);
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
