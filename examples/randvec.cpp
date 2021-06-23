/*
 * The scalar products of a set of random vectors should follow the beta distribution.
 */

#include <morph/Vector.h>
#include <morph/vVector.h>
#include <morph/Random.h>
#include <iostream>

int main()
{
    static constexpr size_t N = 10000;
    static constexpr size_t n = 2;
    morph::vVector<morph::Vector<float, n>> vVecs(N);
    morph::RandUniform<float> rng(0.0f, 1.0f);
    std::vector<float> vf = rng.get(n*N);
    for (size_t i = 0; i < N; ++i) {
        vVecs[i] = {vf[i], vf[N+i]};
        vVecs[i].renormalize();
    }

    // Get scalar products between pairs
    morph::vVector<float> sp (N/n);
    for (size_t i = 0; i < N/n; ++i) {
        sp[i] = vVecs[i].dot (vVecs[i+N/n]);
        std::cout << sp[i] << std::endl;
    }

    // Graph scalar products as histo.
    //morph::histodata = morph::MathAlgo<float>::histo (sp, 20);
    // plot histodata.bins vs histodata.counts

    return 0;
}
