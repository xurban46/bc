/**
 * Tests splitting image to windows.
 * "Stand-alone" test executable - no expected output provided.
 */

#include <stdio.h>
#include <string.h>

#include "../image.h"
#include "../cgp.h"
#include "../fitness.h"


int main(int argc, char const *argv[])
{
    unsigned char data_original[9] = {
        0, 1, 2,
        3, 4, 5,
        6, 7, 8,
    };

    _img_image original = {
        .data = data_original,
        .width = 3,
        .height = 3,
        .comp = 1
    };

    unsigned char data_filtered[9] = {
        0, 10, 2,
        1, 4, 7,
        6, 7, 8,
    };

    _img_image filtered = {
        .data = data_filtered,
        .width = 3,
        .height = 3,
        .comp = 1
    };

    int retval = 0;
    double expected_psnr = (255 * 255 * 9.0) / (81 + 4 + 4);
    double obtained_psnr = fitness_psnr(&original, &filtered);

    if (expected_psnr != obtained_psnr) {
        fprintf(stderr, "Failure. Expected %lf, obtained %lf\n", expected_psnr, obtained_psnr);
        retval = 1;
    }


    return retval;
}
