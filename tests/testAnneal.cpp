/*
 * Test simulated annealing
 */

#include "morph/Anneal.h"
#include <iostream>
#include <unistd.h>

FLT objective (const morph::vVector<FLT>& params)
{
    FLT rtn = 0;
    return rtn;
}

int main()
{
    morph::vVector<FLT> p = { 1, 2, 3 };
    morph::Anneal<FLT> anneal(p);
    // Tune the annealing process by choosing how slowly to reduce the temperature
    anneal.num_operations = 100;

    // Now do the business
    while (anneal.state != morph::Anneal_State::ReadyToStop) {
        if (anneal.state == morph::Anneal_State::NeedToCompute) {
            // Take the candidate parameters from the Anneal object and compute the candidate objective value
            anneal.cand_value = objective (anneal.cand);
        } else {
            throw std::runtime_error ("Unexpected state for anneal object.");
        }
        // A step of the Anneal algoirhtm involves reducing the temperature and stochastically selecting new candidate paramters
        anneal.step();
    }

    std::cout << "FINISHED! Best approximation: (Params: " << anneal.best << ") has value " << anneal.best_value << std::endl;

    int rtn = -1;
    // Add success test

    return rtn;
}
