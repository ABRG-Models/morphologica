/*!
 * A (templated) 4D vector class
 */
#ifndef _VECTOR4_H_
#define _VECTOR4_H_

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

#include <string>
using std::string;
#include <sstream>
using std::stringstream;

#include "tools.h"

namespace morph {

    template <class Flt>
    class Vector4
    {
    public:
        /*!
         * The vector's 3 components
         */
        //@{
        alignas(Flt) Flt x;
        alignas(Flt) Flt y;
        alignas(Flt) Flt z;
        //@}

        /*!
         * The threshold outside of which the vector is no longer
         * considered to be a unit vector.
         */
        const Flt unitThresh = 0.001;

        //! Constructor
        Vector4 (void)
            : x(0.0)
            , y(0.0)
            , z(0.0)
            , w(0.0) {}

        //! Constructor
        Vector4 (Flt _x, Flt _y, Flt _z, Flt _w)
            : x(_x)
            , y(_y)
            , z(_z)
            , w(_w) {}

        Vector4 (const array<Flt, 4> v)
            : x(v[0])
            , y(v[1])
            , z(v[2])
            , w(v[3]) {}

        //! Copy constructor
        Vector4 (const Vector4<Flt>& other) {
            this->x = other.x;
            this->y = other.y;
            this->z = other.z;
            this->w = other.w;
        }

        //! Return the vector as an array
        array<Flt, 4> asArray (void) const {
            array<Flt, 4> v = {x, y, z, w};
            return v;
        }

        void output (void) const {
            cout << "Vector4(" << x << "," << y << "," << z << "," << w << ")" << endl;
        }
        string asString (void) const {
            stringstream ss;
            ss << "(" << x << "," << y << "," << z << "," << w << ")";
            return ss.str();
        }

        /*!
         * Renormalize the vector to 1
         */
        void renormalize (void) {
            Flt denom = sqrt (x*x + y*y + z*z + w*w);
            if (denom != static_cast<Flt>(0.0)) {
                Flt oneovermag = 1.0 / denom;
                this->x *= oneovermag;
                this->y *= oneovermag;
                this->z *= oneovermag;
                this->w *= oneovermag;
            }
        }

        /*!
         * Randomize the vector.
         */
        void randomize (void) {
            this->x = Tools::randF<Flt>();
            this->y = Tools::randF<Flt>();
            this->z = Tools::randF<Flt>();
            this->w = Tools::randF<Flt>();
        }

        /*!
         * Test to see if this vector is a unit vector (it doesn't *have* to be)
         */
        bool checkunit (void) {
            bool rtn = true;
            Flt metric = 1.0 - (x*x + y*y + z*z + w*w);
            if (abs(metric) > morph::Vector4<Flt>::unitThresh) {
                rtn = false;
            }
            return rtn;
        }

        //! Return the length of the vector
        Flt length (void) const {
            Flt len = sqrt (x*x + y*y + z*z + w*w);
            return len;
        }

        //! Assignment operator
        void operator= (const Vector4<Flt>& other) {
            this->x = other.x;
            this->y = other.y;
            this->z = other.z;
            this->w = other.w;
        }
        void operator= (const array<Flt, 4>& other) {
            this->x = other[0];
            this->y = other[1];
            this->z = other[2];
            this->w = other[3];
        }

        //! Comparison operator
        bool operator== (const Vector4<Flt>& other) {
            return (this->x == other.x && this->y == other.y
                    && this->z == other.z && this->w == other.w);
        }

        //! Unary negate
        Vector4<Flt> operator- (void) const {
            return Vector4<Flt>(-this->x, -this->y, -this->z, -this->w);
        }

        //! Unary not
        bool operator! (void) const {
            return (this->length() == static_cast<Flt>(0.0)) ? true : false;
        }

        //! Vector multiply. Cross product of this with another vector v2.
#if 0 // FIXME: Can you compute a cross product of a 4 vector?
        Vector4<Flt> operator* (const Vector4<Flt>& v2) const {
            Vector4<Flt> v;
            v.x = this->y * v2.z - this->z * v2.y;
            v.y = this->z * v2.x - this->x * v2.z;
            v.z = this->x * v2.y - this->y * v2.x;
            v.w = ?;
            return v;
        }
        void operator*= (const Vector4<Flt>& v2) {
            Vector4<Flt> v;
            v.x = this->y * v2.z - this->z * v2.y;
            v.y = this->z * v2.x - this->x * v2.z;
            v.z = this->x * v2.y - this->y * v2.x;
            this->x = v.x;
            this->y = v.y;
            this->z = v.z;
            this->w = ?;
        }
#endif
        //! Scalar product of this with another vector, v2.
        Flt dot (const Vector4<Flt>& v2) const {
            Flt rtn = this->x * v2.x + this->y * v2.y + this->z * v2.z + this->w * v2.w;
            return rtn;
        }

