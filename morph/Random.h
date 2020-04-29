#pragma once

#include <random>
#include <vector>
#include <limits>
#include <type_traits>

/*!
 * Random numbers in the morph namespace, wrapping C++ <random> stuff, with a
 * particular favouring for mt19937_64, the 64 bit Mersenne Twister algorithm. With
 * these classes, generate random numbers using our choice of algorithms from
 * std::random plus I'd like to include a siderand approach to collecting entropy.
 *
 * I've wrapped a selection of distributions, including normal, poisson and
 * uniform. Copy the classes here to add additional ones that you might need from the
 * full list: https://en.cppreference.com/w/cpp/numeric/random (such as weibull,
 * exponential, lognormal and so on).
 */

namespace morph {

    // Note that I considered having a Random<T> base class, but because the
    // distribution object isn't sub-classed, then hardly any code would be
    // de-duplicated. max(), min() and get() methods all need the dist member
    // attribute, so each one has to be written out in each wrapper class. So it goes.

    //! RandUniform to be specialised depending on whether T is integral or not
    template <typename T = double, bool = std::is_integral<std::decay_t<T>>::value>
    class RandUniform {};

    /*!
     * Floating-point number specialization of RandUnifom.
     */
    template <typename T>
    class RandUniform<T, false>
    {
    private:
        //! Random device to provide a seed for the generator
        std::random_device rd{};
        //! Pseudo random number generator engine
        std::mt19937_64 generator{rd()};
        //! Our distribution
        std::uniform_real_distribution<T> dist;
    public:
        //! Default constructor gives RN generator which works in range [0,1)
        RandUniform () {
            typename std::uniform_real_distribution<T>::param_type prms (T{0}, T{1});
            this->dist.param (prms);
        }
        //! This constructor gives RN generator which works in range [0,1) and sets a
        //! fixed seed
        RandUniform (unsigned int _seed) {
            this->generator.seed (_seed);
            typename std::uniform_real_distribution<T>::param_type prms (T{0}, T{1});
            this->dist.param (prms);
        }
        //! This constructor gives RN generator which works in range [a,b)
        RandUniform (T a, T b) {
            typename std::uniform_real_distribution<T>::param_type prms (a, b);
            this->dist.param (prms);
        }
        //! This constructor gives RN generator which works in range [a,b)
        RandUniform (T a, T b, unsigned int _seed) {
            this->generator.seed (_seed);
            typename std::uniform_real_distribution<T>::param_type prms (a, b);
            this->dist.param (prms);
        }
        //! Get 1 random number from the generator
        T get (void) { return this->dist (this->generator); }
        //! Get n random numbers from the generator
        std::vector<T> get (size_t n) {
            std::vector<T> rtn (n, T{0});
            for (size_t i = 0; i < n; ++i) {
                rtn[i] = this->dist (this->generator);
            }
            return rtn;
        }
        T min (void) { return this->dist.min(); }
        T max (void) { return this->dist.max(); }
    };

    /*!
     * Integer specialization: Generate uniform random numbers in a integer format
     */
    template<typename T>
    class RandUniform<T, true>
    {
    private:
        //! Random device to provide a seed for the generator
        std::random_device rd{};
        //! Pseudo random number generator engine
        std::mt19937_64 generator{rd()};
        //! Our distribution
        std::uniform_int_distribution<T> dist;
    public:
        //! Default constructor gives an integer random number generator which works
        //! in range [0,(type max))
        RandUniform () {
            typename std::uniform_int_distribution<T>::param_type prms (std::numeric_limits<T>::min(),
                                                                        std::numeric_limits<T>::max());
            this->dist.param (prms);
        }
        //! This constructor gives an integer random number generator which works
        //! in range [0,(type max)) with fixed seed \a _seed.
        RandUniform (unsigned int _seed) {
            this->generator.seed (_seed);
            typename std::uniform_int_distribution<T>::param_type prms (std::numeric_limits<T>::min(),
                                                                        std::numeric_limits<T>::max());
            this->dist.param (prms);
        }
        //! This constructor gives RN generator which works in range [a,b)
        RandUniform (T a, T b) {
            typename std::uniform_int_distribution<T>::param_type prms (a, b);
            this->dist.param (prms);
        }
        //! This constructor gives RN generator which works in range [a,b) and sets a
        //! fixed seed.
        RandUniform (T a, T b, unsigned int _seed) {
            this->generator.seed (_seed);
            typename std::uniform_int_distribution<T>::param_type prms (a, b);
            this->dist.param (prms);
        }
        //! Get 1 random number from the generator
        T get (void) { return this->dist (this->generator); }
        //! Get n random numbers from the generator
        std::vector<T> get (size_t n) {
            std::vector<T> rtn (n, T{0});
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
        //! Random device to provide a seed for the generator
        std::random_device rd{};
        //! Pseudo random number generator engine
        std::mt19937_64 generator{rd()};
        //! Our distribution
        std::normal_distribution<T> dist;
    public:
        //! Default constructor gives RN generator with mean 0 and standard deviation 1
        RandNormal (void) {
            typename std::normal_distribution<T>::param_type prms (T{0}, T{1});
            this->dist.param (prms);
        }
        //! This constructor gives RN generator with mean 0 and standard deviation 1
        //! and set a fixed seed.
        RandNormal (unsigned int _seed) {
            this->generator.seed (_seed);
            typename std::normal_distribution<T>::param_type prms (T{0}, T{1});
            this->dist.param (prms);
        }
        //! This constructor gives RN generator with mean \a mean and standard deviation \a sigma
        RandNormal (T mean, T sigma) {
            typename std::normal_distribution<T>::param_type prms (mean, sigma);
            this->dist.param (prms);
        }
        //! This constructor gives RN generator with mean \a mean and standard deviation \a sigma
        RandNormal (T mean, T sigma, unsigned int _seed) {
            this->generator.seed (_seed);
            typename std::normal_distribution<T>::param_type prms (mean, sigma);
            this->dist.param (prms);
        }
        //! Get 1 random number from the generator
        T get (void) { return this->dist (this->generator); }
        //! Get n random numbers from the generator
        std::vector<T> get (size_t n) {
            std::vector<T> rtn (n, T{0});
            for (size_t i = 0; i < n; ++i) {
                rtn[i] = this->dist (this->generator);
            }
            return rtn;
        }
        T min (void) { return this->dist.min(); }
        T max (void) { return this->dist.max(); }
    };

