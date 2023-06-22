//
// Created by Conan Maël on 17/06/2023.
//

#include <functional>

#ifndef TIFO_PROJECT_FILTERS_HH
#define TIFO_PROJECT_FILTERS_HH

#include "image.hh"
#include "image_convert.hh"

namespace tifo
{
    gray8_image* sobel_filter(tifo::gray8_image& image);
    gray8_image* laplacien_filter(tifo::gray8_image& image, float k);
    rgb24_image* rgb_laplacian(rgb24_image& image, float k);
    rgb24_image* rgb_apply_filter(rgb24_image& image, std::function<gray8_image*(gray8_image&)> filter);
    rgb24_image* rgb_apply_filter_to_gray(rgb24_image& image, std::function<gray8_image*(gray8_image&)> filter);
}

#endif //TIFO_PROJECT_FILTERS_HH
