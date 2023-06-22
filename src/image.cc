//
// Created by Conan Maël on 17/06/2023.
//
//************************************************
//*                                              *
//*   TP 1&2    (c) 2017 J. FABRIZIO             *
//*                                              *
//*                               LRDE EPITA     *
//*                                              *
//************************************************

#include "image.hh"
#include <cstdlib>

namespace tifo {

    gray8_image::gray8_image(int _sx, int _sy) {
        sx = _sx;
        sy = _sy;

        length = sx*sy;
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

    rgb24_image::rgb24_image(int _sx, int _sy) {
        sx = _sx;
        sy = _sy;

        length = sx*sy*3;
        pixels = (RGB8)aligned_alloc(TL_IMAGE_ALIGNMENT, length);
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

    hsv24_image::hsv24_image(int _sx, int _sy) {
        sx = _sx;
        sy = _sy;

        length = sx*sy*3;
        pixels = (HSV8)aligned_alloc(TL_IMAGE_ALIGNMENT, length);
    }

    hsv24_image::~hsv24_image() {
        free(pixels);
    }

    const HSV8& hsv24_image::get_buffer() const {
        return pixels;
    }

    HSV8& hsv24_image::get_buffer() {
        return pixels;
    }
}