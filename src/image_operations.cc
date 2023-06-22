//
// Created by Conan Maël on 17/06/2023.
//

#include "image_operations.hh"
#include <random>
#include <algorithm>

namespace tifo
{
    void saturation(hsv24_image& image, double s)
    {
        for (int i = 0; i < image.length; i+=3)
        {

            auto sat = static_cast<uint8_t>(image.pixels[i + 1] * s);
            if (sat > 100)
                sat = 100;
            image.pixels[i + 1] = sat;
        }
    }

    rgb24_image* apply_argentique_grain(tifo::rgb24_image& image, float intensity)
    {
        std::default_random_engine generator;
        std::uniform_int_distribution<int> distribution(-intensity, intensity);

        auto new_image = new rgb24_image(image.sx, image.sy);

        RGB8 pixels = image.get_buffer();
        for (int i = 0; i < image.length; i += 3)
        {
            int grain = distribution(generator);
            new_image->pixels[i] = std::clamp(pixels[i] + grain, 0, 255);
            new_image->pixels[i + 1] = std::clamp(pixels[i + 1] + grain, 0, 255);
            new_image->pixels[i + 2] = std::clamp(pixels[i + 2] + grain, 0, 255);
        }

        return new_image;
    }

    rgb24_image* add_vignette(tifo::rgb24_image& image, float intensity)
    {
        int centerX = image.sx / 2;
        int centerY = image.sy / 2;

        RGB8 pixels = image.get_buffer();
        auto new_image = new rgb24_image(image.sx, image.sy);

        for (int y = 0; y < image.sy; ++y)
        {
            for (int x = 0; x < image.sx; ++x)
            {
                int dx = x - centerX;
                int dy = y - centerY;
                float distance = sqrt(dx * dx + dy * dy) / std::min(centerX, centerY);

                int vignette = static_cast<int>(intensity * distance * distance);
                int index = (y * image.sx + x) * 3;
                new_image->pixels[index] = std::clamp(pixels[index] - vignette, 0, 255);
                new_image->pixels[index + 1] = std::clamp(pixels[index + 1] - vignette, 0, 255);
                new_image->pixels[index + 2] = std::clamp(pixels[index + 2] - vignette, 0, 255);
            }
        }

        return new_image;
    }

    rgb24_image* increase_contrast(tifo::rgb24_image& image, float factor)
    {
        RGB8 pixels = image.get_buffer();

        auto new_image = new rgb24_image(image.sx, image.sy);

        for (int i = 0; i < image.length; i += 3)
        {
            for (int j = 0; j < 3; ++j)
            {
                int value = pixels[i + j] - 128;
                value = static_cast<int>(factor * value);
                new_image->pixels[i + j] = std::clamp(value + 128, 0, 255);
            }
        }

        return new_image;
    }

    gray8_image* adjust_black_point(gray8_image& image, uint8_t blackPoint)
    {
        auto new_image = new gray8_image(image.sx, image.sy);

        for (int y = 0; y < image.sy; ++y)
        {
            for (int x = 0; x < image.sx; ++x)
            {
                int pixel = image.pixels[y * image.sx + x];

                if (pixel <= blackPoint)
                {
                    new_image->pixels[y * image.sx + x] = 0; // set to black
                }
                else
                {
                    // rescale the range of intensities
                    new_image->pixels[y * image.sx + x] =
                            static_cast<uint8_t>(((pixel - blackPoint) / (255.0 - blackPoint)) * 255);
                }
            }
        }
        return new_image;
    }

    rgb24_image* adjust_black_point_rgb(rgb24_image& image, uint8_t blackPoint)
    {
        std::vector<gray8_image*> colors = rgb_to_gray_color(image);

        std::vector<gray8_image*> new_colors;
        new_colors.push_back(adjust_black_point(*colors.at(0), blackPoint));
        new_colors.push_back(adjust_black_point(*colors.at(1), blackPoint));
        new_colors.push_back(adjust_black_point(*colors.at(2), blackPoint));

        rgb24_image* new_image = gray_to_rgb_color(new_colors);

        return new_image;
    }

    rgb24_image* argentique_filter(tifo::rgb24_image& image)
    {
        // Convertir en HSV
        tifo::hsv24_image *hsvImage = rgb_to_hsv(image);

        // Désaturation
        saturation(*hsvImage, 0.7f);  // Diminuer la saturation à 70%

        // Égalisation et étirement d'histogramme
        //auto equalized = hsv_equalize(*hsvImage);
        //stretchHistogram(hsvImage);

        // Convertir en RGB
        auto rgb_image = hsv_to_rgb(*hsvImage);

        // Appliquer le filtre Laplacien pour augmenter la netteté
        auto contrast = increase_contrast(*rgb_image, 0.8f);

        auto black_point = adjust_black_point_rgb(*contrast, 20);

        // Appliquer les autres effets dans l'espace RGB
        auto vignette = add_vignette(*black_point, 60);
        auto grain = apply_argentique_grain(*vignette, 20);

        return grain;
    }


    tifo::rgb24_image* swap_channels(tifo::rgb24_image& image, int channel1, int channel2)
    {
        auto new_image = new rgb24_image(image.sx, image.sy);
        for (int i = 0; i < image.length; i += 3)
        {
            auto tmp = image.pixels[i + channel1];
            new_image->pixels[i + channel1] = image.pixels[i + channel2];
            new_image->pixels[i + channel2] = tmp;
        }
        return new_image;
    }

    void increase_channel(rgb24_image& image, int x, int channel)
    {
        for (int i = 0; i < image.length; i += 3)
            image.pixels[i + channel] = std::clamp(image.pixels[i + channel] + x, 0, 255);
    }

    rgb24_image* ir_filter(rgb24_image& image)
    {
        // Red <=> Blue
        auto swap = swap_channels(image, RED, BLUE);
        // Increase contrast
        auto contrast = increase_contrast(*swap, 1.3f);
        // Desaturate blue
        auto hsv = rgb_to_hsv(*contrast);
        saturation(*hsv, 0.7f);
        // Back to rgb
        auto res = hsv_to_rgb(*hsv);
        // Tint the image
        increase_channel(*res, 30, RED);
        return res;
    }

    rgb24_image* negative_filter(rgb24_image& image)
    {
        auto new_image = new rgb24_image(image.sx, image.sy);

        for (int i = 0; i < image.length; i += 3)
        {
            new_image->pixels[i + RED] = 255 - image.pixels[i + RED];
            new_image->pixels[i + GREEN] = 255 - image.pixels[i + GREEN];
            new_image->pixels[i + BLUE] = 255 - image.pixels[i + BLUE];
        }

        return new_image;
    }

}