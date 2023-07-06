#include "histogram.hh"
#include "image_convert.hh"

#ifndef TIFO_PROJECT_HISTOGRAM_OPERATIONS_HH
#define TIFO_PROJECT_HISTOGRAM_OPERATIONS_HH

namespace tifo
{
    hsv24_image* hsv_etirement(hsv24_image& image);
    hsv24_image* hsv_equalize(hsv24_image& image);
    void yCrCb_equalize(yCrCb24_image& image);
}

#endif //TIFO_PROJECT_HISTOGRAM_OPERATIONS_HH
