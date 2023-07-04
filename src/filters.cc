//
// Created by Conan Maël on 17/06/2023.
//
#include "filters.hh"

#include <algorithm>
#include <vector>

namespace tifo
{

    gray8_image* applyMask(gray8_image& image,
                           std::vector<std::vector<float>> mask)
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

                for (int dy = -maskSize / 2; dy <= maskSize / 2; ++dy)
                {
                    for (int dx = -maskSize / 2; dx <= maskSize / 2; ++dx)
                    {
                        auto pixel =
                            image.pixels[(y + dy) * image.sx + (x + dx)];
                        sum +=
                            pixel * mask[dy + maskSize / 2][dx + maskSize / 2];
                    }
                }

                new_image->pixels[y * new_image->sx + x] = sum;
            }
        }

        return new_image;
    }

    void sobel_filter(tifo::gray8_image& image)
    {
        std::vector<std::vector<float>> sobelHorizontal = { { -1, 0, 1 },
                                                            { -2, 0, 2 },
                                                            { -1, 0, 1 } };

        std::vector<std::vector<float>> sobelVertical = { { -1, -2, -1 },
                                                          { 0, 0, 0 },
                                                          { 1, 2, 1 } };

        auto imageHorizontal = applyMask(image, sobelHorizontal);
        auto imageVertical = applyMask(image, sobelVertical);

        for (int i = 0; i < image.length; i++)
        {
            float magnitude =
                sqrt(imageHorizontal->pixels[i] * imageHorizontal->pixels[i]
                     + imageVertical->pixels[i] * imageVertical->pixels[i]);
            image.pixels[i] = static_cast<uint8_t>(magnitude);
        }

        delete imageHorizontal;
        delete imageVertical;
    }

    void laplacien_filter(gray8_image& image, float k)
    {
        std::vector<std::vector<float>> laplacian = { { 0, -1, 0 },
                                                      { -1, 4, -1 },
                                                      { 0, -1, 0 } };

        auto imageLaplacian = applyMask(image, laplacian);

        for (int i = 0; i < image.length; i++)
        {
            float newValue = image.pixels[i] + k * imageLaplacian->pixels[i];

            newValue = std::max(0.0f, std::min(255.0f, newValue));

            image.pixels[i] = static_cast<uint8_t>(newValue);
        }

        delete imageLaplacian;
    }

    void rgb_laplacian(rgb24_image& image, float k)
    {
        // Convert RGB image to grayscale
        gray8_image* gray_image = rgb_to_gray_no_color(image);

        // Apply Laplacian filter to grayscale image
        laplacien_filter(*gray_image, 1.0);

        // Apply "contrast mask" to each channel of the original RGB image
        for (int y = 0; y < image.sy; ++y)
        {
            for (int x = 0; x < image.sx; ++x)
            {
                for (int c = 0; c < 3; ++c)
                {
                    int idx = (y * image.sx + x) * 3 + c;
                    float newValue =
                        image.pixels[idx] + k * image.pixels[y * image.sx + x];

                    // Clamping
                    newValue = std::max(0.0f, std::min(255.0f, newValue));

                    image.pixels[idx] = static_cast<uint8_t>(newValue);
                }
            }
        }

        // Clean up
        delete gray_image;
    }

    void rgb_apply_filter(rgb24_image& image,
                          std::function<gray8_image*(gray8_image&)> filter)
    {
        std::vector<gray8_image*> colors = rgb_to_gray_color(image);

        std::vector<gray8_image*> new_colors;
        new_colors.push_back(filter(*colors.at(0)));
        new_colors.push_back(filter(*colors.at(1)));
        new_colors.push_back(filter(*colors.at(2)));

        rgb24_image* new_image = gray_to_rgb_color(new_colors);
    }

    void
    rgb_apply_filter_to_gray(rgb24_image& image,
                             std::function<gray8_image*(gray8_image&)> filter)
    {
        auto gray = rgb_to_gray_no_color(image); // Convert to grayscale

        auto filtered_gray = filter(*gray); // Apply the filter

        auto new_image =
            gray_to_rgb_no_color(*filtered_gray); // Convert back to RGB

        // Don't forget to deallocate memory!
        delete gray;
        delete filtered_gray;
    }

    std::vector<std::vector<float>> gaussian_filter(int size, float sigma)
    {
        std::vector<std::vector<float>> filter(size, std::vector<float>(size));
        float sum = 0;

        // Generate Gaussian filter.
        int radius = size / 2;
        for (int y = -radius; y <= radius; ++y)
        {
            for (int x = -radius; x <= radius; ++x)
            {
                filter[y + radius][x + radius] =
                    exp(-(x * x + y * y) / (2 * sigma * sigma));
                sum += filter[y + radius][x + radius];
            }
        }

        // Normalize filter.
        for (int y = 0; y < size; ++y)
        {
            for (int x = 0; x < size; ++x)
            {
                filter[y][x] /= sum;
            }
        }

        return filter;
    }

    gray8_image* gaussian_blur(gray8_image& image, int size, float sigma)
    {
        auto filter = gaussian_filter(size, sigma);
        return applyMask(image, filter);
    }

    void glow_filter(tifo::gray8_image& image, int blur_radius, int threshold)
    {
        // Apply a Gaussian blur to the image.
        auto blurred = gaussian_blur(image, 3, blur_radius);

        // Threshold the blurred image.
        for (int y = 0; y < blurred->sy; ++y)
        {
            for (int x = 0; x < blurred->sx; ++x)
            {
                int pixel_index = y * blurred->sx + x;
                if (blurred->pixels[pixel_index] < threshold)
                {
                    blurred->pixels[pixel_index] = 0;
                }
            }
        }

        // Add the glow to the original image.
        auto new_image = new gray8_image(image.sx, image.sy);
        for (int y = 0; y < image.sy; ++y)
        {
            for (int x = 0; x < image.sx; ++x)
            {
                int pixel_index = y * image.sx + x;
                int new_value =
                    image.pixels[pixel_index] + blurred->pixels[pixel_index];
                new_image->pixels[pixel_index] = std::min(new_value, 255);
            }
        }

        delete blurred;
    }

} // namespace tifo