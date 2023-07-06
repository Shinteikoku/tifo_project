#include "histogram.hh"
#include <iostream>

namespace tifo {
    histogram_1d* make_histogram(gray8_image& image, int limit)
    {
        auto hist = new histogram_1d;
        for (int i = 0; i < limit + 1; i++)
            hist->histogram[i] = 0;
        GRAY8 pixels = image.get_buffer();
        int length = image.length;
        for (int i = 0; i < length; i++)
            hist->histogram[pixels[i]]++;
        return hist;
    }

    void save_hist(histogram_1d& hist, const char* path)
    {
        std::ofstream file(path);
        for (int i = 0; i < 256; i++)
            file << hist.histogram[i] << "\n";
    }

    histogram_1d* cumulative_hist(histogram_1d& hist, int limit)
    {
        auto cumulative = new histogram_1d;

        for (int i = 0; i < limit + 1; i++)
        {
            if (i == 0)
                cumulative->histogram[i] = hist.histogram[i];
            else
                cumulative->histogram[i] = cumulative->histogram[i - 1] + hist.histogram[i];
        }
        return cumulative;
    }

}
