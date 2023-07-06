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
    void sobel_rgb(rgb24_image& image);
    void sobel_gray(rgb24_image& image);
    void sobel_hsv(rgb24_image& image);
    void sobel_yCrCb(rgb24_image& image);

    void laplacian_gray(rgb24_image& image, float k);
    void laplacien_filter_rgb(rgb24_image& image, float k);
    void laplacien_filter_yCrCb(rgb24_image& image, float k);
    void laplacien_filter_hsv(hsv24_image& image, float k);

    gray8_image* gaussian_blur(gray8_image& image, int size, float sigma);
    void rgb_gaussian(rgb24_image& image, int size, float sigma);

    void glow_filter(rgb24_image& image, float blur_radius, int threshold);
} // namespace tifo

#endif //TIFO_PROJECT_FILTERS_HH
