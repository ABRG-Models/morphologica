/*
 * 2D Ermentrout system deriving from RD_Base.
 */
#include <morph/tools.h>
#include <morph/HexGrid.h>
#include <morph/ReadCurves.h>
#include <morph/HdfData.h>
#include <morph/RD_Base.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <iomanip>
#include <cmath>
#include <hdf5.h>
#include <unistd.h>

/*
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

#define IF_HAS_NE(hi, yesval, noval)  (HAS_NE(hi)  ? yesval : noval)
#define IF_HAS_NNE(hi, yesval, noval) (HAS_NNE(hi) ? yesval : noval)
#define IF_HAS_NNW(hi, yesval, noval) (HAS_NNW(hi) ? yesval : noval)
#define IF_HAS_NW(hi, yesval, noval)  (HAS_NW(hi)  ? yesval : noval)
#define IF_HAS_NSW(hi, yesval, noval) (HAS_NSW(hi) ? yesval : noval)
#define IF_HAS_NSE(hi, yesval, noval) (HAS_NSE(hi) ? yesval : noval)

/*!
 * Reaction diffusion system; Ermentrout 2009.
 */
template <class Flt>
class RD_Erm : public morph::RD_Base<Flt>
{
public:
    //! Set N>1 for maintaining multiple expression gradients
    alignas(alignof(unsigned int)) unsigned int N = 1;
    //! The c_i(x,t) variables from the Ermentrout paper (chemoattractant concentration)
    alignas(alignof(std::vector<std::vector<Flt> >)) std::vector<std::vector<Flt> > c;
    //! The n_i(x,t) variables from the Ermentrout paper (density of tc axons)
    alignas(alignof(std::vector<std::vector<Flt> >)) std::vector<std::vector<Flt> > n;
    //! Holds the Laplacian
    alignas(alignof(std::vector<std::vector<Flt> >)) std::vector<std::vector<Flt> > lapl;
    //! Holds the Poisson terms (final non-linear term in Ermentrout equation 1)
    alignas(alignof(std::vector<std::vector<Flt> >)) std::vector<std::vector<Flt> > poiss;
    //! Sum of c
    alignas(alignof(std::vector<Flt>)) std::vector<Flt> sum_c;
    //! Sum of n
    alignas(alignof(std::vector<Flt>)) std::vector<Flt> sum_n;

    // Parameters of the Ermentrout model - default values.
    //! Diffusion constant for n
    alignas(Flt) Flt Dn = 0.3;
    //! Diffusion constant for c
    alignas(Flt) Flt Dc = Dn * 0.3;
    //! saturation term in function for production of c
    alignas(Flt) Flt beta = 5.0;
    //! production of new axon branches
    alignas(Flt) Flt a = 1.0;
    //! pruning constant
    alignas(Flt) Flt b = 1.0;
    //! decay of chemoattractant constant
    alignas(Flt) Flt mu = 1.0;
    //! degree of attraction of chemoattractant
    alignas(Flt) Flt chi = Dn;

    //! Frame number, used when saving PNG movie frames.
    unsigned int frameN = 0;

    //! Simple constructor; no arguments.
    RD_Erm() : morph::RD_Base<Flt>() {}

    //! Perform memory allocations, vector resizes and so on.
    virtual void allocate() { morph::RD_Base<Flt>::allocate(); }

    //! Initialise HexGrid, variables. Carry out any one-time computations of the model.
    virtual void init()
    {
        // Resize and zero-initialise the various containers
        this->resize_vector_vector (this->c, this->N);
        this->resize_vector_vector (this->n, this->N);
        this->resize_vector_vector (this->lapl, this->N);
        this->resize_vector_vector (this->poiss, this->N);
        this->sum_n.resize (this->N, Flt{0});
        this->sum_c.resize (this->N, Flt{0});

        // Initialise with noise
        for (unsigned int i  = 0; i < this->N; ++i) {
            this->noiseify_vector_variable (this->n[i], 1., 0.01);
            this->noiseify_vector_variable (this->c[i], beta*0.5, 0.01);
        }
    }

    //! Compute one step of the model
    virtual void step()
    {
        this->stepCount++;

        for (unsigned int i=0; i<this->N; ++i) {

            sum_n[i] = Flt{0};
            sum_c[i] = Flt{0};

            this->compute_poiss (n[i],c[i],i);  // compute the non-linear Poission term in Eq1
            this->compute_lapl (n[i], i);       // populate lapl[i] with laplacian of n

            // integrate n
            for (unsigned int h=0; h<this->nhex; ++h) {
                n[i][h] += (a - b*n[i][h] + Dn*lapl[i][h] - chi*poiss[i][h])*this->dt;
                sum_n[i] += n[i][h];
            }

            this->compute_lapl (c[i], i);       // populate lapl[i] with laplacian of c

            // integrate c
            Flt n2;
            for (unsigned int h=0; h<this->nhex; ++h) {
                n2 = n[i][h]*n[i][h];
                c[i][h] += (beta*n2/(1.+n2) - mu*c[i][h] +Dc*lapl[i][h])*this->dt;
                sum_c[i] += c[i][h];
            }
        }
    }

