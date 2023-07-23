/*
 * Bootstrapping stats
 */

#pragma once

#include <vector>
#include <morph/vvec.h>

namespace morph {

    template <typename T>
    struct bootstrap {

        // Resample B sets from data and place them in resamples
        static void resample_with_replacement (const morph::vvec<T>& data,
                                               std::vector<morph::vvec<T>>& resamples, const unsigned int B)
        {
            unsigned int data_n = data.size();
            resamples.resize (B);
            for (unsigned int i = 0; i < B; ++i) {
                morph::vvec<T> resample (data_n);
                morph::vvec<unsigned int> randindices (data_n);
                randindices.randomize (0, data_n);
                for (unsigned int j = 0; j < data_n; ++j) {
                    resample[j] = data[randindices[j]];
                }
                resamples[i] = resample;
            }
        }

        static T error_of_mean (const morph::vvec<T>& data, const unsigned int B)
        {
            std::cout << "Bootstrap the error of the mean for data, for which SD=" << data.std() << std::endl;
            std::vector<morph::vvec<T>> resamples;
            morph::bootstrap<T>::resample_with_replacement (data, resamples, B);
            morph::vvec<T> r_mean (B, T{0});
            for (unsigned int i = 0; i < B; ++i) {
                r_mean[i] = resamples[i].mean();
            }
            // Standard error is the standard deviation of the resample means
            return r_mean.std();
        }

        // Compute a bootstrapped two sample t statistic as per algorithm 16.2
        // in Efron & Tibshirani.
        // zdata is treatment; ydata is control. B is the number of bootstrap samples to make
        //
        // This tests for equality of means, not whether the populations are identical
        // and may be used on distributions where the variances may not be equal.
        static morph::vec<T, 2> ttest_equalityofmeans (const morph::vvec<T>& _zdata, const morph::vvec<T>& _ydata, const unsigned int B)
        {
            // Ensure that the group which we name zdata is the larger one.
            morph::vvec<T> ydata = _ydata;
            morph::vvec<T> zdata = _zdata;
            if (_zdata.mean() <= _ydata.mean()) { std::swap (ydata, zdata); }

            unsigned int n = zdata.size();
            unsigned int m = ydata.size();

            // combine the data as if they were drawn from a single distribution
            morph::vvec<T> x = ydata;
            x.concat (zdata);
            T xmean = x.mean();

            T ymean = ydata.mean();
            T zmean = zdata.mean();

            // Compute variances for the observed values:
            T obsvarz = (zdata-zmean).pow(2).sum() / (n-1);
            T obsvary = (ydata-ymean).pow(2).sum() / (m-1);

            // Compute the observed value of the studentised statistic (using separate
            // variances, rather than a pooled variance):
            T tobs = (zmean - ymean) / std::sqrt (obsvary/static_cast<T>(m) + obsvarz/static_cast<T>(n));

            // Create shifted distributions; shifted by group mean and combined mean:
            morph::vvec<T> ztilda = zdata - zmean + xmean;
            morph::vvec<T> ytilda = ydata - ymean + xmean;

            // Resample from the shifted (tilda) distributions:
            std::vector<morph::vvec<T>> zstar;
            bootstrap<T>::resample_with_replacement (ztilda, zstar, B);
            std::vector<morph::vvec<T>> ystar;
            bootstrap<T>::resample_with_replacement (ytilda, ystar, B);

            // Create vectors of the means of these resamples:
            morph::vvec<T> zstarmeans (B, T{0});
            morph::vvec<T> ystarmeans (B, T{0});
            for (unsigned int i = 0; i < B; ++i) {
                zstarmeans[i] = zstar[i].mean();
                ystarmeans[i] = ystar[i].mean();
            }

            // Compute the variances. Hmm. This is the only nontrivial conversion
            morph::vvec<T> zvariances (B, T{0});
            morph::vvec<T> yvariances (B, T{0});
            for (unsigned int i = 0; i < B; ++i) {
                zvariances[i] = (zstar[i]-zstarmeans).pow(2).sum() / (n-1);
                yvariances[i] = (ystar[i]-ystarmeans).pow(2).sum() / (m-1);
            }

            morph::vvec<T> top = zstarmeans - ystarmeans;
            morph::vvec<T> bot = (yvariances/static_cast<T>(m) + zvariances/static_cast<T>(n)).sqrt();
            morph::vvec<T> txstar = top / bot;

            T numbeyond = (txstar.element_compare_gteq (tobs)).sum();
            auto asl = numbeyond / static_cast<T>(B);
            T minasl = T{1} / static_cast<T>(B);

            return morph::vec<T, 2> ({asl, minasl});
        }

    };
}
