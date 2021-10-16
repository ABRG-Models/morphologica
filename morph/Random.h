#pragma once

#include <random>
#include <vector>
#include <limits>
#include <type_traits>
#include <string>
#include <ostream>
#include <array>

/*!
 * \file Random.h
 *
 * Random numbers in the morph namespace, wrapping C++ <random> stuff, with a particular
 * favouring for mt19937 and mt19937_64, the 32 and 64 bit Mersenne Twister
 * algorithms. With these classes, generate random numbers using our choice of
 * algorithms from std::random. In future, I'd like to include a siderand approach to
 * collecting entropy.
 *
 * Note on choice of mt19937 vs mt19937_64 as the 'E' template parameter: the 64 bit
 * version is slower, but is suitable if you want to generate random numbers where the
 * repeat of the sequence occurs only after as long a time as possible.
 *
 * Possible values for E:
 *
 * std::mt19937, std::mt19937_64: 32 and 64 bit Mersenne Twister (MT). 32 bit MT seems
 * to be the fastest engine! std::mt19937_64 takes twice the time.
 *
 * std::minstd_rand: linear congruential engine. 'new minimum standard' recommended by
 * Park, Miller, and Stockmeyer in 1993. The linear congruential engine is moderately
 * fast and has a very small storage requirement for state. I found it no faster than
 * std::mt19937 in one test.
 *
 * std::ranlux24, std::ranlux48: subtract-with-carry engine (aka lagged
 * Fibonacci). Seems very slow - ranlux48 is about 20 times slower than mt19937.
 *
 * std::knuth_b: A shuffle order engine. Quite slow.
 *
 * I've wrapped a selection of distributions, including normal, lognormal, poisson and
 * uniform. Copy the classes here to add additional ones that you might need from the
 * full list: https://en.cppreference.com/w/cpp/numeric/random (such as weibull or
 * exponential).
 *
 * See tests/testRandom.cpp for a variety of usage examples. Here is a single, simple
 * example to replace the old (and now removed) morph::Tools::randDouble():
 *
 * \code
 * #include <morph/Random.h>
 * morph::RandUniform<double, std::mt19937_64> randDouble;
 * double sample = randDouble.get();
 * double sample2 = randDouble.get();
 * \endcode
 *
 * A final note: There are some faster RNG algorithms on the
 * block. Xoroshiro/Xoshiro/Xorshift and SplitMix64. These don't appear to be in the c++
 * standard as yet, but they're short and could probably be implemented easily here,
 * another day.
 */

namespace morph {

    // Note that I considered having a Random<T> base class, but because the
    // distribution object isn't sub-classed, then hardly any code would be
    // de-duplicated. max(), min() and get() methods all need the dist member
    // attribute, so each one has to be written out in each wrapper class. So it goes.

    /*!
     * RandUniform to be specialised depending on whether T is integral or not
     *
     * \tparam T The type of the random number to be generated
     *
     * \tparam E The pseudo-random number generator engine. See
     *  https://en.cppreference.com/w/cpp/numeric/random for options. Here, I set the
     *  Mersenne Twister algorithm as default, but beware: it comes in 32 and 64 bit
     *  versions (std::mt19937 adn std::mt19937_64). Use of the excellent 64 bit engine
     *  will slow down code which applies it to generate 32 bit numbers! So, consider
     *  providing both T and E parameters when instantiating your RandUniform objects
     *  (similar for RandNormal etc, too).
     */
    template <typename T = float, typename E = std::mt19937, bool = std::is_integral<std::decay_t<T>>::value>
    class RandUniform {};

