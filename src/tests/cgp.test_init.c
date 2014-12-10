/**
 * Tests CGP initialization.
 * Compile with -DTEST_INIT
 */

#include "../cgp.h"


int main(int argc, char const *argv[])
{
    cgp_init(0, NULL);
    cgp_deinit();
}
