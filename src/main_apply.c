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
#include <stdbool.h>
#include <getopt.h>

#include "image.h"
#include "cgp/cgp.h"


const char* help =
    "Colearning in Coevolutionary Algorithms\n"
    "Bc. Michal Wiglasz <xwigla00@stud.fit.vutbr.cz>\n"
    "\n"
    "Master Thesis\n"
    "2014/2015\n"
    "\n"
    "Supervisor: Ing. Michaela Šikulová <isikulova@fit.vutbr.cz>\n"
    "\n"
    "Faculty of Information Technologies\n"
    "Brno University of Technology\n"
    "http://www.fit.vutbr.cz/\n"
    "     _       _\n"
    "  __(.)=   =(.)__\n"
    "  \\___)     (___/\n"
    "\n"
    "\n"
    "To apply filter:\n"
    "    ./coco_apply --chromosome filter.chr --input noisy.png --output clean.png\n"
    "\n"
    "Various input formats are supported. Output image will always be in PNG file format.\n"
    "\n"
    "Command line options:\n"
    "    --help, -h\n"
    "          Show this help and exit\n"
    "\n"
    "Required:\n"
    "    --chromosome FILE, -c FILE\n"
    "          CGP chromosome describing filter\n"
    "    --input FILE, -i FILE\n"
    "          Input image filename\n"
    "    --output FILE, -o FILE\n"
    "          Output image filename\n";


/******************************************************************************/


int main(int argc, char *argv[])
{
    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},

        {"chromosome", required_argument, 0, 'c'},
        {"input", required_argument, 0, 'i'},
        {"output", required_argument, 0, 'o'},

        {0, 0, 0, 0}
    };

    static const char *short_options = "hc:i:o:";

    img_image_t input_image = NULL;
    img_window_array_t input_image_windows = NULL;
    img_image_t output_image = NULL;
    FILE *output_image_file = NULL;
    ga_chr_t chromosome = ga_alloc_chr(cgp_alloc_genome);
    bool chromosome_loaded = false;
    if (!chromosome) {
        fprintf(stderr, "Failed to allocate memory for CGP chromosome.\n");
        return 1;
    }

    /*
        Parse command line
     */

    while (1) {
        FILE *chromosome_file;
        int option_index;
        int c = getopt_long(argc, argv, short_options, long_options, &option_index);
        if (c == - 1) break;

        switch (c) {
            case 'h':
                puts(help);
                return 1;

            case 'i':
                input_image = img_load(optarg);
                break;

            case 'o':
                printf("output: %s\n", optarg);
                output_image_file = fopen(optarg, "wb");
                break;

            case 'c':
                chromosome_file = fopen(optarg, "r");
                if (!chromosome_file) {
                    fprintf(stderr, "Failed to open chromosome file.\n");
                    return 1;
                }
                if (cgp_load_chr_compat(chromosome, chromosome_file) == 0) {
                    chromosome_loaded = true;
                }
                fclose(chromosome_file);
                break;

            default:
                fprintf(stderr, "Invalid arguments.\n");
                return 1;
        }
    }

    /*
        Check args
     */
    if (!input_image) {
        fprintf(stderr, "Failed to load input image or no file given.\n");
        return 1;
    }

    input_image_windows = img_split_windows(input_image);
    if (!input_image_windows) {
        fprintf(stderr, "Failed to preprocess input image.\n");
        return 1;
    }

    output_image = img_create(input_image->width, input_image->height, input_image->comp);
    if (!output_image) {
        fprintf(stderr, "Failed to allocate memory for output image\n");
        return 1;
    }

    if (!output_image_file) {
        fprintf(stderr, "Failed to open output image file for writing.\n");
        return 1;
    }

    if (!chromosome_loaded) {
        fprintf(stderr, "Failed to load chromosome or no file given.\n");
        return 1;
    }

    /*
        Filter image
    */

    for (int i = 0; i < input_image_windows->size; i++) {
        img_window_t *w = &input_image_windows->windows[i];

        cgp_value_t *inputs = w->pixels;
        cgp_value_t output_pixel;
        cgp_get_output(chromosome, inputs, &output_pixel);

        img_set_pixel(output_image, w->pos_x, w->pos_y, output_pixel);
    }

    /*
        Write PNG
     */

    int len;
    unsigned char *png = img_save_png_to_mem(output_image, &len);

    fwrite(png, sizeof(unsigned char), len, output_image_file);
    fclose(output_image_file);
}