    //! Floating-point number specialization of RandUnifom.
    template <typename T, typename E>
    class RandUniform<T, E, false>
    {
    private:
        //! Random device to provide a seed for the generator
        std::random_device rd{};
        //! Pseudo random number generator engine. NB: 32 bit one is faster.
        E generator{rd()};
        //std::minstd_rand generator{rd()}; // No slower than mt19937, apparently
        //! Our distribution
        std::uniform_real_distribution<T> dist;
    public:
        //! Default constructor gives RN generator which works in range [0,1)
        RandUniform ()
        {
            typename std::uniform_real_distribution<T>::param_type prms (T{0}, T{1});
            this->dist.param (prms);
        }
        //! This constructor gives RN generator which works in range [0,1) and sets a
        //! fixed seed
        RandUniform (unsigned int _seed)
        {
            this->generator.seed (_seed);
            typename std::uniform_real_distribution<T>::param_type prms (T{0}, T{1});
            this->dist.param (prms);
        }
        //! This constructor gives RN generator which works in range [a,b)
        RandUniform (T a, T b)
        {
            typename std::uniform_real_distribution<T>::param_type prms (a, b);
            this->dist.param (prms);
        }
        //! This constructor gives RN generator which works in range [a,b)
        RandUniform (T a, T b, unsigned int _seed)
        {
            this->generator.seed (_seed);
            typename std::uniform_real_distribution<T>::param_type prms (a, b);
            this->dist.param (prms);
        }
        //! Copy constructor copies the parameters of the distribution
        RandUniform (const RandUniform<T>& rng) { this->param (rng.param()); }
        //! Copy assignment operator needs to be explicitly defined
        RandUniform& operator= (const RandUniform<T>& rng)
        {
            if (&rng == this) { return *this; }
            this->param (rng.param());
            return *this;
        }
        //! Reveal the distribution param methods
        typename std::uniform_real_distribution<T>::param_type param() const { return dist.param(); }
        void param (const typename std::uniform_real_distribution<T>::param_type& prms) { this->dist.param(prms); }
        //! Get 1 random number from the generator
        T get (void) { return this->dist (this->generator); }
        //! Get n random numbers from the generator
        std::vector<T> get (size_t n)
        {
            std::vector<T> rtn (n, T{0});
            for (size_t i = 0; i < n; ++i) {
                rtn[i] = this->dist (this->generator);
            }
            return rtn;
        }
        //! Place n random numbers in the array rtn
        template<size_t n>
        void get (std::array<T, n>& rtn)
        {
            for (size_t i = 0; i < n; ++i) { rtn[i] = this->dist (this->generator); }
        }
        T min (void) { return this->dist.min(); }
        T max (void) { return this->dist.max(); }
        //! Change the max/min of the distribution to be in range [a,b)
        void setparams (T a, T b)
        {
            typename std::uniform_real_distribution<T>::param_type prms (a, b);
            this->dist.param (prms);
        }
    };

    //! Integer specialization: Generate uniform random numbers in a integer format
    template<typename T, typename E>
    class RandUniform<T, E, true>
    {
    private:
        //! Random device to provide a seed for the generator
        std::random_device rd{};
        //! Pseudo random number generator engine
        E generator{rd()};
        //! Our distribution
        std::uniform_int_distribution<T> dist;
    public:
        //! Default constructor gives an integer random number generator which works
        //! in range [0,(type max)]
        RandUniform ()
        {
            typename std::uniform_int_distribution<T>::param_type prms (std::numeric_limits<T>::min(),
                                                                        std::numeric_limits<T>::max());
            this->dist.param (prms);
        }
        //! This constructor gives an integer random number generator which works
        //! in range [0,(type max)] with fixed seed \a _seed.
        RandUniform (unsigned int _seed)
        {
            this->generator.seed (_seed);
            typename std::uniform_int_distribution<T>::param_type prms (std::numeric_limits<T>::min(),
                                                                        std::numeric_limits<T>::max());
            this->dist.param (prms);
        }
        //! This constructor gives RN generator which works in range [a,b]
        RandUniform (T a, T b)
        {
            typename std::uniform_int_distribution<T>::param_type prms (a, b);
            this->dist.param (prms);
        }
        //! This constructor gives RN generator which works in range [a,b] and sets a
        //! fixed seed.
        RandUniform (T a, T b, unsigned int _seed)
        {
            this->generator.seed (_seed);
            typename std::uniform_int_distribution<T>::param_type prms (a, b);
            this->dist.param (prms);
        }
        //! Copy constructor copies the distribution parameters
        RandUniform (const RandUniform<T>& rng) { this->param (rng.param()); }
        //! Copy assignment operator needs to be explicitly defined
        RandUniform& operator= (const RandUniform<T>& rng)
        {
            if (&rng == this) { return *this; }
            this->param (rng.param());
            return *this;
        }
        //! Reveal the distribution's param getter
        typename std::uniform_int_distribution<T>::param_type param() const { return dist.param(); }
        //! Reveal the distribution's param setter
        void param (const typename std::uniform_int_distribution<T>::param_type& prms) { this->dist.param(prms); }
        //! Get 1 random number from the generator
        T get (void) { return this->dist (this->generator); }
        //! Get n random numbers from the generator
        std::vector<T> get (size_t n)
        {
            std::vector<T> rtn (n, T{0});
            for (size_t i = 0; i < n; ++i) {
                rtn[i] = this->dist (this->generator);
            }
            return rtn;
        }
        //! Place n random numbers in the array rtn
        template<size_t n>
        void get (std::array<T, n>& rtn)
        {
            for (size_t i = 0; i < n; ++i) { rtn[i] = this->dist (this->generator); }
        }
        //! min wrapper
        T min (void) { return this->dist.min(); }
        //! max wrapper
        T max (void) { return this->dist.max(); }
        //! Change the max/min of the distribution to range [a,b]
        void setparams (T a, T b)
        {
            typename std::uniform_int_distribution<T>::param_type prms (a, b);
            this->dist.param (prms);
        }
    };

