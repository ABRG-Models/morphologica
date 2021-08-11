#pragma once

#include <morph/tools.h>
#include <morph/Random.h>
#include <morph/ReadCurves.h>
#include <morph/HexGrid.h>
#include <morph/HdfData.h>
#include <sstream>
#include <vector>
#include <array>
#include <iomanip>
#include <cmath>
#include <hdf5.h>
#include <unistd.h>
#include <morph/MorphDbg.h>

/*!
 * Macros for testing neighbours. The step along for neighbours on the
 * rows above/below is given by:
 *
 * Dest  | step
 * ----------------------
 * NNE   | +rowlen
 * NNW   | +rowlen - 1
 * NSW   | -rowlen
 * NSE   | -rowlen + 1
 */
//@{
#define NE(hi) (this->hg->d_ne[hi])
#define HAS_NE(hi) (this->hg->d_ne[hi] == -1 ? false : true)

#define NW(hi) (this->hg->d_nw[hi])
#define HAS_NW(hi) (this->hg->d_nw[hi] == -1 ? false : true)

#define NNE(hi) (this->hg->d_nne[hi])
#define HAS_NNE(hi) (this->hg->d_nne[hi] == -1 ? false : true)

#define NNW(hi) (this->hg->d_nnw[hi])
#define HAS_NNW(hi) (this->hg->d_nnw[hi] == -1 ? false : true)

#define NSE(hi) (this->hg->d_nse[hi])
#define HAS_NSE(hi) (this->hg->d_nse[hi] == -1 ? false : true)

#define NSW(hi) (this->hg->d_nsw[hi])
#define HAS_NSW(hi) (this->hg->d_nsw[hi] == -1 ? false : true)
//@}

#define IF_HAS_NE(hi, yesval, noval)  (HAS_NE(hi)  ? yesval : noval)
#define IF_HAS_NNE(hi, yesval, noval) (HAS_NNE(hi) ? yesval : noval)
#define IF_HAS_NNW(hi, yesval, noval) (HAS_NNW(hi) ? yesval : noval)
#define IF_HAS_NW(hi, yesval, noval)  (HAS_NW(hi)  ? yesval : noval)
#define IF_HAS_NSW(hi, yesval, noval) (HAS_NSW(hi) ? yesval : noval)
#define IF_HAS_NSE(hi, yesval, noval) (HAS_NSE(hi) ? yesval : noval)

namespace morph {

    /*!
     * Base class for RD systems
     */
    template <typename Flt>
    class RD_Base
    {
    public:

        /*!
         * Constants
         */
        //@{
        //! Square root of 3 over 2
        const Flt R3_OVER_2 = 0.866025403784439;
        //! Square root of 3
        const Flt ROOT3 = 1.73205080756888;
        //! 2 pi divided by 360 - i.e. degrees to radians
        const Flt TWOPI_OVER_360 = 0.01745329251994;
        //! Passed to HdfData constructor to say we want to read the data
        const bool READ_DATA = true;
        //@}

        /*!
         * Hex to hex d for the grid. Make smaller to increase the number
         * of Hexes being computed.
         */
        alignas(float) float hextohex_d = 0.01;

        /*!
         * The 'span' of the initial hex of hexes that is created, onto which the boundary is then drawn.
         */
        alignas(float) float hexspan = 4;

        /*!
         * Holds the number of hexes in the populated HexGrid
         */
        alignas(Flt) unsigned int nhex = 0;

        /*!
         * Over what length scale should some values fall off to zero
         * towards the boundary? Used in a couple of different locations.
         */
        alignas(Flt) Flt boundaryFalloffDist = 0.02; // 0.02 default

    protected:
        /*!
         * Our choice of dt.
         */
        alignas(Flt) Flt dt = 0.00001;

        /*!
         * Compute half and sixth dt in constructor.
         */
        //@{
        alignas(Flt) Flt halfdt = 0.0;
        alignas(Flt) Flt sixthdt = 0.0;
        //@}

        /*!
         * Hex to hex distance. Populate this from hg.d after hg has been
         * initialised.
         */
        alignas(Flt) Flt d = 1.0;
        alignas(Flt) Flt v = 1;

