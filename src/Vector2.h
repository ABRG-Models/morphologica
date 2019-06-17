/*!
 * A (templated) 2D vector class
 */
#ifndef _VECTOR2_H_
#define _VECTOR2_H_

#include <cmath>
using std::abs;
using std::sqrt;
using std::cos;
using std::sin;

#include <array>
using std::array;

#include <iostream>
using std::cout;
using std::endl;

namespace morph {

    template <class Flt>
    class Vector2
    {
    public:
        /*!
         * The vectors 2 components
         */
        //@{
        alignas(Flt) Flt x;
        alignas(Flt) Flt y;
        //@}

        /*!
         * The threshold outside of which the vector is no longer
         * considered to be a unit vector.
         */
        const Flt unitThresh = 0.001;

        //! Constructor
        Vector2 (void)
            : x(0.0)
            , y(0.0) {}

        //! Constructor
        Vector2 (Flt _x, Flt _y)
            : x(_x)
            , y(_y) {}

        //! Copy constructor
        Vector2 (const Vector2<Flt>& other) {
            this->x = other.x;
            this->y = other.y;
        }

        void output (void) const {
            cout << "Vector2(" << x << "," << y << ")" << endl;
        }

        /*!
         * Renormalize the vector to 1
         */
        void renormalize (void) {
            Flt denom = sqrt (x*x + y*y);
            if (denom != static_cast<Flt>(0.0)) {
                Flt oneovermag = 1.0 / denom;
                this->x *= oneovermag;
                this->y *= oneovermag;
            }
        }

        /*!
         * Test to see if this vector is a unit vector (it doesn't *have* to be)
         */
        bool checkunit (void) {
            bool rtn = true;
            Flt metric = 1.0 - (x*x + y*y);
            if (abs(metric) > morph::Vector2<Flt>::unitThresh) {
                rtn = false;
            }
            return rtn;
        }

        //! Return the length of the vector
        Flt length (void) const {
            Flt len = sqrt (x*x + y*y);
            return len;
        }

        //! Assignment operator
        void operator= (const Vector2<Flt>& other) {
            this->x = other.x;
            this->y = other.y;
        }
        void operator= (const array<Flt, 2>& other) {
            this->x = other[0];
            this->y = other[1];
        }

        //! Comparison operator
        bool operator== (const Vector2<Flt>& other) {
            return (this->x == other.x && this->y == other.y);
        }

        //! Unary negate
        Vector2<Flt> operator- (void) const {
            return Vector2<Flt>(-this->x, -this->y);
        }

        //! Unary not
        bool operator! (void) const {
            return (this->length() == static_cast<Flt>(0.0)) ? true : false;
        }

        //! Scalar multiply.
        //@{
        Vector2<Flt> operator* (const float& f) const {
            return Vector2<Flt>(this->x * static_cast<Flt>(f),
                                this->y * static_cast<Flt>(f));
        }
        Vector2<Flt> operator* (const double& d) const {
            return Vector2<Flt>(this->x * static_cast<Flt>(d),
                                this->y * static_cast<Flt>(d));
        }
        Vector2<Flt> operator* (const int& i) const {
            return Vector2<Flt>(this->x * static_cast<Flt>(i),
                                this->y * static_cast<Flt>(i));
        }
        Vector2<Flt> operator* (const long long int& i) const {
            return Vector2<Flt>(this->x * static_cast<Flt>(i),
                                this->y * static_cast<Flt>(i));
        }
        Vector2<Flt> operator* (const unsigned int& i) const {
            return Vector2<Flt>(this->x * static_cast<Flt>(i),
                                this->y * static_cast<Flt>(i));
        }
        Vector2<Flt> operator* (const long long unsigned int& i) const {
            return Vector2<Flt>(this->x * static_cast<Flt>(i),
                                this->y * static_cast<Flt>(i));
        }
        void operator*= (const float& f) {
            this->x *= static_cast<Flt>(f);
            this->y *= static_cast<Flt>(f);
        }
        void operator*= (const double& d) {
            this->x *= static_cast<Flt>(d);
            this->y *= static_cast<Flt>(d);
        }
        void operator*= (const int& i) {
            this->x *= static_cast<Flt>(i);
            this->y *= static_cast<Flt>(i);
        }
        void operator*= (const long long int& i) {
            this->x *= static_cast<Flt>(i);
            this->y *= static_cast<Flt>(i);
        }
        void operator*= (const unsigned int& i) {
            this->x *= static_cast<Flt>(i);
            this->y *= static_cast<Flt>(i);
        }
        void operator*= (const unsigned long long int& i) {
            this->x *= static_cast<Flt>(i);
            this->y *= static_cast<Flt>(i);
        }
        //@}