    /*!
     * Generate numbers drawn from a random normal distribution.
     *
     * \tparam T The type of the random number to be generated
     *
     * \tparam E The pseudo-random number generator engine.
     */
    template <typename T = double, typename E = std::mt19937_64>
    class RandNormal
    {
    private:
        //! Random device to provide a seed for the generator
        std::random_device rd{};
        //! Pseudo random number generator engine
        E generator{rd()};
        //! Our distribution
        std::normal_distribution<T> dist;
    public:
        //! Default constructor gives RN generator with mean 0 and standard deviation 1
        RandNormal (void)
        {
            typename std::normal_distribution<T>::param_type prms (T{0}, T{1});
            this->dist.param (prms);
        }
        //! This constructor gives RN generator with mean 0 and standard deviation 1
        //! and set a fixed seed.
        RandNormal (unsigned int _seed)
        {
            this->generator.seed (_seed);
            typename std::normal_distribution<T>::param_type prms (T{0}, T{1});
            this->dist.param (prms);
        }
        //! This constructor gives RN generator with mean \a mean and standard deviation \a sigma
        RandNormal (T mean, T sigma)
        {
            typename std::normal_distribution<T>::param_type prms (mean, sigma);
            this->dist.param (prms);
        }
        //! This constructor gives RN generator with mean \a mean and standard deviation \a sigma
        RandNormal (T mean, T sigma, unsigned int _seed)
        {
            this->generator.seed (_seed);
            typename std::normal_distribution<T>::param_type prms (mean, sigma);
            this->dist.param (prms);
        }
        //! Copy constructor copies the distribution parameters
        RandNormal (const RandNormal<T>& rng) { this->param (rng.param()); }
        //! Copy assignment operator needs to be explicitly defined
        RandNormal& operator= (const RandNormal<T>& rng)
        {
            if (&rng == this) { return *this; }
            this->param (rng.param());
            return *this;
        }
        //! Reveal the distribution's param getter
        typename std::normal_distribution<T>::param_type param() const { return dist.param(); }
        //! Reveal the distribution's param setter
        void param (const typename std::normal_distribution<T>::param_type& prms) { this->dist.param(prms); }
        //! Get 1 random number from the generator
        T get (void) { return this->dist (this->generator); }
        //! Get n random numbers from the generator
        std::vector<T> get (size_t n)
        {
            std::vector<T> rtn (n, T{0});
            for (size_t i = 0; i < n; ++i) {
                rtn[i] = this->dist (this->generator);
            }
            return rtn;
        }
        //! Place n random numbers in the array rtn
        template<size_t n>
        void get (std::array<T, n>& rtn)
        {
            for (size_t i = 0; i < n; ++i) { rtn[i] = this->dist (this->generator); }
        }
        T min (void) { return this->dist.min(); }
        T max (void) { return this->dist.max(); }
    };

