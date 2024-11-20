/*
 * Test casting vec to array and back
 */

#include <iostream>
#include <array>
#include <morph/mathconst.h>
#include <morph/vec.h>

void f_const_ptr_a (const std::array<float, 3>* a1)
{
    std::cout << "You have to output all the elements of an array: ";
    for (auto el : (*a1)) { std::cout << el << ","; }
    std::cout << std::endl;
}
void f_const_ptr_v (const morph::vec<float, 3>* v1) { std::cout << "You can stream a vec: " << (*v1) << std::endl; }

void f_nonconst_ptr_a (std::array<float, 3>* a1)
{
    std::cout << "You have to output all the elements of an array: ";
    for (auto el : (*a1)) { std::cout << el << ","; }
    std::cout << std::endl;
}
void f_nonconst_ptr_v (morph::vec<float, 3>* v1) { std::cout << "You can stream a vec: " << (*v1) << std::endl; }

void f_const_ref_a (const std::array<float, 3>& a1)
{
    std::cout << "You have to output all the elements of an array: ";
    for (auto el : a1) { std::cout << el << ","; }
    std::cout << std::endl;
}
void f_const_ref_v (const morph::vec<float, 3>& v1) { std::cout << "You can stream a vec: " << v1 << std::endl; }

void f_nonconst_ref_a (std::array<float, 3>& a1)
{
    std::cout << "You have to output all the elements of an array: ";
    for (auto el : a1) { std::cout << el << ","; }
    std::cout << std::endl;
}
void f_nonconst_ref_v (morph::vec<float, 3>& v1) { std::cout << "You can stream a vec: " << v1 << std::endl; }

