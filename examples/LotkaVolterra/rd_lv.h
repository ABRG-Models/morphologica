/*!
 * A morphologica example; The Lotka-Volterra reaction diffusion population model.
 *
 * Author: Seb James
 */

#include <vector>
#include <array>
#include <sstream>
#include <morph/RD_Base.h>
#include <morph/HdfData.h>

/*!
 * Lotka-Volterra Reaction Diffusion system
 */
template <class Flt>
class RD_lv : public morph::RD_Base<Flt>
{
public:
    alignas(alignof(std::vector<Flt>))
    std::vector<Flt> u;

    alignas(alignof(std::vector<Flt>))
    std::vector<Flt> v;

    alignas(Flt) Flt k1 = 1.0;
    alignas(Flt) Flt k2 = 1.0;
    alignas(Flt) Flt k3 = 1.0;
    alignas(Flt) Flt k4 = 1.0;

    alignas(Flt) Flt D_u = 0.1;
    alignas(Flt) Flt D_v = 0.1;

    RD_lv (void) : morph::RD_Base<Flt>() {}
    ~RD_lv (void) {}

    //! Perform memory allocations, vector resizes and so on.
    void allocate (void)
    {
        // Always call allocate() from the base class first.
        morph::RD_Base<Flt>::allocate();
        // Resize and zero-initialise the various containers. Note that the size of a
        // 'vector variable' is given by the number of hexes in the hex grid which is
        // a member of this class (via its parent, RD_Base)
        this->resize_vector_variable (this->u);
        this->resize_vector_variable (this->v);
    }

    //! Initialise variables and parameters and do any one-time computations
    void init (void)
    {
        // Initialise u, v with noise
        this->noiseify_vector_variable (this->u, 0.5, 1);
        this->noiseify_vector_variable (this->v, 0.6, 1);
    }

    //! Save the variables to HDF5.
    void save (void)
    {
        std::stringstream fname;
        fname << this->logpath << "/dat_";
        fname.width(5);
        fname.fill('0');
        fname << this->stepCount << ".h5";
        morph::HdfData data(fname.str());
        std::stringstream path;
        // The u variables
        path << "/u";
        data.add_contained_vals (path.str().c_str(), this->u);
        // The v variable
        path.str("");
        path.clear();
        path << "/v";
        data.add_contained_vals (path.str().c_str(), this->v);
    }

    void compute_dudt (std::vector<Flt>& u_, std::vector<Flt>& dudt)
    {
        std::vector<Flt> lapu(this->nhex, 0.0);
        this->compute_laplace (u_, lapu);
#pragma omp parallel for
        for (unsigned int h=0; h<this->nhex; ++h) {
            dudt[h] = this->k1 - (this->k2 * u_[h])
                + (this->k3 * u_[h] * u_[h] * this->v[h]) + this->D_u * lapu[h];
        }
    }

    void compute_dvdt (std::vector<Flt>& v_, std::vector<Flt>& dvdt)
    {
        std::vector<Flt> lapv(this->nhex, 0.0);
        this->compute_laplace (v_, lapv);
#pragma omp parallel for
        for (unsigned int h=0; h<this->nhex; ++h) {
            // G = k4        - k3 u^2 v
            dvdt[h] = this->k4 - (this->k3 * this->u[h] * this->u[h] * v_[h]) + this->D_v * lapv[h];
        }
    }

    void step (void)
    {
        this->stepCount++;

        // 1. 4th order Runge-Kutta computation for u
        {
            // utst: "u at a test point". utst is a temporary estimate for u.
            std::vector<Flt> utst(this->nhex, 0.0);
            std::vector<Flt> dudt(this->nhex, 0.0);
            std::vector<Flt> K1(this->nhex, 0.0);
            std::vector<Flt> K2(this->nhex, 0.0);
            std::vector<Flt> K3(this->nhex, 0.0);
            std::vector<Flt> K4(this->nhex, 0.0);

            // RK Stage 1
            this->compute_dudt (this->u, dudt);
#pragma omp parallel for
            for (unsigned int h=0; h<this->nhex; ++h) {
                K1[h] = dudt[h] * this->dt;
                utst[h] = this->u[h] + K1[h] * 0.5 ;
            }

            // RK Stage 2
            this->compute_dudt (utst, dudt);
#pragma omp parallel for
            for (unsigned int h=0; h<this->nhex; ++h) {
                K2[h] = dudt[h] * this->dt;
                utst[h] = this->u[h] + K2[h] * 0.5;
            }

            // RK Stage 3
            this->compute_dudt (utst, dudt);
#pragma omp parallel for
            for (unsigned int h=0; h<this->nhex; ++h) {
                K3[h] = dudt[h] * this->dt;
                utst[h] = this->u[h] + K3[h];
            }

            // RK Stage 4
            this->compute_dudt (utst, dudt);
#pragma omp parallel for
            for (unsigned int h=0; h<this->nhex; ++h) {
                K4[h] = dudt[h] * this->dt;
            }

            /*
             * Final sum together. This could be incorporated in the for loop for
             * Stage 4, but I've separated it out for pedagogy.
             */
#pragma omp parallel for
            for (unsigned int h=0; h<this->nhex; ++h) {
                u[h] += ((K1[h] + 2.0 * (K2[h] + K3[h]) + K4[h])/(Flt)6.0);
            }
        }

        // 2. 4th order Runge-Kutta computation of v
        {
            // Btst: "B at a test point". Btst is a temporary estimate for B.
            std::vector<Flt> Btst(this->nhex, 0.0);
            std::vector<Flt> dBdt(this->nhex, 0.0);
            std::vector<Flt> K1(this->nhex, 0.0);
            std::vector<Flt> K2(this->nhex, 0.0);
            std::vector<Flt> K3(this->nhex, 0.0);
            std::vector<Flt> K4(this->nhex, 0.0);

            // RK Stage 1
            this->compute_dBdt (this->B, dBdt);
#pragma omp parallel for
            for (unsigned int h=0; h<this->nhex; ++h) {
                K1[h] = dBdt[h] * this->dt;
                Btst[h] = this->B[h] + K1[h] * 0.5 ;
            }

            // RK Stage 2
            this->compute_dBdt (Btst, dBdt);
#pragma omp parallel for
            for (unsigned int h=0; h<this->nhex; ++h) {
                K2[h] = dBdt[h] * this->dt;
                Btst[h] = this->B[h] + K2[h] * 0.5;
            }

            // RK Stage 3
            this->compute_dBdt (Btst, dBdt);
#pragma omp parallel for
            for (unsigned int h=0; h<this->nhex; ++h) {
                K3[h] = dBdt[h] * this->dt;
                Btst[h] = this->B[h] + K3[h];
            }

            // RK Stage 4
            this->compute_dBdt (Btst, dBdt);
#pragma omp parallel for
            for (unsigned int h=0; h<this->nhex; ++h) {
                K4[h] = dBdt[h] * this->dt;
            }

            // Final sum together.
#pragma omp parallel for
            for (unsigned int h=0; h<this->nhex; ++h) {
                B[h] += ((K1[h] + 2.0 * (K2[h] + K3[h]) + K4[h])/(Flt)6.0);
            }
        }
    }

}; // RD_lv
