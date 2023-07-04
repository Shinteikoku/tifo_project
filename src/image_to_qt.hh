//
// Created by Conan Maël on 03/07/2023.
//
#pragma once

#include <QImage>

#include "image.hh"

tifo::rgb24_image* qimage_to_rgb(const QImage& inputImage);
QImage rgb_to_qimage(const tifo::rgb24_image& inputImage);