/*!
 * \file
 *
 * Expression SFINAE template incantations.
 *
 * This file contains numerous classes which can be used to test for features in class
 * types. It's useful for if constexpr () tests, and was initially
 * written/collected/adapted from internet examples for morph::Winder.
 *
 * \author Seb James
 * \date May 2020
 */

#pragma once
#include <type_traits>

namespace morph {

    //! Expression SFINAE approach to testing for possibility of a-b. Could also make a
    //! class which used std::is_arithmetic here.
    template<typename T>
    class has_subtraction
    {
	template<typename U> static auto test(const U& a, const U& b) -> decltype(a-b, std::true_type());
	template<typename> static std::false_type test(...);
    public:
	static constexpr bool value = std::is_same<decltype(test<T>(T{}, T{})),std::true_type>::value;
    };

    //! Expression SFINAE approach to testing for possibility of a+b.
    template<typename T>
    class has_addition
    {
	template<typename U> static auto test(const U& a, const U& b) -> decltype(a+b, std::true_type());
	template<typename> static std::false_type test(...);
    public:
	static constexpr bool value = std::is_same<decltype(test<T>(T{}, T{})),std::true_type>::value;
    };

    //! An expression SFINAE approach to testing for x() and y() methods
    template<typename T>
    class has_xy_methods
    {
        // See the right arrow?                    v--- there it is. It's a different function declaration syntax
	template<typename U> static auto test(int) -> decltype(std::declval<U>().x() == 1
                                                               && std::declval<U>().y() == 1, std::true_type());
	template<typename> static std::false_type test(...); // This uses the more typical syntax for fn declaration
    public:
	static constexpr bool value = std::is_same<decltype(test<T>(0)), std::true_type>::value;
    };

    //! Expression SFINAE approach to testing for resize(size_t) method. Can be used to
    //! distinguish std::array from std::vector.
    template<typename T>
    class has_resize_method
    {
        // in decltype: If the expression preceding the comma operator is valid, expression post-comma is processed by decltype() and its type returned.
	template<typename U> static auto test(int sz) -> decltype(std::declval<U>().resize(sz), std::true_type());
	template<typename> static std::false_type test(...);
    public:
	static constexpr bool value = std::is_same< decltype(test<T>(2)), std::true_type >::value;
    };

    //! Expression SFINAE approach to testing for x and y member attributes. I use this
    //! to detect a class like cv::Point which has its coordinates set/accessed with .x
    //! and .y
    template<typename T>
    class has_xy_members
    {
	template<typename U> static auto test(int) -> decltype(std::declval<U>().x == 1
                                                               && std::declval<U>().y == 1, std::true_type());
	template<typename> static std::false_type test(...);
    public:
	static constexpr bool value = std::is_same<decltype(test<T>(0)),std::true_type>::value;
    };

    // Expression SFINAE approach to testing for first and second member attributes
    template<typename T>
    class has_firstsecond_members
    {
	template<typename U> static auto test(int) -> decltype(std::declval<U>().first == 1
                                                               && std::declval<U>().second == 1, std::true_type());
	template<typename> static std::false_type test(...);
    public:
	static constexpr bool value = std::is_same<decltype(test<T>(0)), std::true_type>::value;
    };

    // Expression SFINAE approach to testing for ability to access like an array (i.e. std::array, morph::Vector)
    template<typename T>
    class array_access_possible
    {
	template<typename U> static auto test(int) -> decltype(std::declval<U>()[0] == 1, std::true_type());
	template<typename> static std::false_type test(...);
    public:
	static constexpr bool value = std::is_same<decltype(test<T>(0)),std::true_type>::value;
    };

} // morph::
