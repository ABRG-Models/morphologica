/*!
 * \file
 *
 * Defines number_type, and has some ifdeffed, but potentially useful code.
 *
 * This set of template meta programming incantations creates a number_type template,
 * which will set its value to 1 if its template argument is scalar or 0 if it is not
 * scalar. It would be possible to avoid the use of number_type in the classes which use
 * it (instead using is_scalar directly in those class templates), but it affords some
 * potential flexibility to do it this way.
 *
 * Previously, it set value to one of three integer values signifying whether it is a
 * resizable 'vector' type, such as std::vector or std::list (value=0), a fixed-size
 * 'vector' type, such as std::array (value=1) OR a scalar (value=2).
 */

#pragma once

#include <type_traits>

#if 0 // Required only for more flexible testing of types
// specialize a type for resizable stl containers
#include <vector>
#include <deque>
#include <list>
#include <forward_list>
#include <stack>
#include <queue>
namespace is_resizable_vector_impl {
    template <typename T>       struct is_resizable_vector:std::false_type{};
    template <typename... Args> struct is_resizable_vector<std::vector            <Args...>>:std::true_type{};
    template <typename... Args> struct is_resizable_vector<std::deque             <Args...>>:std::true_type{};
    template <typename... Args> struct is_resizable_vector<std::list              <Args...>>:std::true_type{};
    template <typename... Args> struct is_resizable_vector<std::forward_list      <Args...>>:std::true_type{};
    template <typename... Args> struct is_resizable_vector<std::stack             <Args...>>:std::true_type{};
    template <typename... Args> struct is_resizable_vector<std::queue             <Args...>>:std::true_type{};
}

#include <array>
#include "morph/Vector.h"
namespace is_fixedsize_vector_impl {
    template <typename T>                struct is_fixedsize_vector:std::false_type{};
    template <typename T, std::size_t N> struct is_fixedsize_vector<std::array<T,N>>        : std::true_type {};
    template <typename T, std::size_t N> struct is_fixedsize_vector<morph::Vector<T,N>>     : std::true_type {};
    //template <typename T>                struct is_fixedsize_vector<std::pair<T,T>>      : std::true_type {}; // 2D vector
}
#endif

/*! \brief A class to distinguish between scalars and vectors
 *
 * From the typename T, set a #value attribute which says whether T is a scalar (like
 * float, double), or vector (basically, anything else).
 *
 * I did experiment with code (which is ifdeffed out in the file number_type.h) which
 * would determine whether \a T is a resizable list-like type (vector, list etc) or
 * whether it is a fixed-size list-like type, such as std::array. This was because I
 * erroneously thought I would have to have separate implementations for each. The
 * Ifdeffed code is left for future reference.
 *
 * \tparam T the type to distinguish
 */
template <typename T>
struct number_type {
    //! is_scalar test
    static constexpr bool const scalar = std::is_scalar<std::decay_t<T>>::value;
#if 0 // For a more flexible set of tests:
    static constexpr bool const resizable = is_resizable_vector_impl::is_resizable_vector<std::decay_t<T>>::value;
    static constexpr bool const fixedsize = is_fixedsize_vector_impl::is_fixedsize_vector<std::decay_t<T>>::value;

    // set value from the tests above:
    //                                                         0 for default impl (vector-common)
    // 1 scalar == false and resizable == true   => value 0 or 1 for resizable vector
    // 2 scalar == false and fixedsize == true   => value 1    2 for fixed-size vector
    // scalar == true and whatever               => value 2    3 for scalar
    static constexpr int const value = scalar ? 3 : (resizable ? 1 : 2);
#else
    //! Set value simply from the is_scalar test. 0 for vector, 1 for scalar
    static constexpr int const value = scalar ? 1 : 0;
#endif
};