int main()
{
    int rtn = 0;

    morph::vec<float, 3> v1 = { 1, 2, 3 };
    std::array<float, 3> a1 = { 3, 2, 1 };

    // Casting of const pointers is fine
    std::cout << "\nCasting to const pointer args\n";
    // f_nonconst_ptr_a takes a const array pointer arg
    f_const_ptr_a (&a1);
    // a vec pointer can be cast to an array pointer with static_cast
    f_const_ptr_a (static_cast<std::array<float, 3>*>(&v1));
    // or reinterpret_cast
    f_const_ptr_a (reinterpret_cast<std::array<float, 3>*>(&v1));
    f_const_ptr_a (dynamic_cast<std::array<float, 3>*>(&v1));
    // error: invalid 'const_cast' from type 'morph::vec<float, 3>*' to type 'std::array<float, 3>*'
    // f_const_ptr_a (const_cast<std::array<float, 3>*>(&v1));
    f_const_ptr_a (static_cast<const std::array<float, 3>*>(&v1));
    f_const_ptr_a (reinterpret_cast<const std::array<float, 3>*>(&v1));
    f_const_ptr_a (dynamic_cast<const std::array<float, 3>*>(&v1));


    // f_const_ptr_v takes a const vec pointer arg
    f_const_ptr_v (&v1);
    // an array pointer can be cast to a vec
    f_const_ptr_v (static_cast<morph::vec<float, 3>*>(&a1));
    f_const_ptr_v (reinterpret_cast<morph::vec<float, 3>*>(&a1));
    // error: cannot 'dynamic_cast' '& a1' (of type 'struct std::array<float, 3>*') to type 'struct morph::vec<float, 3>*' (source type is not polymorphic)
    //f_const_ptr_v (dynamic_cast<morph::vec<float, 3>*>(&a1));
    f_const_ptr_v (static_cast< const morph::vec<float, 3>* >(&a1));
    f_const_ptr_v (reinterpret_cast< const morph::vec<float, 3>* >(&a1));

    // Casting of pointers is fine
    std::cout << "\nCasting to non-const pointer args\n";
    // f_nonconst_ptr_a takes an array pointer arg
    f_nonconst_ptr_a (&a1);
    // a vec pointer can be cast to an array pointer with static_cast
    f_nonconst_ptr_a (static_cast<std::array<float, 3>*>(&v1));
    // or reinterpret_cast
    f_nonconst_ptr_a (reinterpret_cast<std::array<float, 3>*>(&v1));
    // error: invalid 'const_cast' from type 'morph::vec<float, 3>*' to type 'std::array<float, 3>*'
    // f_nonconst_ptr_a (const_cast<std::array<float, 3>*>(&v1));
    // f_nonconst_ptr_a (static_cast< const std::array<float, 3>* >(&v1)); // invalid conversion
    // f_nonconst_ptr_a (reinterpret_cast< const std::array<float, 3>* >(&v1)); // invalid conversion

    // f_nonconst_ptr_v takes a vec pointer arg
    f_nonconst_ptr_v (&v1);
    // an array pointer can be cast to a vec
    f_nonconst_ptr_v (static_cast<morph::vec<float, 3>*>(&a1));

    // casting of a const reference morph::vec to std::array mostly works
    std::cout << "\nCasting to const ref args\n";
    f_const_ref_a (a1);
    f_const_ref_a (static_cast<decltype(a1)>(v1));
    // can static cast vec to array
    f_const_ref_a (static_cast<std::array<float, 3>>(v1));
    // can static cast vec to array&
    f_const_ref_a (static_cast<std::array<float, 3>& >(v1));
    // can dynamic cast vec to array&
    f_const_ref_a (dynamic_cast<std::array<float, 3>&>(v1));
    // cannot dynamic cast vec to array
    // f_const_ref_a (dynamic_cast<std::array<float, 3>>(v1));
    // error: invalid 'const_cast' from type 'morph::vec<float, 3>*' to type 'std::array<float, 3>*'
    // f_const_ref_a (const_cast<std::array<float, 3>&>(v1));
    f_const_ref_a (reinterpret_cast<std::array<float, 3>&>(v1));
    //f_const_ref_a (reinterpret_cast<std::array<float, 3>>(v1)); // invalid cast

    // but casting when the args are const array&/const vec& is more constrained
    f_const_ref_v (v1);

    // error: no matching function for call to 'morph::vec<float, 3>::vec(std::array<float, 3>&)'
    // f_const_ref_v (static_cast< morph::vec<float, 3> >(a1));

    // error: no matching function for call to 'morph::vec<float, 3>::vec(std::array<float, 3>&)'
    // f_const_ref_v (static_cast< const morph::vec<float, 3> >(a1));

    // ok (note we're casting to const vec<>&
    f_const_ref_v (static_cast< const morph::vec<float, 3>& >(a1));
    f_const_ref_v (reinterpret_cast< const morph::vec<float, 3>& >(a1));
    f_const_ref_v (static_cast< morph::vec<float, 3>& >(a1));
    f_const_ref_v (reinterpret_cast< morph::vec<float, 3>& >(a1));

    // error: invalid 'const_cast' from type 'std::array<float, 3>*' to type 'const morph::vec<float, 3>*'
    // f_const_ref_v (const_cast< const morph::vec<float, 3>& >(a1));

    // error: cannot 'dynamic_cast' 'a1' (of type 'struct std::array<float, 3>') to type 'const struct morph::vec<float, 3>&' (source type is not polymorphic)
    // f_const_ref_v (dynamic_cast< const morph::vec<float, 3>& >(a1));


    // reinterpret cast is also invalid and doesn't compile:
    // error: invalid cast from type 'std::array<float, 3>' to type 'morph::vec<float, 3>'
    //
    // f_const_ref_v (reinterpret_cast< morph::vec<float, 3> >(a1));
    // f_const_ref_v (reinterpret_cast< const morph::vec<float, 3> >(a1));

    std::cout << "\nCasting to non-const ref args\n";
    // casting of a non-const reference morph::vec to std::array is not ok
    f_nonconst_ref_a (a1);

    // error: cannot bind non-const lvalue reference of type 'std::array<float, 3>&' to an rvalue of type 'std::array<float, 3>'
    // f_nonconst_ref_a (static_cast<decltype(a1)>(v1));

    // error: cannot bind non-const lvalue reference of type 'std::array<float, 3>&' to an rvalue of type 'std::array<float, 3>'
    // f_nonconst_ref_a (static_cast<std::array<float, 3>>(v1));

    // You can dynamic or static cast to array&
    f_nonconst_ref_a (dynamic_cast<std::array<float, 3>&>(v1));
    f_nonconst_ref_a (static_cast<std::array<float, 3>&>(v1));

    f_nonconst_ref_v (v1);
    // You can static cast to vec&
    f_nonconst_ref_v (static_cast< morph::vec<float, 3>& >(a1));

    // error: cannot 'dynamic_cast' 'a1' (of type 'struct std::array<float, 3>') to type 'struct morph::vec<float, 3>&' (source type is not polymorphic)
    // f_nonconst_ref_v (dynamic_cast< morph::vec<float, 3>& >(a1));

    return rtn;
}