        //! Scalar division.
        //@{
        Vector2<Flt> operator/ (const float& f) const {
            return Vector2<Flt>(this->x / static_cast<Flt>(f),
                                this->y / static_cast<Flt>(f));
        }
        Vector2<Flt> operator/ (const double& d) const {
            return Vector2<Flt>(this->x / static_cast<Flt>(d),
                                this->y / static_cast<Flt>(d));
        }
        Vector2<Flt> operator/ (const int& i) const {
            return Vector2<Flt>(this->x / static_cast<Flt>(i),
                                this->y / static_cast<Flt>(i));
        }
        Vector2<Flt> operator/ (const long long int& i) const {
            return Vector2<Flt>(this->x / static_cast<Flt>(i),
                                this->y / static_cast<Flt>(i));
        }
        Vector2<Flt> operator/ (const unsigned int& i) const {
            return Vector2<Flt>(this->x / static_cast<Flt>(i),
                                this->y / static_cast<Flt>(i));
        }
        Vector2<Flt> operator/ (const long long unsigned int& i) const {
            return Vector2<Flt>(this->x / static_cast<Flt>(i),
                                this->y / static_cast<Flt>(i));
        }
        void operator/= (const float& f) {
            this->x /= static_cast<Flt>(f);
            this->y /= static_cast<Flt>(f);
        }
        void operator/= (const double& d) {
            this->x /= static_cast<Flt>(d);
            this->y /= static_cast<Flt>(d);
        }
        void operator/= (const int& i) {
            this->x /= static_cast<Flt>(i);
            this->y /= static_cast<Flt>(i);
        }
        void operator/= (const long long int& i) {
            this->x /= static_cast<Flt>(i);
            this->y /= static_cast<Flt>(i);
        }
        void operator/= (const unsigned int& i) {
            this->x /= static_cast<Flt>(i);
            this->y /= static_cast<Flt>(i);
        }
        void operator/= (const unsigned long long int& i) {
            this->x /= static_cast<Flt>(i);
            this->y /= static_cast<Flt>(i);
        }
        //@}

        //! Vector addition
        //@{
        Vector2<Flt> operator+ (const Vector2<Flt>& v2) const {
            Vector2<Flt> v;
            v.x = this->x + v2.x;
            v.y = this->y + v2.y;
            return v;
        }
        void operator+= (const Vector2<Flt>& v2) {
            this->x += v2.x;
            this->y += v2.y;
        }
        //@}

        //! Vector subtraction
        //@{
        Vector2<Flt> operator- (const Vector2<Flt>& v2) const {
            Vector2<Flt> v;
            v.x = this->x - v2.x;
            v.y = this->y - v2.y;
            return v;
        }
        void operator-= (const Vector2<Flt>& v2) {
            this->x -= v2.x;
            this->y -= v2.y;
        }
        //@}

