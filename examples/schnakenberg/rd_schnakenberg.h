/*!
 * A morphologica example; a 2D Reaction Diffusion system.
 *
 * Author: Seb James
 */

#include <vector>
#include <array>
#include <sstream>
#include "morph/RD_Base.h"
#include "morph/HdfData.h"

/*!
 * Two component Schnakenberg Reaction Diffusion system
 */
template <class Flt>
class RD_Schnakenberg : public morph::RD_Base<Flt>
{
public:
    /*!
     * Reactant A
     */
    alignas(alignof(std::vector<Flt>))
    std::vector<Flt> A;

    /*!
     * Reactant B
     */
    alignas(alignof(std::vector<Flt>))
    std::vector<Flt> B;

    /*!
     * J(x,t) - the "flux current". This is a vector field. May need J_A and J_B.
     */
    alignas(alignof(std::array<std::vector<Flt>, 2>))
    std::array<std::vector<Flt>, 2> J;

    /*!
     * Schnakenberg
     * F = k1 - k2 A + k3 A^2 B
     * G = k4        - k3 A^2 B
     */
    alignas(Flt) Flt k1 = 1.0;
    alignas(Flt) Flt k2 = 1.0;
    alignas(Flt) Flt k3 = 1.0;
    alignas(Flt) Flt k4 = 1.0;

    /*!
     * The diffusion parameters.
     */
    //@{
    alignas(Flt) Flt D_A = 0.1;
    alignas(Flt) Flt D_B = 0.1;
    //@}

    /*!
     * Simple constructor; no arguments. Simply call RD_Base constructor.
     */
    RD_Schnakenberg (void) :
        morph::RD_Base<Flt>() {
    }

    /*!
     * Destructor
     */
    ~RD_Schnakenberg (void) {
        // No operation. RD_Base destructor will free HexGrid.
    }

    /*!
     * Perform memory allocations, vector resizes and so on.
     */
    void allocate (void) {
        // Always call allocate() from the base class first.
        morph::RD_Base<Flt>::allocate();
        // Resize and zero-initialise the various containers. Note that the size of a
        // 'vector variable' is given by the number of hexes in the hex grid which is
        // a member of this class (via its parent, RD_Base)
        this->resize_vector_variable (this->A);
        this->resize_vector_variable (this->B);
    }

    /*!
     * Initialise variables and parameters. Carry out one-time computations required
     * of the model.
     */
    void init (void) {
        // Initialise A, B with noise
        this->noiseify_vector_variable (this->A, 0.5, 1);
        this->noiseify_vector_variable (this->B, 0.6, 1);
    }


    /*!
     * HDF5 file saving/loading methods.
     */
    //@{

    /*!
     * Save the variables.
     */
    void save (void) {
        std::stringstream fname;
        fname << this->logpath << "/dat_";
        fname.width(5);
        fname.fill('0');
        fname << this->stepCount << ".h5";
        morph::HdfData data(fname.str());
        std::stringstream path;
        // The A variables
        path << "/A";
        data.add_contained_vals (path.str().c_str(), this->A);
        // The B variable
        path.str("");
        path.clear();
        path << "/B";
        data.add_contained_vals (path.str().c_str(), this->B);
    }

    /*!
     * Computation methods
     */
    //@{

    /*!
     * Schnakenberg functions
     */
    //@{
    void compute_dAdt (std::vector<Flt>& A_, std::vector<Flt>& dAdt) {
        std::vector<Flt> lapA(this->nhex, 0.0);
        this->compute_laplace (A_, lapA);
#pragma omp parallel for
        for (unsigned int h=0; h<this->nhex; ++h) {
            dAdt[h] = this->k1 - (this->k2 * A_[h])
                + (this->k3 * A_[h] * A_[h] * this->B[h]) + this->D_A * lapA[h];
        }
    }
    void compute_dBdt (std::vector<Flt>& B_, std::vector<Flt>& dBdt) {
        std::vector<Flt> lapB(this->nhex, 0.0);
        this->compute_laplace (B_, lapB);
#pragma omp parallel for
        for (unsigned int h=0; h<this->nhex; ++h) {
            // G = k4        - k3 A^2 B
            dBdt[h] = this->k4 - (this->k3 * this->A[h] * this->A[h] * B_[h]) + this->D_B * lapB[h];
        }
    }
    //@}

