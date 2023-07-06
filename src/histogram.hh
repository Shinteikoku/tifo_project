#ifndef HISTOGRAM_HH
#define	HISTOGRAM_HH

#include "image.hh"
#include <fstream>

namespace tifo {

    typedef struct { unsigned int histogram[IMAGE_NB_LEVELS]; } histogram_1d;

    histogram_1d* make_histogram(gray8_image& image, int limit);
    void save_hist(histogram_1d& hist, const char* path);
    histogram_1d* cumulative_hist(histogram_1d& hist, int limit);
}

#endif