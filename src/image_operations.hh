#include "image.hh"
#include "image_convert.hh"
#include "histogram_operations.hh"
#include "filters.hh"

#ifndef TIFO_PROJECT_IMAGE_OPERATIONS_HH
#define TIFO_PROJECT_IMAGE_OPERATIONS_HH

#define RED 0
#define GREEN 1
#define BLUE 2

namespace tifo
{
    // HSV
    void rgb_hue(rgb24_image& image, int s);
    void rgb_saturation(rgb24_image& image, int s);
    void rgb_value(rgb24_image& image, int s);

    // PROCESSING
    void increase_contrast(tifo::rgb24_image& image, int f);
    void adjust_black_point(rgb24_image& image, int blackPoint);
    void grayscale(rgb24_image& image);
    void swap_channels(tifo::rgb24_image& image, int channel1, int channel2);
    void increase_channel(rgb24_image& image, int x, int channel);
    void yCrCb_increase_channel(yCrCb24_image& image, int x, int channel);

    // FILTERS
    void argentique_filter(tifo::rgb24_image& image);
    void ir_filter(rgb24_image& image);
    void negative_filter(rgb24_image& image);
    void horizontal_flip(rgb24_image& image);
    void vertical_flip(rgb24_image& image);
    rgb24_image* rotate_image(const rgb24_image& original, int deg);

    // OTHER
    void add_vignette(tifo::rgb24_image& image, int intensity);
    void apply_argentique_grain(tifo::rgb24_image& image, int intensity);
} // namespace tifo

#endif //TIFO_PROJECT_IMAGE_OPERATIONS_HH