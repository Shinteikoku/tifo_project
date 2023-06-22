#include "src/histogram_operations.hh"
#include "src/image_operations.hh"
#include "src/filters.hh"
#include "src/image_io.hh"

int main()
{
    tifo::rgb24_image* image = tifo::load_image("../data/20160805_105246.tga");
    //auto new_image = tifo::argentique_filter(*image);
    //auto new_image = tifo::rgb_laplacian(*image, -0.2f);
    //auto new_image = tifo::rgb_apply_filter_to_gray(*image, tifo::sobel_filter);
    auto new_image = tifo::ir_filter(*image);
    //auto new_image = tifo::negative_filter(*image);
    tifo::save_image(*new_image, "../output/laplacian.tga");
    return 0;
}