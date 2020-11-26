#pragma once

#include <morph/ColourMap_Lists.h>

#include <stdexcept>
#include <cmath>

namespace morph {

    //! Different colour maps types.
    enum class ColourMapType
    {
        Jet,
        Rainbow,
        RainbowZeroBlack, // As Rainbow, but if datum is 0, then colour is pure black.
        RainbowZeroWhite, // As Rainbow, but if datum is 0, then colour is pure white.
        Magma,      // Like matplotlib's magma
        Inferno,    // matplotlib's inferno
        Plasma,     // etc
        Viridis,
        Cividis,
        Twilight,
        Greyscale,    // Greyscale is any hue; saturation=0; *value* varies. High signal (datum ->1) gives dark greys to black
        GreyscaleInv, // Inverted Greyscale. High signal gives light greys to white
        Monochrome,   // Monochrome is 'monohue': fixed hue; vary the *saturation* with value fixed at 1.
        MonochromeRed,
        MonochromeBlue,
        MonochromeGreen,
        Fixed         // Fixed colour. Should return same colour for any datum. User must set hue, sat, val.
    };

    //! Different ways of specifying colour exist
    enum class ColourOrder
    {
        RGB,
        BGR
        //RGBA?
        //BGRA?
    };

    /*!
     * Colour mapping
     *
     * \tparam T The type of the datum used to traverse the colour map. When this is a
     * floating point type, then the input datum should be in the range 0.0 to 1.0. If
     * an integral type, then what? For char/unsigned char then 0-127 or 0-255. When
     * unsigned short then 0-MAX also. What about if unsigned int or larger? Surely not
     * the full range of these? Allow a runtime choice?
     */
    template <typename T>
    class ColourMap
    {
    private:
        //! Type of map
        ColourMapType type = ColourMapType::Jet;
        //! The hue (range 0 to 1.0f) as used in HSV colour values for Monochrome maps.
        float hue = 0.0;
        //! The saturation, used for ColourMapType::Fixed only
        float sat = 1.0;
        //! The value, used for ColourMapType::Fixed only
        float val = 1.0;

    public:
        //! Default constructor is required, but need not do anything.
        ColourMap() {}
        //! Construct with a type
        ColourMap (ColourMapType _t) { this->type = _t; }

        static std::array<float, 3> nanColour (ColourMapType _t)
        {
            std::array<float, 3> c = {0.0f, 0.0f, 0.0f};
            switch (_t) {
            case ColourMapType::Jet:
            {
                // Red is part of Jet, but purple isn't
                c = {1.0, 0.0f, 1.0f};
                break;
            }
            case ColourMapType::Rainbow:
            {
                c = {1.0, 1.0f, 1.0f};
                break;
            }
            case ColourMapType::RainbowZeroBlack:
            {
                c = {1.0, 1.0f, 1.0f};
                break;
            }
            case ColourMapType::RainbowZeroWhite:
            {
                break;
            }
            case ColourMapType::Magma:
            {
                c = {1.0, 1.0f, 1.0f};
                break;
            }
            case ColourMapType::Inferno:
            {
                c = {1.0, 0.0f, 0.0f};
                break;
            }
            case ColourMapType::Plasma:
            {
                c = {1.0, 0.0f, 0.0f};
                break;
            }
            case ColourMapType::Viridis:
            {
                c = {1.0, 0.0f, 0.0f};
                break;
            }
            case ColourMapType::Cividis:
            {
                c = {1.0, 0.0f, 0.0f};
                break;
            }
            case ColourMapType::Twilight:
            {
                c = {1.0, 0.0f, 0.0f};
                break;
            }
            case ColourMapType::Greyscale:
            {
                c = {1.0, 0.0f, 0.0f};
                break;
            }
            case ColourMapType::GreyscaleInv:
            {
                // The 'inverted' greyscale tends to white for maximum signal
                c = {1.0, 0.0f, 0.0f};
                break;
            }
            case ColourMapType::Monochrome:
            case ColourMapType::MonochromeRed:
            case ColourMapType::MonochromeBlue:
            case ColourMapType::MonochromeGreen:
            {
                c = {1.0, 0.0f, 0.0f};
                break;
            }
            case ColourMapType::Fixed:
            {
                c = {1.0, 0.0f, 0.0f};
                break;
            }
            default:
            {
                break;
            }
            }

            return c;
        }