        /*!
         * Parameters that depend on d and v:
         */
        //@{
        alignas(Flt) Flt oneoverd = 1.0/this->d;
        alignas(Flt) Flt oneover2d = 1.0/(this->d+this->d);
        alignas(Flt) Flt oneover3d = 1.0/(3*this->d);
        alignas(Flt) Flt oneover3dd = 1.0 / 3*this->d*this->d;
        alignas(Flt) Flt twoover3dd = 2.0 / 3*this->d*this->d;

        alignas(Flt) Flt oneoverv = 1.0/this->v;
        alignas(Flt) Flt twov = this->v+this->v;
        alignas(Flt) Flt oneover2v = 1.0/this->twov;
        alignas(Flt) Flt oneover4v = 1.0/(this->twov+this->twov);
        //@}

    public:

        /*!
         * Track the number of computational steps that we've carried
         * out. Only to show a message saying "100 steps done...", but
         * that's reason enough.
         */
        alignas(Flt) unsigned int stepCount = 0;

        /*!
         * ALIGNAS REGION ENDS.
         *
         * Below here, there's no need to worry about alignas keywords.
         */

        /*!
         * Hold on to the ReadCurves object, so that the additional contours are available.
         */
        ReadCurves r;

        /*!
         * The HexGrid "background" for the Reaction Diffusion system.
         */
        HexGrid* hg;

        /*!
         * The logpath for this model. Used when saving data out.
         */
        std::string logpath = "logs";

        /*!
         * Setter which attempts to ensure the path exists.
         */
        void setLogpath (const std::string p) {
            this->logpath = p;
            // Ensure log directory exists
            morph::Tools::createDir (this->logpath);
        }

        /*!
         * Make the svgpath something that can be set by client code. If empty, then
         * set an elliptical boundary
         */
        std::string svgpath = "./trial.svg";

        /*!
         * Ellipse a and b values. Used to specify a boundary only if svgpath is empty.
         */
        //@{
        float ellipse_a = 1.0f;
        float ellipse_b = 1.0f;
        //@}

        /*!
         * Simple constructor; no arguments.
         */
        RD_Base (void) {
            this->halfdt = this->dt/2.0;
            this->sixthdt = this->dt/6.0;
        }

        /*!
         * Destructor required to free up HexGrid memory
         */
        ~RD_Base (void) {
            delete (this->hg);
        }

        /*!
         * Utility functions to resize/zero vector-vectors that hold N
         * different RD variables.
         */
        //@{
        void resize_vector_vector (std::vector<std::vector<Flt> >& vv, unsigned int N) {
            vv.resize (N);
            for (unsigned int i=0; i<N; ++i) {
                vv[i].resize (this->nhex, 0.0);
            }
        }
        void zero_vector_vector (std::vector<std::vector<Flt> >& vv, unsigned int N) {
            for (unsigned int i=0; i<N; ++i) {
                vv[i].assign (this->nhex, 0.0);
            }
        }
        void set_vector_vector (std::vector<std::vector<Flt> >& vv, unsigned int N, Flt value) {
            for (unsigned int i=0; i<N; ++i) {
                vv[i].assign (this->nhex, value);
            }
        }
        void resize_vector_vector (std::vector<std::vector<Flt> >& vv, unsigned int N, unsigned int M) {
            vv.resize (N);
            for (unsigned int i=0; i<N; ++i) {
                vv[i].resize (M, 0.0);
            }
        }
        void zero_vector_vector (std::vector<std::vector<Flt> >& vv, unsigned int N, unsigned int M) {
            for (unsigned int i=0; i<N; ++i) {
                vv[i].assign (M, 0.0);
            }
        }
        void set_vector_vector (std::vector<std::vector<Flt> >& vv, unsigned int N, unsigned int M, Flt value) {
            for (unsigned int i=0; i<N; ++i) {
                vv[i].assign (M, value);
            }
        }
        //@}

