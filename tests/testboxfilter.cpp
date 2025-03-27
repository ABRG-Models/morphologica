// Test (and profile) the box filter
#include <morph/MathAlgo.h>
#include <morph/vvec.h>
#include <chrono>
#include <cstdint>

int main()
{
    using namespace std::chrono;
    using sc = std::chrono::steady_clock;

    constexpr int img_w = 256;
    constexpr int img_h = 64;
    constexpr int data_sz = img_w * img_h;
    constexpr bool onlysum_false = false;

    // Single precision
    morph::vvec<float> input_f (data_sz, 0.0f);
    morph::vvec<float> output_f (data_sz, 0.0f);
    input_f.randomize();

    sc::time_point t0_f = sc::now();
    morph::MathAlgo::boxfilter_2d<float, 17, img_w> (input_f, output_f);
    sc::time_point t1_f = sc::now();

    sc::duration t_d_f = t1_f - t0_f;
    std::cout << data_sz << " pixels boxfiltered (17x17, float) in " << duration_cast<microseconds>(t_d_f).count() << " us\n";

    // Double precision
    morph::vvec<double> input_d (data_sz, 0.0f);
    morph::vvec<double> output_d (data_sz, 0.0f);
    input_d.randomize();

    sc::time_point t0_d = sc::now();
    morph::MathAlgo::boxfilter_2d<double, 17, img_w> (input_d, output_d);
    sc::time_point t1_d = sc::now();

    sc::duration t_d_d = t1_d - t0_d;
    std::cout << data_sz << " pixels boxfiltered (17x17, double) in " << duration_cast<microseconds>(t_d_d).count() << " us\n";

    // Multi precision
    sc::time_point t0_m = sc::now();
    morph::MathAlgo::boxfilter_2d<double, 17, img_w, onlysum_false, float> (input_d, output_f);
    sc::time_point t1_m = sc::now();

    sc::duration t_d_m = t1_m - t0_m;
    std::cout << data_sz << " pixels boxfiltered (17x17, double in, float out) in " << duration_cast<microseconds>(t_d_m).count() << " us\n";


    // Multi precision with uint8_t input
#ifndef __GNUC__
    // Strictly, we can't vvec::randomize with a 1 byte type
    // (std::uniform_int_distribution is undefined for 1 byte IntTypes)...
    morph::vvec<uint16_t> input_u16 (data_sz, 0);
    input_u16.randomize(0, 255);
    morph::vvec<uint8_t> input_u8 = input_u16.as<uint8_t>();
#else
    // ...but gcc generously allows it
    morph::vvec<uint8_t> input_u8 (data_sz, 0);
    input_u8.randomize();
#endif
    unsigned int uisum = input_u8.sum<false, unsigned int>();
    unsigned int uisum2 = input_u8.sum(); // auto template param deduction? No.
    std::cout << "input_u8: " << uisum << " or " << uisum2  << std::endl;

    sc::time_point t0_u = sc::now();
    morph::MathAlgo::boxfilter_2d<uint8_t, 17, img_w, onlysum_false, float> (input_u8, output_f);
    sc::time_point t1_u = sc::now();

    std::cout << "output_flt: " << output_f.sum() << std::endl;

    sc::duration t_d_u = t1_u - t0_u;
    std::cout << data_sz << " pixels boxfiltered (17x17, uint8_t in, float out) in " << duration_cast<microseconds>(t_d_u).count() << " us\n";

    return 0;
}