        //! Scalar multiply.
        //@{
        Vector4<Flt> operator* (const float& f) const {
            return Vector4<Flt>(this->x * static_cast<Flt>(f),
                                this->y * static_cast<Flt>(f),
                                this->z * static_cast<Flt>(f),
                                this->w * static_cast<Flt>(f));
        }
        Vector4<Flt> operator* (const double& d) const {
            return Vector4<Flt>(this->x * static_cast<Flt>(d),
                                this->y * static_cast<Flt>(d),
                                this->z * static_cast<Flt>(d),
                                this->w * static_cast<Flt>(d));
        }
        Vector4<Flt> operator* (const int& i) const {
            return Vector4<Flt>(this->x * static_cast<Flt>(i),
                                this->y * static_cast<Flt>(i),
                                this->z * static_cast<Flt>(i),
                                this->w * static_cast<Flt>(i));
        }
        Vector4<Flt> operator* (const long long int& i) const {
            return Vector4<Flt>(this->x * static_cast<Flt>(i),
                                this->y * static_cast<Flt>(i),
                                this->z * static_cast<Flt>(i),
                                this->w * static_cast<Flt>(i));
        }
        Vector4<Flt> operator* (const unsigned int& i) const {
            return Vector4<Flt>(this->x * static_cast<Flt>(i),
                                this->y * static_cast<Flt>(i),
                                this->z * static_cast<Flt>(i),
                                this->w * static_cast<Flt>(i));
        }
        Vector4<Flt> operator* (const long long unsigned int& i) const {
            return Vector4<Flt>(this->x * static_cast<Flt>(i),
                                this->y * static_cast<Flt>(i),
                                this->z * static_cast<Flt>(i),
                                this->w * static_cast<Flt>(i));
        }
        void operator*= (const float& f) {
            this->x *= static_cast<Flt>(f);
            this->y *= static_cast<Flt>(f);
            this->z *= static_cast<Flt>(f);
            this->w *= static_cast<Flt>(f);
        }
        void operator*= (const double& d) {
            this->x *= static_cast<Flt>(d);
            this->y *= static_cast<Flt>(d);
            this->z *= static_cast<Flt>(d);
            this->w *= static_cast<Flt>(d);
        }
        void operator*= (const int& i) {
            this->x *= static_cast<Flt>(i);
            this->y *= static_cast<Flt>(i);
            this->z *= static_cast<Flt>(i);
            this->w *= static_cast<Flt>(i);
        }
        void operator*= (const long long int& i) {
            this->x *= static_cast<Flt>(i);
            this->y *= static_cast<Flt>(i);
            this->z *= static_cast<Flt>(i);
            this->w *= static_cast<Flt>(i);
        }
        void operator*= (const unsigned int& i) {
            this->x *= static_cast<Flt>(i);
            this->y *= static_cast<Flt>(i);
            this->z *= static_cast<Flt>(i);
            this->w *= static_cast<Flt>(i);
        }
        void operator*= (const unsigned long long int& i) {
            this->x *= static_cast<Flt>(i);
            this->y *= static_cast<Flt>(i);
            this->z *= static_cast<Flt>(i);
            this->w *= static_cast<Flt>(i);
        }
        //@}

