/*!
 * This set of template meta programming incantations creates a number_type template,
 * which will set its value to one of three integer values signifying whether it is a
 * resizable 'vector' type, such as std::vector or std::list (value=0), a fixed-size
 * 'vector' type, such as std::array (value=1) OR a scalar (value=2).
 */

#pragma once

#include <deque>
#include <queue>
#include <forward_list>
#include <list>
#include <map>
#include <stack>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>
#include <type_traits>

// morph's hokey vector classes. I don't know how to get their element type at the
// moment, so they're excluded from this number_type code. If I converted VectorN.h to
// derive from std::array? Or add additional number_type::values? 3: Vector3, 4:
// Vector4 etc? Seems a bit crappy and I'd prefer to make a Vector<T,N> class derived
// from/containing a std::array<T,N>
//
//#include "Vector2.h"
//#include "Vector3.h"
//#include "Vector4.h"

// specialize a type for resizable stl containers
namespace is_resizable_vector_impl {
    template <typename T>       struct is_resizable_vector:std::false_type{};
    template <typename... Args> struct is_resizable_vector<std::vector            <Args...>>:std::true_type{};
    template <typename... Args> struct is_resizable_vector<std::deque             <Args...>>:std::true_type{};
    template <typename... Args> struct is_resizable_vector<std::list              <Args...>>:std::true_type{};
    template <typename... Args> struct is_resizable_vector<std::forward_list      <Args...>>:std::true_type{};
    template <typename... Args> struct is_resizable_vector<std::stack             <Args...>>:std::true_type{};
    template <typename... Args> struct is_resizable_vector<std::queue             <Args...>>:std::true_type{};
}

namespace is_fixedsize_vector_impl {
    template <typename T>                struct is_fixedsize_vector:std::false_type{};
    template <typename T, std::size_t N> struct is_fixedsize_vector<std::array<T,N>>        : std::true_type {};
    //template <typename T>                struct is_fixedsize_vector<morph::Vector2<T>>      : std::true_type {};
    //template <typename T>                struct is_fixedsize_vector<morph::Vector3<T>>      : std::true_type {};
    //template <typename T>                struct is_fixedsize_vector<morph::Vector4<T>>      : std::true_type {};
    // Also, what about:
    //template <typename T>                struct is_fixedsize_vector<std::pair<T,T>>      : std::true_type {}; // 2D vector
}

// From the typename T, set a value attribute which says whether T is a scalar (like
// float, double), or whether it is a resizable list-like type (vector, list etc) or
// whether it is a fixed-size list-like type, such as std::array.
template <typename T>
struct number_type {

    static constexpr bool const scalar = std::is_scalar<std::decay_t<T>>::value;
    static constexpr bool const resizable = is_resizable_vector_impl::is_resizable_vector<std::decay_t<T>>::value;
    static constexpr bool const fixedsize = is_fixedsize_vector_impl::is_fixedsize_vector<std::decay_t<T>>::value;

    // value needs to be either of:
    //                                                         0 for default impl (vector-common)
    // 1 scalar == false and resizable == true   => value 0 or 1 for resizable vector
    // 2 scalar == false and fixedsize == true   => value 1    2 for fixed-size vector
    // scalar == true and whatever               => value 2    3 for scalar
    static constexpr int const value = scalar ? 3 : (resizable ? 1 : 2);
};

#if 0
// A 'does it have a const iterator?' helper
template<typename T>
struct has_const_iterator
{
private:
    template<typename C> static char test(typename C::const_iterator*);
    template<typename C> static int  test(...);
public:
    enum { value = sizeof(test<T>(0)) == sizeof(char) };
};
#endif
