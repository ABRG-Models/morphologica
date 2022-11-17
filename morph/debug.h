#pragma once
#include <iostream>

// This macro expands a function-like call VAR(varname) into a line of std function
// calls to show the name and value of a variable.
//
//   int a = 2;
//   VAR (a);
// will give output: "a = 2"
//
// And, even better:
//   int b = 4;
//   VAR (a/b);
// will give output "a/b = 0.5"
#define VAR(varname) std::cout << #varname << " = " << (varname) << std::endl;
