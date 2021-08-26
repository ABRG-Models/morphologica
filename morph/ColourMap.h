#pragma once

#include <morph/ColourMap_Lists.h>

#include <stdexcept>
#include <cmath>
#include <morph/tools.h>

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
        Duochrome,    // Two fixed hues, vary saturation of each with two input numbers.
        Trichrome,    // As for Duochrome, but with three inputs
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
        float hue = 0.0f;
        //! The saturation, used for ColourMapType::Fixed only
        float sat = 1.0f;
        //! The value, used for ColourMapType::Fixed only
        float val = 1.0f;

        // Used by Duochrome
        float hue2 = 0.33f;
        float sat2 = 1.0f;
        float val2 = 1.0f;

        // Used by Trichrome
        float hue3 = 0.66f;
        float sat3 = 1.0f;
        float val3 = 1.0f;

    public:
        //! Default constructor is required, but need not do anything.
        ColourMap() {}
        //! Construct with a type
        ColourMap (ColourMapType _t) { this->type = _t; }

        //! If s is a string that matches a ColourMapType, return that colour map
        //! type. If string doesn't match, return the default, Jet
        static ColourMapType strToColourMapType (const std::string& s)
        {
            ColourMapType cmt = morph::ColourMapType::Jet;
            std::string _s = s;
            morph::Tools::toLowerCase (_s);
            if (_s == "fixed") {
                cmt = morph::ColourMapType::Fixed;
            } else if (_s == "trichrome") {
                cmt = morph::ColourMapType::Trichrome;
            } else if (_s == "duochrome") {
                cmt = morph::ColourMapType::Duochrome;
            } else if (_s == "monochromegreen") {
                cmt = morph::ColourMapType::MonochromeGreen;
            } else if (_s == "monochromeblue") {
                cmt = morph::ColourMapType::MonochromeBlue;
            } else if (_s == "monochromered") {
                cmt = morph::ColourMapType::MonochromeRed;
            } else if (_s == "monochrome") {
                cmt = morph::ColourMapType::Monochrome;
            } else if (_s == "greyscale") {
                cmt = morph::ColourMapType::Greyscale;
            } else if (_s == "greyscaleinv") {
                cmt = morph::ColourMapType::GreyscaleInv;
            } else if (_s == "twilight") {
                cmt = morph::ColourMapType::Twilight;
            } else if (_s == "cividis") {
                cmt = morph::ColourMapType::Cividis;
            } else if (_s == "viridis") {
                cmt = morph::ColourMapType::Viridis;
            } else if (_s == "plasma") {
                cmt = morph::ColourMapType::Plasma;
            } else if (_s == "inferno") {
                cmt = morph::ColourMapType::Inferno;
            } else if (_s == "magma") {
                cmt = morph::ColourMapType::Magma;
            } else if (_s == "rainbowzerowhite") {
                cmt = morph::ColourMapType::RainbowZeroWhite;
            } else if (_s == "rainbowzeroblack") {
                cmt = morph::ColourMapType::RainbowZeroBlack;
            } else if (_s == "rainbow") {
                cmt = morph::ColourMapType::Rainbow;
            }
            return cmt;
        }

        //! Return the colour that represents not-a-number
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

        //! An overload of convert for DuoChrome ColourMaps
        std::array<float, 3> convert (T _datum1, T _datum2)
        {
            if (this->type != ColourMapType::Duochrome) {
                throw std::runtime_error ("Set ColourMapType to Duochrome.");
            }
            return this->duochrome (_datum1, _datum2);
        }

        //! An overload of convert for TriChrome ColourMaps
        std::array<float, 3> convert (T _datum1, T _datum2, T _datum3)
        {
            if (this->type != ColourMapType::Trichrome) {
                throw std::runtime_error ("Set ColourMapType to Trichrome.");
            }
            return this->trichrome (_datum1, _datum2, _datum3);
        }

        //! Convert the scalar datum into an RGB (or BGR) colour
        std::array<float, 3> convert (T _datum)
        {
            float datum = 0.0f;

            // Convert T into a suitable value (with a suitable scaling as necessary) to
            // make the conversion to array<float,3> colour.
            if constexpr (std::is_same<std::decay_t<T>, double>::value == true) {
                // Copy, enforce range
                datum = _datum > T{1} ? 1.0f : static_cast<float>(_datum);
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
                datum = datum > 1.0f ? 1.0f : datum;

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
                if (datum != T{0}) {
                    c = ColourMap::rainbow (datum);
                }
                break;
            }
            case ColourMapType::RainbowZeroWhite:
            {
                if (datum != T{0}) {
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
                c = this->greyscale (T{1}-datum);
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

        //! Getter for type, the ColourMapType of this ColourMap.
        ColourMapType getType() const { return this->type; }

        //! Setter for type, the ColourMapType of this ColourMap.
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

        //! Set Duochrome to be Red-blue
        void setHueRB()
        {
            if (this->type != ColourMapType::Duochrome) {
                throw std::runtime_error ("red-blue colour map hues only makes sense for ColourMapType::Duochrome");
            }
            this->hue = 0.0f;
            this->hue2 = 0.6667f;
        }
        // Set Duochrome to be Blue-red
        void setHueBR()
        {
            if (this->type != ColourMapType::Duochrome) {
                throw std::runtime_error ("blue-red colour map hues only makes sense for ColourMapType::Duochrome");
            }
            this->hue = 0.6667f;
            this->hue2 = 0.0f;
        }

        //! Set Duochrome to be Green-Blue
        void setHueGB()
        {
            if (this->type != ColourMapType::Duochrome) {
                throw std::runtime_error ("green-blue colour map hues only makes sense for ColourMapType::Duochrome");
            }
            this->hue = 0.3333f;
            this->hue2 = 0.6667f;
        }
        //! Set Duochrome to be Blue-Green
        void setHueBG()
        {
            if (this->type != ColourMapType::Duochrome) {
                throw std::runtime_error ("blue-green colour map hues only makes sense for ColourMapType::Duochrome");
            }
            this->hue = 0.66667f;
            this->hue2 = 0.3333f;
        }

        //! Set Duochrome to be Red-Green
        void setHueRG()
        {
            if (this->type != ColourMapType::Duochrome) {
                throw std::runtime_error ("red-green colour map hues only makes sense for ColourMapType::Duochrome");
            }
            this->hue = 0.0f;
            this->hue2 = 0.3333f;
        }
        //! Set Duochrome to be Green-Red
        void setHueGR()
        {
            if (this->type != ColourMapType::Duochrome) {
                throw std::runtime_error ("green-red colour map hues only makes sense for ColourMapType::Duochrome");
            }
            this->hue = 0.33333f;
            this->hue2 = 0.0f;
        }

        //! Set up a Cyan-Magenta Duochrome colour scheme
        void setHueCM()
        {
            if (this->type != ColourMapType::Duochrome) {
                throw std::runtime_error ("cyan-magenta colour map hues only makes sense for ColourMapType::Duochrome");
            }
            this->hue = 0.5f;
            this->hue2 = 0.8333f;
        }
        //! Set up a Magenta-Cyan Duochrome colour scheme
        void setHueMC()
        {
            if (this->type != ColourMapType::Duochrome) {
                throw std::runtime_error ("magenta-cyan colour map hues only makes sense for ColourMapType::Duochrome");
            }
            this->hue = 0.83333f;
            this->hue2 = 0.5f;
        }

        //! Set a ColourMapType::Duochrome map using h as the first hue and h+0.3333 as the second hue
        void setDualHue(const float& h)
        {
            if (this->type != ColourMapType::Duochrome) {
                throw std::runtime_error ("Dual-hue colour map hues only makes sense for ColourMapType::Duochrome");
            }
            this->hue = h;
            this->hue2 = h+0.3333f;
            if (hue2 > 1.0f) { hue2 -= 1.0f; }
        }
        //! Set a ColourMapType::DuoChrome map using h as the first hue and h-0.3333 as the second hue
        void setDualAntiHue(const float& h)
        {
            if (this->type != ColourMapType::Duochrome) {
                throw std::runtime_error ("Dual-hue colour map hues only makes sense for ColourMapType::Duochrome");
            }
            this->hue = h;
            this->hue2 = h-0.3333f;
            if (hue2 < 0.0f) { hue2 += 1.0f; }
        }

        //! Set the hue... unless you can't/shouldn't
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
            case ColourMapType::Duochrome:
            {
                // Special case duochrome - take first hue, and set second hue to be orthogonal
                this->hue = h;
                this->hue2 = h + 0.3333f;
                if (this->hue2 > 1.0f) { this->hue2 -= 1.0f; }
                break;
            }
            case ColourMapType::Trichrome:
            {
                this->hue = h;
                this->hue2 = h + 0.3333f;
                if (this->hue2 > 1.0f) { this->hue2 -= 1.0f; }
                this->hue3 = h + 0.6667f;
                if (this->hue3 > 1.0f) { this->hue3 -= 1.0f; }
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

        //! Set just the colour's value (ColourMapType::Fixed only)
        void setVal (const float& _v)
        {
            if (this->type != ColourMapType::Fixed) {
                throw std::runtime_error ("Only ColourMapType::Fixed allows setting of value");
            }
            this->val = _v;
        }

        //! Set the colour by hue, saturation and value (ColourMapType::Fixed only)
        void setHSV (const float& h, const float& s, const float& v)
        {
            if (this->type != ColourMapType::Fixed) {
                throw std::runtime_error ("Only ColourMapType::Fixed allows setting of saturation/value");
            }
            this->hue = h;
            this->sat = s;
            this->val = v;
        }

        //! Set the colour by hue, saturation and value (defined in an array) (ColourMapType::Fixed only)
        void setHSV (const std::array<float,3> hsv) { this->setHSV (hsv[0],hsv[1],hsv[2]); }

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
         * @param datum1 gray value from 0.0 to 1.0
         * @param datum2 gray value from 0.0 to 1.0
         *
         * @returns RGB value in a dual-colour map, with colour this->hue and this->hue2;
         */
        std::array<float,3> duochrome (float datum1, float datum2)
        {
            std::array<float,3> clr1 = ColourMap::hsv2rgb (this->hue, datum1, datum1);
            std::array<float,3> clr2 = ColourMap::hsv2rgb (this->hue2, datum2, datum2);
            clr1[0] += clr2[0];
            clr1[1] += clr2[1];
            clr1[2] += clr2[2];
            clr1[0] = clr1[0] > 1.0f ? 1.0f : clr1[0];
            clr1[1] = clr1[1] > 1.0f ? 1.0f : clr1[1];
            clr1[2] = clr1[2] > 1.0f ? 1.0f : clr1[2];
            return clr1;
        }

        /*!
         * @param datum1 gray value from 0.0 to 1.0
         * @param datum2 gray value from 0.0 to 1.0
         * @param datum3 gray value from 0.0 to 1.0
         *
         * @returns RGB value in a tri-colour map, with colour this->hue, this->hue2
         * and this->hue3.
         */
        std::array<float,3> trichrome (float datum1, float datum2, float datum3)
        {
            std::array<float,3> clr1 = ColourMap::hsv2rgb (this->hue, datum1, datum1);
            std::array<float,3> clr2 = ColourMap::hsv2rgb (this->hue2, datum2, datum2);
            std::array<float,3> clr3 = ColourMap::hsv2rgb (this->hue3, datum3, datum3);
            clr1[0] += clr2[0] + clr3[0];
            clr1[1] += clr2[1] + clr3[1];
            clr1[2] += clr2[2] + clr3[2];
            return clr1;
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
