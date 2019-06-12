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

namespace morph {

    template <class Flt>
    class Vector3
    {
    public:
        Vector3 (void)
            : x(0.0)
            , y(0.0)
            , z(0.0) {}

        Vector3 (Flt _x, Flt _y, Flt _z)
            : x(_x)
            , y(_y)
            , z(_z) {}

        /*!
         * The vectors 3 components
         */
        //@{
        alignas(Flt) Flt x;
        alignas(Flt) Flt y;
        alignas(Flt) Flt z;
        //@}

        void output (void) const {
            cout << "Vector3(" << x << "," << y << "," << z << ")" << endl;
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
         * The threshold outside of which the vector is no longer
         * considered to be a unit vector.
         */
        const Flt unitThresh = 0.001;

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

        //! Unary negate
        Vector3<Flt> operator- (void) const {
            return Vector3<Flt>(-this->x, -this->y, -this->z);
        }

        //! Vector multiply. Cross product.
        Vector3<Flt> operator* (const Vector3<Flt>& v2) const {
            Vector3<Flt> v;
            v.x = this->y * v2.z - this->z * v2.y;
            v.y = this->z * v2.x - this->x * v2.z;
            v.z = this->x * v2.y - this->y * v2.x;
            return v;
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
        //@}

        //! Vector addition
        Vector3<Flt> operator+ (const Vector3<Flt>& v2) const {
            Vector3<Flt> v;
            v.x = this->x + v2.x;
            v.y = this->y + v2.y;
            v.z = this->z + v2.z;
            return v;
        }

        //! Vector subtraction
        Vector3<Flt> operator- (const Vector3<Flt>& v2) const {
            Vector3<Flt> v;
            v.x = this->x - v2.x;
            v.y = this->y - v2.y;
            v.z = this->z - v2.z;
            return v;
        }

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
        //@}
    };

} // namespace morph

#endif // _VECTOR3_H_
