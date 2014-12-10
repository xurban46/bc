/**
 * Tests splitting image to windows.
 * "Stand-alone" test executable - no expected output provided.
 * Source files: image.o
 */

#include <stdio.h>
#include <string.h>

#include "../image.h"


int main(int argc, char const *argv[])
{
    unsigned char data[9] = {
        0, 1, 2,
        3, 4, 5,
        6, 7, 8,
    };

    struct img_image img = {
        .data = data,
        .width = 3,
        .height = 3,
        .comp = 1
    };


    unsigned char expected_windows[9][9] = {
        {
            0, 0, 1,
            0, 0, 1,
            3, 3, 4,
        },
        {
            0, 1, 2,
            0, 1, 2,
            3, 4, 5,
        },
        {
            1, 2, 2,
            1, 2, 2,
            4, 5, 5,
        },

        {
            0, 0, 1,
            3, 3, 4,
            6, 6, 7,
        },
        {
            0, 1, 2,
            3, 4, 5,
            6, 7, 8,
        },
        {
            1, 2, 2,
            4, 5, 5,
            7, 8, 8,
        },

        {
            3, 3, 4,
            6, 6, 7,
            6, 6, 7,
        },
        {
            3, 4, 5,
            6, 7, 8,
            6, 7, 8,
        },
        {
            4, 5, 5,
            7, 8, 8,
            7, 8, 8,
        }
    };

    unsigned char expected_simd[WINDOW_SIZE][9] = {
        {0, 0, 1, 0, 0, 1, 3, 3, 4},
        {0, 1, 2, 0, 1, 2, 3, 4, 5},
        {1, 2, 2, 1, 2, 2, 4, 5, 5},
        {0, 0, 1, 3, 3, 4, 6, 6, 7},
        {0, 1, 2, 3, 4, 5, 6, 7, 8},
        {1, 2, 2, 4, 5, 5, 7, 8, 8},
        {3, 3, 4, 6, 6, 7, 6, 6, 7},
        {3, 4, 5, 6, 7, 8, 6, 7, 8},
        {4, 5, 5, 7, 8, 8, 7, 8, 8},
    };

    unsigned char *simd[WINDOW_SIZE] = {NULL};

    img_split_windows_simd(&img, simd);
    img_window_array_t w = img_split_windows(&img);
    int retval = 0;

    for (int x = 0; x < img.width; x++) {
        for (int y = 0; y < img.height; y++) {
            int index = img_pixel_index(&img, x, y);

            if (x != w->windows[index].pos_x || y != w->windows[index].pos_y) {
                fprintf(stderr, "Failure, x = %d, y = %d\n", x, y);
                fprintf(stderr, "Got x = %d, y = %d\n", w->windows[index].pos_x, w->windows[index].pos_y);
                retval = 1;
            }

            if (memcmp(w->windows[index].pixels, expected_windows[index], 9) != 0) {
                fprintf(stderr, "Failure, x = %d, y = %d\n", x, y);
                fprintf(stderr, "Got: {%3d, %3d, %3d,    Expected: {%3d, %3d, %3d,\n"
                                "      %3d, %3d, %3d,               %3d, %3d, %3d,\n"
                                "      %3d, %3d, %3d}               %3d, %3d, %3d}\n",
                        w->windows[index].pixels[0], w->windows[index].pixels[1], w->windows[index].pixels[2],
                        expected_windows[index][0], expected_windows[index][1], expected_windows[index][2],

                        w->windows[index].pixels[3], w->windows[index].pixels[4], w->windows[index].pixels[5],
                        expected_windows[index][3], expected_windows[index][4], expected_windows[index][5],

                        w->windows[index].pixels[6], w->windows[index].pixels[7], w->windows[index].pixels[8],
                        expected_windows[index][6], expected_windows[index][7], expected_windows[index][8]
                );
                retval = 1;
            }
        }
    }

    for (int i = 0; i < WINDOW_SIZE; i++) {
        if (simd[i] == NULL) {
            fprintf(stderr, "Failure: simd[%d] == NULL\n", i);
        }

        for (int x = 0; x < 9; x++) {
            fprintf(stderr, "%d, ", simd[i][x]);
        }
        fprintf(stderr, "\n");

        if (memcmp(simd[i], expected_simd[i], 9) != 0) {
            fprintf(stderr, "Failure: simd[%d] == expected_simd[%d]\n", i, i);
        }
    }

    img_windows_destroy(w);
    return retval;
}