    /*!
     * Generate numbers drawn from a random log-normal distribution.
     *
     * \tparam T The type of the random number to be generated
     *
     * \tparam E The pseudo-random number generator engine.
     */
    template <typename T = double, typename E = std::mt19937_64>
    class RandLogNormal
    {
    private:
        //! Random device to provide a seed for the generator
        std::random_device rd{};
        //! Pseudo random number generator engine
        E generator{rd()};
        //! Our distribution
        std::lognormal_distribution<T> dist;
    public:
        //! Default constructor gives RN generator with mean-of-the-log 0 and standard
        //! deviation-of-the-log 1
        RandLogNormal (void)
        {
            typename std::lognormal_distribution<T>::param_type prms (T{0}, T{1});
            this->dist.param (prms);
        }
        //! This constructor gives RN generator with mean-of-the-log 0 and standard
        //! deviation-of-the-log 1. Sets a fixed seed.
        RandLogNormal (unsigned int _seed)
        {
            this->generator.seed (_seed);
            typename std::lognormal_distribution<T>::param_type prms (T{0}, T{1});
            this->dist.param (prms);
        }
        //! This constructor gives RN generator with mean-of-the-log \a mean and
        //! standard deviation \a sigma
        RandLogNormal (T mean, T sigma)
        {
            typename std::lognormal_distribution<T>::param_type prms (mean, sigma);
            this->dist.param (prms);
        }
        //! This constructor gives RN generator with mean-of-the-log \a mean and
        //! standard deviation \a sigma and sets a seed.
        RandLogNormal (T mean, T sigma, unsigned int _seed)
        {
            this->generator.seed (_seed);
            typename std::lognormal_distribution<T>::param_type prms (mean, sigma);
            this->dist.param (prms);
        }
        //! Copy constructor copies the distribution parameters
        RandLogNormal (const RandLogNormal<T>& rng) { this->param (rng.param()); }
        //! Copy assignment operator needs to be explicitly defined
        RandLogNormal& operator= (const RandLogNormal<T>& rng)
        {
            if (&rng == this) { return *this; }
            this->param (rng.param());
            return *this;
        }
        //! Reveal the distribution's param getter
        typename std::lognormal_distribution<T>::param_type param() const { return dist.param(); }
        //! Reveal the distribution's param setter
        void param (const typename std::lognormal_distribution<T>::param_type& prms) { this->dist.param(prms); }
        //! Get 1 random number from the generator
        T get (void) { return this->dist (this->generator); }
        //! Get n random numbers from the generator
        std::vector<T> get (size_t n)
        {
            std::vector<T> rtn (n, T{0});
            for (size_t i = 0; i < n; ++i) {
                rtn[i] = this->dist (this->generator);
            }
            return rtn;
        }
        //! Place n random numbers in the array rtn
        template<size_t n>
        void get (std::array<T, n>& rtn)
        {
            for (size_t i = 0; i < n; ++i) { rtn[i] = this->dist (this->generator); }
        }
        T min (void) { return this->dist.min(); }
        T max (void) { return this->dist.max(); }
    };