        //! Scalar division.
        //@{
        Vector4<Flt> operator/ (const float& f) const {
            return Vector4<Flt>(this->x / static_cast<Flt>(f),
                                this->y / static_cast<Flt>(f),
                                this->z / static_cast<Flt>(f),
                                this->w / static_cast<Flt>(f));
        }
        Vector4<Flt> operator/ (const double& d) const {
            return Vector4<Flt>(this->x / static_cast<Flt>(d),
                                this->y / static_cast<Flt>(d),
                                this->z / static_cast<Flt>(d),
                                this->w / static_cast<Flt>(d));
        }
        Vector4<Flt> operator/ (const int& i) const {
            return Vector4<Flt>(this->x / static_cast<Flt>(i),
                                this->y / static_cast<Flt>(i),
                                this->z / static_cast<Flt>(i),
                                this->w / static_cast<Flt>(i));
        }
        Vector4<Flt> operator/ (const long long int& i) const {
            return Vector4<Flt>(this->x / static_cast<Flt>(i),
                                this->y / static_cast<Flt>(i),
                                this->z / static_cast<Flt>(i),
                                this->w / static_cast<Flt>(i));
        }
        Vector4<Flt> operator/ (const unsigned int& i) const {
            return Vector4<Flt>(this->x / static_cast<Flt>(i),
                                this->y / static_cast<Flt>(i),
                                this->z / static_cast<Flt>(i),
                                this->w / static_cast<Flt>(i));
        }
        Vector4<Flt> operator/ (const long long unsigned int& i) const {
            return Vector4<Flt>(this->x / static_cast<Flt>(i),
                                this->y / static_cast<Flt>(i),
                                this->z / static_cast<Flt>(i),
                                this->w / static_cast<Flt>(i));
        }
        void operator/= (const float& f) {
            this->x /= static_cast<Flt>(f);
            this->y /= static_cast<Flt>(f);
            this->z /= static_cast<Flt>(f);
            this->w /= static_cast<Flt>(f);
        }
        void operator/= (const double& d) {
            this->x /= static_cast<Flt>(d);
            this->y /= static_cast<Flt>(d);
            this->z /= static_cast<Flt>(d);
            this->w /= static_cast<Flt>(d);
        }
        void operator/= (const int& i) {
            this->x /= static_cast<Flt>(i);
            this->y /= static_cast<Flt>(i);
            this->z /= static_cast<Flt>(i);
            this->w /= static_cast<Flt>(i);
        }
        void operator/= (const long long int& i) {
            this->x /= static_cast<Flt>(i);
            this->y /= static_cast<Flt>(i);
            this->z /= static_cast<Flt>(i);
            this->w /= static_cast<Flt>(i);
        }
        void operator/= (const unsigned int& i) {
            this->x /= static_cast<Flt>(i);
            this->y /= static_cast<Flt>(i);
            this->z /= static_cast<Flt>(i);
            this->w /= static_cast<Flt>(i);
        }
        void operator/= (const unsigned long long int& i) {
            this->x /= static_cast<Flt>(i);
            this->y /= static_cast<Flt>(i);
            this->z /= static_cast<Flt>(i);
            this->w /= static_cast<Flt>(i);
        }
        //@}

        //! Vector addition
        //@{
        Vector4<Flt> operator+ (const Vector4<Flt>& v2) const {
            Vector4<Flt> v;
            v.x = this->x + v2.x;
            v.y = this->y + v2.y;
            v.z = this->z + v2.z;
            v.w = this->w + v2.w;
            return v;
        }
        void operator+= (const Vector4<Flt>& v2) {
            this->x += v2.x;
            this->y += v2.y;
            this->z += v2.z;
            this->w += v2.w;
        }
        //@}

        //! Vector subtraction
        //@{
        Vector4<Flt> operator- (const Vector4<Flt>& v2) const {
            Vector4<Flt> v;
            v.x = this->x - v2.x;
            v.y = this->y - v2.y;
            v.z = this->z - v2.z;
            v.w = this->w - v2.w;
            return v;
        }
        void operator-= (const Vector4<Flt>& v2) {
            this->x -= v2.x;
            this->y -= v2.y;
            this->z -= v2.z;
            this->w -= v2.w;
        }
        //@}

        //! Scalar addition
        //@{
        Vector4<Flt> operator+ (const float& f) const {
            Vector4<Flt> v;
            v.x = this->x + static_cast<Flt>(f);
            v.y = this->y + static_cast<Flt>(f);
            v.z = this->z + static_cast<Flt>(f);
            v.w = this->w + static_cast<Flt>(f);
            return v;
        }
        Vector4<Flt> operator+ (const double& d) const {
            Vector4<Flt> v;
            v.x = this->x + static_cast<Flt>(d);
            v.y = this->y + static_cast<Flt>(d);
            v.z = this->z + static_cast<Flt>(d);
            v.w = this->w + static_cast<Flt>(d);
            return v;
        }
        Vector4<Flt> operator+ (const int& i) const {
            Vector4<Flt> v;
            v.x = this->x + static_cast<Flt>(i);
            v.y = this->y + static_cast<Flt>(i);
            v.z = this->z + static_cast<Flt>(i);
            v.w = this->w + static_cast<Flt>(i);
            return v;
        }
        Vector4<Flt> operator+ (const long long int& i) const {
            Vector4<Flt> v;
            v.x = this->x + static_cast<Flt>(i);
            v.y = this->y + static_cast<Flt>(i);
            v.z = this->z + static_cast<Flt>(i);
            v.w = this->w + static_cast<Flt>(i);
            return v;
        }
        Vector4<Flt> operator+ (const unsigned int& i) const {
            Vector4<Flt> v;
            v.x = this->x + static_cast<Flt>(i);
            v.y = this->y + static_cast<Flt>(i);
            v.z = this->z + static_cast<Flt>(i);
            v.w = this->w + static_cast<Flt>(i);
            return v;
        }
        Vector4<Flt> operator+ (const unsigned long long int& i) const {
            Vector4<Flt> v;
            v.x = this->x + static_cast<Flt>(i);
            v.y = this->y + static_cast<Flt>(i);
            v.z = this->z + static_cast<Flt>(i);
            v.w = this->w + static_cast<Flt>(i);
            return v;
        }
        void operator+= (const float& f) {
            this->x += static_cast<Flt>(f);
            this->y += static_cast<Flt>(f);
            this->z += static_cast<Flt>(f);
            this->w += static_cast<Flt>(f);
        }
        void operator+= (const double& d) {
            this->x += static_cast<Flt>(d);
            this->y += static_cast<Flt>(d);
            this->z += static_cast<Flt>(d);
            this->w += static_cast<Flt>(d);
        }
        void operator+= (const int& i) {
            this->x += static_cast<Flt>(i);
            this->y += static_cast<Flt>(i);
            this->z += static_cast<Flt>(i);
            this->w += static_cast<Flt>(i);
        }
        void operator+= (const long long int& i) {
            this->x += static_cast<Flt>(i);
            this->y += static_cast<Flt>(i);
            this->z += static_cast<Flt>(i);
            this->w += static_cast<Flt>(i);
        }
        void operator+= (const unsigned int& i) {
            this->x += static_cast<Flt>(i);
            this->y += static_cast<Flt>(i);
            this->z += static_cast<Flt>(i);
            this->w += static_cast<Flt>(i);
        }
        void operator+= (const unsigned long long int& i) {
            this->x += static_cast<Flt>(i);
            this->y += static_cast<Flt>(i);
            this->z += static_cast<Flt>(i);
            this->w += static_cast<Flt>(i);
        }
        //@}

