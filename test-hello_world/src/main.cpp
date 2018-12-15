#include <karbon/drive.h>
#include <karbon/app.h>
#include <vector>
#include <iostream>
#include <assert.h>


/* ----------------------------------------------------------- Application -- */


void
startup()
{
        /* show vendor string */
        int size;
        auto ok = kd_ctx_get_vendor_string(0, &size);
        assert(ok == KD_RESULT_OK);

        std::vector<char> str(size);
        
        if ((int)str.size() >= size) {
                char *p_str = str.data();
                ok = kd_ctx_get_vendor_string(p_str, 0);
                assert(ok == KD_RESULT_OK);
        }
        
        std::cout << "Vendor String: " << str.data() << std::endl;
}


/* ----------------------------------------------- Application Description -- */


KD_APP_NAME("Hello World")
KD_APP_DESC("A very simple Karbon Drive App")
KD_APP_STARTUP_FN(startup)
