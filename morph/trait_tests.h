/*!
 * \file
 *
 * Type traits template incantations.
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

    //! Traits approach to testing for possibility of a-b. Could also make a class which used
    //! std::is_arithmetic here.
    template<typename T>
    class has_subtraction
    {
	template<typename U> static auto test(const U& a, const U& b) -> decltype(a-b, std::true_type());
	template<typename> static std::false_type test(...);
    public:
	static constexpr bool value = std::is_same<decltype(test<T>(T{}, T{})),std::true_type>::value;
    };

    //! Traits approach to testing for possibility of a+b.
    template<typename T>
    class has_addition
    {
	template<typename U> static auto test(const U& a, const U& b) -> decltype(a+b, std::true_type());
	template<typename> static std::false_type test(...);
    public:
	static constexpr bool value = std::is_same<decltype(test<T>(T{}, T{})),std::true_type>::value;
    };

    //! Traits approach to testing for x() and y() methods
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

    //! Traits approach to testing for a resize(size_t) method. Can be used to distinguish std::array
    //! from std::vector.
    template<typename T>
    class has_resize_method
    {
        // in decltype: If the expression preceding the comma operator is valid, expression post-comma is processed by decltype() and its type returned.
	template<typename U> static auto test(int sz) -> decltype(std::declval<U>().resize(sz), std::true_type());
	template<typename> static std::false_type test(...);
    public:
	static constexpr bool value = std::is_same< decltype(test<T>(2)), std::true_type >::value;
    };

    //! Traits approach to testing for x and y member attributes. I use this to detect a class like
    //! cv::Point which has its coordinates set/accessed with .x and .y
    template<typename T>
    class has_xy_members
    {
	template<typename U> static auto test(int) -> decltype(std::declval<U>().x == 1
                                                               && std::declval<U>().y == 1, std::true_type());
	template<typename> static std::false_type test(...);
    public:
	static constexpr bool value = std::is_same<decltype(test<T>(0)),std::true_type>::value;
    };

    // Traits approach to testing for first and second member attributes
    template<typename T>
    class has_firstsecond_members
    {
	template<typename U> static auto test(int) -> decltype(std::declval<U>().first == 1
                                                               && std::declval<U>().second == 1, std::true_type());
	template<typename> static std::false_type test(...);
    public:
	static constexpr bool value = std::is_same<decltype(test<T>(0)), std::true_type>::value;
    };

    // Traits approach to testing for ability to access like an array (i.e. std::array, morph::vec)
    template<typename T>
    class array_access_possible
    {
	template<typename U> static auto test(int) -> decltype(std::declval<U>()[0] == 1, std::true_type());
	template<typename> static std::false_type test(...);
    public:
	static constexpr bool value = std::is_same<decltype(test<T>(0)),std::true_type>::value;
    };

#if 0 // haven't figured out how to make this work.
    template<typename T>
    class has_find_method
    {
	template<typename U> static auto test(int) -> decltype(std::declval<U>().find(std::declval<U>().end()), std::true_type());
	template<typename> static std::false_type test(...);
    public:
	static constexpr bool value = std::is_same< decltype(test<T>(0)), std::true_type >::value;
    };
#endif

    // Does T have a const_iterator which satisfies the requirements of LegacyInputIterator?
    // Note this is NOT yet complete - I don't test std::iterator_traits.
    // The tests here more or less tell me if I have a copyable container
    template<typename T>
    class is_copyable_container
    {
        // Test C's const_iterator for traits copy constructible, copy assignable, destructible, swappable and equality comparable
	template<typename C> static auto test(int) -> decltype(std::is_copy_constructible<typename C::const_iterator>::value == true
                                                               && std::is_copy_assignable<typename C::const_iterator>::value == true
                                                               && std::is_destructible<typename C::const_iterator>::value == true
                                                               && std::is_swappable<typename C::const_iterator>::value == true
                                                               && std::declval<typename C::const_iterator> == std::declval<typename C::const_iterator>
                                                               , std::true_type());

        template<typename C> static int test(...);

    public:
	static constexpr bool value = std::is_same<decltype(test<T>(0)), std::true_type>::value;
    };

    // Is T a copyable container AND has a constexpr size() method? If so, it's probably std::array or morph::vec
    template<typename T>
    class is_copyable_fixedsize
    {
        // This will only compile if T has constexpr size()
        template<typename C> static constexpr bool size_is_constexpr()
        {
            constexpr C c = {};
            constexpr typename C::size_type sz = c.size();
            return sz >= 0 ? true : false;
        }
        static constexpr bool constexpr_size = size_is_constexpr<T>();

	template<typename C> static auto test(int) -> decltype(morph::is_copyable_container<C>::value == true
                                                               && constexpr_size == true, std::true_type());
        template<typename C> static int test(...);

    public:
	static constexpr bool value = std::is_same<decltype(test<T>(0)), std::true_type>::value;
    };

    // Test for std::complex by looking for real() and imag() methods
    template<typename T>
    class is_complex
    {
        // See the right arrow?                 v--- there it is. It's a different function declaration syntax
	template<typename U> static auto test(int) -> decltype(std::declval<U>().real() == 1
                                                            && std::declval<U>().imag() == 1, std::true_type());
	template<typename> static std::false_type test(...); // This uses the more typical syntax for fn declaration
    public:
	static constexpr bool value = std::is_same<decltype(test<T>(1)), std::true_type>::value;
    };

    // morph::value_type to allow us to write code that will accept float::value_type and std::vector<float>::value_type
    template <class T, class = void> struct value_type { using type = T; };
    template <class T> struct value_type<T, std::void_t<typename T::value_type>> { using type = typename T::value_type; };

    // This gets the value_type of a class that has value_type or the type of itself for a class
    // that doesn't have value_type. For example morph::value_type_t<float> is float and
    // morph::value_type_t<std::array<float, 2>> is also float.
    template <class T> using value_type_t = typename morph::value_type<T>::type;

    /*! \brief A class to distinguish between scalars and vectors
     *
     * From the typename T, set a #value attribute which says whether T is a scalar (like
     * float, double), or vector (basically, anything else).
     *
     * Query the attribute `value`, which will be:
     *
     * 0 for containers of scalars (which includes vectors, arrays. Essentially, this is a mathematical vector)
     * 1 for scalars
     * 2 for complex scalars
     * 3 for containers of complex (a vector<complex<float>> etc)
     * -1 for non-number types
     *
     * You can use this trait test in template classes like morph::Scale for scalar/vector
     * implementations.
     *
     * \tparam T the type to distinguish
     */
    template <typename T>
    struct number_type {
        //! is_scalar test
        static constexpr bool const scalar = std::is_scalar<std::decay_t<T>>::value;
        static constexpr bool const cplx = morph::is_complex<std::decay_t<T>>::value;
        static constexpr bool const container = morph::is_copyable_container<std::decay_t<T>>::value;
        // if container, check the element type
        static constexpr bool const container_of_scalars = container == true ? std::is_scalar<morph::value_type_t<T>>::value : false;
        static constexpr bool const container_of_complex = container == true ? morph::is_complex<morph::value_type_t<T>>::value : false;
        //! Set value. 0 for vector, 1 for scalar, 2 for complex scalar, 3 for vector of complex, -1 for non-number type
        static constexpr int const value = scalar ? 1 : cplx ? 2 : container_of_scalars ? 0 : container_of_complex ? 3 : -1;
    };

} // morph::
