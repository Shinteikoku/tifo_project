//
// Created by Conan Maël on 17/06/2023.
//
#include "image.hh"

#ifndef TIFO_PROJECT_IMAGE_CONVERT_HH
#define TIFO_PROJECT_IMAGE_CONVERT_HH

namespace tifo
{
    rgb24_image* gray_to_rgb_no_color(gray8_image& image);
    gray8_image* rgb_to_gray_no_color(rgb24_image& image);
    std::vector<gray8_image*> rgb_to_gray_color(rgb24_image& image);
    rgb24_image* gray_to_rgb_color(std::vector<gray8_image*> colors);
    std::vector<gray8_image*> hsv_to_gray_color(hsv24_image& image);
    hsv24_image* gray_to_hsv_color(std::vector<gray8_image*> colors);
    hsv24_image* rgb_to_hsv(rgb24_image& image);
    rgb24_image* hsv_to_rgb(hsv24_image& image);
}

#endif //TIFO_PROJECT_IMAGE_CONVERT_HH