        //! The maximum number for the range of the datum to convert to a colour. 1 for
        //! floating point variables.
        T range_max = ColourMap<T>::range_max_init();
        static constexpr T range_max_init()
        {
            T rm = T{1};
            // If integral type, might need to change this
            if constexpr (std::is_same<std::decay_t<T>, unsigned char>::value == true) {
                rm = 255;
            } else if constexpr (std::is_same<std::decay_t<T>, char>::value == true) {
                rm = 127;
            } else if constexpr (std::is_same<std::decay_t<T>, bool>::value == true) {
                rm = true;
            } else if constexpr (std::is_same<std::decay_t<T>, unsigned short>::value == true
                                 || std::is_same<std::decay_t<T>, short>::value == true
                                 || std::is_same<std::decay_t<T>, unsigned int>::value == true
                                 || std::is_same<std::decay_t<T>, int>::value == true
                                 || std::is_same<std::decay_t<T>, unsigned long long int>::value == true
                                 || std::is_same<std::decay_t<T>, long long int>::value == true) {
                // For long types, default to 8 bit input; user can change afterwards.
                rm = 255;
            }
            return rm;
        }

        //! Convert the scalar datum into an RGB (or BGR) colour
        std::array<float, 3> convert (T _datum)
        {
            float datum = 0.0f;

            // Convert T into a suitable value (with a suitable scaling as necessary) to
            // make the conversion to array<float,3> colour.
            if constexpr (std::is_same<std::decay_t<T>, double>::value == true) {
                // Copy, enforce range
                datum = _datum > T{1.0} ? 1.0f : static_cast<float>(_datum);
                datum = datum < 0.0f ? 0.0f : datum;

            } else if constexpr (std::is_same<std::decay_t<T>, float>::value == true) {
                // Copy, and enforce range of datum
                datum = _datum > 1.0f ? 1.0f : _datum;
                datum = datum < 0.0f ? 0.0f : datum;

            } else if constexpr (std::is_same<std::decay_t<T>, bool>::value == true) {
                datum = _datum ? 1.0f : 0.0f;

            } else if constexpr (std::is_integral<std::decay_t<T>>::value == true) {
                // For integral types, there's a 'max input range' value
                datum = _datum < 0 ? 0.0f : (float)_datum / static_cast<float>(this->range_max);
                datum = datum < 0.0f ? 0.0f : datum;

            } else {
                throw std::runtime_error ("Unhandled ColourMap data type.");
            }

            std::array<float, 3> c = {0.0f, 0.0f, 0.0f};

            // Check for nan and return a 'nan' colour for the colour map
            if constexpr (std::is_same<std::decay_t<T>, double>::value == true
                          || std::is_same<std::decay_t<T>, float>::value == true) {
                if (std::isnan(datum) == true) { c = ColourMap<T>::nanColour(this->type); return c; }
            }

            switch (this->type) {
            case ColourMapType::Jet:
            {
                c = ColourMap::jetcolour (datum);
                break;
            }
            case ColourMapType::Rainbow:
            {
                c = ColourMap::rainbow (datum);
                break;
            }
            case ColourMapType::RainbowZeroBlack:
            {
                if (datum != T{0.0}) {
                    c = ColourMap::rainbow (datum);
                }
                break;
            }
            case ColourMapType::RainbowZeroWhite:
            {
                if (datum != T{0.0}) {
                    c = ColourMap::rainbow (datum);
                } else {
                    c = {1.0f, 1.0f, 1.0f};
                }
                break;
            }
            case ColourMapType::Magma:
            {
                c = ColourMap::magma (datum);
                break;
            }
            case ColourMapType::Inferno:
            {
                c = ColourMap::inferno (datum);
                break;
            }
            case ColourMapType::Plasma:
            {
                c = ColourMap::plasma (datum);
                break;
            }
            case ColourMapType::Viridis:
            {
                c = ColourMap::viridis (datum);
                break;
            }
            case ColourMapType::Cividis:
            {
                c = ColourMap::cividis (datum);
                break;
            }
            case ColourMapType::Twilight:
            {
                c = ColourMap::twilight (datum);
                break;
            }
            case ColourMapType::Greyscale:
            {
                // The standard Greyscale Colourmap is best (and matches python Greys
                // colour map) if white means minimum signal and black means maximum
                // signal; hence pass 1-datum to ColourMap::greyscale().
                c = this->greyscale (T{1.0}-datum);
                break;
            }
            case ColourMapType::GreyscaleInv:
            {
                // The 'inverted' greyscale tends to white for maximum signal
                c = this->greyscale (datum);
                break;
            }
            case ColourMapType::Monochrome:
            case ColourMapType::MonochromeRed:
            case ColourMapType::MonochromeBlue:
            case ColourMapType::MonochromeGreen:
            {
                c = this->monochrome (datum);
                break;
            }
            case ColourMapType::Fixed:
            {
                c = ColourMap::hsv2rgb (this->hue, this->sat, this->val);
                break;
            }
            default:
            {
                break;
            }
            }
#if 0
            if (this->order == ColourOrder::RGB) {
            } else if (this->order == ColourOrder::BGR) {
            }
#endif
            return c;
        }

