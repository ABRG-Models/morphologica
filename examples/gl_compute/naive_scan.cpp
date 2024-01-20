// An implementation of:
// Example 2. A Double-Buffered Version of the Sum Scan from Algorithm 1
// From GPU Gems chapter-39-parallel-prefix-sum-scan-cuda
//
// 1: for d = 1 to log_2(n) do
// 2: for all k in parallel do
// 3: if k >= 2^d then
// 4:   x[out][k] = x[in][k - 2^(d-1)] + x[in][k]
// 5: else
// 6:   x[out][k] = x[in][k]
//
// This implementation is in C++, and was used to help debug shader_naive_scan.cpp/naive_scan.glsl

#include <iostream>
#include <morph/vec.h>

int main()
{
    // static constexpr int n = 32;
    morph::vec<float, 32> g_idata;
    morph::vec<float, 32> g_odata;
    morph::vec<float, 32> g_dbg;
    morph::vec<float, 32> g_dbg2;

    for (int k = 0; k < 32; ++k) {
        g_idata[k] = float(k);
        g_odata[k] = g_idata[k];
    }

    std::cout << "Prefix sum input:\n" << g_idata << std::endl;

    int dbg = 5; // Choose at which stage to record debug data. As 2^5 == 32, 5 will give the correct result.
    int powd = 1; // 2^d with d=1 to log2(n)
    for (int d = 1; d <= dbg; d++)   { // 2^5 == 32
        powd *= 2;

        std::cout << "d = " << d << ", 2^"<<d<<" (powd) = " << powd << " powd/2 = " << (powd/2) << std::endl;

        for (int k = 0; k < 32; ++k) {
            if (k >= powd/2) {
                g_odata[k] = g_idata[k] + g_idata[k - (powd/2)];
                if (d==dbg) {
                    g_dbg[k] = g_idata[k];
                    g_dbg2[k] = g_idata[k - (powd/2)];
                }

            } else {
                g_odata[k] = g_idata[k];
                if (d==dbg) {
                    g_dbg[k] = g_idata[k];
                    g_dbg2[k] = 0.0;
                }
            }
        } // acts as the barrier():
        for (int k = 0; k < 32; ++k) { g_idata[k] = g_odata[k]; }
    }

    std::cout << "\nDebug data1:\n" << g_dbg << std::endl;
    std::cout << "Debug data2:\n" << g_dbg2 << std::endl;
    std::cout << "\nPrefix sum result:\n" << g_odata << std::endl;

    return 0;
}
