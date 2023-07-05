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

#ifndef IMAGE_HH
#define IMAGE_HH

#include <cstdint>
#include <vector>

#define IMAGE_NB_LEVELS 256
#define IMAGE_MAX_LEVEL 255
#define TL_IMAGE_ALIGNMENT 64

namespace tifo
{

    typedef uint8_t* __restrict__ __attribute__((aligned(TL_IMAGE_ALIGNMENT)))
    GRAY8;
    typedef uint8_t* __restrict__ __attribute__((aligned(TL_IMAGE_ALIGNMENT)))
    RGB8;

    /**
     * Gray scale image with pixels on 8 bits.
     * @author J. Fabrizio
     */
    class gray8_image
    {
    public:
        /**
         * Image creation and allocation.
         * @param sx width of the image in pixel
         * @param sy height of the image in pixel
         */
        gray8_image(int sx, int sy);
        ~gray8_image();

        /**
         * Gives the pixel buffer aligned according to TL_IMAGE_ALIGNMENT
         * macro.
         * @return the pixel buffer.
         */
        const GRAY8& get_buffer() const;

        /**
         * Gives the pixel buffer aligned according to TL_IMAGE_ALIGNMENT
         * macro.
         * @return the pixel buffer.
         */
        GRAY8& get_buffer();

    public:
        /**Width of the image in pixels.*/
        int sx;
        /**Height of the image in pixels.*/
        int sy;
        /**Size of the reserved area in bytes.*/
        int length;
        /**Buffer*/
        GRAY8 pixels;
    };

    /**
     * Color image with pixels on 3*8 bits.
     * @author J. Fabrizio
     */
    class rgb24_image
    {
    public:
        /**
         * Image creation and allocation.
         * @param sx width of the image in pixel
         * @param sy height of the image in pixel
         */
        rgb24_image(int sx, int sy);
        ~rgb24_image();

        rgb24_image(const rgb24_image& other)
            : sx(other.sx)
            , sy(other.sy)
            , length(other.length)
        {
            length =
                (length + TL_IMAGE_ALIGNMENT - 1) & ~(TL_IMAGE_ALIGNMENT - 1);

            pixels = (RGB8)aligned_alloc(TL_IMAGE_ALIGNMENT, length);

            if (!pixels)
            {
                perror("aligned_alloc failed");
            }

            for (int i = 0; i < sx * sy * 3; i++)
                pixels[i] = other.pixels[i];
        }

        /**
         * Gives the pixel buffer aligned according to TL_IMAGE_ALIGNMENT
         * macro.
         * @return the pixel buffer.
         */
        const RGB8& get_buffer() const;

        /**
         * Gives the pixel buffer aligned according to TL_IMAGE_ALIGNMENT
         * macro.
         * @return the pixel buffer.
         */
        RGB8& get_buffer();

    public:
        /**Width of the image in pixels.*/
        int sx;
        /**Height of the image in pixels.*/
        int sy;
        /**Size of the reserved area in bytes.*/
        int length;
        /**Buffer*/
        RGB8 pixels;
    };

    typedef rgb24_image hsv24_image;

} // namespace tifo
#endif /* IMAGE_HH */
