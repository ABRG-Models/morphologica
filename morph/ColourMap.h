#pragma once

#include "ColourMap_Lists.h"

#include <stdexcept>
using std::runtime_error;

#include <cmath>
using std::round;
using std::abs;

namespace morph {

    //! Different colour maps types.
    enum class ColourMapType {
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
        Greyscale,  // Greyscale is any hue; saturation=0; *value* varies.
        Monochrome, // Monochrome is 'monohue': fixed hue; vary the *saturation* with value fixed at 1.
        MonochromeRed,
        MonochromeBlue,
        MonochromeGreen,
        Fixed       // Fixed colour. Should return same colour for any datum. User must set hue, sat, val.
    };

    //! Different ways of specifying colour exist
    enum class ColourOrder {
        RGB,
        BGR
        //RGBA?
        //BGRA?
    };

    template <class Flt>
    class ColourMap {
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
        //! Convert the scalar datum into an RGB (or BGR) colour
        array<float, 3> convert (Flt datum) {
            array<float, 3> c = {0.0f, 0.0f, 0.0f};
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
                if (datum != static_cast<Flt>(0.0)) {
                    c = ColourMap::rainbow (datum);
                }
                break;
            }
            case ColourMapType::RainbowZeroWhite:
            {
                if (datum != static_cast<Flt>(0.0)) {
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
        //array<float, 4> convertAlpha (Flt datum);

        // Set the colour map type.
        void setType (const ColourMapType& tp) {
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
        void setHue (const float& h) {
            switch (this->type) {
            case ColourMapType::MonochromeRed:
            case ColourMapType::MonochromeBlue:
            case ColourMapType::MonochromeGreen:
            {
                throw runtime_error ("This colour map does not accept changes to the hue");
                break;
            }
            default:
            {
                this->hue = h;
                break;
            }
            }
        }

        void setHSV (const float& h, const float& s, const float& v) {
            if (this->type != ColourMapType::Fixed) {
                throw runtime_error ("Only ColourMapType::Fixed allows setting of saturation/value");
            }
            this->hue = h;
            this->sat = s;
            this->val = v;
        }

        void setHSV (const array<float,3> hsv) {
            this->setHSV (hsv[0],hsv[1],hsv[2]);
        }

#if 0
        void setRGB (const float& r, const float& g, const float& b) {
            if (this->type != ColourMapType::Fixed) {
                throw runtime_error ("Only ColourMapType::Fixed allows setting of RGB");
            }
            // Set hue, sat, val by converting from RGB.
        }
#endif

        //! Get the hue, in its most saturated form
        array<float, 3> getHueRGB (void) {
            return ColourMap::hsv2rgb (this->hue, 1.0f, 1.0f);
        }

        //! Format of colours
        ColourOrder order = ColourOrder::RGB;

        /*!
         * @param datum gray value from 0.0 to 1.0
         *
         * @returns RGB value in jet colormap
         */
        static array<float,3> jetcolour (Flt datum) {
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
            array<float,3> col = {0.0f, 0.0f, 0.0f};
            float ivl = 1.0/8.0;
            for (int i=0; i<8; i++) {
                Flt llim = (i==0) ? static_cast<Flt>(0.0) : (Flt)i/static_cast<Flt>(8.0);
                Flt ulim = (i==7) ? static_cast<Flt>(1.0) : ((Flt)(i+1))/static_cast<Flt>(8.0);
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

        /*!
         * Convert hue, saturation, value to RGB. single precision arguments.
         */
        static array<float,3> hsv2rgb (float h, float s, float v) {
            array<float, 3> rgb = {0.0f, 0.0f, 0.0f};
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
        array<float,3> monochrome (Flt datum) {
            return ColourMap::hsv2rgb (this->hue, static_cast<float>(datum), 1.0f);
        }

        /*!
         * @param datum gray value from 0.0 to 1.0
         *
         * @returns Generate RGB value for which all entries are equal and the
         * brightness gives the map value.
         */
        array<float,3> greyscale (Flt datum) {
            return ColourMap::hsv2rgb (this->hue, 0.0f, static_cast<float>(datum));
            // or
            //return {datum, datum, datum}; // assuming 0 <= datum <= 1
        }

        /*!
         * A colour map which is a rainbow through the colour space, varying the hue.
         */
        array<float,3> rainbow (Flt datum) {
            return ColourMap::hsv2rgb ((float)datum, 1.0f, 1.0f);
        }

        /*!
         * A copy of matplotlib's magma colourmap
         */
        array<float,3> magma (Flt datum) {
            // let's just try the closest colour from the map, with no interpolation
            size_t datum_i = static_cast<size_t>(abs (round (datum * (Flt)(morph::cm_magma_len-1))));
            array<float,3> c = {morph::cm_magma[datum_i][0], morph::cm_magma[datum_i][1], morph::cm_magma[datum_i][2]};
            return c;
        }

        /*!
         * A copy of matplotlib's inferno colourmap
         */
        array<float,3> inferno (Flt datum) {
            // let's just try the closest colour from the map, with no interpolation
            size_t datum_i = static_cast<size_t>(abs (round (datum * (Flt)(morph::cm_inferno_len-1))));
            array<float,3> c = {morph::cm_inferno[datum_i][0], morph::cm_inferno[datum_i][1], morph::cm_inferno[datum_i][2]};
            return c;
        }

        /*!
         * A copy of matplotlib's plasma colourmap
         */
        array<float,3> plasma (Flt datum) {
            // let's just try the closest colour from the map, with no interpolation
            size_t datum_i = static_cast<size_t>(abs (round (datum * (Flt)(morph::cm_plasma_len-1))));
            array<float,3> c = {morph::cm_plasma[datum_i][0], morph::cm_plasma[datum_i][1], morph::cm_plasma[datum_i][2]};
            return c;
        }

        /*!
         * A copy of matplotlib's viridis colourmap
         */
        array<float,3> viridis (Flt datum) {
            // let's just try the closest colour from the map, with no interpolation
            size_t datum_i = static_cast<size_t>(abs (round (datum * (Flt)(morph::cm_viridis_len-1))));
            array<float,3> c = {morph::cm_viridis[datum_i][0], morph::cm_viridis[datum_i][1], morph::cm_viridis[datum_i][2]};
            return c;
        }

        /*!
         * A copy of matplotlib's cividis colourmap
         */
        array<float,3> cividis (Flt datum) {
            // let's just try the closest colour from the map, with no interpolation
            size_t datum_i = static_cast<size_t>(abs (round (datum * (Flt)(morph::cm_cividis_len-1))));
            array<float,3> c = {morph::cm_cividis[datum_i][0], morph::cm_cividis[datum_i][1], morph::cm_cividis[datum_i][2]};
            return c;
        }

        /*!
         * A copy of matplotlib's twilight colourmap
         */
        array<float,3> twilight (Flt datum) {
            // let's just try the closest colour from the map, with no interpolation
            size_t datum_i = static_cast<size_t>(abs (round (datum * (Flt)(morph::cm_twilight_len-1))));
            array<float,3> c = {morph::cm_twilight[datum_i][0], morph::cm_twilight[datum_i][1], morph::cm_twilight[datum_i][2]};
            return c;
        }
    };

} // namespace morph