        //! Scalar addition
        //@{
        Vector2<Flt> operator+ (const float& f) const {
            Vector2<Flt> v;
            v.x = this->x + static_cast<Flt>(f);
            v.y = this->y + static_cast<Flt>(f);
            return v;
        }
        Vector2<Flt> operator+ (const double& d) const {
            Vector2<Flt> v;
            v.x = this->x + static_cast<Flt>(d);
            v.y = this->y + static_cast<Flt>(d);
            return v;
        }
        Vector2<Flt> operator+ (const int& i) const {
            Vector2<Flt> v;
            v.x = this->x + static_cast<Flt>(i);
            v.y = this->y + static_cast<Flt>(i);
            return v;
        }
        Vector2<Flt> operator+ (const long long int& i) const {
            Vector2<Flt> v;
            v.x = this->x + static_cast<Flt>(i);
            v.y = this->y + static_cast<Flt>(i);
            return v;
        }
        Vector2<Flt> operator+ (const unsigned int& i) const {
            Vector2<Flt> v;
            v.x = this->x + static_cast<Flt>(i);
            v.y = this->y + static_cast<Flt>(i);
            return v;
        }
        Vector2<Flt> operator+ (const unsigned long long int& i) const {
            Vector2<Flt> v;
            v.x = this->x + static_cast<Flt>(i);
            v.y = this->y + static_cast<Flt>(i);
            return v;
        }
        void operator+= (const float& f) {
            this->x += static_cast<Flt>(f);
            this->y += static_cast<Flt>(f);
        }
        void operator+= (const double& d) {
            this->x += static_cast<Flt>(d);
            this->y += static_cast<Flt>(d);
        }
        void operator+= (const int& i) {
            this->x += static_cast<Flt>(i);
            this->y += static_cast<Flt>(i);
        }
        void operator+= (const long long int& i) {
            this->x += static_cast<Flt>(i);
            this->y += static_cast<Flt>(i);
        }
        void operator+= (const unsigned int& i) {
            this->x += static_cast<Flt>(i);
            this->y += static_cast<Flt>(i);
        }
        void operator+= (const unsigned long long int& i) {
            this->x += static_cast<Flt>(i);
            this->y += static_cast<Flt>(i);
        }
        //@}

        //! Scalar subtraction
        //@{
        Vector2<Flt> operator- (const float& f) const {
            Vector2<Flt> v;
            v.x = this->x - static_cast<Flt>(f);
            v.y = this->y - static_cast<Flt>(f);
            return v;
        }
        Vector2<Flt> operator- (const double& d) const {
            Vector2<Flt> v;
            v.x = this->x - static_cast<Flt>(d);
            v.y = this->y - static_cast<Flt>(d);
            return v;
        }
        Vector2<Flt> operator- (const int& i) const {
            Vector2<Flt> v;
            v.x = this->x - static_cast<Flt>(i);
            v.y = this->y - static_cast<Flt>(i);
            return v;
        }
        Vector2<Flt> operator- (const long long int& i) const {
            Vector2<Flt> v;
            v.x = this->x - static_cast<Flt>(i);
            v.y = this->y - static_cast<Flt>(i);
            return v;
        }
        Vector2<Flt> operator- (const unsigned int& i) const {
            Vector2<Flt> v;
            v.x = this->x - static_cast<Flt>(i);
            v.y = this->y - static_cast<Flt>(i);
            return v;
        }
        Vector2<Flt> operator- (const unsigned long long int& i) const {
            Vector2<Flt> v;
            v.x = this->x - static_cast<Flt>(i);
            v.y = this->y - static_cast<Flt>(i);
            return v;
        }
        void operator-= (const float& f) {
            this->x -= static_cast<Flt>(f);
            this->y -= static_cast<Flt>(f);
        }
        void operator-= (const double& d) {
            this->x -= static_cast<Flt>(d);
            this->y -= static_cast<Flt>(d);
        }
        void operator-= (const int& i) {
            this->x -= static_cast<Flt>(i);
            this->y -= static_cast<Flt>(i);
        }
        void operator-= (const long long int& i) {
            this->x -= static_cast<Flt>(i);
            this->y -= static_cast<Flt>(i);
        }
        void operator-= (const unsigned int& i) {
            this->x -= static_cast<Flt>(i);
            this->y -= static_cast<Flt>(i);
        }
        void operator-= (const unsigned long long int& i) {
            this->x -= static_cast<Flt>(i);
            this->y -= static_cast<Flt>(i);
        }
        //@}
    };

} // namespace morph

#endif // _VECTOR2_H_
