#pragma once

/*
 * Helper to load PNG images into morph::vvec<morph::vec<float>> format and similar.
 *
 * Note: You have to #include this before morph/Visual.h
 */
#define LODEPNG_NO_COMPILE_ANCILLARY_CHUNKS 1
#include <morph/lodepng.h>

#include <type_traits>
#include <vector>
#include <string>
#include <cstddef>
#include <stdexcept>
#include <morph/vec.h>
#include <morph/vvec.h>

namespace morph {

    /*
     * Wrap lodepng::decode to load a PNG from file, placing the data into the
     * image_data array. Figure out based on the type of T, how to scale the numbers.
     *
     * Use with T as float, double, unsigned char/int or morph::vec<float, 3> etc
     *
     * If flip[0] is true, then flip the order of the rows to do a left/right flip of
     * the image during loading.
     *
     * If flip[1] is true, then flip the order of the rows to do an up/down flip of the
     * image during loading.
     *
     * Note: The default for flip is {false, true}, which means that by default,
     * image_data will be filled in a bottom-left to top-right order.
     */
    template <typename T>
    static morph::vec<unsigned int, 2> loadpng (const std::string& filename, morph::vvec<T>& image_data,
                                                const morph::vec<bool,2> flip = {false, true})
    {
        std::vector<unsigned char> png;
        unsigned int w = 0;
        unsigned int h = 0;
        // Assume RGBA and bit depth of 8
        unsigned lprtn = lodepng::decode (png, w, h, filename, LCT_RGBA, 8);
        if (lprtn != 0) {
            std::string err = "morph::loadpng: lodepng::decode returned error code "
            + std::to_string(lprtn) + std::string(": ") + std::string(lodepng_error_text (lprtn));
            throw std::runtime_error (err);
        }
        // For return:
        morph::vec<unsigned int, 2> dims = {w, h};

        // Now convert out into a value placed in image_data
        // If T is float or double, then get mean RGB, convert to range 0 to 1
        // If T is of integer type, then get mean and encode in range 0-255

        unsigned int vsz = png.size();
        if (vsz % 4 != 0) {
            throw std::runtime_error ("morph::loadpng: Expect png vector to have size divisible by 4.");
        }

        image_data.resize (vsz/4);

        for (unsigned int c = 0; c < dims[1]; ++c) {
            for (unsigned int r = 0; r < dims[0]; ++r) {
                // Offset into png
                unsigned int i = 4*r + 4*dims[0]*c;
                // Offset into image_data depends on what flips the caller wants
                unsigned int idx = flip[0] == true ?
                (flip[1]==true ? ((dims[0]-r-1) + dims[0]*(dims[1]-c-1)) : ((dims[0]-r-1) + dims[0]*c))
                : (flip[1]==true ? (r + dims[0]*(dims[1]-c-1)) : (r + dims[0]*c));

                // The above unpacks as:

                // flip[0]==false and flip[1]==false: (no flipping)
                // idx = r + dims[0]*c

                // flip[0]==false and flip[1]==true: (vertical flip)
                // idx = (r + dims[0]*(dims[1]-c-1))

                // flip[0]==true and flip[1]==false: (horizontal flip)
                // ((dims[0]-r-1) + dims[0]*c))

                // flip[0]==true and flip[1]==true:  (v-h flip)
                // ((dims[0]-r-1) + dims[0]*(dims[1]-c-1))

                if constexpr (std::is_same<std::decay_t<T>, float>::value == true
                              || std::is_same<std::decay_t<T>, double>::value == true) {
                    // monochrome 0-1 values
                    image_data[idx] = (static_cast<T>(png[i] + png[i+1] + png[i+2]))/T{765}; // 3*255

                } else if constexpr (std::is_same<std::decay_t<T>, unsigned int>::value == true
                                     || std::is_same<std::decay_t<T>, unsigned char>::value == true) {
                    // monochrome, 0-255 values
                    image_data[idx] = (static_cast<T>(png[i] + png[i+1] + png[i+2]))/T{3};

                } else {
                    // C++-20 mechanism to trigger a compiler error for the else case. Not user friendly!
                    //[]<bool flag = false>() { static_assert(flag, "no match"); }();
                    throw std::runtime_error ("morph::loadpng: type failure");
                }
            }
        }

        return dims;
    }