    /*!
     * Do a single step through the model.
     */
    void step (void) {

        this->stepCount++;

        // 1. 4th order Runge-Kutta computation for A
        {
            // Atst: "A at a test point". Atst is a temporary estimate for A.
            std::vector<Flt> Atst(this->nhex, 0.0);
            std::vector<Flt> dAdt(this->nhex, 0.0);
            std::vector<Flt> K1(this->nhex, 0.0);
            std::vector<Flt> K2(this->nhex, 0.0);
            std::vector<Flt> K3(this->nhex, 0.0);
            std::vector<Flt> K4(this->nhex, 0.0);

            /*
             * Stage 1
             */
            this->compute_dAdt (this->A, dAdt);
#pragma omp parallel for
            for (unsigned int h=0; h<this->nhex; ++h) {
                K1[h] = dAdt[h] * this->dt;
                Atst[h] = this->A[h] + K1[h] * 0.5 ;
            }

            /*
             * Stage 2
             */
            this->compute_dAdt (Atst, dAdt);
#pragma omp parallel for
            for (unsigned int h=0; h<this->nhex; ++h) {
                K2[h] = dAdt[h] * this->dt;
                Atst[h] = this->A[h] + K2[h] * 0.5;
            }

            /*
             * Stage 3
             */
            this->compute_dAdt (Atst, dAdt);
#pragma omp parallel for
            for (unsigned int h=0; h<this->nhex; ++h) {
                K3[h] = dAdt[h] * this->dt;
                Atst[h] = this->A[h] + K3[h];
            }

            /*
             * Stage 4
             */
            this->compute_dAdt (Atst, dAdt);
#pragma omp parallel for
            for (unsigned int h=0; h<this->nhex; ++h) {
                K4[h] = dAdt[h] * this->dt;
            }

            /*
             * Final sum together. This could be incorporated in the for loop for
             * Stage 4, but I've separated it out for pedagogy.
             */
#pragma omp parallel for
            for (unsigned int h=0; h<this->nhex; ++h) {
                A[h] += ((K1[h] + 2.0 * (K2[h] + K3[h]) + K4[h])/(Flt)6.0);
            }
        }

        // 2. 4th order Runge-Kutta computation of B
        {
            // Btst: "B at a test point". Btst is a temporary estimate for B.
            std::vector<Flt> Btst(this->nhex, 0.0);
            std::vector<Flt> dBdt(this->nhex, 0.0);
            std::vector<Flt> K1(this->nhex, 0.0);
            std::vector<Flt> K2(this->nhex, 0.0);
            std::vector<Flt> K3(this->nhex, 0.0);
            std::vector<Flt> K4(this->nhex, 0.0);

            /*
             * Stage 1
             */
            this->compute_dBdt (this->B, dBdt);
#pragma omp parallel for
            for (unsigned int h=0; h<this->nhex; ++h) {
                K1[h] = dBdt[h] * this->dt;
                Btst[h] = this->B[h] + K1[h] * 0.5 ;
            }

            /*
             * Stage 2
             */
            this->compute_dBdt (Btst, dBdt);
#pragma omp parallel for
            for (unsigned int h=0; h<this->nhex; ++h) {
                K2[h] = dBdt[h] * this->dt;
                Btst[h] = this->B[h] + K2[h] * 0.5;
            }

            /*
             * Stage 3
             */
            this->compute_dBdt (Btst, dBdt);
#pragma omp parallel for
            for (unsigned int h=0; h<this->nhex; ++h) {
                K3[h] = dBdt[h] * this->dt;
                Btst[h] = this->B[h] + K3[h];
            }

            /*
             * Stage 4
             */
            this->compute_dBdt (Btst, dBdt);
#pragma omp parallel for
            for (unsigned int h=0; h<this->nhex; ++h) {
                K4[h] = dBdt[h] * this->dt;
            }

            /*
             * Final sum together. This could be incorporated in the for loop for
             * Stage 4, but I've separated it out for pedagogy.
             */
#pragma omp parallel for
            for (unsigned int h=0; h<this->nhex; ++h) {
                B[h] += ((K1[h] + 2.0 * (K2[h] + K3[h]) + K4[h])/(Flt)6.0);
            }
        }
    }

}; // RD_Schnakenberg
