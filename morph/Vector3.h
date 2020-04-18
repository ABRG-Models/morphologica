/*!
 * A (templated) 3D vector class
 */
#ifndef _VECTOR3_H_
#define _VECTOR3_H_

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

    template <typename Flt>
    class Vector3
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
        Vector3 (void)
            : x(0.0)
            , y(0.0)
            , z(0.0) {}

        //! Constructor
        Vector3 (Flt _x, Flt _y, Flt _z)
            : x(_x)
            , y(_y)
            , z(_z) {}

        Vector3 (const array<Flt, 3> v)
            : x(v[0])
            , y(v[1])
            , z(v[2]) {}

        //! Construct from an array of *four* floats, assuming that this is a 4d vector
        //! of the sort used with transfomration matrix and that v[3] is 'w', and can be
        //! discarded.
        Vector3 (const array<Flt, 4> v)
            : x(v[0])
            , y(v[1])
            , z(v[2]) {}

        //! Copy constructor
        Vector3 (const Vector3<Flt>& other) {
            this->x = other.x;
            this->y = other.y;
            this->z = other.z;
        }

        //! Access to members using the square brackets
        Flt operator[] (int idx) {
            Flt rtn = this->x;
            rtn = idx == 1 ? this->y : rtn;
            rtn = idx == 2 ? this->z : rtn;
            return rtn;
        }

        //! To help MathAlgo code (size() and data() methods in common with
        //! std::vector, etc)
        size_t size() const noexcept {
            return 3;
        }

        //! To help MathAlgo code (size() and data() methods in common with
        //! std::vector, etc)
        Flt* data() {
            return &this->x;
        }

        //! Return the vector as an array
        array<Flt, 3> asArray (void) const {
            array<Flt, 3> v = {x, y, z};
            return v;
        }

        void output (void) const {
            cout << "Vector3(" << x << "," << y << "," << z << ")" << endl;
        }
        string asString (void) const {
            stringstream ss;
            ss << "(" << x << "," << y << "," << z << ")";
            return ss.str();
        }

        /*!
         * Renormalize the vector to 1
         */
        void renormalize (void) {
            Flt denom = sqrt (x*x + y*y + z*z);
            if (denom != static_cast<Flt>(0.0)) {
                Flt oneovermag = 1.0 / denom;
                this->x *= oneovermag;
                this->y *= oneovermag;
                this->z *= oneovermag;
            }
        }

        /*!
         * Randomize the vector.
         */
        void randomize (void) {
            this->x = Tools::randF<Flt>();
            this->y = Tools::randF<Flt>();
            this->z = Tools::randF<Flt>();
        }

        /*!
         * Test to see if this vector is a unit vector (it doesn't *have* to be)
         */
        bool checkunit (void) {
            bool rtn = true;
            Flt metric = 1.0 - (x*x + y*y + z*z);
            if (abs(metric) > morph::Vector3<Flt>::unitThresh) {
                rtn = false;
            }
            return rtn;
        }

        //! Return the length of the vector
        Flt length (void) const {
            Flt len = sqrt (x*x + y*y + z*z);
            return len;
        }

        //! Assignment operator
        void operator= (const Vector3<Flt>& other) {
            this->x = other.x;
            this->y = other.y;
            this->z = other.z;
        }
        void operator= (const array<Flt, 3>& other) {
            this->x = other[0];
            this->y = other[1];
            this->z = other[2];
        }
        //! Special assignment for array of 4, in form [x,y,z,w]. w is discarded.
        void operator= (const array<Flt, 4>& other) {
            this->x = other[0];
            this->y = other[1];
            this->z = other[2];
        }

        //! Comparison operator
        bool operator== (const Vector3<Flt>& other) {
            return (this->x == other.x && this->y == other.y && this->z == other.z);
        }

        //! Unary negate
        Vector3<Flt> operator- (void) const {
            return Vector3<Flt>(-this->x, -this->y, -this->z);
        }

        //! Unary not
        bool operator! (void) const {
            return (this->length() == static_cast<Flt>(0.0)) ? true : false;
        }

        //! Vector multiply. Cross product of this with another vector v2.
        Vector3<Flt> operator* (const Vector3<Flt>& v2) const {
            Vector3<Flt> v;
            v.x = this->y * v2.z - this->z * v2.y;
            v.y = this->z * v2.x - this->x * v2.z;
            v.z = this->x * v2.y - this->y * v2.x;
            return v;
        }
        void operator*= (const Vector3<Flt>& v2) {
            Vector3<Flt> v;
            v.x = this->y * v2.z - this->z * v2.y;
            v.y = this->z * v2.x - this->x * v2.z;
            v.z = this->x * v2.y - this->y * v2.x;
            this->x = v.x;
            this->y = v.y;
            this->z = v.z;
        }

        //! Scalar product of this with another vector, v2.
        Flt dot (const Vector3<Flt>& v2) const {
            Flt rtn = this->x * v2.x + this->y * v2.y + this->z * v2.z;
            return rtn;
        }

        //! Scalar multiply.
        //@{
        Vector3<Flt> operator* (const float& f) const {
            return Vector3<Flt>(this->x * static_cast<Flt>(f),
                                this->y * static_cast<Flt>(f),
                                this->z * static_cast<Flt>(f));
        }
        Vector3<Flt> operator* (const double& d) const {
            return Vector3<Flt>(this->x * static_cast<Flt>(d),
                                this->y * static_cast<Flt>(d),
                                this->z * static_cast<Flt>(d));
        }
        Vector3<Flt> operator* (const int& i) const {
            return Vector3<Flt>(this->x * static_cast<Flt>(i),
                                this->y * static_cast<Flt>(i),
                                this->z * static_cast<Flt>(i));
        }
        Vector3<Flt> operator* (const long long int& i) const {
            return Vector3<Flt>(this->x * static_cast<Flt>(i),
                                this->y * static_cast<Flt>(i),
                                this->z * static_cast<Flt>(i));
        }
        Vector3<Flt> operator* (const unsigned int& i) const {
            return Vector3<Flt>(this->x * static_cast<Flt>(i),
                                this->y * static_cast<Flt>(i),
                                this->z * static_cast<Flt>(i));
        }
        Vector3<Flt> operator* (const long long unsigned int& i) const {
            return Vector3<Flt>(this->x * static_cast<Flt>(i),
                                this->y * static_cast<Flt>(i),
                                this->z * static_cast<Flt>(i));
        }
        void operator*= (const float& f) {
            this->x *= static_cast<Flt>(f);
            this->y *= static_cast<Flt>(f);
            this->z *= static_cast<Flt>(f);
        }
        void operator*= (const double& d) {
            this->x *= static_cast<Flt>(d);
            this->y *= static_cast<Flt>(d);
            this->z *= static_cast<Flt>(d);
        }
        void operator*= (const int& i) {
            this->x *= static_cast<Flt>(i);
            this->y *= static_cast<Flt>(i);
            this->z *= static_cast<Flt>(i);
        }
        void operator*= (const long long int& i) {
            this->x *= static_cast<Flt>(i);
            this->y *= static_cast<Flt>(i);
            this->z *= static_cast<Flt>(i);
        }
        void operator*= (const unsigned int& i) {
            this->x *= static_cast<Flt>(i);
            this->y *= static_cast<Flt>(i);
            this->z *= static_cast<Flt>(i);
        }
        void operator*= (const unsigned long long int& i) {
            this->x *= static_cast<Flt>(i);
            this->y *= static_cast<Flt>(i);
            this->z *= static_cast<Flt>(i);
        }
        //@}

        //! Scalar division.
        //@{
        Vector3<Flt> operator/ (const float& f) const {
            return Vector3<Flt>(this->x / static_cast<Flt>(f),
                                this->y / static_cast<Flt>(f),
                                this->z / static_cast<Flt>(f));
        }
        Vector3<Flt> operator/ (const double& d) const {
            return Vector3<Flt>(this->x / static_cast<Flt>(d),
                                this->y / static_cast<Flt>(d),
                                this->z / static_cast<Flt>(d));
        }
        Vector3<Flt> operator/ (const int& i) const {
            return Vector3<Flt>(this->x / static_cast<Flt>(i),
                                this->y / static_cast<Flt>(i),
                                this->z / static_cast<Flt>(i));
        }
        Vector3<Flt> operator/ (const long long int& i) const {
            return Vector3<Flt>(this->x / static_cast<Flt>(i),
                                this->y / static_cast<Flt>(i),
                                this->z / static_cast<Flt>(i));
        }
        Vector3<Flt> operator/ (const unsigned int& i) const {
            return Vector3<Flt>(this->x / static_cast<Flt>(i),
                                this->y / static_cast<Flt>(i),
                                this->z / static_cast<Flt>(i));
        }
        Vector3<Flt> operator/ (const long long unsigned int& i) const {
            return Vector3<Flt>(this->x / static_cast<Flt>(i),
                                this->y / static_cast<Flt>(i),
                                this->z / static_cast<Flt>(i));
        }
        void operator/= (const float& f) {
            this->x /= static_cast<Flt>(f);
            this->y /= static_cast<Flt>(f);
            this->z /= static_cast<Flt>(f);
        }
        void operator/= (const double& d) {
            this->x /= static_cast<Flt>(d);
            this->y /= static_cast<Flt>(d);
            this->z /= static_cast<Flt>(d);
        }
        void operator/= (const int& i) {
            this->x /= static_cast<Flt>(i);
            this->y /= static_cast<Flt>(i);
            this->z /= static_cast<Flt>(i);
        }
        void operator/= (const long long int& i) {
            this->x /= static_cast<Flt>(i);
            this->y /= static_cast<Flt>(i);
            this->z /= static_cast<Flt>(i);
        }
        void operator/= (const unsigned int& i) {
            this->x /= static_cast<Flt>(i);
            this->y /= static_cast<Flt>(i);
            this->z /= static_cast<Flt>(i);
        }
        void operator/= (const unsigned long long int& i) {
            this->x /= static_cast<Flt>(i);
            this->y /= static_cast<Flt>(i);
            this->z /= static_cast<Flt>(i);
        }
        //@}

        //! Vector addition
        //@{
        Vector3<Flt> operator+ (const Vector3<Flt>& v2) const {
            Vector3<Flt> v;
            v.x = this->x + v2.x;
            v.y = this->y + v2.y;
            v.z = this->z + v2.z;
            return v;
        }
        void operator+= (const Vector3<Flt>& v2) {
            this->x += v2.x;
            this->y += v2.y;
            this->z += v2.z;
        }
        //@}

        //! Vector subtraction
        //@{
        Vector3<Flt> operator- (const Vector3<Flt>& v2) const {
            Vector3<Flt> v;
            v.x = this->x - v2.x;
            v.y = this->y - v2.y;
            v.z = this->z - v2.z;
            return v;
        }
        void operator-= (const Vector3<Flt>& v2) {
            this->x -= v2.x;
            this->y -= v2.y;
            this->z -= v2.z;
        }
        //@}

        //! Scalar addition
        //@{
        Vector3<Flt> operator+ (const float& f) const {
            Vector3<Flt> v;
            v.x = this->x + static_cast<Flt>(f);
            v.y = this->y + static_cast<Flt>(f);
            v.z = this->z + static_cast<Flt>(f);
            return v;
        }
        Vector3<Flt> operator+ (const double& d) const {
            Vector3<Flt> v;
            v.x = this->x + static_cast<Flt>(d);
            v.y = this->y + static_cast<Flt>(d);
            v.z = this->z + static_cast<Flt>(d);
            return v;
        }
        Vector3<Flt> operator+ (const int& i) const {
            Vector3<Flt> v;
            v.x = this->x + static_cast<Flt>(i);
            v.y = this->y + static_cast<Flt>(i);
            v.z = this->z + static_cast<Flt>(i);
            return v;
        }
        Vector3<Flt> operator+ (const long long int& i) const {
            Vector3<Flt> v;
            v.x = this->x + static_cast<Flt>(i);
            v.y = this->y + static_cast<Flt>(i);
            v.z = this->z + static_cast<Flt>(i);
            return v;
        }
        Vector3<Flt> operator+ (const unsigned int& i) const {
            Vector3<Flt> v;
            v.x = this->x + static_cast<Flt>(i);
            v.y = this->y + static_cast<Flt>(i);
            v.z = this->z + static_cast<Flt>(i);
            return v;
        }
        Vector3<Flt> operator+ (const unsigned long long int& i) const {
            Vector3<Flt> v;
            v.x = this->x + static_cast<Flt>(i);
            v.y = this->y + static_cast<Flt>(i);
            v.z = this->z + static_cast<Flt>(i);
            return v;
        }
        void operator+= (const float& f) {
            this->x += static_cast<Flt>(f);
            this->y += static_cast<Flt>(f);
            this->z += static_cast<Flt>(f);
        }
        void operator+= (const double& d) {
            this->x += static_cast<Flt>(d);
            this->y += static_cast<Flt>(d);
            this->z += static_cast<Flt>(d);
        }
        void operator+= (const int& i) {
            this->x += static_cast<Flt>(i);
            this->y += static_cast<Flt>(i);
            this->z += static_cast<Flt>(i);
        }
        void operator+= (const long long int& i) {
            this->x += static_cast<Flt>(i);
            this->y += static_cast<Flt>(i);
            this->z += static_cast<Flt>(i);
        }
        void operator+= (const unsigned int& i) {
            this->x += static_cast<Flt>(i);
            this->y += static_cast<Flt>(i);
            this->z += static_cast<Flt>(i);
        }
        void operator+= (const unsigned long long int& i) {
            this->x += static_cast<Flt>(i);
            this->y += static_cast<Flt>(i);
            this->z += static_cast<Flt>(i);
        }
        //@}

        //! Scalar subtraction
        //@{
        Vector3<Flt> operator- (const float& f) const {
            Vector3<Flt> v;
            v.x = this->x - static_cast<Flt>(f);
            v.y = this->y - static_cast<Flt>(f);
            v.z = this->z - static_cast<Flt>(f);
            return v;
        }
        Vector3<Flt> operator- (const double& d) const {
            Vector3<Flt> v;
            v.x = this->x - static_cast<Flt>(d);
            v.y = this->y - static_cast<Flt>(d);
            v.z = this->z - static_cast<Flt>(d);
            return v;
        }
        Vector3<Flt> operator- (const int& i) const {
            Vector3<Flt> v;
            v.x = this->x - static_cast<Flt>(i);
            v.y = this->y - static_cast<Flt>(i);
            v.z = this->z - static_cast<Flt>(i);
            return v;
        }
        Vector3<Flt> operator- (const long long int& i) const {
            Vector3<Flt> v;
            v.x = this->x - static_cast<Flt>(i);
            v.y = this->y - static_cast<Flt>(i);
            v.z = this->z - static_cast<Flt>(i);
            return v;
        }
        Vector3<Flt> operator- (const unsigned int& i) const {
            Vector3<Flt> v;
            v.x = this->x - static_cast<Flt>(i);
            v.y = this->y - static_cast<Flt>(i);
            v.z = this->z - static_cast<Flt>(i);
            return v;
        }
        Vector3<Flt> operator- (const unsigned long long int& i) const {
            Vector3<Flt> v;
            v.x = this->x - static_cast<Flt>(i);
            v.y = this->y - static_cast<Flt>(i);
            v.z = this->z - static_cast<Flt>(i);
            return v;
        }
        void operator-= (const float& f) {
            this->x -= static_cast<Flt>(f);
            this->y -= static_cast<Flt>(f);
            this->z -= static_cast<Flt>(f);
        }
        void operator-= (const double& d) {
            this->x -= static_cast<Flt>(d);
            this->y -= static_cast<Flt>(d);
            this->z -= static_cast<Flt>(d);
        }
        void operator-= (const int& i) {
            this->x -= static_cast<Flt>(i);
            this->y -= static_cast<Flt>(i);
            this->z -= static_cast<Flt>(i);
        }
        void operator-= (const long long int& i) {
            this->x -= static_cast<Flt>(i);
            this->y -= static_cast<Flt>(i);
            this->z -= static_cast<Flt>(i);
        }
        void operator-= (const unsigned int& i) {
            this->x -= static_cast<Flt>(i);
            this->y -= static_cast<Flt>(i);
            this->z -= static_cast<Flt>(i);
        }
        void operator-= (const unsigned long long int& i) {
            this->x -= static_cast<Flt>(i);
            this->y -= static_cast<Flt>(i);
            this->z -= static_cast<Flt>(i);
        }
        //@}
    };

} // namespace morph

#endif // _VECTOR3_H_
