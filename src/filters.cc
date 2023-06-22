//
// Created by Conan Maël on 17/06/2023.
//

#include "filters.hh"

namespace tifo
{
#include <vector>

    gray8_image* applyMask(gray8_image& image, std::vector<std::vector<int>> mask)
    {
        int maskSize = mask.size();
        if (maskSize % 2 == 0)
        {
            throw std::invalid_argument("Le masque doit être impair");
        }

        auto new_image = new gray8_image(image.sx, image.sy);

        for (int y = maskSize / 2; y < image.sy - maskSize / 2; ++y)
        {
            for (int x = maskSize / 2; x < image.sx - maskSize / 2; ++x)
            {
                uint8_t sum = 0;

                for (int dy = -maskSize / 2; dy <= maskSize / 2; ++dy) {
                    for (int dx = -maskSize / 2; dx <= maskSize / 2; ++dx) {
                        auto pixel = image.pixels[(y + dy) * image.sx + (x + dx)];
                        sum += pixel * mask[dy + maskSize / 2][dx + maskSize / 2];
                    }
                }

                new_image->pixels[y * new_image->sx + x] = sum;
            }
        }

        return new_image;
    }

    gray8_image* sobel_filter(tifo::gray8_image& image)
    {
        std::vector<std::vector<int>> sobelHorizontal = {
                {-1, 0, 1},
                {-2, 0, 2},
                {-1, 0, 1}
        };

        std::vector<std::vector<int>> sobelVertical = {
                {-1, -2, -1},
                { 0,  0,  0},
                { 1,  2,  1}
        };

        auto new_image = new gray8_image(image.sx, image.sy);
        auto imageHorizontal = applyMask(image, sobelHorizontal);
        auto imageVertical = applyMask(image, sobelVertical);

        for (int y = 0; y < image.sy; ++y)
        {
            for (int x = 0; x < image.sx; ++x)
            {
                float magnitude = sqrt(
                        imageHorizontal->pixels[y * image.sx + x] * imageHorizontal->pixels[y * image.sx + x] +
                        imageVertical->pixels[y * image.sx + x] * imageVertical->pixels[y * image.sx + x]
                );
                new_image->pixels[y * image.sx + x] = static_cast<uint8_t>(magnitude);
            }
        }

        return new_image;
    }

    gray8_image* laplacien_filter(gray8_image& image, float k)
    {
        std::vector<std::vector<int>> laplacian = {
                {0, -1, 0},
                {-1, 4, -1},
                {0, -1, 0}
        };

        auto new_image = new gray8_image(image.sx, image.sy);
        auto imageLaplacian = applyMask(image, laplacian);

        for (int y = 0; y < image.sy; ++y)
        {
            for (int x = 0; x < image.sx; ++x)
            {
                float newValue = image.pixels[y * image.sx + x] + k * imageLaplacian->pixels[y * image.sx + x];

                newValue = std::max(0.0f, std::min(255.0f, newValue));

                new_image->pixels[y * image.sx + x] = static_cast<uint8_t>(newValue);
            }
        }
        return new_image;
    }

    rgb24_image* rgb_laplacian(rgb24_image& image, float k)
    {
        // Convert RGB image to grayscale
        gray8_image* gray_image = rgb_to_gray_no_color(image);

        // Apply Laplacian filter to grayscale image
        gray8_image* laplacian_image = laplacien_filter(*gray_image, 1.0);

        // Apply "contrast mask" to each channel of the original RGB image
        auto new_image = new rgb24_image(image.sx, image.sy);
        for (int y = 0; y < image.sy; ++y)
        {
            for (int x = 0; x < image.sx; ++x)
            {
                for (int c = 0; c < 3; ++c)
                {
                    int idx = (y * image.sx + x) * 3 + c;
                    float newValue = image.pixels[idx] + k * laplacian_image->pixels[y * image.sx + x];

                    // Clamping
                    newValue = std::max(0.0f, std::min(255.0f, newValue));

                    new_image->pixels[idx] = static_cast<uint8_t>(newValue);
                }
            }
        }

        // Clean up
        delete gray_image;
        delete laplacian_image;

        return new_image;
    }

    rgb24_image* rgb_apply_filter(rgb24_image& image, std::function<gray8_image*(gray8_image&)> filter)
    {
        std::vector<gray8_image*> colors = rgb_to_gray_color(image);

        std::vector<gray8_image*> new_colors;
        new_colors.push_back(filter(*colors.at(0)));
        new_colors.push_back(filter(*colors.at(1)));
        new_colors.push_back(filter(*colors.at(2)));

        rgb24_image* new_image = gray_to_rgb_color(new_colors);

        return new_image;
    }

    rgb24_image* rgb_apply_filter_to_gray(rgb24_image& image, std::function<gray8_image*(gray8_image&)> filter)
    {
        auto gray = rgb_to_gray_no_color(image);  // Convert to grayscale

        auto filtered_gray = filter(*gray);  // Apply the filter

        auto new_image = gray_to_rgb_no_color(*filtered_gray);  // Convert back to RGB

        // Don't forget to deallocate memory!
        delete gray;
        delete filtered_gray;

        return new_image;
    }


}