    /*
     * This overload pf loadpng reads the image into a vvec of vecs of dimension 3 or 4.
     *
     * \tparam T The type of the individual channels. Expected to be unsigned int,
     * unsigned char, float or double.
     *
     * \tparam N The number of channels (3 for RGB; 4 for RGBA, anything else will lead
     * to errors)
     */
    template <typename T, std::size_t N>
    static morph::vec<unsigned int, 2> loadpng (const std::string& filename,
                                                morph::vvec<morph::vec<T, N>>& image_data,
                                                const morph::vec<bool,2> flip = {false, true})
    {
        std::vector<unsigned char> png;
        unsigned int w = 0;
        unsigned int h = 0;
        // Assume RGBA and bit depth of 8
        unsigned lprtn = lodepng::decode (png, w, h, filename, LCT_RGBA, 8);
        if (lprtn != 0) {
            std::string err = "morph::loadpng: lodepng::decode returned error code "
            + std::to_string(lprtn) + std::string(": ") + std::string(lodepng_error_text (lprtn));
            throw std::runtime_error (err);
        }
        // For return:
        morph::vec<unsigned int, 2> dims = {w, h};

        // Now convert out into a value placed in image_data
        // If T is float or double, then get mean RGB, convert to range 0 to 1
        // If T is of integer type, then get mean and encode in range 0-255

        unsigned int vsz = png.size();
        if (vsz % 4 != 0) {
            throw std::runtime_error ("morph::loadpng: Expect png vector to have size divisible by 4.");
        }

        image_data.resize (vsz/4);

        for (unsigned int c = 0; c < dims[1]; ++c) {
            for (unsigned int r = 0; r < dims[0]; ++r) {

                // Offset into png
                unsigned int i = 4*r + 4*dims[0]*c;
                // Offset into image_data depends on what flips the caller wants
                unsigned int idx = flip[0] == true ?
                (flip[1]==true ? ((dims[0]-r-1) + dims[0]*(dims[1]-c-1)) : ((dims[0]-r-1) + dims[0]*c))
                : (flip[1]==true ? (r + dims[0]*(dims[1]-c-1)) : (r + dims[0]*c));

                if constexpr ((std::is_same<std::decay_t<T>, float>::value == true
                               || std::is_same<std::decay_t<T>, double>::value == true) && N==3) {
                    // RGB, 0-1 values
                    unsigned char p0 = png[i];
                    unsigned char p1 = png[i+1];
                    unsigned char p2 = png[i+2];
                    image_data[idx] = { static_cast<T>(p0), static_cast<T>(p1), static_cast<T>(p2) };
                    image_data[idx] /= T{255};

                } else if constexpr ((std::is_same<std::decay_t<T>, float>::value == true
                                      || std::is_same<std::decay_t<T>, double>::value == true) && N==4) {
                    // RGBA 0-1 values
                    image_data[idx][0] = static_cast<T>(png[i]) / T{255};
                    image_data[idx][1] = static_cast<T>(png[i+1]) / T{255};
                    image_data[idx][2] = static_cast<T>(png[i+2]) / T{255};
                    image_data[idx][3] = static_cast<T>(png[i+3]) / T{255};

                } else if constexpr ((std::is_same<std::decay_t<T>, unsigned char>::value == true
                                      || std::is_same<std::decay_t<T>, unsigned int>::value == true) && N==3) {
                    // RGB, 0-255 values
                    image_data[idx][0] = static_cast<T>(png[i]);
                    image_data[idx][1] = static_cast<T>(png[i+1]);
                    image_data[idx][2] = static_cast<T>(png[i+2]);

                } else if constexpr ((std::is_same<std::decay_t<T>, unsigned char>::value == true
                                      || std::is_same<std::decay_t<T>, unsigned int>::value == true) && N==4) {
                    // RGBA, 0-255 values
                    image_data[idx][0] = static_cast<T>(png[i]);
                    image_data[idx][1] = static_cast<T>(png[i+1]);
                    image_data[idx][2] = static_cast<T>(png[i+2]);
                    image_data[idx][3] = static_cast<T>(png[i+3]);

                } else {
                    // C++-20 mechanism to trigger a compiler error for the else case. Not user friendly!
                    //[]<bool flag = false>() { static_assert(flag, "no match"); }();
                    throw std::runtime_error ("morph::loadpng: type failure (or N is not 3 or 4)");
                }
            }
        }

        return dims;
    }