    /*!
     * Generate Poisson random numbers in a integer format - valid Ts are short, int,
     * long, long long, unsigned short, unsigned int, unsigned long, or unsigned long
     * long.
     *
     * \tparam T The type of the random number to be generated
     *
     * \tparam E The pseudo-random number generator engine.
     */
    template <typename T = int, typename E = std::mt19937>
    class RandPoisson
    {
    private:
        //! Random device to provide a seed for the generator
        std::random_device rd{};
        //! Pseudo random number generator engine
        E generator{rd()};
        //! Our distribution
        std::poisson_distribution<T> dist;
    public:
        //! Default constructor gives a Poisson random number generator with mean 0.
        RandPoisson (void)
        {
            typename std::poisson_distribution<T>::param_type prms (T{0});
            this->dist.param (prms);
        }
        //! Default constructor gives a Poisson random number generator with mean
        //! 0. Sets fixed seed \a _seed.
        RandPoisson (unsigned int _seed)
        {
            this->generator.seed (_seed);
            typename std::poisson_distribution<T>::param_type prms (T{0});
            this->dist.param (prms);
        }
        //! This constructor gives RN generator with mean \a mean.
        RandPoisson (T mean)
        {
            typename std::poisson_distribution<T>::param_type prms (mean);
            this->dist.param (prms);
        }
        //! This constructor gives RN generator with mean \a mean.
        RandPoisson (T mean, unsigned int _seed)
        {
            this->generator.seed (_seed);
            typename std::poisson_distribution<T>::param_type prms (mean);
            this->dist.param (prms);
        }
        //! Copy constructor copies the distribution parameters
        RandPoisson (const RandPoisson<T>& rng) { this->param (rng.param()); }
        //! Copy assignment operator needs to be explicitly defined
        RandPoisson& operator= (const RandPoisson<T>& rng)
        {
            if (&rng == this) { return *this; }
            this->param (rng.param());
            return *this;
        }
        //! Reveal the distribution's param getter
        typename std::poisson_distribution<T>::param_type param() const { return dist.param(); }
        //! Reveal the distribution's param setter
        void param (const typename std::poisson_distribution<T>::param_type& prms) { this->dist.param(prms); }
        //! Get 1 random number from the generator
        T get (void) { return this->dist (this->generator); }
        //! Get n random numbers from the generator
        std::vector<T> get (size_t n)
        {
            std::vector<T> rtn (n, T{0});
            for (size_t i = 0; i < n; ++i) {
                rtn[i] = this->dist (this->generator);
            }
            return rtn;
        }
        //! Place n random numbers in the array rtn
        template<size_t n>
        void get (std::array<T, n>& rtn)
        {
            for (size_t i = 0; i < n; ++i) { rtn[i] = this->dist (this->generator); }
        }
        //! min wrapper
        T min (void) { return this->dist.min(); }
        //! max wrapper
        T max (void) { return this->dist.max(); }
    };

    //! Enumerated class defining groups of characters, such as AlphaNumericUpperCase,
    //! AlphaNumericLowerCase etc.
    enum class CharGroup
    {
        AlphaNumeric,          // 0-9A-Za-z                   62 chars
        Alpha,                 // A-Za-z                      52 chars
        AlphaNumericUpperCase, // 0123456789ABCDEF... ...XYZ  36 chars
        AlphaNumericLowerCase, // 0123456789abcdef... ...xyz  36 chars
        AlphaUpperCase,        // A-Z                         26 chars
        AlphaLowerCase,        // a-z                         26 chars
        HexUpperCase,          // 0123456789ABCDEF            16 chars
        HexLowerCase,          // 0123456789abcdef            16 chars
        Decimal,               // 0123456789                  10 chars
        BinaryTF,              // TF                           2 chars
        Binary                 // 01                           2 chars
    };

    //! Generate strings of random characters
    class RandString
    {
    public:
        //! Default constructor gives a string generator that generates 8 HexLowerCase characters
        RandString() : length(8) { this->setupRNG(); }
        //! Construct to generate a string of a particular length \a l, in default HexLowerCase format
        RandString(const size_t l) : length(l) { this->setupRNG(); }
        //! Construct with given length \a l and character group \a _cg.
        RandString(const size_t l, const CharGroup& _cg) : length(l), cg(_cg) { this->setupRNG(); }
        //! Deconstructor cleans up memory
        ~RandString()
        {
            if (this->rng != (RandUniform<unsigned short>*)0) {
                delete this->rng;
            }
        }