        //! Scalar subtraction
        //@{
        Vector4<Flt> operator- (const float& f) const {
            Vector4<Flt> v;
            v.x = this->x - static_cast<Flt>(f);
            v.y = this->y - static_cast<Flt>(f);
            v.z = this->z - static_cast<Flt>(f);
            v.w = this->w - static_cast<Flt>(f);
            return v;
        }
        Vector4<Flt> operator- (const double& d) const {
            Vector4<Flt> v;
            v.x = this->x - static_cast<Flt>(d);
            v.y = this->y - static_cast<Flt>(d);
            v.z = this->z - static_cast<Flt>(d);
            v.w = this->w - static_cast<Flt>(d);
            return v;
        }
        Vector4<Flt> operator- (const int& i) const {
            Vector4<Flt> v;
            v.x = this->x - static_cast<Flt>(i);
            v.y = this->y - static_cast<Flt>(i);
            v.z = this->z - static_cast<Flt>(i);
            v.w = this->w - static_cast<Flt>(i);
            return v;
        }
        Vector4<Flt> operator- (const long long int& i) const {
            Vector4<Flt> v;
            v.x = this->x - static_cast<Flt>(i);
            v.y = this->y - static_cast<Flt>(i);
            v.z = this->z - static_cast<Flt>(i);
            v.w = this->w - static_cast<Flt>(i);
            return v;
        }
        Vector4<Flt> operator- (const unsigned int& i) const {
            Vector4<Flt> v;
            v.x = this->x - static_cast<Flt>(i);
            v.y = this->y - static_cast<Flt>(i);
            v.z = this->z - static_cast<Flt>(i);
            v.w = this->w - static_cast<Flt>(i);
            return v;
        }
        Vector4<Flt> operator- (const unsigned long long int& i) const {
            Vector4<Flt> v;
            v.x = this->x - static_cast<Flt>(i);
            v.y = this->y - static_cast<Flt>(i);
            v.z = this->z - static_cast<Flt>(i);
            v.w = this->w - static_cast<Flt>(i);
            return v;
        }
        void operator-= (const float& f) {
            this->x -= static_cast<Flt>(f);
            this->y -= static_cast<Flt>(f);
            this->z -= static_cast<Flt>(f);
            this->w -= static_cast<Flt>(f);
        }
        void operator-= (const double& d) {
            this->x -= static_cast<Flt>(d);
            this->y -= static_cast<Flt>(d);
            this->z -= static_cast<Flt>(d);
            this->w -= static_cast<Flt>(d);
        }
        void operator-= (const int& i) {
            this->x -= static_cast<Flt>(i);
            this->y -= static_cast<Flt>(i);
            this->z -= static_cast<Flt>(i);
            this->w -= static_cast<Flt>(i);
        }
        void operator-= (const long long int& i) {
            this->x -= static_cast<Flt>(i);
            this->y -= static_cast<Flt>(i);
            this->z -= static_cast<Flt>(i);
            this->w -= static_cast<Flt>(i);
        }
        void operator-= (const unsigned int& i) {
            this->x -= static_cast<Flt>(i);
            this->y -= static_cast<Flt>(i);
            this->z -= static_cast<Flt>(i);
            this->w -= static_cast<Flt>(i);
        }
        void operator-= (const unsigned long long int& i) {
            this->x -= static_cast<Flt>(i);
            this->y -= static_cast<Flt>(i);
            this->z -= static_cast<Flt>(i);
            this->w -= static_cast<Flt>(i);
        }
        //@}
    };

} // namespace morph

#endif // _VECTOR4_H_
