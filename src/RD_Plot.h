#ifndef _RD_PLOT_H_
#define _RD_PLOT_H_

#include "morph/display.h"
#include "morph/HexGrid.h"
#include <iostream>
#include <vector>
#include <array>
#include <list>

using std::vector;
using std::array;
using std::list;
using morph::HexGrid;
using morph::Gdisplay;

namespace morph {

    /*!
     * A helper class for the plotting of hex grids. The template type Flt
     * is the floating point type predominantly used in the computation
     * class with which this code will interface. Note that float and
     * double may also be used directly, where they relate to the
     * morph/display.h code.
     */
    template <class Flt>
    class RD_Plot
    {
    public:
        /*!
         * Constructors ensure fix, eye and rot are set up.
         */
        //@{
        RD_Plot (void) {}

        RD_Plot (double f, double e, double r) {
            this->fix = {3, f};
            this->eye = {3, e};
            this->rot = {3, r};
        }

        RD_Plot (vector<double>& f, vector<double>& e, vector<double>& r) {
            this->fix = f;
            this->eye = e;
            this->rot = r;
        }
        //@}

        //! Set to true to use single colours for the scalar fields.
        bool scalarFieldsSingleColour = false;
        //! A single colour for the hue. Set >= 0.0 to specify the hue.
        double singleColourHue = -1.0;

        /*!
         * Used by plotting functions
         */
        //@{
        alignas(8) vector<double> fix = {3, 0.0};
        alignas(8) vector<double> eye = {3, 0.0};
        alignas(8) vector<double> rot = {3, 0.0};
        //@}

        /*!
         * Plot a single scalar field co-opting the overloaded code below.
         */
        void scalarfields (Gdisplay& disp,
                           HexGrid* hg,
                           vector<Flt>& f,
                           Flt mina = +1e7,
                           Flt maxa = -1e7) {
            vector<vector<Flt> > vf;
            vf.push_back (f);
            this->scalarfields (disp, hg, vf, mina, maxa);
        }

        /*!
         * Take the first element of the array and create a
         * vector<vector<Flt> > to plot
         */
        vector<vector<Flt> > separateVectorField (vector<array<vector<Flt>, 2> >& f,
                                                  unsigned int arrayIdx) {
            vector<vector<Flt> > vf;
            for (array<vector<Flt>, 2> fia : f) {
                vector<Flt> tmpv = fia[arrayIdx];
                vf.push_back (tmpv);
            }
            return vf;
        }

        /*!
         * On Gdisplay disp, plot all of the scalar fields stored in f on
         * the HexGrid hg. These are plotted in a row; it's up to the
         * programmer to make the window large enough when instantiating
         * the Gdisplay.
         *
         * Optionally pass in a min and a max to help scale the gradients
         *
         * @overallOffset can be optonally set to shift the fields in the horizontal
         * axis.
         */
        void scalarfields (Gdisplay& disp,
                           HexGrid* hg,
                           vector<vector<Flt> >& f,
                           Flt mina = +1e7, Flt maxa = -1e7, Flt overallOffset = 0.0) {

            disp.resetDisplay (this->fix, this->eye, this->rot);

            unsigned int N = f.size();
            unsigned int nhex = hg->num();

            // Determines min and max
            for (unsigned int hi=0; hi<nhex; ++hi) {
                Hex* h = hg->vhexen[hi];
                if (h->onBoundary() == false) {
                    for (unsigned int i = 0; i<N; ++i) {
                        if (f[i][h->vi]>maxa) { maxa = f[i][h->vi]; }
                        if (f[i][h->vi]<mina) { mina = f[i][h->vi]; }
                    }
                }
            }

            Flt scalea = 1.0 / (maxa-mina);

            // Determine a colour from min, max and current value
            vector<vector<Flt> > norm_a;
            norm_a.resize (N);
            for (unsigned int i=0; i<N; ++i) {
                norm_a[i].resize (nhex, 0.0);
            }
            for (unsigned int i = 0; i<N; ++i) {
                for (unsigned int h=0; h<nhex; h++) {
                    norm_a[i][h] = fmin (fmax (((f[i][h]) - mina) * scalea, 0.0), 1.0);
                }
            }

            // Create an offset which we'll increment by the width of the
            // map, starting from the left-most map (f[0])

            float hgwidth = hg->getXmax() - hg->getXmin();

            // Need to correctly apply N/2 depending on whether N is even or odd.
            float w = hgwidth+(hgwidth/20.0f);
            array<float,3> offset = { 0.0f , 0.0f, 0.0f };
            float half_minus_half_N = 0.5f - ((float)N/2.0f) + overallOffset;
            for (unsigned int i = 0; i<N; ++i) {
                offset[0] = (half_minus_half_N + (float)i) * w;
                // Note: OpenGL isn't thread-safe, so no omp parallel for here.
                for (auto h : hg->hexen) {

                    // Colour can be single colour or red through to blue.
                    array<float,3> cl_a = {{0,0,0}};
                    if (this->scalarFieldsSingleColour == true) {
                        if (this->singleColourHue >= 0.0 && this->singleColourHue <= 1.0) {
                            cl_a = morph::Tools::HSVtoRGB (this->singleColourHue, norm_a[i][h.vi], 1.0);
                        } else {
                            cl_a = morph::Tools::HSVtoRGB ((float)i/(float)N, norm_a[i][h.vi], 1.0);
                        }
                    } else {
                        cl_a = morph::Tools::getJetColorF (norm_a[i][h.vi]);
                    }
                    disp.drawHex (h.position(), offset, (h.d/2.0f), cl_a);
                }
            }
            disp.redrawDisplay();
        }