        /*!
         * Resize/zero M vectors of N vectors of nhex Flts.
         */
        //@{
        void resize_vector_vector_vector (std::vector<std::vector<std::vector<Flt> > >& vvv, unsigned int N, unsigned int M) {
            vvv.resize (M);
            for (unsigned int m=0; m<M; ++m) {
                vvv[m].resize (N);
                for (unsigned int i=0; i<N; ++i) {
                    vvv[m][i].resize (this->nhex, 0.0);
                }
            }
        }
        void zero_vector_vector_vector (std::vector<std::vector<std::vector<Flt> > >& vvv, unsigned int N, unsigned int M) {
            for (unsigned int m=0; m<M; ++m) {
                for (unsigned int i=0; i<N; ++i) {
                    vvv[m][i].assign (M, 0.0);
                }
            }
        }
        //@}

        /*!
         * Resize/zero a variable that'll be nhex elements long
         */
        //@{
        void resize_vector_variable (std::vector<Flt>& v) {
            v.resize (this->nhex, 0.0);
        }
        void zero_vector_variable (std::vector<Flt>& v) {
            v.assign (this->nhex, 0.0);
        }
        //@}

        /*!
         * Resize/zero a parameter that'll be N elements long
         */
        //@{
        void resize_vector_param (std::vector<Flt>& p, unsigned int N) {
            p.resize (N, 0.0);
        }
        void zero_vector_param (std::vector<Flt>& p, unsigned int N) {
            p.assign (N, 0.0);
        }
        //@}

        /*!
         * Resize/zero a vector of M vectors of parameters that'll each be N
         * elements long
         */
        //@{
        void resize_vector_vector_param (std::vector<std::vector<Flt> >& vp, unsigned int N, unsigned int M) {
            vp.resize (M);
            for (unsigned int m = 0; m<M; ++m) {
                vp[m].resize (N, 0.0);
            }
        }
        void zero_vector_vector_param (std::vector<std::vector<Flt> >& vp, unsigned int N, unsigned int M) {
            for (unsigned int m = 0; m<M; ++m) {
                vp[m].assign (N, 0.0);
            }
        }
        //@}

        /*!
         * Resize/zero a gradient field
         */
        //@{
        void resize_gradient_field (std::array<std::vector<Flt>, 2>& g) {
            g[0].resize (this->nhex, 0.0);
            g[1].resize (this->nhex, 0.0);
        }
        void zero_gradient_field (std::array<std::vector<Flt>, 2>& g) {
            g[0].assign (this->nhex, 0.0);
            g[1].assign (this->nhex, 0.0);
        }
        //@}

        /*!
         * Resize/zero a vector size N containing arrays of two vector<Flt>s
         * which are the x and y components of a (mathematical) vector
         * field.
         */
        //@{
        void resize_vector_array_vector (std::vector<std::array<std::vector<Flt>, 2> >& vav, unsigned int N) {
            vav.resize (N);
            for (unsigned int n = 0; n<N; ++n) {
                this->resize_gradient_field (vav[n]);
            }
        }
        void zero_vector_array_vector (std::vector<std::array<std::vector<Flt>, 2> >& vav, unsigned int N) {
            for (unsigned int i = 0; i<N; ++i) {
                this->zero_gradient_field (vav[i]);
            }
        }
        //@}

        /*!
         * Resize/zero a vector M containing vectors of size N containing arrays of
         * two vector<Flt>s which are the x and y components of a (mathematical)
         * vector field.
         */
        //@{
        void resize_vector_vector_array_vector (std::vector<std::vector<std::array<std::vector<Flt>, 2> > >& vvav, unsigned int N, unsigned int M) {
            vvav.resize (M);
            for (unsigned int j = 0; j<M; ++j) {
                vvav[j].resize (N);
                for (unsigned int n = 0; n<N; ++n) {
                    this->resize_gradient_field (vvav[j][n]);
                }
            }
        }
        void zero_vector_vector_array_vector (std::vector<std::vector<std::array<std::vector<Flt>, 2> > >& vvav, unsigned int N, unsigned int M) {
            for (unsigned int j = 0; j<M; ++j) {
                for (unsigned int i = 0; i<N; ++i) {
                    this->zero_gradient_field (vvav[j][i]);
                }
            }
        }
        //@}