    // Load a colour PNG and return a vector of type T with elements ordered as RGBRGBRGB...
    template <typename T>
    static morph::vec<unsigned int, 2> loadpng_rgb (const std::string& filename, morph::vvec<T>& image_data,
                                                    const morph::vec<bool,2> flip = {false, true})
    {
        std::vector<unsigned char> png;
        unsigned int w = 0;
        unsigned int h = 0;
        // Assume RGBA and bit depth of 8
        unsigned lprtn = lodepng::decode (png, w, h, filename, LCT_RGBA, 8);
        if (lprtn != 0) {
            std::string err = "morph::loadpng_rgb: lodepng::decode returned error code "
            + std::to_string(lprtn) + std::string(": ") + std::string(lodepng_error_text (lprtn));
            throw std::runtime_error (err);
        }
        // For return:
        morph::vec<unsigned int, 2> dims = {w, h};

        // Now convert out into a value placed in image_data
        // If T is float or double, then for each in RGB, convert to range 0 to 1
        // If T is of integer type, then for each in RGB encode in range 0-255

        unsigned int vsz = png.size();
        if (vsz % 4 != 0) {
            throw std::runtime_error ("morph::loadpng_rgb: Expect png vector to have size divisible by 4.");
        }

        image_data.resize (3*vsz/4);

        for (unsigned int c = 0; c < dims[1]; ++c) {
            for (unsigned int r = 0; r < dims[0]; ++r) {
                // Offset into png
                unsigned int i = 4*r + 4*dims[0]*c;
                // Offset into image_data depends on what flips the caller wants
                unsigned int idx_r = flip[0] == true ?
                (flip[1]==true ? ((dims[0]-r-1) + dims[0]*(dims[1]-c-1)) : ((dims[0]-r-1) + dims[0]*c))
                : (flip[1]==true ? (r + dims[0]*(dims[1]-c-1)) : (r + dims[0]*c));
                idx_r *= 3; // Because our output is rgbrgb...
                unsigned int idx_g = flip[0] == true ? idx_r-1 : idx_r+1;
                unsigned int idx_b = flip[0] == true ? idx_r-2 : idx_r+2;

                if constexpr (std::is_same<std::decay_t<T>, float>::value == true
                              || std::is_same<std::decay_t<T>, double>::value == true) {
                    image_data[idx_r] = static_cast<T>(png[i])/T{255};
                    image_data[idx_g] = static_cast<T>(png[i+1])/T{255};
                    image_data[idx_b] = static_cast<T>(png[i+2])/T{255};

                } else if constexpr (std::is_same<std::decay_t<T>, unsigned int>::value == true
                                     || std::is_same<std::decay_t<T>, unsigned char>::value == true) {
                    // Copy RGB, 0-255 values
                    image_data[idx_r] = static_cast<T>(png[i]);
                    image_data[idx_g] = static_cast<T>(png[i+1]);
                    image_data[idx_b] = static_cast<T>(png[i+2]);

                } else {
                    // C++-20 mechanism to trigger a compiler error for the else case. Not user friendly!
                    //[]<bool flag = false>() { static_assert(flag, "no match"); }();
                    throw std::runtime_error ("morph::loadpng_rgb: type failure");
                }
            }
        }

        return dims;
    }

    // Load a colour PNG and return a vector of type T with elements ordered as RGBARGBARGBA...
    template <typename T>
    static morph::vec<unsigned int, 2> loadpng_rgba (const std::string& filename, morph::vvec<T>& image_data,
                                                     const morph::vec<bool,2> flip = {false, true})
    {
        std::vector<unsigned char> png;
        unsigned int w = 0;
        unsigned int h = 0;
        // Assume RGBA and bit depth of 8
        unsigned lprtn = lodepng::decode (png, w, h, filename, LCT_RGBA, 8);
        if (lprtn != 0) {
            std::string err = "morph::loadpng_rgba: lodepng::decode returned error code "
            + std::to_string(lprtn) + std::string(": ") + std::string(lodepng_error_text(lprtn));
            throw std::runtime_error (err);
        }
        // For return:
        morph::vec<unsigned int, 2> dims = {w, h};

        // Now convert out into a value placed in image_data
        // If T is float or double, then get mean RGB, convert to range 0 to 1
        // If T is of integer type, then get mean and encode in range 0-255

        unsigned int vsz = png.size();
        if (vsz % 4 != 0) {
            throw std::runtime_error ("morph::loadpng_rgba: Expect png vector to have size divisible by 4.");
        }

        image_data.resize (vsz);

        for (unsigned int c = 0; c < dims[1]; ++c) {
            for (unsigned int r = 0; r < dims[0]; ++r) {
                // Offset into png
                unsigned int i = 4*r + 4*dims[0]*c;
                // Offset into image_data depends on what flips the caller wants
                unsigned int idx_r = flip[0] == true ?
                (flip[1]==true ? ((dims[0]-r-1) + dims[0]*(dims[1]-c-1)) : ((dims[0]-r-1) + dims[0]*c))
                : (flip[1]==true ? (r + dims[0]*(dims[1]-c-1)) : (r + dims[0]*c));
                idx_r *= 4; // Because our output is rgbargba...
                unsigned int idx_g = flip[0] == true ? idx_r-1 : idx_r+1;
                unsigned int idx_b = flip[0] == true ? idx_r-2 : idx_r+2;
                unsigned int idx_a = flip[0] == true ? idx_r-3 : idx_r+3;

                if constexpr (std::is_same<std::decay_t<T>, float>::value == true
                              || std::is_same<std::decay_t<T>, double>::value == true) {
                    image_data[idx_r] = static_cast<T>(png[i])/T{255};
                    image_data[idx_g] = static_cast<T>(png[i+1])/T{255};
                    image_data[idx_b] = static_cast<T>(png[i+2])/T{255};
                    image_data[idx_a] = static_cast<T>(png[i+3])/T{255};

                } else if constexpr (std::is_same<std::decay_t<T>, unsigned int>::value == true
                                     || std::is_same<std::decay_t<T>, unsigned char>::value == true) {
                    // Copy RGB, 0-255 values
                    image_data[idx_r] = static_cast<T>(png[i]);
                    image_data[idx_g] = static_cast<T>(png[i+1]);
                    image_data[idx_b] = static_cast<T>(png[i+2]);
                    image_data[idx_a] = static_cast<T>(png[i+3]);

                } else {
                    // C++-20 mechanism to trigger a compiler error for the else case. Not user friendly!
                    //[]<bool flag = false>() { static_assert(flag, "no match"); }();
                    throw std::runtime_error ("morph::loadpng_rgba: type failure");
                }
            }
        }

        return dims;
    }

