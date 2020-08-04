#include "morph/RecurrentNetworkModel.h"

int main (int argc, char **argv){
    if(argc<4){ std::cout<<"Run using e.g., './recurrentnet data/test 0 10000'.\n Supply path to folder containing the config.json file and any .h5 map files, seed, and training value T. Check when using T<1 values for plotting commands (some require additional command line params)."<<std::endl<<std::flush; return 0; }
    RecurrentNetworkModel N(argv[1]);
    srand(std::stoi(argv[2]));
    int T = std::stoi(argv[3]);

    if(T>0){
        // TRAINING
        N.run(T,1000);
        N.saveError();
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
                if(argc<7){ std::cout<<"Supply contextIndex, nodeA, nodeB as additional arguments"<<std::endl; return 0; }
                N.setColourMap(morph::ColourMapType::Jet);
                // params: contextIndex, nodeA, nodeB
                N.plotDomainNodeDiff(std::stoi(argv[4]), std::stoi(argv[5]), std::stoi(argv[6]));
            } break;
            case(-4):{
                if(argc<7){ std::cout<<"Supply nodeIndex, contextA, contextB as additional arguments"<<std::endl; return 0; }
                N.setColourMap(morph::ColourMapType::Jet);
                // params: nodeIndex, contextA, contextB
                N.plotDomainContextDiff(std::stoi(argv[4]), std::stoi(argv[5]), std::stoi(argv[6]));
            } break;
            case(-5):{
                if(argc<6){ std::cout<<"Supply contextA, contextB as additional arguments"<<std::endl; return 0; }
                N.setColourMap(morph::ColourMapType::Jet);
                // params: contextA, contextB
                N.plotDomainContextDiffOutputNodes(std::stoi(argv[4]),std::stoi(argv[5]));
            } break;

            default:{
                std::cout<<"Invalid option: "<< T << std::endl;
            } break;
        }

    }

    return 0;
}