        /*!
         * Initialise a vector with noise, but with sigmoidal roll-off to
         * zero at the boundary.
         *
         * I apply a sigmoid to the boundary hexes, so that the noise
         * drops away towards the edge of the domain.
         */
        void noiseify_vector_variable (std::vector<Flt>& v, Flt offset, Flt gain) {
            morph::RandUniform<Flt> rng;
            for (auto h : this->hg->hexen) {
                // boundarySigmoid. Jumps sharply (100, larger is
                // sharper) over length scale 0.05 to 1. So if
                // distance from boundary > 0.05, noise has normal
                // value. Close to boundary, noise is less.
                v[h.vi] = rng.get() * gain + offset;
                if (h.distToBoundary > -0.5) { // It's possible that distToBoundary is set to -1.0
                    Flt bSig = 1.0 / ( 1.0 + exp (-100.0*(h.distToBoundary-this->boundaryFalloffDist)) );
                    v[h.vi] = v[h.vi] * bSig;
                }
            }
        }

        /*!
         * Perform memory allocations, vector resizes and so on.
         */
        virtual void allocate (void) {
            // Create a HexGrid. 3 is the 'x span' which determines how
            // many hexes are initially created. 0 is the z co-ordinate for the HexGrid.
            this->hg = new HexGrid (this->hextohex_d, this->hexspan, 0, morph::HexDomainShape::Boundary);
            DBG ("Initial hexagonal HexGrid has " << this->hg->num() << " hexes");

            // Either set a boundary using the svgpath, or set it as an ellipse
            if (this->svgpath != "") {
                // Read the curves which make a boundary
                this->r.init (this->svgpath);
                // Set the boundary in the HexGrid
                this->hg->setBoundary (this->r.getCorticalPath());
            } else {
                this->hg->setEllipticalBoundary (this->ellipse_a, this->ellipse_b);
            }
            // Compute the distances from the boundary
            this->hg->computeDistanceToBoundary();
            // Vector size comes from number of Hexes in the HexGrid
            this->nhex = this->hg->num();
            DBG ("After setting boundary, HexGrid has " << this->nhex << " hexes");
            // Spatial d comes from the HexGrid, too.
            this->set_d(this->hg->getd());
            DBG ("HexGrid says d = " << this->d);
            this->set_v(this->hg->getv());
            DBG ("HexGrid says v = " << this->v);
        }

        /*!
         * Initialise variables and parameters. Carry out one-time
         * computations required of the model.
         */
        virtual void init (void) = 0;

    protected:
        /*!
         * Require private setters for d and v as there are several other
         * members that have to be updated at the same time.
         */
        //@{
        virtual void set_d (Flt d_) {
            this->d = d_;
            this->oneoverd = 1.0/this->d;
            this->oneover2d = 1.0/(2*this->d);
            this->oneover3d = 1.0/(3*this->d);
            this->oneover3dd = 1.0 / (3*this->d*this->d);
            this->twoover3dd = 2.0 / (3*this->d*this->d);
        }

        virtual void set_v (Flt v_) {
            this->v = v_;
            this->oneoverv = 1.0/this->v;
            this->twov = this->v+this->v;
            this->oneover2v = 1.0/this->twov;
            this->oneover4v = 1.0/(this->twov+this->twov);
        }
        //@}

    public:
        /*!
         * Public getters for d and v
         */
        //@{
        Flt get_d (void) {
            return this->d;
        }

        Flt get_v (void) {
            return this->v;
        }
        //@}

        /*!
         * Public accessors for dt
         */
        //@{
        void set_dt (Flt _dt) {
            this->dt = _dt;
            this->halfdt = this->dt/2.0;
            this->sixthdt = this->dt/6.0;
        }
        Flt get_dt (void) {
            return this->dt;
        }
        //@}

    public:

        /*!
         * HDF5 file saving/loading methods.
         */
        //@{
        /*!
         * Save a data frame
         */
        virtual void save (void) { }

        /*!
         * Save position information
         */
        void savePositions (void) {
            std::stringstream fname;
            fname << this->logpath << "/positions.h5";
            HdfData data(fname.str());
            data.add_val ("/area", this->hg->num() * this->hg->getHexArea());
            this->saveHexPositions (data);
        }

