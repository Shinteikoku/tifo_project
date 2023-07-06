//
// Created by Conan Maël on 17/06/2023.
//
#include "filters.hh"

#include <QDebug>
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

    void sobel_filter(gray8_image& image)
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

    void sobel_gray(rgb24_image& image)
    {
        auto gray = gaussian_blur(*rgb_to_gray_no_color(image), 5, 2.0);

        sobel_filter(*gray);

        for (int i = 0; i < image.sx * image.sy; i++)
        {
            image.pixels[i * 3] = gray->pixels[i];
            image.pixels[i * 3 + 1] = gray->pixels[i];
            image.pixels[i * 3 + 2] = gray->pixels[i];
        }

        delete gray;
    }

    void sobel_rgb(rgb24_image& image)
    {
        // Split the RGB image into three separate grayscale images
        rgb_gaussian(image, 5, 2.0);

        auto colors = rgb_to_gray_color(image);

        // Apply Laplacian filter to each channel
        sobel_filter(*colors[0]);
        sobel_filter(*colors[1]);
        sobel_filter(*colors[2]);

        // Combine the blurred channels back into a single RGB image
        for (int y = 0; y < image.sy; ++y)
        {
            for (int x = 0; x < image.sx; ++x)
            {
                image.pixels[(y * image.sx + x) * 3] =
                    colors[0]->pixels[y * image.sx + x];
                image.pixels[(y * image.sx + x) * 3 + 1] =
                    colors[1]->pixels[y * image.sx + x];
                image.pixels[(y * image.sx + x) * 3 + 2] =
                    colors[2]->pixels[y * image.sx + x];
            }
        }

        for (auto obj : colors)
        {
            delete obj;
        }
    }

    void laplacien_filter(gray8_image& image, float k)
    {
        std::vector<std::vector<float>> laplacian = { { 0, -1, 0 },
                                                      { -1, 4, -1 },
                                                      { 0, -1, 0 } };

        auto imageLaplacian = applyMask(image, laplacian);

        for (int i = 0; i < image.sx * image.sy; i++)
        {
            float newValue =
                (float)image.pixels[i] + k * (float)imageLaplacian->pixels[i];

            newValue = std::max(0.0f, std::min(255.0f, newValue));

            image.pixels[i] = static_cast<uint8_t>(newValue);
        }

        delete imageLaplacian;
    }

    void laplacien_filter_rgb(rgb24_image& image, float k)
    {
        // Split the RGB image into three separate grayscale images
        rgb_gaussian(image, 5, 2.0);

        rgb_to_YCrCb(image);

        auto colors = rgb_to_gray_color(image);

        // Apply Laplacian filter to each channel
        laplacien_filter(*colors[0], k);
        laplacien_filter(*colors[1], k);
        laplacien_filter(*colors[2], k);

        // Combine the blurred channels back into a single RGB image
        for (int y = 0; y < image.sy; ++y)
        {
            for (int x = 0; x < image.sx; ++x)
            {
                image.pixels[(y * image.sx + x) * 3] =
                    colors[0]->pixels[y * image.sx + x];
                image.pixels[(y * image.sx + x) * 3 + 1] =
                    colors[1]->pixels[y * image.sx + x];
                image.pixels[(y * image.sx + x) * 3 + 2] =
                    colors[2]->pixels[y * image.sx + x];
            }
        }

        for (auto obj : colors)
        {
            delete obj;
        }
    }

    void laplacien_filter_yCrCb(yCrCb24_image& image, float k)
    {
        // Split the RGB image into three separate grayscale images
        rgb_gaussian(image, 5, 2.0);

        rgb_to_YCrCb(image);

        auto colors = rgb_to_gray_color(image);

        // Apply Laplacian filter to each channel
        laplacien_filter(*colors[0], k);

        // Combine the blurred channels back into a single RGB image
        for (int y = 0; y < image.sy; ++y)
        {
            for (int x = 0; x < image.sx; ++x)
            {
                image.pixels[(y * image.sx + x) * 3] =
                    colors[0]->pixels[y * image.sx + x];
            }
        }

        yCrCb_to_rgb(image);

        for (auto obj : colors)
        {
            delete obj;
        }
    }

    void sobel_yCrCb(rgb24_image& image)
    {
        // Split the RGB image into three separate grayscale images
        rgb_gaussian(image, 5, 2.0);

        rgb_to_YCrCb(image);

        auto colors = rgb_to_gray_color(image);

        // Apply Laplacian filter to each channel
        sobel_filter(*colors[0]);

        // Combine the blurred channels back into a single RGB image
        for (int y = 0; y < image.sy; ++y)
        {
            for (int x = 0; x < image.sx; ++x)
            {
                image.pixels[(y * image.sx + x) * 3] =
                    colors[0]->pixels[y * image.sx + x];
            }
        }

        yCrCb_to_rgb(image);

        for (auto obj : colors)
        {
            delete obj;
        }
    }

    void laplacien_filter_hsv(hsv24_image& image, float k)
    {
        // Split the RGB image into three separate grayscale images
        rgb_gaussian(image, 5, 2.0);

        rgb_to_hsv(image);

        auto colors = hsv_to_gray_color(image);

        // Apply Laplacian filter to each channel
        laplacien_filter(*colors[2], k);

        // Combine the blurred channels back into a single RGB image
        for (int y = 0; y < image.sy; ++y)
        {
            for (int x = 0; x < image.sx; ++x)
            {
                image.pixels[(y * image.sx + x) * 3 + 2] =
                    std::clamp(colors[2]->pixels[y * image.sx + x], (uint8_t)0,
                               (uint8_t)100);
            }
        }

        hsv_to_rgb(image);

        for (auto obj : colors)
        {
            delete obj;
        }
    }

    void sobel_hsv(hsv24_image& image)
    {
        // Split the RGB image into three separate grayscale images
        rgb_gaussian(image, 5, 2.0);

        rgb_to_hsv(image);

        auto colors = hsv_to_gray_color(image);

        // Apply Laplacian filter to each channel
        sobel_filter(*colors[2]);

        // Combine the blurred channels back into a single RGB image
        for (int y = 0; y < image.sy; ++y)
        {
            for (int x = 0; x < image.sx; ++x)
            {
                image.pixels[(y * image.sx + x) * 3 + 2] =
                    std::clamp(colors[2]->pixels[y * image.sx + x], (uint8_t)0,
                               (uint8_t)100);
            }
        }

        hsv_to_rgb(image);

        for (auto obj : colors)
        {
            delete obj;
        }
    }

    void laplacian_gray(rgb24_image& image, float k)
    {
        auto gray = gaussian_blur(*rgb_to_gray_no_color(image), 5, 2.0);

        laplacien_filter(*gray, k);

        for (int i = 0; i < image.sx * image.sy; i++)
        {
            image.pixels[i * 3] = gray->pixels[i];
            image.pixels[i * 3 + 1] = gray->pixels[i];
            image.pixels[i * 3 + 2] = gray->pixels[i];
        }

        delete gray;
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

    void rgb_gaussian(rgb24_image& image, int size, float sigma)
    {
        // Split the RGB image into three separate grayscale images
        auto colors = rgb_to_gray_color(image);

        auto filter = gaussian_filter(size, sigma);

        // Apply Gaussian blur to each channel
        auto blurredRed = applyMask(*colors[0], filter);
        auto blurredGreen = applyMask(*colors[1], filter);
        auto blurredBlue = applyMask(*colors[2], filter);

        // Combine the blurred channels back into a single RGB image
        for (int y = 0; y < image.sy; ++y)
        {
            for (int x = 0; x < image.sx; ++x)
            {
                image.pixels[(y * image.sx + x) * 3] =
                    blurredRed->pixels[y * image.sx + x];
                image.pixels[(y * image.sx + x) * 3 + 1] =
                    blurredGreen->pixels[y * image.sx + x];
                image.pixels[(y * image.sx + x) * 3 + 2] =
                    blurredBlue->pixels[y * image.sx + x];
            }
        }

        delete blurredRed;
        delete blurredGreen;
        delete blurredBlue;

        for (auto obj : colors)
        {
            delete obj;
        }
    }

    void glow_filter(rgb24_image& image, float blur_radius, int threshold)
    {
        auto tmp = new rgb24_image(image);
        // Apply a Gaussian blur to the image.
        rgb_gaussian(*tmp, 5, blur_radius);

        int px;
        // Threshold the blurred image.
        for (int i = 0; i < image.sx * image.sy * 3; i++)
        {
            if (tmp->pixels[i] < threshold)
                px = 0;
            else
                px = tmp->pixels[i] - threshold;

            image.pixels[i] = std::min(image.pixels[i] + px, 255);
        }

        delete tmp;
    }

} // namespace tifo