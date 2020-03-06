#pragma once

namespace morph {

    //! Different colour maps types.
    enum class ColourMapType {
        Jet,
        Rainbow,
        Monochrome,
        MonochromeRed,
        MonochromeBlue,
        MonochromeGreen
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
            case ColourMapType::Monochrome:
            case ColourMapType::MonochromeRed:
            case ColourMapType::MonochromeBlue:
            case ColourMapType::MonochromeGreen:
            {
                c = this->monochrome (datum);
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
                this->hue = 0.0f;
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

        //! Format of colours
        ColourOrder order = ColourOrder::RGB;

        //! The hue (range 0 to 1.0f) as used in HSV colour values.
        float hue = 0.0;

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
            array<float,3> col;
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
         * A colour map which is a rainbow through the colour space, varying the hue.
         */
        array<float,3> rainbow (Flt datum) {
            return ColourMap::hsv2rgb ((float)datum, 1.0f, 1.0f);
        }
    };

} // namespace morph
