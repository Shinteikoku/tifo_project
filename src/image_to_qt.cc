//
// Created by Conan Maël on 03/07/2023.
//
#include "image_to_qt.hh"

#include <QDebug>

QImage rgb_to_qimage(const tifo::rgb24_image& inputImage)
{
    int width = inputImage.sx;
    int height = inputImage.sy;

    int k = 0;

    QImage outputImage(width, height, QImage::Format_RGB32);

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            int red = inputImage.pixels[k];
            int green = inputImage.pixels[k + 1];
            int blue = inputImage.pixels[k + 2];

            k += 3;

            QColor color(red, green, blue);
            outputImage.setPixelColor(x, y, color);
        }
    }

    return outputImage;
}

tifo::rgb24_image* qimage_to_rgb(const QImage& inputImage)
{
    int width = inputImage.width();
    int height = inputImage.height();
    int k = 0;

    auto outputImage = new tifo::rgb24_image(width, height);

    if (outputImage->pixels == nullptr)
        qDebug() << "Pixels not allocated";

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            QColor color = inputImage.pixelColor(x, y);
            int red = color.red();
            int green = color.green();
            int blue = color.blue();

            outputImage->pixels[k] = red;
            outputImage->pixels[k + 1] = green;
            outputImage->pixels[k + 2] = blue;

            k += 3;
        }
    }

    return outputImage;
}