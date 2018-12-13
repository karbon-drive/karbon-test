#include <karbon/drive.h>
#include <assert.h>
#include <vector>
#include <iostream>


KD_API KD_EXPORT void
kd_setup() {
        /* setup your project */
}


KD_API KD_EXPORT void
kd_shutdown() {
        /* shutdown your project */
}


KD_API KD_EXPORT void
kd_think() {
        /* project tick */
}


KD_API KD_EXPORT int
kd_project_entry()
{
        /* show vendor string */
        int size;
        auto ok = kd_ctx_get_vendor_string(0, &size);
        assert(ok == KD_RESULT_OK);

        std::vector<char> str(size);
        
        if (str.size() >= size) {
                char *p_str = str.data();
                ok = kd_ctx_get_vendor_string(&p_str, 0);
                assert(ok == KD_RESULT_OK);
        }
        
        std::cout << "Vendor String: " << str.data() << std::endl;

        return 1; /* 1 for good, 0 for fail */
}