    //! Computes the Laplacian Stable with dt = 0.0001;
    void compute_lapl (std::vector<Flt>& fa, unsigned int i)
    {
#pragma omp parallel for schedule(static) shared(lapl, fa, i)
        for (unsigned int hi=0; hi<this->nhex; ++hi) {

            // 1. The D Del^2 term

            // Compute the sum around the neighbours
            Flt thesum = -6 * fa[hi];
            if (HAS_NE(hi)) {
                thesum += fa[NE(hi)];
            } else {
                thesum += fa[hi]; // A ghost neighbour-east with same value as Hex_0
            }
            if (HAS_NNE(hi)) {
                thesum += fa[NNE(hi)];
            } else {
                thesum += fa[hi];
            }
            if (HAS_NNW(hi)) {
                thesum += fa[NNW(hi)];
            } else {
                thesum += fa[hi];
            }
            if (HAS_NW(hi)) {
                thesum += fa[NW(hi)];
            } else {
                thesum += fa[hi];
            }
            if (HAS_NSW(hi)) {
                thesum += fa[NSW(hi)];
            } else {
                thesum += fa[hi];
            }
            if (HAS_NSE(hi)) {
                thesum += fa[NSE(hi)];
            } else {
                thesum += fa[hi];
            }

            this->lapl[i][hi] = this->twoover3dd * thesum;
        }
    }

    //! Computes the Poisson term. Stable with dt = 0.0001;
    void compute_poiss (std::vector<Flt>& fa1, std::vector<Flt>& fa2, unsigned int i)
    {
        // Compute non-linear term
#pragma omp parallel for schedule(static) shared(poiss, fa1, fa2)
        for (unsigned int hi=0; hi<this->nhex; ++hi) {

            std::vector<Flt> dum1(6,fa1[hi]);
            std::vector<Flt> dum2(6,fa2[hi]);

            if (HAS_NE(hi)) {
                dum1[0] = fa1[NE(hi)];
                dum2[0] = fa2[NE(hi)];
            }
            if (HAS_NNE(hi)) {
                dum1[1] = fa1[NNE(hi)];
                dum2[1] = fa2[NNE(hi)];
            }
            if (HAS_NNW(hi)) {
                dum1[2] = fa1[NNW(hi)];
                dum2[2] = fa2[NNW(hi)];
            }
            if (HAS_NW(hi)) {
                dum1[3] = fa1[NW(hi)];
                dum2[3] = fa2[NW(hi)];
            }
            if (HAS_NSW(hi)) {
                dum1[4] = fa1[NSW(hi)];
                dum2[4] = fa2[NSW(hi)];
            }
            if (HAS_NSE(hi)) {
                dum1[5] = fa1[NSE(hi)];
                dum2[5] = fa2[NSE(hi)];
            }

            // John Brooke's final thesis solution (based on 'finite volume method'
            // of Lee et al. https://doi.org/10.1080/00207160.2013.864392
            Flt val = (dum1[0]+fa1[hi]) * (dum2[0]-fa2[hi])
                + (dum1[1]+fa1[hi]) * (dum2[1]-fa2[hi])
                + (dum1[2]+fa1[hi]) * (dum2[2]-fa2[hi])
                + (dum1[3]+fa1[hi]) * (dum2[3]-fa2[hi])
                + (dum1[4]+fa1[hi]) * (dum2[4]-fa2[hi])
                + (dum1[5]+fa1[hi]) * (dum2[5]-fa2[hi]);
            this->poiss[i][hi] = val * this->oneover3dd;// / (3 * this->d * this->d);
        }
    }

    //! Save state to HDF5
    void saveState()
    {
        std::string fname = this->logpath + "/2Derm.h5";
        morph::HdfData data (fname);

        // Save some variables
        for (unsigned int i = 0; i<this->N; ++i) {
            std::stringstream vss;
            vss << "c_" << i;
            std::string vname = vss.str();
            data.add_contained_vals (vname.c_str(), this->c[i]);
            vname[0] = 'n';
            data.add_contained_vals (vname.c_str(), this->n[i]);
        }

        // Parameters
        data.add_val ("/Dn", this->Dn);
        data.add_val ("/Dc", this->Dc);
        data.add_val ("/beta", this->beta);
        data.add_val ("/a", this->a);
        data.add_val ("/b", this->b);
        data.add_val ("/mu", this->mu);
        data.add_val ("/chi", this->chi);

        // HexGrid information
        this->saveHexPositions (data);
    }

}; // RD_Erm
