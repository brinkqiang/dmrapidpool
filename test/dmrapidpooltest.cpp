
#include "dmrapidpool.h"

int main( int argc, char* argv[] ) {

    dmrapidpool_interface* module = dmrapidpoolGetModule();
    if (module)
    {
        module->Release();
    }
    return 0;
}
