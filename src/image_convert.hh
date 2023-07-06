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

    void rgb_to_hsv(rgb24_image& image);
    void hsv_to_rgb(hsv24_image& image);

    void rgb_to_YCrCb(rgb24_image& image);
    void yCrCb_to_rgb(rgb24_image& image);
} // namespace tifo

#endif //TIFO_PROJECT_IMAGE_CONVERT_HH