        void scalarfields_noreset (Gdisplay& disp,
                                   HexGrid* hg,
                                   vector<vector<Flt> >& f,
                                   Flt mina = +1e7, Flt maxa = -1e7, Flt overallOffset = 0.0) {

            unsigned int N = f.size();
            unsigned int nhex = hg->num();

            // Determines min and max
            for (unsigned int hi=0; hi<nhex; ++hi) {
                Hex* h = hg->vhexen[hi];
                if (h->onBoundary() == false) {
                    for (unsigned int i = 0; i<N; ++i) {
                        if (f[i][h->vi]>maxa) { maxa = f[i][h->vi]; }
                        if (f[i][h->vi]<mina) { mina = f[i][h->vi]; }
                    }
                }
            }

            Flt scalea = 1.0 / (maxa-mina);

            // Determine a colour from min, max and current value
            vector<vector<Flt> > norm_a;
            norm_a.resize (N);
            for (unsigned int i=0; i<N; ++i) {
                norm_a[i].resize (nhex, 0.0);
            }
            for (unsigned int i = 0; i<N; ++i) {
                for (unsigned int h=0; h<nhex; h++) {
                    norm_a[i][h] = fmin (fmax (((f[i][h]) - mina) * scalea, 0.0), 1.0);
                }
            }

            // Create an offset which we'll increment by the width of the
            // map, starting from the left-most map (f[0])

            float hgwidth = hg->getXmax() - hg->getXmin();

            // Need to correctly apply N/2 depending on whether N is even or odd.
            float w = hgwidth+(hgwidth/20.0f);
            array<float,3> offset = { 0.0f , 0.0f, 0.0f };
            float half_minus_half_N = 0.5f - ((float)N/2.0f) + overallOffset;
            for (unsigned int i = 0; i<N; ++i) {
                offset[0] = (half_minus_half_N + (float)i) * w;
                // Note: OpenGL isn't thread-safe, so no omp parallel for here.
                for (auto h : hg->hexen) {
                    array<float,3> cl_a = {{0,0,0}};
                    if (this->scalarFieldsSingleColour == true) {
                        if (this->singleColourHue >= 0.0 && this->singleColourHue <= 1.0) {
                            cl_a = morph::Tools::HSVtoRGB (this->singleColourHue, norm_a[i][h.vi], 1.0);
                        } else {
                            cl_a = morph::Tools::HSVtoRGB ((float)i/(float)N, norm_a[i][h.vi], 1.0);
                        }
                    } else {
                        cl_a = morph::Tools::getJetColorF (norm_a[i][h.vi]);
                    }
                    disp.drawHex (h.position(), offset, (h.d/2.0f), cl_a);
                }
            }
        }

        /*!
         * Plot the contour described by contourHexes, with these hexes coloured in.
         */
        void plot_contour (Gdisplay& disp, HexGrid* hg, vector<list<Hex> >& contourHexes) {
            disp.resetDisplay (this->fix, this->eye, this->rot);
            this->add_contour_plot (disp, hg, contourHexes);
            disp.redrawDisplay();
        }

