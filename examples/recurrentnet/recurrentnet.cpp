#include "RecurrentNetworkModel.h"

int main (int argc, char **argv){
    if(argc<4){ cout<<"Run using e.g., './modelVis data/test 0 10000'.\n\n Supply path to folder containing the config.json file and any .h5 map files, seed, and training value T. Check when using T<1 values for plotting commands (some require additional command line params)."<<endl<<flush; return 0; }
    RecurrentNetworkModel N(argv[1]);
    srand(stoi(argv[2]));
    int T = stoi(argv[3]);

    if(T>0){
        // TRAINING
        N.run(T,1000,100,false);
        N.saveOutputs();
        N.saveWeights();
    } else {
        // TESTING
        N.loadWeights();

        switch(T){

            case(0):{
                N.plotDomainsAllContexts();
            } break;
            case(-1):{
                N.plotMapTargets();
            } break;
            case(-2):{
                N.plotMapResponsesAllMaps();
            } break;
            case(-3):{
                N.setColourMap(morph::ColourMapType::Jet);
                // params: contextIndex, nodeA, nodeB
                N.plotDomainNodeDiff(stoi(argv[4]), stoi(argv[5]), stoi(argv[6]));
            } break;
            case(-4):{
                N.setColourMap(morph::ColourMapType::Jet);
                // params: nodeIndex, contextA, contextB
                N.plotDomainContextDiff(stoi(argv[4]), stoi(argv[5]), stoi(argv[6]));
            } break;
            case(-5):{
                N.setColourMap(morph::ColourMapType::Jet);
                // params: contextA, contextB
                N.plotDomainContextDiffOutputNodes(stoi(argv[4]),stoi(argv[5]));
            } break;


            default:{
                cout<<"Invalid option: "<< T << endl;
            } break;
        }

    }

    return 0;
}
