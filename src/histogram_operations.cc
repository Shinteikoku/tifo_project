//
// Created by Conan Maël on 17/06/2023.
//

#include "histogram_operations.hh"

namespace tifo
{
    gray8_image* equalize(gray8_image& image, histogram_1d& hist, int b_sup)
    {
        auto new_image = new gray8_image(image.sx, image.sy);
        histogram_1d* cumul = cumulative_hist(hist, b_sup);

        int nb_pix = image.length;

        for (int i = 0; i < image.length; i++)
        {
            new_image->pixels[i] = (b_sup * cumul->histogram[image.pixels[i]]) / nb_pix;
        }

        return new_image;
    }

    rgb24_image* rgb_equalize(rgb24_image& image)
    {
        std::vector<gray8_image*> colors = rgb_to_gray_color(image);
        histogram_1d* red_hist = make_histogram(*colors.at(0), 255);
        histogram_1d* green_hist = make_histogram(*colors.at(1), 255);
        histogram_1d* blue_hist = make_histogram(*colors.at(2), 255);

        std::vector<gray8_image*> new_colors;
        new_colors.push_back(equalize(*colors.at(0), *red_hist, 255));
        new_colors.push_back(equalize(*colors.at(1), *green_hist, 255));
        new_colors.push_back(equalize(*colors.at(2), *blue_hist, 255));

        rgb24_image* new_image = gray_to_rgb_color(new_colors);

        return new_image;
    }

    hsv24_image* hsv_equalize(hsv24_image& image)
    {
        std::vector<gray8_image*> colors = hsv_to_gray_color(image);
        histogram_1d* h_hist = make_histogram(*colors.at(2), 100);

        std::vector<gray8_image*> new_colors;

        new_colors.push_back(colors.at(0));
        new_colors.push_back(colors.at(1));
        new_colors.push_back(equalize(*colors.at(2), *h_hist, 100));

        hsv24_image* new_image = gray_to_hsv_color(new_colors);

        return new_image;
    }

    int find_min(histogram_1d& hist, int limit)
    {
        unsigned int min = hist.histogram[0];
        int i_min = 0;
        for (int i = 1; i <= limit; i++)
        {
            if (hist.histogram[i] < min) {
                min = hist.histogram[i];
                i_min = i;
            }
        }
        return i_min;
    }

    int find_max(histogram_1d& hist, int limit)
    {
        unsigned int max = hist.histogram[0];
        int i_max = 0;
        for (int i = 1; i <= limit; i++)
        {
            if (hist.histogram[i] > max) {
                max = hist.histogram[i];
                i_max = i;
            }
        }
        return i_max;
    }

    gray8_image* etirement(gray8_image& image, histogram_1d& hist, int limit)
    {
        gray8_image* new_image = new gray8_image(image.sx, image.sy);

        int b_sup = find_max(hist, limit);
        int b_inf = find_min(hist, limit);
        int new_max = 100;
        int new_min = 0;

        for (int i = 0; i < image.length; i++)
        {
            int new_pixel = (image.pixels[i] - b_inf) * ((new_max - new_min) / (double)(b_sup - b_inf)) + new_min;
            float scale = (new_pixel * limit) / 255;
            new_image->pixels[i] = std::max(0, std::min(100, (int)scale));
        }

        return new_image;
    }

    hsv24_image* hsv_etirement(hsv24_image& image)
    {
        std::vector<gray8_image*> colors = hsv_to_gray_color(image);
        histogram_1d* h_hist = make_histogram(*colors.at(2), 100);

        std::vector<gray8_image*> new_colors;

        new_colors.push_back(colors.at(0));
        new_colors.push_back(colors.at(1));
        new_colors.push_back(etirement(*colors.at(2), *h_hist, 100));

        hsv24_image* new_image = gray_to_hsv_color(new_colors);

        return new_image;
    }
}