        /*!
         * Plot the contours where the fields f cross threshold. Plot on
         * disp.
         */
        void plot_contour (morph::Gdisplay& disp, HexGrid* hg, vector<vector<Flt> >& f, Flt threshold) {
            disp.resetDisplay (this->fix, this->eye, this->rot);
            this->add_contour_plot (disp, hg, f, threshold);
            disp.redrawDisplay();
        }

        /*!
         * Plot the contour described by contourHexes, with these hexes coloured in
         * AND a scalarfield graph. Next door to each other.
         */
        void plot_contour_and_scalar (Gdisplay& disp, HexGrid* hg, vector<list<Hex> >& contourHexes, vector<Flt>& f) {

            disp.resetDisplay (this->fix, this->eye, this->rot);

            // "Research code" alert! This is rather hacky, coded to work for one
            // window to get a job done.
            Flt shift = 0.65;
            Flt shift_r = 0.2;

            this->add_contour_plot (disp, hg, contourHexes, +shift+shift_r);

            vector<vector<Flt> > vf;
            vf.push_back (f);
            Flt mina = +1e7;
            Flt maxa = -1e7;
            this->scalarfields_noreset (disp, hg, vf, mina, maxa, -shift+shift_r);

            disp.redrawDisplay();
        }

        /*!
         * Add a contour plot to the Gdisplay @disp for HexGrid hg. The
         * contourHexes are provided in contourHexes.
         */
        void add_contour_plot (morph::Gdisplay& disp, HexGrid* hg, vector<list<Hex> >& contourHexes, Flt overallOffset = 0.0) {

            unsigned int N = contourHexes.size();

            array<float,3> offset_ar = {0.0f, 0.0f, 0.0f};
            offset_ar[0] += overallOffset;

            // Coloured boundaries
            float r = hg->hexen.begin()->getSR();
            for (unsigned int i = 0; i<N; ++i) {
#ifdef _OLD_
                array<float,3> cl_b = morph::Tools::HSVtoRGB ((Flt)i/(Flt)N, 1.0, 1.0);
#else
                array<float,3> cl_b = morph::Tools::getJetColorF ((Flt)i/(Flt)N);
#endif
                for (auto h : contourHexes[i]) {
                    disp.drawHex (h.position(), offset_ar, r, cl_b);
                }
            }

            // Used both for zero_offset and cl_blk
            array<float,3> zero_ar = {0.0f, 0.0f, 0.0f};
            for (auto h : hg->hexen) {
#ifdef DEBUG__ // Show a black hex in a known location
                if (h.ri==-28 && h.gi==-21) {
                    disp.drawHex (h.position(), r, zero_ar);
                }
#endif
                if (h.onBoundary() == true) {
                    if (!h.has_ne()) {
                        disp.drawHexSeg (h.position(), offset_ar, r, zero_ar, 0);
                    }
                    if (!h.has_nne()) {
                        disp.drawHexSeg (h.position(), offset_ar, r, zero_ar, 1);
                    }
                    if (!h.has_nnw()) {
                        disp.drawHexSeg (h.position(), offset_ar, r, zero_ar, 2);
                    }
                    if (!h.has_nw()) {
                        disp.drawHexSeg (h.position(), offset_ar, r, zero_ar, 3);
                    }
                    if (!h.has_nsw()) {
                        disp.drawHexSeg (h.position(), offset_ar, r, zero_ar, 4);
                    }
                    if (!h.has_nse()) {
                        disp.drawHexSeg (h.position(), offset_ar, r, zero_ar, 5);
                    }
                }
            }
        }

