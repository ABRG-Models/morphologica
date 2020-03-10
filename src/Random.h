#pragma once

#include <random>
using std::random_device;
using std::mt19937_64;
using std::uniform_real_distribution;
using std::uniform_int_distribution;
using std::normal_distribution;

#include <vector>
using std::vector;

#include <limits>
using std::numeric_limits;

/*!
 * Random numbers in the morph namespace, wrapping C++ <random> stuff, with a particular favouring
 * for mt19937_64, the 64 bit Mersenne Twister algorithm. With these classes, generate random
 * numbers using our choice of algorithms from std::random plus I'd like to include a siderand
 * approach to collecting entropy.
 */

namespace morph {

    // Note that I considered having a Random<T> base class, but because the distribution object
    // isn't sub-classed, then hardly any code would be de-duplicated. For example, get() methods
    // all need the dist member attribute, so each one has to be written out in each wrapper
    // class. So it goes.

    /*!
     * Generate uniform random numbers in a floating point format - valid Ts are float, double and
     * long double.
     */
    template <typename T = double>
    class RandUniformReal
    {
    private:
        //! Random device and random generator engine
        random_device rd{};
        mt19937_64 generator{rd()};
        //! Have Rands of different types? Or one for each wrapper class?
        uniform_real_distribution<T> dist; // If T is int type, then uniform_int_distribution<T> is correct
    public:
        //! Default constructor gives RN generator which works in range [0,1)
        RandUniformReal (void) {
            typename uniform_real_distribution<T>::param_type prms(static_cast<T>(0.0), static_cast<T>(1.0));
            this->dist.param (prms);
        }
        //! This constructor gives RN generator which works in range [a,b)
        RandUniformReal (T a, T b) {
            typename uniform_real_distribution<T>::param_type prms (a, b);
            this->dist.param (prms);
        }
        //! Get 1 random number from the generator
        T get (void) { return this->dist (this->generator); }
        //! Get n random numbers from the generator
        vector<T> get (size_t n) {
            vector<T> rtn (n, static_cast<T>(0));
            for (size_t i = 0; i < n; ++i) {
                rtn[i] = this->dist (this->generator);
            }
            return rtn;
        }
        T min (void) { return this->dist.min(); }
        T max (void) { return this->dist.max(); }
    };

    /*!
     * Generate uniform random numbers in a integer format - valid Ts are short, int, long, long
     * long, unsigned short, unsigned int, unsigned long, or unsigned long long.
     */
    template <typename T = unsigned long long int>
    class RandUniformInt
    {
    private:
        //! Random device and random generator engine
        random_device rd{};
        mt19937_64 generator{rd()};
        //! Have Rands of different types? Or one for each wrapper class?
        uniform_int_distribution<T> dist; // If T is int type, then uniform_int_distribution<T> is correct
    public:
        //! Default constructor gives an integer random number generator which works in range [0,(type max))
        RandUniformInt (void) {
            typename uniform_int_distribution<T>::param_type prms(numeric_limits<T>::min(),
                                                                  numeric_limits<T>::max());
            this->dist.param (prms);
        }
        //! This constructor gives RN generator which works in range [a,b)
        RandUniformInt (T a, T b) {
            typename uniform_int_distribution<T>::param_type prms (a, b);
            this->dist.param (prms);
        }
        //! Get 1 random number from the generator
        T get (void) { return this->dist (this->generator); }
        //! Get n random numbers from the generator
        vector<T> get (size_t n) {
            vector<T> rtn (n, static_cast<T>(0));
            for (size_t i = 0; i < n; ++i) {
                rtn[i] = this->dist (this->generator);
            }
            return rtn;
        }
        //! min wrapper
        T min (void) { return this->dist.min(); }
        //! max wrapper
        T max (void) { return this->dist.max(); }
    };

    /*!
     * Generate numbers drawn from a random normal distribution.
     */
    template <typename T = double>
    class RandNormal
    {
    private:
        //! Random device and random generator engine
        random_device rd{};
        mt19937_64 generator{rd()};
        //! Have Rands of different types? Or one for each wrapper class?
        normal_distribution<T> dist; // If T is int type, then uniform_int_distribution<T> is correct
    public:
        //! Default constructor gives RN generator with mean 0 and standard deviation 1
        RandNormal (void) {
            typename normal_distribution<T>::param_type prms(static_cast<T>(0.0), static_cast<T>(1.0));
            this->dist.param (prms);
        }
        //! This constructor gives RN generator with mean @mean and standard deviation @sigma
        RandNormal (T mean, T sigma) {
            typename normal_distribution<T>::param_type prms (mean, sigma);
            this->dist.param (prms);
        }
        //! Get 1 random number from the generator
        T get (void) { return this->dist (this->generator); }
        //! Get n random numbers from the generator
        vector<T> get (size_t n) {
            vector<T> rtn (n, static_cast<T>(0));
            for (size_t i = 0; i < n; ++i) {
                rtn[i] = this->dist (this->generator);
            }
            return rtn;
        }
        T min (void) { return this->dist.min(); }
        T max (void) { return this->dist.max(); }
    };
}
