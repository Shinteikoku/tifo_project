//
// Created by Conan Maël on 17/06/2023.
//

#include "image_convert.hh"

#include <iostream>

namespace tifo
{
    rgb24_image* gray_to_rgb_no_color(gray8_image& image)
    {
        auto rgb_image = new rgb24_image(image.sx, image.sy);
        int j = 0;
        for (int i = 0; i < image.sx * image.sy * 3; i += 3)
        {
            rgb_image->pixels[i] = image.pixels[j];
            rgb_image->pixels[i + 1] = image.pixels[j];
            rgb_image->pixels[i + 2] = image.pixels[j];
            j++;
        }
        return rgb_image;
    }

    rgb24_image* gray_to_rgb_color(std::vector<gray8_image*> colors)
    {
        auto image = new rgb24_image(colors.at(0)->sx, colors.at(0)->sy);
        int j = 0;

        for (int i = 0; i < image->sx * image->sy * 3; i += 3)
        {
            image->pixels[i] = colors.at(0)->pixels[j];
            image->pixels[i + 1] = colors.at(1)->pixels[j];
            image->pixels[i + 2] = colors.at(2)->pixels[j];
            j++;
        }

        return image;
    }

    gray8_image* rgb_to_gray_no_color(rgb24_image& image)
    {
        auto gray_image = new gray8_image(image.sx, image.sy);
        int j = 0;
        for (int i = 0; i < image.sx * image.sy * 3; i += 3)
        {
            gray_image->pixels[j] =
                (image.pixels[i] + image.pixels[i + 1] + image.pixels[i + 2])
                / 3;
            j++;
        }
        return gray_image;
    }

    std::vector<gray8_image*> rgb_to_gray_color(rgb24_image& image)
    {
        std::vector<gray8_image*> colors;
        auto red = new gray8_image(image.sx, image.sy);
        auto green = new gray8_image(image.sx, image.sy);
        auto blue = new gray8_image(image.sx, image.sy);
        int j = 0;

        for (int i = 0; i < image.sx * image.sy * 3; i += 3)
        {
            red->pixels[j] = image.pixels[i];
            green->pixels[j] = image.pixels[i + 1];
            blue->pixels[j] = image.pixels[i + 2];
            j++;
        }

        colors.push_back(red);
        colors.push_back(green);
        colors.push_back(blue);

        return colors;
    }

    std::vector<gray8_image*> hsv_to_gray_color(hsv24_image& image)
    {
        std::vector<gray8_image*> canaux;
        auto hue = new gray8_image(image.sx, image.sy);
        auto sat = new gray8_image(image.sx, image.sy);
        auto val = new gray8_image(image.sx, image.sy);
        int j = 0;

        for (int i = 0; i < image.sx * image.sy * 3; i += 3)
        {
            hue->pixels[j] = image.pixels[i];
            sat->pixels[j] = image.pixels[i + 1];
            val->pixels[j] = image.pixels[i + 2];
            j++;
        }

        canaux.push_back(hue);
        canaux.push_back(sat);
        canaux.push_back(val);

        return canaux;
    }

    hsv24_image* gray_to_hsv_color(std::vector<gray8_image*> colors)
    {
        auto image = new hsv24_image(colors.at(0)->sx, colors.at(0)->sy);
        int j = 0;

        for (int i = 0; i < image->sx * image->sy * 3; i += 3)
        {
            image->pixels[i] = colors.at(0)->pixels[j];
            image->pixels[i + 1] = colors.at(1)->pixels[j];
            image->pixels[i + 2] = colors.at(2)->pixels[j];
            j++;
        }

        return image;
    }

    void rgb_color_hsv(std::vector<int>& rgb)
    {
        double r = static_cast<double>(rgb.at(0)) / 255;
        double g = static_cast<double>(rgb.at(1)) / 255;
        double b = static_cast<double>(rgb.at(2)) / 255;

        double cmin = std::min(std::min(r, g), b);
        double cmax = std::max(std::max(r, g), b);

        double delta = cmax - cmin;

        double h = 0;

        if (delta > 0)
        {
            if (cmax == r)
                h = 60 * fmod(((g - b) / delta), 6);
            else if (cmax == g)
                h = 60 * (((b - r) / delta) + 2);
            else
                h = 60 * (((r - g) / delta) + 4);
        }

        if (h < 0)
            h += 360;

        double s = (cmax == 0) ? 0 : (delta / cmax) * 100;

        double v = cmax * 100;

        rgb[0] = static_cast<int>(round(h));
        rgb[1] = static_cast<int>(round(s));
        rgb[2] = static_cast<int>(round(v));
    }

    void hsv_color_rgb(std::vector<int>& hsv)
    {
        int h = hsv.at(0);
        int s = hsv.at(1);
        int v = hsv.at(2);

        double _v = static_cast<double>(v) / 100;
        double _s = static_cast<double>(s) / 100;
        double c = _v * _s;
        double x =
            c * (1 - std::abs(fmod((static_cast<double>(h) / 60), 2) - 1));
        double m = _v - c;

        double r, g, b;
        if (h < 60)
        {
            r = c;
            g = x;
            b = 0;
        }
        else if (h < 120)
        {
            r = x;
            g = c;
            b = 0;
        }
        else if (h < 180)
        {
            r = 0;
            g = c;
            b = x;
        }
        else if (h < 240)
        {
            r = 0;
            g = x;
            b = c;
        }
        else if (h < 300)
        {
            r = x;
            g = 0;
            b = c;
        }
        else
        {
            r = c;
            g = 0;
            b = x;
        }

        r = ((r + m) * 255);
        g = ((g + m) * 255);
        b = ((b + m) * 255);

        hsv[0] = static_cast<int>(round(r));
        hsv[1] = static_cast<int>(round(g));
        hsv[2] = static_cast<int>(round(b));
    }

    void rgb_to_hsv(rgb24_image& image)
    {
        std::vector<int> colors(3);

        for (int i = 0; i < image.sx * image.sy * 3; i += 3)
        {
            colors[0] = image.pixels[i];
            colors[1] = image.pixels[i + 1];
            colors[2] = image.pixels[i + 2];

            rgb_color_hsv(colors);

            image.pixels[i] = colors[0];
            image.pixels[i + 1] = colors[1];
            image.pixels[i + 2] = colors[2];
        }
    }

    void hsv_to_rgb(hsv24_image& image)
    {
        std::vector<int> colors(3);

        for (int i = 0; i < image.sx * image.sy * 3; i += 3)
        {
            colors[0] = image.pixels[i];
            colors[1] = image.pixels[i + 1];
            colors[2] = image.pixels[i + 2];

            hsv_color_rgb(colors);

            image.pixels[i] = colors[0];
            image.pixels[i + 1] = colors[1];
            image.pixels[i + 2] = colors[2];
        }
    }
} // namespace tifo