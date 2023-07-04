//
// Created by Conan Maël on 17/06/2023.
//

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
    // PROCESSING
    void saturation(hsv24_image& image, int sat);
    void increase_contrast(tifo::rgb24_image& image, int f);
    void adjust_black_point(rgb24_image& image, int blackPoint);
    void grayscale(rgb24_image& image);
    void swap_channels(tifo::rgb24_image& image, int channel1, int channel2);

    // FILTERS
    void argentique_filter(tifo::rgb24_image& image);
    void ir_filter(rgb24_image& image);
    void negative_filter(rgb24_image& image);
} // namespace tifo

#endif //TIFO_PROJECT_IMAGE_OPERATIONS_HH