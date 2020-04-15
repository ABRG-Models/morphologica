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
    // 1 scalar == false and resizable == true   => value 0
    // 2 scalar == false and fixedsize == true   => value 1
    // scalar == true and whatever               => value 2
    static constexpr int const value = scalar ? 2 : (resizable ? 0 : 1);
};