        /*!
         * Save positions of the hexes - note using two vector<float>s
         * that have been populated with the positions from the HexGrid,
         * to fit in with the HDF API.
         */
        void saveHexPositions (HdfData& dat) {
            dat.add_contained_vals ("/x", this->hg->d_x);
            dat.add_contained_vals ("/y", this->hg->d_y);

            // Add the neighbour information too.
            std::vector<float> x_ne = this->hg->d_x;
            std::vector<float> y_ne = this->hg->d_y;
            unsigned int count = 0;
            for (int i : this->hg->d_ne) {
                if (i >= 0) {
                    x_ne[count] = this->hg->d_x[i];
                    y_ne[count] = this->hg->d_y[i];
                }
                ++count;
            }
            dat.add_contained_vals ("/x_ne", x_ne);
            dat.add_contained_vals ("/y_ne", y_ne);

            std::vector<float> x_nne = this->hg->d_x;
            std::vector<float> y_nne = this->hg->d_y;
            count = 0;
            for (int i : this->hg->d_nne) {
                if (i >= 0) {
                    x_nne[count] = this->hg->d_x[i];
                    y_nne[count] = this->hg->d_y[i];
                }
                ++count;
            }
            dat.add_contained_vals ("/x_nne", x_nne);
            dat.add_contained_vals ("/y_nne", y_nne);

            std::vector<float> x_nnw = this->hg->d_x;
            std::vector<float> y_nnw = this->hg->d_y;
            count = 0;
            for (int i : this->hg->d_nnw) {
                if (i >= 0) {
                    x_nnw[count] = this->hg->d_x[i];
                    y_nnw[count] = this->hg->d_y[i];
                }
                ++count;
            }
            dat.add_contained_vals ("/x_nnw", x_nnw);
            dat.add_contained_vals ("/y_nnw", y_nnw);

            std::vector<float> x_nw = this->hg->d_x;
            std::vector<float> y_nw = this->hg->d_y;
            count = 0;
            for (int i : this->hg->d_nw) {
                if (i >= 0) {
                    x_nw[count] = this->hg->d_x[i];
                    y_nw[count] = this->hg->d_y[i];
                }
                ++count;
            }
            dat.add_contained_vals ("/x_nw", x_nw);
            dat.add_contained_vals ("/y_nw", y_nw);

            std::vector<float> x_nsw = this->hg->d_x;
            std::vector<float> y_nsw = this->hg->d_y;
            count = 0;
            for (int i : this->hg->d_nsw) {
                if (i >= 0) {
                    x_nsw[count] = this->hg->d_x[i];
                    y_nsw[count] = this->hg->d_y[i];
                }
                ++count;
            }
            dat.add_contained_vals ("/x_nsw", x_nsw);
            dat.add_contained_vals ("/y_nsw", y_nsw);

            std::vector<float> x_nse = this->hg->d_x;
            std::vector<float> y_nse = this->hg->d_y;
            count = 0;
            for (int i : this->hg->d_nse) {
                if (i >= 0) {
                    x_nse[count] = this->hg->d_x[i];
                    y_nse[count] = this->hg->d_y[i];
                }
                ++count;
            }
            dat.add_contained_vals ("/x_nse", x_nse);
            dat.add_contained_vals ("/y_nse", y_nse);

            // And hex to hex distance:
            dat.add_val ("/d", this->d);
        }
        //@} // HDF5

        /*!
         * Computation methods
         */
        //@{

        /*!
         * Normalise the vector of Flts f.
         */
        void normalise (std::vector<Flt>& f) {

            Flt maxf = -1e7;
            Flt minf = +1e7;

            // Determines min and max
            for (auto val : f) {
                if (val>maxf) { maxf = val; }
                if (val<minf) { minf = val; }
            }
            Flt scalef = 1.0 /(maxf - minf);

            std::vector<std::vector<Flt> > norm_a;
            this->resize_vector_vector (norm_a);
            for (unsigned int fi = 0; fi < f.size(); ++fi) {
                f[fi] = fmin (fmax (((f[fi]) - minf) * scalef, 0.0), 1.0);
            }
        }