    /*!
     * Generate numbers drawn from a random log-normal distribution.
     */
    template <typename T = double>
    class RandLogNormal
    {
    private:
        //! Random device to provide a seed for the generator
        std::random_device rd{};
        //! Pseudo random number generator engine
        std::mt19937_64 generator{rd()};
        //! Our distribution
        std::lognormal_distribution<T> dist;
    public:
        //! Default constructor gives RN generator with mean-of-the-log 0 and standard
        //! deviation-of-the-log 1
        RandLogNormal (void) {
            typename std::lognormal_distribution<T>::param_type prms (T{0}, T{1});
            this->dist.param (prms);
        }
        //! This constructor gives RN generator with mean-of-the-log 0 and standard
        //! deviation-of-the-log 1. Sets a fixed seed.
        RandLogNormal (unsigned int _seed) {
            this->generator.seed (_seed);
            typename std::lognormal_distribution<T>::param_type prms (T{0}, T{1});
            this->dist.param (prms);
        }
        //! This constructor gives RN generator with mean-of-the-log \a mean and
        //! standard deviation \a sigma
        RandLogNormal (T mean, T sigma) {
            typename std::lognormal_distribution<T>::param_type prms (mean, sigma);
            this->dist.param (prms);
        }
        //! This constructor gives RN generator with mean-of-the-log \a mean and
        //! standard deviation \a sigma and sets a seed.
        RandLogNormal (T mean, T sigma, unsigned int _seed) {
            this->generator.seed (_seed);
            typename std::lognormal_distribution<T>::param_type prms (mean, sigma);
            this->dist.param (prms);
        }
        //! Get 1 random number from the generator
        T get (void) { return this->dist (this->generator); }
        //! Get n random numbers from the generator
        std::vector<T> get (size_t n) {
            std::vector<T> rtn (n, T{0});
            for (size_t i = 0; i < n; ++i) {
                rtn[i] = this->dist (this->generator);
            }
            return rtn;
        }
        T min (void) { return this->dist.min(); }
        T max (void) { return this->dist.max(); }
    };

    /*!
     * Generate Poisson random numbers in a integer format - valid Ts are short, int,
     * long, long long, unsigned short, unsigned int, unsigned long, or unsigned long
     * long.
     */
    template <typename T = int>
    class RandPoisson
    {
    private:
        //! Random device to provide a seed for the generator
        std::random_device rd{};
        //! Pseudo random number generator engine
        std::mt19937_64 generator{rd()};
        //! Our distribution
        std::poisson_distribution<T> dist;
    public:
        //! Default constructor gives a Poisson random number generator with mean 0.
        RandPoisson (void) {
            typename std::poisson_distribution<T>::param_type prms (T{0});
            this->dist.param (prms);
        }
        //! Default constructor gives a Poisson random number generator with mean
        //! 0. Sets fixed seed \a _seed.
        RandPoisson (unsigned int _seed) {
            this->generator.seed (_seed);
            typename std::poisson_distribution<T>::param_type prms (T{0});
            this->dist.param (prms);
        }
        //! This constructor gives RN generator with mean \a mean.
        RandPoisson (T mean) {
            typename std::poisson_distribution<T>::param_type prms (mean);
            this->dist.param (prms);
        }
        //! This constructor gives RN generator with mean \a mean.
        RandPoisson (T mean, unsigned int _seed) {
            this->generator.seed (_seed);
            typename std::poisson_distribution<T>::param_type prms (mean);
            this->dist.param (prms);
        }
        //! Get 1 random number from the generator
        T get (void) { return this->dist (this->generator); }
        //! Get n random numbers from the generator
        std::vector<T> get (size_t n) {
            std::vector<T> rtn (n, T{0});
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
}
