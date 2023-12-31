#include "image.hh"
#include <cstdlib>

namespace tifo {

    gray8_image::gray8_image(int _sx, int _sy)
    {
        sx = _sx;
        sy = _sy;

        length = sx * sy;

        length = (length + TL_IMAGE_ALIGNMENT - 1) & ~(TL_IMAGE_ALIGNMENT - 1);

        pixels = (GRAY8)aligned_alloc(TL_IMAGE_ALIGNMENT, length);
    }

    gray8_image::~gray8_image() {
        free(pixels);
    }

    const GRAY8& gray8_image::get_buffer() const {
        return pixels;
    }

    GRAY8& gray8_image::get_buffer() {
        return pixels;
    }

    rgb24_image::rgb24_image(int _sx, int _sy)
    {
        sx = _sx;
        sy = _sy;

        length = sx * sy * 3;

        length = (length + TL_IMAGE_ALIGNMENT - 1) & ~(TL_IMAGE_ALIGNMENT - 1);

        pixels = (RGB8)aligned_alloc(TL_IMAGE_ALIGNMENT, length);

        if (!pixels)
        {
            perror("aligned_alloc failed");
        }
    }

    rgb24_image::~rgb24_image() {
        free(pixels);
    }

    const RGB8& rgb24_image::get_buffer() const {
        return pixels;
    }

    RGB8& rgb24_image::get_buffer() {
        return pixels;
    }


}