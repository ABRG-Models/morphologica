// A dual-context GeneNet
#pragma once

#include <morph/bn/Genome.h>
#include <morph/bn/GeneNet.h>

namespace morph {
    namespace  bn {
        template <size_t N=5, size_t K=5>
        struct GeneNetDual : public GeneNet<N, K>
        {
            state_t state_pos;
            state_t state_ant;

            void develop (const Genome<N,K>& genome)
            {
                GeneNet<N,K>::develop (this->state_ant, genome);
                GeneNet<N,K>::develop (this->state_pos, genome);
            }
        };
    }
}