        //! Convert for 4 component colours
        //array<float, 4> convertAlpha (T datum);

        // Set the colour map type.
        void setType (const ColourMapType& tp)
        {
            this->type = tp;
            // Set hue if necessary
            switch (tp) {
            case ColourMapType::Monochrome:
            {
                break;
            }
            case ColourMapType::MonochromeRed:
            {
                this->hue = 1.0f;
                break;
            }
            case ColourMapType::MonochromeBlue:
            {
                this->hue = 0.667f;
                break;
            }
            case ColourMapType::MonochromeGreen:
            {
                this->hue = 0.333f;
                break;
            }
            default:
            {
                break;
            }
            }
        }

        // Set the hue... unless you can't/shouldn't
        void setHue (const float& h)
        {
            switch (this->type) {
            case ColourMapType::MonochromeRed:
            case ColourMapType::MonochromeBlue:
            case ColourMapType::MonochromeGreen:
            {
                throw std::runtime_error ("This colour map does not accept changes to the hue");
                break;
            }
            default:
            {
                this->hue = h;
                break;
            }
            }
        }

        //! Set the saturation. For many colour maps, this will make little difference,
        //! but for fixed, it allows you to have white, with hue=anything, sat=0, val=1
        void setSat (const float& _s)
        {
            if (this->type != ColourMapType::Fixed) {
                throw std::runtime_error ("Only ColourMapType::Fixed allows setting of saturation");
            }
            this->sat = _s;
        }

        void setVal (const float& _v)
        {
            if (this->type != ColourMapType::Fixed) {
                throw std::runtime_error ("Only ColourMapType::Fixed allows setting of value");
            }
            this->val = _v;
        }

        void setHSV (const float& h, const float& s, const float& v)
        {
            if (this->type != ColourMapType::Fixed) {
                throw std::runtime_error ("Only ColourMapType::Fixed allows setting of saturation/value");
            }
            this->hue = h;
            this->sat = s;
            this->val = v;
        }

        void setHSV (const std::array<float,3> hsv) { this->setHSV (hsv[0],hsv[1],hsv[2]); }

#if 0
        void setRGB (const float& r, const float& g, const float& b)
        {
            if (this->type != ColourMapType::Fixed) {
                throw std::runtime_error ("Only ColourMapType::Fixed allows setting of RGB");
            }
            // Set hue, sat, val by converting from RGB.
        }
#endif

        //! Get the hue, in its most saturated form
        std::array<float, 3> getHueRGB (void) { return ColourMap::hsv2rgb (this->hue, 1.0f, 1.0f); }

        //! Format of colours
        ColourOrder order = ColourOrder::RGB;

        /*!
         * @param datum gray value from 0.0 to 1.0
         *
         * @returns RGB value in jet colormap
         */
        static std::array<float,3> jetcolour (float datum)
        {
            float color_table[][3] = {
                {0.0, 0.0, 0.5}, // #00007F
                {0.0, 0.0, 1.0}, // blue
                {0.0, 0.5, 1.0}, // #007FFF
                {0.0, 1.0, 1.0}, // cyan
                {0.5, 1.0, 0.5}, // #7FFF7F
                {1.0, 1.0, 0.0}, // yellow
                {1.0, 0.5, 0.0}, // #FF7F00
                {1.0, 0.0, 0.0}, // red
                {0.5, 0.0, 0.0}, // #7F0000
            };
            std::array<float,3> col = {0.0f, 0.0f, 0.0f};
            float ivl = 1.0/8.0;
            for (int i=0; i<8; i++) {
                float llim = (i==0) ? 0.0f : (float)i/8.0f;
                float ulim = (i==7) ? 1.0f : ((float)(i+1))/8.0f;
                if (datum >= llim && datum <= ulim) {
                    for (int j=0; j<3; j++) {
                        float c = static_cast<float>(datum - llim);
                        col[j] = (color_table[i][j]*(ivl-c)/ivl + color_table[i+1][j]*c/ivl);
                    }
                    break;
                }
            }
            return col;
        }

