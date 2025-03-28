#pragma once

#include <string>
#include <sstream>
#include <iostream>
#include <chrono>
#include <morph/loadpng.h>

namespace morph_example
{

namespace m = morph;

constexpr int first_png = 1;
constexpr int last_png = 80;
constexpr int num_pngs = last_png - first_png + 1;

constexpr int img_w = 320;
constexpr int img_h = 180;
constexpr int img_sz = img_w * img_h;

std::vector<m::vvec<m::vec<float, 3>>> load_imgs()
{
    // First load the images into memory. All of 'em should fit.
    std::cout << "Load images into memory...\n";
    std::vector<m::vvec<m::vec<float, 3>>> images(num_pngs);
    m::vec<bool, 2> flip = {false, true};
    for (int idx = first_png; idx <= last_png; ++idx) {
        std::stringstream ss;
        ss << "./../examples/components/frames/frame-";
        if (idx < 10) {ss << "0";}
        if (idx < 100) {ss << "0";}
        ss << idx << ".png";
        std::string fpath = ss.str();
        images[idx - first_png].resize(img_w * img_h);
        std::cout << "path = " << fpath << std::endl;
        m::loadpng<float, 3>(fpath, images[idx - first_png], flip);
    }

    return images;
}

} // namespace morph_example
