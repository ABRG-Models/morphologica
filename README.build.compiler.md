# Minimum compiler versions

morphologica makes extensive use of C++-20. For this reason, there
are minimum supported versions of common compilers to be able to
compile the examples and any program against morphologica. The general rule is that the compiler should
provide full C++-20 support.

Note that some of the headers will have more relaxed compiler
requirements. If you are only using a small subset of morphologica
headers in your code, you may get away with a compiler that does not
fulfil the requirements given here.


## Tested compiler versions

| OS           | Compiler | Version | Result and reason                        |
| :-------:    | :------: | :-----: | ---------------------------------------- |
| Ubuntu 24.04 | g++      | 10.5    | Fail: on constexpr code in morph::Gridct |
| Ubuntu 24.04 | g++      | 11.4    | Pass (make && make test)                 |
| Ubuntu 24.04 | g++      | 12.3    | Pass (make && make test)                 |
| Ubuntu 24.04 | g++      | 13.2    | Pass (make && make test)                 |
| Ubuntu 24.04 | clang++  | 14.0    | Fail: on colourmaps_mono target (`#include <format>` problem)  |
| Ubuntu 24.04 | clang++  | 15.0    | Fail: (constexpr problems)               |
| Ubuntu 24.04 | clang++  | 16.0    | Pass (make && make test)                 |
| Ubuntu 24.04 | clang++  | 17      | Pass (make && make test)                 |
| Ubuntu 24.04 | clang++  | 18.1    | Pass (make && make test)                 |

The build also succeeds on various versions of Mac OS with
clang. Entries in the table for clang on Mac are to follow.

## Default compilers on different OS platforms

| OS           | Default Compiler Family | Version | Support |
| :-------:    | :------:                | :-----: | :--:    |
| Ubuntu 20.04 | gcc                     | 9       | No      |
| Ubuntu 20.10 | gcc                     | 10      | No      |
| Ubuntu 22.04 | gcc                     | 11      | Yes     |
| Ubuntu 22.04 | gcc                     | 13      | Yes     |
| Ubuntu 22.10 | gcc                     | 12      | Yes     |
| Ubuntu 24.04 | gcc                     | 13      | Yes     |
| Ubuntu 24.04 | gcc                     | 14      | Yes     |
| Fedora 35    | gcc                     | 11      | Yes     |
| Fedora 36    | gcc                     | 12      | Yes     |
| Fedora 37    | gcc                     | 12      | Yes     |
| Fedora 38    | gcc                     | 13      | Yes     |
| Fedora 39    | gcc                     | 13      | Yes     |
| Fedora 40    | gcc                     | 14      | Yes*    |

*Well, probably/hopefully/presumably :)

## Building with clang on Linux

Install clang (which on Ubuntu provides clang++) and a suitable version of libstdc++.

On Ubuntu 24, I used `clang-18` and `libstdc++-14-dev` together.

You then call cmake with

```bash
mkdir build_clang
cd build_clang
CC=clang CXX=clang++ cmake .. # Or maybe CC=clang-18 CXX=clang++-18
make
```
(You probably don't need CC=clang)
