//
// Created by Conan Maël on 17/06/2023.
//

#include "image_operations.hh"

#include <algorithm>
#include <random>

namespace tifo
{
    void saturation(hsv24_image& image, int sat)
    {
        auto s = sat / 100.0;

        for (int i = 0; i < image.sx * image.sy * 3; i += 3)
        {
            auto sat = static_cast<uint8_t>((double)image.pixels[i + 1] * s);
            if (sat > 100)
                sat = 100;
            image.pixels[i + 1] = sat;
        }
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
        saturation(image, 70); // Diminuer la saturation à 70%

        // Égalisation et étirement d'histogramme
        // auto equalized = hsv_equalize(*hsvImage);
        // stretchHistogram(hsvImage);

        // Convertir en RGB
        hsv_to_rgb(image);

        // Appliquer le filtre Laplacien pour augmenter la netteté
        increase_contrast(image, 80);

        adjust_black_point(image, 20);

        // Appliquer les autres effets dans l'espace RGB
        add_vignette(image, 60);
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
        saturation(image, 70);
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

} // namespace tifo