        //! Get a random string of RandString::length characters chosen from the given
        //! CharGroup RandString::cg
        std::string get() const
        {
            // Initialise a string of the correct length
            std::string s(this->length, ' ');

            for (size_t i = 0; i < this->length; ++i) {

                // Get a random number
                unsigned short rn = this->rng->get();

                // Turn the random number into a character, depending on the CharGroup
                switch (this->cg) {
                case CharGroup::AlphaNumeric:
                {
                    if (rn < 26) {
                        // lower case
                        s[i] = (char)0x61 + (char)rn;
                    } else if (rn > 51) {
                        // numerals
                        s[i] = (char)0x30 + (char)rn - 52;
                    } else {
                        // upper case
                        s[i] = (char)0x41 + (char)rn - 26;
                    }
                    break;
                }
                case CharGroup::Alpha:
                {
                    s[i] = (rn < 26 ? (char)0x61 + (char)rn : (char)0x41 + (char)rn - 26);
                    break;
                }
                case CharGroup::AlphaNumericUpperCase:
                {
                    s[i] = (rn < 26 ? (char)0x41 + (char)rn : (char)0x30 + (char)rn - 26);
                    break;
                }
                case CharGroup::AlphaNumericLowerCase:
                {
                    s[i] = (rn < 26 ? (char)0x61 + (char)rn : (char)0x30 + (char)rn - 26);
                    break;
                }
                case CharGroup::AlphaUpperCase:
                {
                    s[i] = (char)0x41 + (char)rn;
                    break;
                }
                case CharGroup::AlphaLowerCase:
                {
                    s[i] = (char)0x61 + (char)rn;
                    break;
                }
                case CharGroup::HexUpperCase:
                {
                    s[i] = (rn < 10 ? (char)rn + 0x30 : (char)rn + 0x41 - 10);
                    break;
                }
                case CharGroup::HexLowerCase:
                {
                    s[i] = (rn < 10 ? (char)rn + 0x30 : (char)rn + 0x61 - 10);
                    break;
                }
                case CharGroup::Decimal:
                {
                    s[i] = (char)rn + 0x30;
                    break;
                }
                case CharGroup::BinaryTF:
                {
                    s[i] = rn ? 'T' : 'F';
                    break;
                }
                case CharGroup::Binary:
                {
                    s[i] = rn ? '1' : '0';
                    break;
                }
                default: { break; }
                }
            }
            return s;
        }

        //! Get a particular length of string - updates RandString::length first
        std::string get (const size_t l)
        {
            this->length = l;
            return this->get();
        }

        //! Set a new CharGroup
        void setCharGroup (const CharGroup& _cg)
        {
            this->cg = _cg;
            this->setupRNG();
        }

    private:

        //! When CharGroup changes, the rng has to be re-set up.
        void setupRNG()
        {
            // Set rng to generate random numbers in correct range
            if (this->rng != (RandUniform<unsigned short>*)0) {
                delete this->rng;
                this->rng = (RandUniform<unsigned short>*)0;
            }
            this->rng = new RandUniform<unsigned short>(0, this->numChars()-1);
        }

        //! Return the number of characters total in each CharGroup
        unsigned short numChars() const
        {
            unsigned short n = 0;
            switch (this->cg) {
            case CharGroup::AlphaNumeric: { n = 62; break; }
            case CharGroup::Alpha: { n = 52; break; }
            case CharGroup::AlphaNumericUpperCase:
            case CharGroup::AlphaNumericLowerCase: { n = 36; break; }
            case CharGroup::AlphaUpperCase:
            case CharGroup::AlphaLowerCase: { n = 26; break; }
            case CharGroup::HexUpperCase:
            case CharGroup::HexLowerCase: { n = 16; break; }
            case CharGroup::Decimal: { n = 10; break; }
            case CharGroup::BinaryTF:
            case CharGroup::Binary: { n = 2; break; }
            default: { n = 0; break; }
            }
            return n;
        }

        //! The number generator
        RandUniform<unsigned short>* rng = (RandUniform<unsigned short>*)0;

        //! The number of characters to generate
        size_t length;

        //! The group of characters from which to generate a string
        CharGroup cg = CharGroup::HexLowerCase;
    };
}
