/*
 * Helper to load PNG images into morph::vVector<Vector<float>> format and similar.
 *
 * Note: You have to #include this before morph/Visual.h
 */
#define LODEPNG_NO_COMPILE_ANCILLARY_CHUNKS 1
#include <morph/lodepng.h>

#include <type_traits>
#include <vector>
#include <string>
#include <stdexcept>
#include <morph/Vector.h>
#include <morph/vVector.h>

namespace morph {

    /*
     * Wrap lodepng::decode to load a PNG from file, placing the data into the
     * image_data array. Figure out based on the type of T, how to scale the numbers.
     *
     * Use with T as float, double, unsigned char/int or morph::Vector<float, 3> etc
     */
    template <typename T>
    static morph::Vector<unsigned int, 2> loadpng (const std::string& filename, morph::vVector<T>& image_data)
    {
        std::vector<unsigned char> png;
        unsigned int w = 0;
        unsigned int h = 0;
        // Assume RGBA and bit depth of 8
        lodepng::decode (png, w, h, filename, LCT_RGBA, 8);
        // For return:
        morph::Vector<unsigned int, 2> dims = {w, h};

        // Now convert out into a value placed in image_data
        // If T is float or double, then get mean RGB, convert to range 0 to 1
        // If T is of integer type, then get mean and encode in range 0-255

        unsigned int vsz = png.size();
        if (vsz % 4 != 0) {
            throw std::runtime_error ("Expect png vector to have size divisible by 4.");
        }

        image_data.resize (vsz/4);

        for (unsigned int i = 0; i < vsz; i+=4) {

            if (std::is_same<std::decay_t<T>, float>::value == true
                       || std::is_same<std::decay_t<T>, double>::value == true) {
                // monochrome 0-1 values
                image_data[i/4] = (static_cast<T>(png[i] + png[i+1] + png[i+2]))/T{765}; // 3*255

            } else if (std::is_same<std::decay_t<T>, unsigned int>::value == true
                       || std::is_same<std::decay_t<T>, unsigned char>::value == true) {
                // monochrome, 0-255 values
                image_data[i/4] = (static_cast<T>(png[i] + png[i+1] + png[i+2]))/T{3};

            } else {
                // C++-20 mechanism to trigger a compiler error for the else case. Not user friendly!
                //[]<bool flag = false>() { static_assert(flag, "no match"); }();
                throw std::runtime_error ("type failure");
            }
        }

        return dims;
    }

    template <typename T, size_t N>
    static morph::Vector<unsigned int, 2> loadpng (const std::string& filename, morph::vVector<morph::Vector<T, N>>& image_data)
    {
        std::vector<unsigned char> png;
        unsigned int w = 0;
        unsigned int h = 0;
        // Assume RGBA and bit depth of 8
        lodepng::decode (png, w, h, filename, LCT_RGBA, 8);
        // For return:
        morph::Vector<unsigned int, 2> dims = {w, h};

        // Now convert out into a value placed in image_data
        // If T is float or double, then get mean RGB, convert to range 0 to 1
        // If T is of integer type, then get mean and encode in range 0-255

        unsigned int vsz = png.size();
        if (vsz % 4 != 0) {
            throw std::runtime_error ("Expect png vector to have size divisible by 4.");
        }

        image_data.resize (vsz/4);

        for (unsigned int i = 0; i < vsz; i+=4) {

            if constexpr ((std::is_same<std::decay_t<T>, float>::value == true
                           || std::is_same<std::decay_t<T>, double>::value == true) && N==3) {
                // RGB, 0-1 values
                unsigned char p0 = png[i];
                unsigned char p1 = png[i+1];
                unsigned char p2 = png[i+2];
                image_data[i/4] = { static_cast<T>(p0), static_cast<T>(p1), static_cast<T>(p2) };
                image_data[i/4] /= T{255};

            } else if constexpr ((std::is_same<std::decay_t<T>, float>::value == true
                                  || std::is_same<std::decay_t<T>, double>::value == true) && N==4) {
                // RGBA 0-1 values
                image_data[i/4][0] = static_cast<T>(png[i]) / T{255};
                image_data[i/4][1] = static_cast<T>(png[i+1]) / T{255};
                image_data[i/4][2] = static_cast<T>(png[i+2]) / T{255};
                image_data[i/4][3] = static_cast<T>(png[i+3]) / T{255};

            } else if constexpr ((std::is_same<std::decay_t<T>, unsigned char>::value == true
                                  || std::is_same<std::decay_t<T>, unsigned int>::value == true) && N==3) {
                // RGB, 0-255 values
                image_data[i/4][0] = static_cast<T>(png[i]);
                image_data[i/4][1] = static_cast<T>(png[i+1]);
                image_data[i/4][2] = static_cast<T>(png[i+2]);

            } else if constexpr ((std::is_same<std::decay_t<T>, unsigned char>::value == true
                                  || std::is_same<std::decay_t<T>, unsigned int>::value == true) && N==4) {
                // RGBA, 0-255 values
                image_data[i/4][0] = static_cast<T>(png[i]);
                image_data[i/4][1] = static_cast<T>(png[i+1]);
                image_data[i/4][2] = static_cast<T>(png[i+2]);
                image_data[i/4][3] = static_cast<T>(png[i+3]);

            } else {
                // C++-20 mechanism to trigger a compiler error for the else case. Not user friendly!
                //[]<bool flag = false>() { static_assert(flag, "no match"); }();
                throw std::runtime_error ("type failure");
            }
        }

        return dims;
    }

} // namespace
