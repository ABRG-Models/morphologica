// Example 39-1. CUDA C Code for the Naive Scan Algorithm
// From GPU Gems chapter-39-parallel-prefix-sum-scan-cuda
// An implementation of:
// Example 2. A Double-Buffered Version of the Sum Scan from Algorithm 1
//
// 1: for d = 1 to log_2(n) do
// 2: for all k in parallel do
// 3: if k >= 2^d then
// 4:   x[out][k] = x[in][k - 2^(d-1)] + x[in][k]
// 5: else
// 6:   x[out][k] = x[in][k]
//

// To be the naive scan glsl....
#version 310 es

// How large are our arrays?
#define N 32

layout (local_size_x = N, local_size_y = 1, local_size_z = 1) in;
layout (std430, binding = 1) coherent buffer InputBlock { float g_idata[N]; };
layout (std430, binding = 2) coherent buffer OutputBlock { float g_odata[N]; };
layout (std430, binding = 3) coherent buffer DebugBlock { float g_dbg[N]; };
layout (std430, binding = 4) coherent buffer DebugBlock2 { float g_dbg2[N]; };

void main()
{
    int k = int(gl_GlobalInvocationID.x);
    int n = N;
    int dmax = int(log2(float(n)));
    int dbg = 1; // You can use this to look at values at different 'stages' of the summing

    g_odata[k] = g_idata[k];
    barrier();

    int powd = 1; // 2^d with d=1 to log2(n)
    for (int d = 1; d <= dmax; d++)   { // 2^5 == 32
        if (k >= powd) {
            g_odata[k] = g_idata[k] + g_idata[k - powd];

            if (d==dbg) {
                g_dbg[k] = g_idata[k];
                g_dbg2[k] = g_idata[k - powd];
            }

        } else {
            g_odata[k] = g_idata[k];

            if (d==dbg) {
                g_dbg[k] = g_idata[k];
                g_dbg2[k] = 0.0;
            }

        }
        powd *= 2;
        g_idata[k] = g_odata[k];
        barrier();
    }
}