    // Load a colour PNG and return a vector of type T with elements ordered as RGBARGBARGBA...
    template <typename T, unsigned int im_w, unsigned int im_h>
    static morph::vec<unsigned int, 2> loadpng_rgba (const std::string& filename, morph::vec<T, 4*im_w*im_h>& image_data,
                                                     const morph::vec<bool,2> flip = {false, true})
    {
        std::vector<unsigned char> png;
        unsigned int w = 0;
        unsigned int h = 0;
        // Assume RGBA and bit depth of 8
        unsigned lprtn = lodepng::decode (png, w, h, filename, LCT_RGBA, 8);
        if (lprtn != 0) {
            std::string err = "morph::loadpng_rgba: lodepng::decode returned error code "
            + std::to_string(lprtn) + std::string(": ") + std::string(lodepng_error_text(lprtn));
            throw std::runtime_error (err);
        }
        // For return:
        morph::vec<unsigned int, 2> dims = {w, h};
        if (w != im_w || h != im_h) {
            throw std::runtime_error ("morph::loadpng_rgba: Expect png to be the size specified in the template args.");
        }

        // Now convert out into a value placed in image_data
        // If T is float or double, then get mean RGB, convert to range 0 to 1
        // If T is of integer type, then get mean and encode in range 0-255

        unsigned int vsz = png.size();
        if (vsz % 4 != 0) {
            throw std::runtime_error ("morph::loadpng_rgba: Expect png vector to have size divisible by 4.");
        }

        for (unsigned int c = 0; c < dims[1]; ++c) {
            for (unsigned int r = 0; r < dims[0]; ++r) {
                // Offset into png
                unsigned int i = 4*r + 4*dims[0]*c;
                // Offset into image_data depends on what flips the caller wants
                unsigned int idx_r = flip[0] == true ?
                (flip[1]==true ? ((dims[0]-r-1) + dims[0]*(dims[1]-c-1)) : ((dims[0]-r-1) + dims[0]*c))
                : (flip[1]==true ? (r + dims[0]*(dims[1]-c-1)) : (r + dims[0]*c));
                idx_r *= 4; // Because our output is rgbargba...
                unsigned int idx_g = flip[0] == true ? idx_r-1 : idx_r+1;
                unsigned int idx_b = flip[0] == true ? idx_r-2 : idx_r+2;
                unsigned int idx_a = flip[0] == true ? idx_r-3 : idx_r+3;

                if constexpr (std::is_same<std::decay_t<T>, float>::value == true
                              || std::is_same<std::decay_t<T>, double>::value == true) {
                    image_data[idx_r] = static_cast<T>(png[i])/T{255};
                    image_data[idx_g] = static_cast<T>(png[i+1])/T{255};
                    image_data[idx_b] = static_cast<T>(png[i+2])/T{255};
                    image_data[idx_a] = static_cast<T>(png[i+3])/T{255};

                } else if constexpr (std::is_same<std::decay_t<T>, unsigned int>::value == true
                                     || std::is_same<std::decay_t<T>, unsigned char>::value == true) {
                    // Copy RGB, 0-255 values
                    image_data[idx_r] = static_cast<T>(png[i]);
                    image_data[idx_g] = static_cast<T>(png[i+1]);
                    image_data[idx_b] = static_cast<T>(png[i+2]);
                    image_data[idx_a] = static_cast<T>(png[i+3]);

                } else {
                    // C++-20 mechanism to trigger a compiler error for the else case. Not user friendly!
                    //[]<bool flag = false>() { static_assert(flag, "no match"); }();
                    throw std::runtime_error ("morph::loadpng_rgba: type failure");
                }
            }
        }

        return dims;
    }

} // namespace
