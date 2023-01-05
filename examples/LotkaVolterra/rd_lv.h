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

    alignas(Flt) Flt a1 = 1.0;
    alignas(Flt) Flt b1 = 1.0;
    alignas(Flt) Flt c1 = 1.0;
    alignas(Flt) Flt a2 = 1.0;
    alignas(Flt) Flt b2 = 1.0;
    alignas(Flt) Flt c2 = 1.0;

    alignas(Flt) Flt D1 = 0.1;
    alignas(Flt) Flt D2 = 0.1;

    RD_lv() : morph::RD_Base<Flt>() {}
    ~RD_lv() {}

    //! Perform memory allocations, vector resizes and so on.
    void allocate()
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
    void init()
    {
        // Initialise u, v with noise
        this->noiseify_vector_variable (this->u, 0.5, 1);
        this->noiseify_vector_variable (this->v, 0.6, 1);
    }

    //! Save the variables to HDF5.
    void save()
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
            dudt[h] = this->D1 * lapu[h] + (u_[h] * (a1 - b1 * u_[h] - c1 * v[h]));
        }
    }

    void compute_dvdt (std::vector<Flt>& v_, std::vector<Flt>& dvdt)
    {
        std::vector<Flt> lapv(this->nhex, 0.0);
        this->compute_laplace (v_, lapv);
#pragma omp parallel for
        for (unsigned int h=0; h<this->nhex; ++h) {
            dvdt[h] = this->D2 * lapv[h] + (v_[h] * (a2 - b2 * v_[h] - c2 * u[h]));
        }
    }

    void step()
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
            // vtst: "v at a test point". vtst is a temporary estimate for v.
            std::vector<Flt> vtst(this->nhex, 0.0);
            std::vector<Flt> dvdt(this->nhex, 0.0);
            std::vector<Flt> K1(this->nhex, 0.0);
            std::vector<Flt> K2(this->nhex, 0.0);
            std::vector<Flt> K3(this->nhex, 0.0);
            std::vector<Flt> K4(this->nhex, 0.0);

            // RK Stage 1
            this->compute_dvdt (this->v, dvdt);
#pragma omp parallel for
            for (unsigned int h=0; h<this->nhex; ++h) {
                K1[h] = dvdt[h] * this->dt;
                vtst[h] = this->v[h] + K1[h] * 0.5 ;
            }

            // RK Stage 2
            this->compute_dvdt (vtst, dvdt);
#pragma omp parallel for
            for (unsigned int h=0; h<this->nhex; ++h) {
                K2[h] = dvdt[h] * this->dt;
                vtst[h] = this->v[h] + K2[h] * 0.5;
            }

            // RK Stage 3
            this->compute_dvdt (vtst, dvdt);
#pragma omp parallel for
            for (unsigned int h=0; h<this->nhex; ++h) {
                K3[h] = dvdt[h] * this->dt;
                vtst[h] = this->v[h] + K3[h];
            }

            // RK Stage 4
            this->compute_dvdt (vtst, dvdt);
#pragma omp parallel for
            for (unsigned int h=0; h<this->nhex; ++h) {
                K4[h] = dvdt[h] * this->dt;
            }

            // Final sum together.
#pragma omp parallel for
            for (unsigned int h=0; h<this->nhex; ++h) {
                v[h] += ((K1[h] + 2.0 * (K2[h] + K3[h]) + K4[h])/(Flt)6.0);
            }
        }
    }

}; // RD_lv
