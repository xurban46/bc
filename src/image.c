/*
 * Colearning in Coevolutionary Algorithms
 * Bc. Michal Wiglasz <xwigla00@stud.fit.vutbr.cz>
 *
 * Master Thesis
 * 2014/2015
 *
 * Supervisor: Ing. Michaela Šikulová <isikulova@fit.vutbr.cz>
 *
 * Faculty of Information Technologies
 * Brno University of Technology
 * http://www.fit.vutbr.cz/
 *
 * Started on 28/07/2014.
 *      _       _
 *   __(.)=   =(.)__
 *   \___)     (___/
 */


#include <stdlib.h>
#include <stdio.h>

#include "cpu.h"
#include "image.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"


const int COMP = 1;


/**
 * Create new image - image data are not initialized!
 * @param  filename
 * @return
 */
img_image_t img_create(int width, int height, int comp) {
    img_image_t img = (img_image_t) malloc(sizeof(struct img_image));
    if (img == NULL) return NULL;

    img->width = width;
    img->height = height;
    img->comp = comp;
    img->data = (img_pixel_t*) malloc(sizeof(img_pixel_t) * width * height * comp);

    return img;
}


/**
 * Loads image from file
 * @param  filename
 * @return
 */
img_image_t img_load(char const *filename) {
    img_image_t img = (img_image_t) malloc(sizeof(struct img_image));
    if (img == NULL) return NULL;

    img->data = stbi_load(filename, &(img->width), &(img->height), &(img->comp), COMP);
    if (img->data == NULL) {
        printf("%s", stbi__g_failure_reason);
        free(img);
        return NULL;
    }

    img->comp = COMP;
    return img;
}


/**
 * Store image to BMP file
 * @param  img
 * @return 0 on failure, non-zero on success
 */
int img_save_bmp(img_image_t img, char const *filename) {
    return stbi_write_bmp(filename, img->width, img->height, img->comp, img->data);
}


/**
 * Store image to PNG file
 * @param  img
 * @return 0 on failure, non-zero on success
 */
int img_save_png(img_image_t img, char const *filename) {
    return stbi_write_png(filename, img->width, img->height, img->comp, img->data, 0);
}


/**
 * Store image in PNG format to memory
 * @param  img
 * @param  len Will be filled with number of bytes returned
 * @return NULL on failure, array of bytes on success
 */
unsigned char *img_save_png_to_mem(img_image_t img, int *len) {
    return stbi_write_png_to_mem(img->data, 0, img->width, img->height, img->comp, len);
}


/**
 * Clears all data associated with image from memory
 * @param img
 */
void img_destroy(img_image_t img) {
    if (img != NULL) free(img->data);
    free(img);
}


/**
 * Clears all data associated with image windows from memory
 * @param img
 */
void img_windows_destroy(img_window_array_t arr) {
    if (arr != NULL) free(arr->windows);
    free(arr);
}



/**
 * Returns index of neighbour with given offset
 * @param  baseX
 * @param  baseY
 * @param  image width
 * @param  image height
 * @param  offX
 * @param  offY
 * @return
 */
static inline int get_neighbour_index(int baseX, int baseY, int width, int height, int offX, int offY) {
    int x = baseX + offX;
    int y = baseY + offY;
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x >= width) x = width - 1;
    if (y >= height) y = height - 1;
    return (y * width) + x;
}


/**
 * Splits image into windows
 * @param  filename
 * @return
 */
