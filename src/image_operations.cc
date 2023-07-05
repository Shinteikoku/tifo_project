//
// Created by Conan Maël on 17/06/2023.
//

#include "image_operations.hh"

#include <algorithm>
#include <random>

namespace tifo
{
    /*void saturation(hsv24_image& image, int satur)
    {
        auto s = satur / 100.0;

        for (int i = 0; i < image.sx * image.sy * 3; i += 3)
        {
            auto sat = static_cast<uint8_t>((double)image.pixels[i + 1] * s);
            if (sat > 100)
                sat = 100;
            image.pixels[i + 1] = sat;
        }
    }*/

    void saturation(hsv24_image& image, int s)
    {
        for (int i = 0; i < image.sx * image.sy * 3; i += 3)
        {
            int sat = static_cast<int>(image.pixels[i + 1]) + s;
            sat = std::clamp(sat, 0, 100);
            image.pixels[i + 1] = static_cast<uint8_t>(sat);
        }
    }

    void rgb_saturation(rgb24_image& image, int s)
    {
        rgb_to_hsv(image);
        saturation(image, s);
        hsv_to_rgb(image);
    }

    void apply_argentique_grain(tifo::rgb24_image& image, float intensity)
    {
        std::default_random_engine generator;
        std::uniform_int_distribution<int> distribution(-intensity, intensity);

        RGB8 pixels = image.get_buffer();
        for (int i = 0; i < image.sx * image.sy * 3; i += 3)
        {
            int grain = distribution(generator);
            image.pixels[i] = std::clamp(pixels[i] + grain, 0, 255);
            image.pixels[i + 1] = std::clamp(pixels[i + 1] + grain, 0, 255);
            image.pixels[i + 2] = std::clamp(pixels[i + 2] + grain, 0, 255);
        }
    }

    void add_vignette(tifo::rgb24_image& image, float intensity)
    {
        int centerX = image.sx / 2;
        int centerY = image.sy / 2;

        RGB8 pixels = image.get_buffer();

        for (int y = 0; y < image.sy; ++y)
        {
            for (int x = 0; x < image.sx; ++x)
            {
                int dx = x - centerX;
                int dy = y - centerY;
                float distance =
                    sqrt(dx * dx + dy * dy) / std::min(centerX, centerY);

                int vignette =
                    static_cast<int>(intensity * distance * distance);
                int index = (y * image.sx + x) * 3;
                image.pixels[index] =
                    std::clamp(pixels[index] - vignette, 0, 255);
                image.pixels[index + 1] =
                    std::clamp(pixels[index + 1] - vignette, 0, 255);
                image.pixels[index + 2] =
                    std::clamp(pixels[index + 2] - vignette, 0, 255);
            }
        }
    }

    void increase_contrast(tifo::rgb24_image& image, int f)
    {
        auto factor = f / 100.0;

        RGB8 pixels = image.get_buffer();

        for (int i = 0; i < image.sx * image.sy * 3; i += 3)
        {
            for (int j = 0; j < 3; ++j)
            {
                int value = pixels[i + j] - 128;
                value = static_cast<int>(factor * value);
                image.pixels[i + j] = std::clamp(value + 128, 0, 255);
            }
        }
    }

    void adjust_black_point(rgb24_image& image, int blackPoint)
    {
        for (int i = 0; i < image.sx * image.sy * 3; i += 3)
        {
            for (int j = 0; j < 3; j++)
            {
                double pixel = image.pixels[i + j];

                if (pixel <= blackPoint)
                {
                    image.pixels[i + j] = 0;
                }
                else
                {
                    image.pixels[i + j] = image.pixels[i + j] - blackPoint;
                }
            }
        }
    }

    void swap_channels(tifo::rgb24_image& image, int channel1, int channel2)
    {
        for (int i = 0; i < image.sx * image.sy * 3; i += 3)
        {
            auto tmp = image.pixels[i + channel1];
            image.pixels[i + channel1] = image.pixels[i + channel2];
            image.pixels[i + channel2] = tmp;
        }
    }

    void increase_channel(rgb24_image& image, int x, int channel)
    {
        for (int i = 0; i < image.sx * image.sy * 3; i += 3)
            image.pixels[i + channel] =
                std::clamp(image.pixels[i + channel] + x, 0, 255);
    }

    void argentique_filter(tifo::rgb24_image& image)
    {
        // Convertir en HSV
        rgb_to_hsv(image);

        // Désaturation
        saturation(image, -10); // Diminuer la saturation à 70%

        // Égalisation et étirement d'histogramme
        // auto equalized = hsv_equalize(*hsvImage);
        // stretchHistogram(hsvImage);

        // Convertir en RGB
        hsv_to_rgb(image);

        // Appliquer le filtre Laplacien pour augmenter la netteté
        increase_contrast(image, 80);

        // adjust_black_point(image, 20);

        // Appliquer les autres effets dans l'espace RGB
        add_vignette(image, 40);
        apply_argentique_grain(image, 20);
    }

    void ir_filter(rgb24_image& image)
    {
        // Red <=> Blue
        swap_channels(image, RED, BLUE);
        // Increase contrast
        increase_contrast(image, 130);
        // Desaturate blue
        rgb_to_hsv(image);
        saturation(image, -20);
        // Back to rgb
        hsv_to_rgb(image);
        // Tint the image
        increase_channel(image, 30, RED);
    }

    void negative_filter(rgb24_image& image)
    {
        for (int i = 0; i < image.sx * image.sy * 3; i += 3)
        {
            image.pixels[i + RED] = 255 - image.pixels[i + RED];
            image.pixels[i + GREEN] = 255 - image.pixels[i + GREEN];
            image.pixels[i + BLUE] = 255 - image.pixels[i + BLUE];
        }
    }

    void grayscale(rgb24_image& image)
    {
        for (int i = 0; i < image.sx * image.sy * 3; i += 3)
        {
            auto gray = (image.pixels[i + RED] + image.pixels[i + GREEN]
                         + image.pixels[i + BLUE])
                / 3;
            image.pixels[i + RED] = gray;
            image.pixels[i + GREEN] = gray;
            image.pixels[i + BLUE] = gray;
        }
    }

    void horizontal_flip(rgb24_image& image)
    {
        int half_width = image.sx / 2; // half of the width
        for (int i = 0; i < image.sy; i++) // for each row
        {
            for (int j = 0; j < half_width;
                 j++) // for each pixel up to half width
            {
                // Calculate the corresponding positions in the row
                int left = (i * image.sx + j) * 3;
                int right = (i * image.sx + image.sx - 1 - j) * 3;

                // Swap the RGB values of the pixel
                std::swap(image.pixels[left], image.pixels[right]); // R
                std::swap(image.pixels[left + 1], image.pixels[right + 1]); // G
                std::swap(image.pixels[left + 2], image.pixels[right + 2]); // B
            }
        }
    }

    void vertical_flip(rgb24_image& image)
    {
        int half_height = image.sy / 2; // half of the height
        for (int i = 0; i < half_height; i++) // for each row up to half height
        {
            for (int j = 0; j < image.sx; j++) // for each pixel in the row
            {
                // Calculate the corresponding positions in the column
                int top = (i * image.sx + j) * 3;
                int bottom = ((image.sy - 1 - i) * image.sx + j) * 3;

                // Swap the RGB values of the pixel
                std::swap(image.pixels[top], image.pixels[bottom]); // R
                std::swap(image.pixels[top + 1], image.pixels[bottom + 1]); // G
                std::swap(image.pixels[top + 2], image.pixels[bottom + 2]); // B
            }
        }
    }

} // namespace tifo