        /*!
         * Do a single step through the model.
         */
        virtual void step (void) = 0;

        /*!
         * 2D spatial integration of the function f. Result placed in gradf.
         *
         * For each Hex, work out the gradient in x and y directions
         * using whatever neighbours can contribute to an estimate.
         */
        void spacegrad2D (std::vector<Flt>& f, std::array<std::vector<Flt>, 2>& gradf) {

            // Note - East is positive x; North is positive y.
#pragma omp parallel for schedule(static)
            for (unsigned int hi=0; hi<this->nhex; ++hi) {

                // Find x gradient
                if (HAS_NE(hi) && HAS_NW(hi)) {
                    gradf[0][hi] = (f[NE(hi)] - f[NW(hi)]) * oneover2d;
                } else if (HAS_NE(hi)) {
                    gradf[0][hi] = (f[NE(hi)] - f[hi]) * oneoverd;
                } else if (HAS_NW(hi)) {
                    gradf[0][hi] = (f[hi] - f[NW(hi)]) * oneoverd;
                } else {
                    // zero gradient in x direction as no neighbours in
                    // those directions? Or possibly use the average of
                    // the gradient between the nw,ne and sw,se neighbours
                    gradf[0][hi] = 0.0;
                }

                // Find y gradient
                if (HAS_NNW(hi) && HAS_NNE(hi) && HAS_NSW(hi) && HAS_NSE(hi)) {
                    // Full complement. Compute the mean of the nse->nne and nsw->nnw gradients
                    gradf[1][hi] = ( (f[NNE(hi)] - f[NSE(hi)]) + (f[NNW(hi)] - f[NSW(hi)]) ) * oneover4v;
                } else if (HAS_NNW(hi) && HAS_NNE(hi)) {
                    gradf[1][hi] = ( (f[NNE(hi)] + f[NNW(hi)]) * 0.5 - f[hi]) * oneoverv;
                } else if (HAS_NSW(hi) && HAS_NSE(hi)) {
                    gradf[1][hi] = (f[hi] - (f[NSE(hi)] + f[NSW(hi)]) * 0.5) * oneoverv;
                } else if (HAS_NNW(hi) && HAS_NSW(hi)) {
                    gradf[1][hi] = (f[NNW(hi)] - f[NSW(hi)]) * oneover2v;
                } else if (HAS_NNE(hi) && HAS_NSE(hi)) {
                    gradf[1][hi] = (f[NNE(hi)] - f[NSE(hi)]) * oneover2v;
                } else {
                    // Leave grady at 0
                    gradf[1][hi] = 0.0;
                }
            }
        }

        /*!
         * Compute laplacian of scalar field F, with result placed in lapF.
         */
        virtual void compute_laplace (const std::vector<Flt>& F, std::vector<Flt>& lapF) {

            Flt norm  = (Flt)2 / (Flt)(3.0 * this->d * this->d);

#pragma omp parallel for schedule(static)
            for (unsigned int hi=0; hi<this->nhex; ++hi) {

                // 1. The D Del^2 term

                // Compute the sum around the neighbours
                Flt thesum = -6 * F[hi];
                if (HAS_NE(hi)) {
                    thesum += F[NE(hi)];
                } else {
                    thesum += F[hi]; // A ghost neighbour-east with same value as Hex_0
                }
                if (HAS_NNE(hi)) {
                    thesum += F[NNE(hi)];
                } else {
                    thesum += F[hi];
                }
                if (HAS_NNW(hi)) {
                    thesum += F[NNW(hi)];
                } else {
                    thesum += F[hi];
                }
                if (HAS_NW(hi)) {
                    thesum += F[NW(hi)];
                } else {
                    thesum += F[hi];
                }
                if (HAS_NSW(hi)) {
                    thesum += F[NSW(hi)];
                } else {
                    thesum += F[hi];
                }
                if (HAS_NSE(hi)) {
                    thesum += F[NSE(hi)];
                } else {
                    thesum += F[hi];
                }

                lapF[hi] = norm * thesum;
            }
        }

    }; // RD_Base

} // namespace morph
