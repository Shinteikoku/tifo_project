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
    void saturation(hsv24_image& image, double s);
    rgb24_image* argentique_filter(tifo::rgb24_image& image);
    rgb24_image* ir_filter(rgb24_image& image);
    rgb24_image* negative_filter(rgb24_image& image);
}

#endif //TIFO_PROJECT_IMAGE_OPERATIONS_HH