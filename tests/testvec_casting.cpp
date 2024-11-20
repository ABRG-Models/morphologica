/*
 * Test casting vec to array and back
 */

#include <iostream>
#include <array>
#include <morph/mathconst.h>
#include <morph/vec.h>

void f00 (const std::array<float, 3>* a1)
{
    std::cout << "You have to output all the elements of an array: ";
    for (auto el : (*a1)) { std::cout << el << ","; }
    std::cout << std::endl;
}
void f0 (const morph::vec<float, 3>* v1) { std::cout << "You can stream a vec: " << (*v1) << std::endl; }

void f1 (std::array<float, 3>* a1)
{
    std::cout << "You have to output all the elements of an array: ";
    for (auto el : (*a1)) { std::cout << el << ","; }
    std::cout << std::endl;
}
void f2 (morph::vec<float, 3>* v1) { std::cout << "You can stream a vec: " << (*v1) << std::endl; }

void f3 (const std::array<float, 3>& a1)
{
    std::cout << "You have to output all the elements of an array: ";
    for (auto el : a1) { std::cout << el << ","; }
    std::cout << std::endl;
}
void f4 (const morph::vec<float, 3>& v1) { std::cout << "You can stream a vec: " << v1 << std::endl; }

void f5 (std::array<float, 3>& a1)
{
    std::cout << "You have to output all the elements of an array: ";
    for (auto el : a1) { std::cout << el << ","; }
    std::cout << std::endl;
}
void f6 (morph::vec<float, 3>& v1) { std::cout << "You can stream a vec: " << v1 << std::endl; }

int main()
{
    int rtn = 0;

    morph::vec<float, 3> v1 = { 1, 2, 3 };
    std::array<float, 3> a1 = { 3, 2, 1 };

    // Casting of pointers is fine
    std::cout << "\nCasting to const pointer args\n";
    // f1 takes a const array pointer arg
    f00 (&a1);
    // a vec pointer can be cast to an array pointer with static_cast
    f00 (static_cast<std::array<float, 3>*>(&v1));
    // or reinterpret_cast
    f00 (reinterpret_cast<std::array<float, 3>*>(&v1));
    f00 (dynamic_cast<std::array<float, 3>*>(&v1));
    // error: invalid 'const_cast' from type 'morph::vec<float, 3>*' to type 'std::array<float, 3>*'
    // f00 (const_cast<std::array<float, 3>*>(&v1));

    // f0 takes a const vec pointer arg
    f0 (&v1);
    // an array pointer can be cast to a vec
    f0 (static_cast<morph::vec<float, 3>*>(&a1));
    f0 (reinterpret_cast<morph::vec<float, 3>*>(&a1));
    // error: cannot 'dynamic_cast' '& a1' (of type 'struct std::array<float, 3>*') to type 'struct morph::vec<float, 3>*' (source type is not polymorphic)
    //f0 (dynamic_cast<morph::vec<float, 3>*>(&a1));

    // Casting of pointers is fine
    std::cout << "\nCasting to non-const pointer args\n";
    // f1 takes an array pointer arg
    f1 (&a1);
    // a vec pointer can be cast to an array pointer with static_cast
    f1 (static_cast<std::array<float, 3>*>(&v1));
    // or reinterpret_cast
    f1 (reinterpret_cast<std::array<float, 3>*>(&v1));
    // error: invalid 'const_cast' from type 'morph::vec<float, 3>*' to type 'std::array<float, 3>*'
    // f1 (const_cast<std::array<float, 3>*>(&v1));

    // f2 takes a vec pointer arg
    f2 (&v1);
    // an array pointer can be cast to a vec
    f2 (static_cast<morph::vec<float, 3>*>(&a1));

    // casting of a const reference morph::vec to std::array mostly works
    std::cout << "\nCasting to const ref args\n";
    f3 (a1);
    f3 (static_cast<decltype(a1)>(v1));
    // can static cast vec to array
    f3 (static_cast<std::array<float, 3>>(v1));
    // can static cast vec to array&
    f3 (static_cast<std::array<float, 3>& >(v1));
    // can dynamic cast vec to array&
    f3 (dynamic_cast<std::array<float, 3>&>(v1));
    // cannot dynamic cast vec to array
    // f3 (dynamic_cast<std::array<float, 3>>(v1));
    // error: invalid 'const_cast' from type 'morph::vec<float, 3>*' to type 'std::array<float, 3>*'
    // f3 (const_cast<std::array<float, 3>&>(v1));

    // but casting when the args are const array&/const vec& is more constrained
    f4 (v1);

    // error: no matching function for call to 'morph::vec<float, 3>::vec(std::array<float, 3>&)'
    // f4 (static_cast< morph::vec<float, 3> >(a1));

    // error: no matching function for call to 'morph::vec<float, 3>::vec(std::array<float, 3>&)'
    // f4 (static_cast< const morph::vec<float, 3> >(a1));

    // ok (note we're casting to const vec<>&
    f4 (static_cast< const morph::vec<float, 3>& >(a1));

    // error: invalid 'const_cast' from type 'std::array<float, 3>*' to type 'const morph::vec<float, 3>*'
    // f4 (const_cast< const morph::vec<float, 3>& >(a1));

    // error: cannot 'dynamic_cast' 'a1' (of type 'struct std::array<float, 3>') to type 'const struct morph::vec<float, 3>&' (source type is not polymorphic)
    // f4 (dynamic_cast< const morph::vec<float, 3>& >(a1));


    // reinterpret cast is also invalid and doesn't compile:
    // error: invalid cast from type 'std::array<float, 3>' to type 'morph::vec<float, 3>'
    //
    // f4 (reinterpret_cast< morph::vec<float, 3> >(a1));
    // f4 (reinterpret_cast< const morph::vec<float, 3> >(a1));

    std::cout << "\nCasting to non-const ref args\n";
    // casting of a non-const reference morph::vec to std::array is not ok
    f5 (a1);

    // error: cannot bind non-const lvalue reference of type 'std::array<float, 3>&' to an rvalue of type 'std::array<float, 3>'
    // f5 (static_cast<decltype(a1)>(v1));

    // error: cannot bind non-const lvalue reference of type 'std::array<float, 3>&' to an rvalue of type 'std::array<float, 3>'
    // f5 (static_cast<std::array<float, 3>>(v1));

    // You can dynamic or static cast to array&
    f5 (dynamic_cast<std::array<float, 3>&>(v1));
    f5 (static_cast<std::array<float, 3>&>(v1));

    f6 (v1);
    // You can static cast to vec&
    f6 (static_cast< morph::vec<float, 3>& >(a1));

    // error: cannot 'dynamic_cast' 'a1' (of type 'struct std::array<float, 3>') to type 'struct morph::vec<float, 3>&' (source type is not polymorphic)
    // f6 (dynamic_cast< morph::vec<float, 3>& >(a1));

    return rtn;
}