img_window_array_t img_split_windows(img_image_t img) {
    int size = img->width * img->height;
    img_window_t *windows = (img_window_t*) malloc(sizeof(img_window_t) * size);
    if (windows == NULL) return NULL;

    img_window_array_t arr = (img_window_array_t) malloc(sizeof(struct img_window_array));
    if (arr == NULL) {
        free(windows);
        return NULL;
    }

    arr->size = size;
    arr->windows = windows;

    for (int x = 0; x < img->width; x++) {
        for (int y = 0; y < img->height; y++) {
            int index = img_pixel_index(img, x, y);
            windows[index].pos_x = x;
            windows[index].pos_y = y;
            windows[index].pixels[0] = img->data[get_neighbour_index(x, y, img->width, img->height, -1, -1)];
            windows[index].pixels[1] = img->data[get_neighbour_index(x, y, img->width, img->height,  0, -1)];
            windows[index].pixels[2] = img->data[get_neighbour_index(x, y, img->width, img->height, +1, -1)];
            windows[index].pixels[3] = img->data[get_neighbour_index(x, y, img->width, img->height, -1,  0)];
            windows[index].pixels[4] = img->data[get_neighbour_index(x, y, img->width, img->height,  0,  0)];
            windows[index].pixels[5] = img->data[get_neighbour_index(x, y, img->width, img->height, +1,  0)];
            windows[index].pixels[6] = img->data[get_neighbour_index(x, y, img->width, img->height, -1, +1)];
            windows[index].pixels[7] = img->data[get_neighbour_index(x, y, img->width, img->height,  0, +1)];
            windows[index].pixels[8] = img->data[get_neighbour_index(x, y, img->width, img->height, +1, +1)];
        }
    }

    return arr;
}


/**
 * Splits image into windows, suitably for SIMD processing
 * @param  filename
 * @return 0 on success
 */
int img_split_windows_simd(img_image_t img, img_pixel_t *out[WINDOW_SIZE])
{
    // align length to 256bits
    int size = img->width * img->height;
    int padding = SIMD_PADDING_BYTES - (size % SIMD_PADDING_BYTES);

    for (int i = 0; i < WINDOW_SIZE; i++) {
        out[i] = (img_pixel_t*) malloc(sizeof(img_pixel_t) * (size + padding));
        if (out[i] == NULL) {
            // TODO: dealloc
            return -1;
        }
    }

    for (int x = 0; x < img->width; x++) {
        for (int y = 0; y < img->height; y++) {
            int index = img_pixel_index(img, x, y);
            out[0][index] = img->data[get_neighbour_index(x, y, img->width, img->height, -1, -1)];
            out[1][index] = img->data[get_neighbour_index(x, y, img->width, img->height,  0, -1)];
            out[2][index] = img->data[get_neighbour_index(x, y, img->width, img->height, +1, -1)];
            out[3][index] = img->data[get_neighbour_index(x, y, img->width, img->height, -1,  0)];
            out[4][index] = img->data[get_neighbour_index(x, y, img->width, img->height,  0,  0)];
            out[5][index] = img->data[get_neighbour_index(x, y, img->width, img->height, +1,  0)];
            out[6][index] = img->data[get_neighbour_index(x, y, img->width, img->height, -1, +1)];
            out[7][index] = img->data[get_neighbour_index(x, y, img->width, img->height,  0, +1)];
            out[8][index] = img->data[get_neighbour_index(x, y, img->width, img->height, +1, +1)];
        }
    }

    // set padding bits to zero
    for (int i = 0; i < 9; i++) {
        memset(&(out[i][size]), 0, sizeof(img_pixel_t) * padding);
    }

    return 0;
}


/**
 * Calculates fitness using the PSNR (peak signal-to-noise ratio) function.
 * The higher the value, the better the filter.
 *
 * @param  original image
 * @param  filtered image
 * @return fitness value (PSNR)
 */
double img_psnr(img_image_t original, img_image_t filtered)
{
    assert(original->width == filtered->width);
    assert(original->height == filtered->height);
    assert(original->comp == filtered->comp);

    double coef = 255.0 * 255.0 * original->width * original->height;
    double sum = 0;

    for (int x = 0; x < original->width; x++) {
        for (int y = 0; y < original->height; y++) {
            double diff = img_get_pixel(filtered, x, y) - img_get_pixel(original, x, y);
            sum += diff * diff;
        }
    }

    return coef / sum;
}