        //! Convert hue, saturation, value to RGB. single precision arguments.
        static std::array<float,3> hsv2rgb (float h, float s, float v)
        {
            std::array<float, 3> rgb = {0.0f, 0.0f, 0.0f};
            int i = floor(h * 6);
            float f = h * 6. - i;
            float p = v * (1. - s);
            float q = v * (1. - f * s);
            float t = v * (1. - (1. - f) * s);
            switch (i % 6) {
            case 0: rgb[0] = v, rgb[1] = t, rgb[2] = p; break;
            case 1: rgb[0] = q, rgb[1] = v, rgb[2] = p; break;
            case 2: rgb[0] = p, rgb[1] = v, rgb[2] = t; break;
            case 3: rgb[0] = p, rgb[1] = q, rgb[2] = v; break;
            case 4: rgb[0] = t, rgb[1] = p, rgb[2] = v; break;
            case 5: rgb[0] = v, rgb[1] = p, rgb[2] = q; break;
            default: break;
            }
            return rgb;
        }

    private:
        /*!
         * @param datum gray value from 0.0 to 1.0
         *
         * @returns RGB value in a mono-colour map, with main colour this->hue;
         */
        std::array<float,3> monochrome (float datum)
        {
            return ColourMap::hsv2rgb (this->hue, datum, 1.0f);
        }

        /*!
         * @param datum gray value from 0.0 to 1.0
         *
         * @returns Generate RGB value for which all entries are equal and the
         * brightness gives the map value. Thus, \a datum = 1 gives white and \a datum =
         * 0 gives black.
         */
        std::array<float,3> greyscale (float datum)
        {
            return ColourMap::hsv2rgb (this->hue, 0.0f, datum);
            // or
            //return {datum, datum, datum}; // assuming 0 <= datum <= 1
        }

        //! A colour map which is a rainbow through the colour space, varying the hue.
        std::array<float,3> rainbow (float datum)
        {
            return ColourMap::hsv2rgb (datum, 1.0f, 1.0f);
        }

        //! A copy of matplotlib's magma colourmap
        std::array<float,3> magma (float datum)
        {
            // let's just try the closest colour from the map, with no interpolation
            size_t datum_i = static_cast<size_t>(std::abs (std::round (datum * (float)(morph::cm_magma_len-1))));
            std::array<float,3> c = {morph::cm_magma[datum_i][0], morph::cm_magma[datum_i][1], morph::cm_magma[datum_i][2]};
            return c;
        }

        //! A copy of matplotlib's inferno colourmap
        std::array<float,3> inferno (float datum)
        {
            // let's just try the closest colour from the map, with no interpolation
            size_t datum_i = static_cast<size_t>(std::abs (std::round (datum * (float)(morph::cm_inferno_len-1))));
            std::array<float,3> c = {morph::cm_inferno[datum_i][0], morph::cm_inferno[datum_i][1], morph::cm_inferno[datum_i][2]};
            return c;
        }

        //! A copy of matplotlib's plasma colourmap
        std::array<float,3> plasma (float datum)
        {
            // let's just try the closest colour from the map, with no interpolation
            size_t datum_i = static_cast<size_t>(std::abs (std::round (datum * (float)(morph::cm_plasma_len-1))));
            std::array<float,3> c = {morph::cm_plasma[datum_i][0], morph::cm_plasma[datum_i][1], morph::cm_plasma[datum_i][2]};
            return c;
        }

        //! A copy of matplotlib's viridis colourmap
        std::array<float,3> viridis (float datum)
        {
            // let's just try the closest colour from the map, with no interpolation
            size_t datum_i = static_cast<size_t>(std::abs (std::round (datum * (float)(morph::cm_viridis_len-1))));
            std::array<float,3> c = {morph::cm_viridis[datum_i][0], morph::cm_viridis[datum_i][1], morph::cm_viridis[datum_i][2]};
            return c;
        }

        //! A copy of matplotlib's cividis colourmap
        std::array<float,3> cividis (float datum)
        {
            // let's just try the closest colour from the map, with no interpolation
            size_t datum_i = static_cast<size_t>(std::abs (std::round (datum * (float)(morph::cm_cividis_len-1))));
            std::array<float,3> c = {morph::cm_cividis[datum_i][0], morph::cm_cividis[datum_i][1], morph::cm_cividis[datum_i][2]};
            return c;
        }

        //! A copy of matplotlib's twilight colourmap
        std::array<float,3> twilight (float datum)
        {
            // let's just try the closest colour from the map, with no interpolation
            size_t datum_i = static_cast<size_t>(std::abs (std::round (datum * (float)(morph::cm_twilight_len-1))));
            std::array<float,3> c = {morph::cm_twilight[datum_i][0], morph::cm_twilight[datum_i][1], morph::cm_twilight[datum_i][2]};
            return c;
        }
    };

} // namespace morph
