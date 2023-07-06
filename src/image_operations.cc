//
// Created by Conan Maël on 17/06/2023.
//

#include "image_operations.hh"

#include <algorithm>
#include <random>

namespace tifo
{
    void hue(hsv24_image& image, int h)
    {
        for (int i = 0; i < image.sx * image.sy * 3; i += 3)
        {
            image.pixels[i] = std::clamp(image.pixels[i] + h, 0, 360);
        }
    }

    void saturation(hsv24_image& image, int s)
    {
        for (int i = 0; i < image.sx * image.sy * 3; i += 3)
        {
            image.pixels[i + 1] = std::clamp(image.pixels[i + 1] + s, 0, 100);
        }
    }

    void value(hsv24_image& image, int v)
    {
        for (int i = 0; i < image.sx * image.sy * 3; i += 3)
        {
            image.pixels[i + 2] = std::clamp(image.pixels[i + 2] + v, 0, 360);
        }
    }

    void rgb_hue(rgb24_image& image, int h)
    {
        rgb_to_hsv(image);
        hue(image, h);
        hsv_to_rgb(image);
    }

    void rgb_saturation(rgb24_image& image, int s)
    {
        rgb_to_hsv(image);
        saturation(image, s);
        hsv_to_rgb(image);
    }

    void rgb_value(rgb24_image& image, int v)
    {
        rgb_to_hsv(image);
        value(image, v);
        hsv_to_rgb(image);
    }

    void apply_argentique_grain(tifo::rgb24_image& image, int intensity)
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

    void add_vignette(tifo::rgb24_image& image, int intensity)
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
                    static_cast<int>((float)intensity * distance * distance);
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

    void yCrCb_increase_channel(yCrCb24_image& image, int x, int channel)
    {
        rgb_to_YCrCb(image);
        increase_channel(image, x, channel);
        yCrCb_to_rgb(image);
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
        // Change to HSV
        rgb_to_hsv(image);
        hue(image, -50); // LOWER HUE BY 50
        saturation(image, -50); // LOWER SATURATION BY 50
        // Back to RGB
        hsv_to_rgb(image);

        swap_channels(image, RED, BLUE);

        increase_channel(image, 20, BLUE);

        rgb_to_YCrCb(image);

        yCrCb_equalize(image);

        yCrCb_to_rgb(image);
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

    std::array<uint8_t, 3> interpolate_pixel(const rgb24_image& image, float x,
                                             float y)
    {
        // Get the four pixels enclosing (x, y)
        int x1 = floor(x);
        int y1 = floor(y);
        int x2 = x1 + 1;
        int y2 = y1 + 1;

        // Calculate the weights for each pixel
        float w1 = (x2 - x) * (y2 - y);
        float w2 = (x - x1) * (y2 - y);
        float w3 = (x2 - x) * (y - y1);
        float w4 = (x - x1) * (y - y1);

        // Get the color of each pixel
        std::array<uint8_t, 3> c1 = {
            image.pixels[(y1 * image.sx + x1) * 3],
            image.pixels[(y1 * image.sx + x1) * 3 + 1],
            image.pixels[(y1 * image.sx + x1) * 3 + 2]
        };
        std::array<uint8_t, 3> c2 = {
            image.pixels[(y1 * image.sx + x2) * 3],
            image.pixels[(y1 * image.sx + x2) * 3 + 1],
            image.pixels[(y1 * image.sx + x2) * 3 + 2]
        };
        std::array<uint8_t, 3> c3 = {
            image.pixels[(y2 * image.sx + x1) * 3],
            image.pixels[(y2 * image.sx + x1) * 3 + 1],
            image.pixels[(y2 * image.sx + x1) * 3 + 2]
        };
        std::array<uint8_t, 3> c4 = {
            image.pixels[(y2 * image.sx + x2) * 3],
            image.pixels[(y2 * image.sx + x2) * 3 + 1],
            image.pixels[(y2 * image.sx + x2) * 3 + 2]
        };

        // Interpolate the color at (x, y)
        std::array<uint8_t, 3> c;
        for (int i = 0; i < 3; ++i)
            c[i] = std::clamp(
                std::round(c1[i] * w1 + c2[i] * w2 + c3[i] * w3 + c4[i] * w4),
                0.0f, 255.0f);

        return c;
    }

    rgb24_image* rotate_image(const rgb24_image& original, int deg)
    {
        double rad = (deg * M_PI) / 180.0;

        if (std::abs(rad - 0) < 0.01)
        {
            return new rgb24_image(original);
        }
        // Calculate dimensions of the new image
        double sin_angle = std::abs(sin(rad));
        double cos_angle = std::abs(cos(rad));
        int new_width = original.sx * cos_angle + original.sy * sin_angle;
        int new_height = original.sx * sin_angle + original.sy * cos_angle;

        auto rotated =
            new rgb24_image(new_width, new_height); // Create new image
        rotated->sx = new_width;
        rotated->sy = new_height;
        rotated->pixels =
            new uint8_t[new_width * new_height * 3]; // Allocate memory

        int original_mid_x = original.sx / 2;
        int original_mid_y = original.sy / 2;
        int rotated_mid_x = new_width / 2;
        int rotated_mid_y = new_height / 2;

        for (int y = 0; y < new_height; ++y)
        {
            for (int x = 0; x < new_width; ++x)
            {
                // Translate point to origin for the new image
                int dx = x - rotated_mid_x;
                int dy = y - rotated_mid_y;

                // Apply rotation
                float old_x = original_mid_x + (dx * cos(rad) - dy * sin(rad));
                float old_y = original_mid_y + (dx * sin(rad) + dy * cos(rad));

                // Bilinear interpolation
                if (old_x >= 0 && old_x < original.sx - 1 && old_y >= 0
                    && old_y < original.sy - 1)
                {
                    std::array<uint8_t, 3> color =
                        interpolate_pixel(original, old_x, old_y);
                    rotated->pixels[(y * new_width + x) * 3] = color[0];
                    rotated->pixels[(y * new_width + x) * 3 + 1] = color[1];
                    rotated->pixels[(y * new_width + x) * 3 + 2] = color[2];
                }
                // Fill the remaining area with a specific color (e.g., black)
                else
                {
                    rotated->pixels[(y * new_width + x) * 3] = 0;
                    rotated->pixels[(y * new_width + x) * 3 + 1] = 0;
                    rotated->pixels[(y * new_width + x) * 3 + 2] = 0;
                }
            }
        }

        return rotated;
    }

} // namespace tifo