        /*!
         * Do the work of adding the contours of the fields f to the
         * display disp.
         */
        void add_contour_plot (Gdisplay& disp, HexGrid* hg, vector<vector<Flt> >& f, Flt threshold) {

            unsigned int N = f.size();
            unsigned int nhex = hg->num();

            // Copies data to plot out of the model
            vector<Flt> maxf (N, -1e7);
            vector<Flt> minf (N, +1e7);

            // Determines min and max
            for (auto h : hg->hexen) {
                if (h.onBoundary() == false) {
                    for (unsigned int i = 0; i<N; ++i) {
                        if (f[i][h.vi] > maxf[i]) { maxf[i] = f[i][h.vi]; }
                        if (f[i][h.vi] < minf[i]) { minf[i] = f[i][h.vi]; }
                    }
                }
            }

            vector<Flt> scalef (5, 0);
            for (unsigned int i = 0; i<N; ++i) {
                scalef[i] = 1.0 / (maxf[i]-minf[i]);
            }

            // Re-normalize
            vector<vector<Flt> > norm_f;
            norm_f.resize (N);
            for (unsigned int i=0; i<N; ++i) {
                norm_f[i].resize (nhex, 0.0);
            }
            for (unsigned int i = 0; i<N; ++i) {
                for (unsigned int h=0; h<nhex; h++) {
                    norm_f[i][h] = fmin (fmax (((f[i][h]) - minf[i]) * scalef[i], 0.0), 1.0);
                }
            }

            // Draw
            array<float,3> cl_blk = {0.0f, 0.0f, 0.0f};
            array<float,3> zero_offset = {0.0f, 0.0f, 0.0f};

            for (unsigned int i = 0; i<N; ++i) {
#ifdef _OLD_
                array<float,3> cl_b = morph::Tools::HSVtoRGB ((Flt)i/(Flt)N, 1.0, 1.0);
#else
                array<float,3> cl_b = morph::Tools::getJetColorF ((Flt)i/(Flt)N);
#endif
                for (auto h : hg->hexen) {
                    if (h.onBoundary() == false) {
                        if (norm_f[i][h.vi]<threshold) {
                            if (h.has_ne() && norm_f[i][h.ne->vi] > threshold) {
                                disp.drawHexSeg (h.position(), zero_offset, (h.d/2.0f), cl_b, 0);
                            }
                            if (h.has_nne() && norm_f[i][h.nne->vi] > threshold) {
                                disp.drawHexSeg (h.position(), zero_offset, (h.d/2.0f), cl_b, 1);
                            }
                            if (h.has_nnw() && norm_f[i][h.nnw->vi] > threshold) {
                                disp.drawHexSeg (h.position(), zero_offset, (h.d/2.0f), cl_b, 2);
                            }
                            if (h.has_nw() && norm_f[i][h.nw->vi] > threshold) {
                                disp.drawHexSeg (h.position(), zero_offset, (h.d/2.0f), cl_b, 3);
                            }
                            if (h.has_nsw() && norm_f[i][h.nsw->vi] > threshold) {
                                disp.drawHexSeg (h.position(), zero_offset, (h.d/2.0f), cl_b, 4);
                            }
                            if (h.has_nse() && norm_f[i][h.nse->vi] > threshold) {
                                disp.drawHexSeg (h.position(), zero_offset, (h.d/2.0f), cl_b, 5);
                            }
                        }

                    } else { // h.onBoundary() is true

                        if (!h.has_ne()) {
                            disp.drawHexSeg (h.position(), zero_offset, (h.d/2.0f), cl_blk, 0);
                        }
                        if (!h.has_nne()) {
                            disp.drawHexSeg (h.position(), zero_offset, (h.d/2.0f), cl_blk, 1);
                        }
                        if (!h.has_nnw()) {
                            disp.drawHexSeg (h.position(), zero_offset, (h.d/2.0f), cl_blk, 2);
                        }
                        if (!h.has_nw()) {
                            disp.drawHexSeg (h.position(), zero_offset, (h.d/2.0f), cl_blk, 3);
                        }
                        if (!h.has_nsw()) {
                            disp.drawHexSeg (h.position(), zero_offset, (h.d/2.0f), cl_blk, 4);
                        }
                        if (!h.has_nse()) {
                            disp.drawHexSeg (h.position(), zero_offset, (h.d/2.0f), cl_blk, 5);
                        }
                    }
                }
            }
        }

        /*!
         * Save PNG images
         */
        void savePngs (const string& logpath, const string& name,
                       unsigned int frameN, Gdisplay& disp) {
            stringstream ff1;
            ff1 << logpath << "/" << name<< "_";
            ff1 << std::setw(5) << std::setfill('0') << frameN;
            ff1 << ".png";
            disp.saveImage (ff1.str());
        }

    };

} // namespace morph

#endif // _RD_PLOT_H_
