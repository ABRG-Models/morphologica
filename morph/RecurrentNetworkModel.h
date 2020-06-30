/*
 Implementation of recurrent backprop algorithm following Pineda (1987)
 */

// With the correct OpenGL definitions (-DGL3_PROTOTYPES etc) you probably don't need this for Apple
#ifdef __OSX__
# include "OpenGL/gl3.h"
#endif

#include "morph/HdfData.h"
#include "morph/Visual.h"
#include "morph/QuadsVisual.h"
#include "morph/HexGridVisual.h"
#include "morph/ColourMap.h"
#include "morph/tools.h"
#include <morph/Config.h>
#include "morph/Scale.h"
#include "Vector.h"
#include "RecurrentNetworkTools.h"
#include "RecurrentNetwork.h"

#include "morph/ReadCurves.h"
#include "morph/RD_Base.h"

using morph::Config;
using morph::Visual;
using morph::ColourMapType;
using morph::Tools;
using morph::QuadsVisual;
using morph::HexGridVisual;
using morph::Scale;
using morph::HdfData;
using morph::Vector;


using namespace tools;


std::vector<std::array<float, 12>> getQuads(std::vector<double> X, std::vector<double> Y){

    std::vector<std::array<float, 12>> quads;

    double xRange = tools::getMax(X) - tools::getMin(X);
    double yRange = tools::getMax(Y) - tools::getMin(Y);
    double xOff = -0.5*xRange;
    double yOff = -0.5*yRange;
    double maxDim = xRange;
    if(yRange>maxDim){ maxDim = yRange; }
    double xScale = xRange/maxDim;
    double yScale = yRange/maxDim;

    std::vector<double> uniqueX = tools::getUnique(X);
    int cols = uniqueX.size();
    std::vector<int> colID(X.size(),0);
    std::vector<std::vector<double> > yByCol(cols);
    std::vector<int> count(cols,0);
    for(int i=0;i<X.size();i++){
        for(int j=0;j<cols;j++){
            if(X[i]==uniqueX[j]){
                colID[i]=j;
                yByCol[j].push_back(Y[i]);
                count[j]++;
            }
        }
    }

    std::vector<double> colRange(cols);
    for(int i=0;i<cols;i++){
        colRange[i] = tools::getMax(yByCol[i])-tools::getMin(yByCol[i]);
    }
    double xSep = 0.5*xRange/((double)cols-1);
    std::vector<double> ySep;
    for(int i=0;i<X.size();i++){
        ySep.push_back(0.5*colRange[colID[i]]/((double)count[colID[i]]-1));
    }

    std::array<float, 12> sbox;
    for (int i=0; i<X.size(); i++) {
        // corner 1 x,y,z
        sbox[0] = xScale*(xOff+X[i]-xSep);
        sbox[1] = yScale*(yOff+Y[i]-ySep[i]);
        sbox[2] = 0.0;
        // corner 2 x,y,z
        sbox[3] = xScale*(xOff+X[i]-xSep);
        sbox[4] = yScale*(yOff+Y[i]+ySep[i]);
        sbox[5] = 0.0;
        // corner 3 x,y,z
        sbox[6] = xScale*(xOff+X[i]+xSep);
        sbox[7] = yScale*(yOff+Y[i]+ySep[i]);
        sbox[8] = 0.0;
        // corner 4 x,y,z
        sbox[9] = xScale*(xOff+X[i]+xSep);
        sbox[10]= yScale*(yOff+Y[i]-ySep[i]);
        sbox[11]= 0.0;
        quads.push_back(sbox);
    }
    return quads;
}


template <class Flt>
class Domain : public morph::RD_Base<Flt> {
public:
    double ellipseA=1.0;
    double ellipseB=1.0;

    alignas(alignof(std::vector<Flt>))
    std::vector<Flt> X;

    alignas(alignof(std::vector<Flt>))
    std::vector<Flt> Y;

    virtual void init (void) {
        this->stepCount = 0;
    }
    void setEllipse(double ellipseA, double ellipseB, double hextohex_d){
        this->ellipseA = ellipseA;
        this->ellipseB = ellipseB;
        this->hextohex_d = hextohex_d;
    }
    virtual void allocate (void) {

        this->hg = new morph::HexGrid (this->hextohex_d, this->hexspan, 0, morph::HexDomainShape::Boundary);
        DBG ("Initial hexagonal HexGrid has " << this->hg->num() << " hexes");
        this->hg->setEllipticalBoundary (ellipseA, ellipseB);
        // Compute the distances from the boundary
        this->hg->computeDistanceToBoundary();
        // std::vector size comes from number of Hexes in the HexGrid
        this->nhex = this->hg->num();
        DBG ("After setting boundary, HexGrid has " << this->nhex << " hexes");
        // Spatial d comes from the HexGrid, too.
        this->set_d(this->hg->getd());
        DBG ("HexGrid says d = " << this->d);
        this->set_v(this->hg->getv());
        DBG ("HexGrid says v = " << this->v);

        this->resize_vector_variable (this->X);
        this->resize_vector_variable (this->Y);
    }
    virtual void step (void) {
        this->stepCount++;
    }

    void setAxes(double xScale, double yScale, double xOffset, double yOffset){
        for (unsigned int h=0; h<this->nhex; ++h) {
            this->X[h] = this->hg->vhexen[h]->x*xScale+xOffset;
            this->Y[h] = this->hg->vhexen[h]->y*yScale+yOffset;
        }
    }
};



class Map{

    /*
     Structure for storing a map (pre-defined X F, in a HdfData file).
     */

public:
    int N;
    std::vector<std::vector<double> > X;
    std::vector<double> maxX, minX;
    std::vector<double> F, Fscaled;
    double minF, maxF;
    int outputID, contextID;
    double contextVal;
    std::vector<std::array<float, 12>> quads;

    void init(std::string filename){

        outputID = -1; // flag for not set
        contextID = -1; // flag for not set
        contextVal = 0.0; // default

        HdfData network(filename,1);

        network.read_contained_vals ("F", F);
        N = F.size();

        /*
        std::vector<double> x;   network.read_contained_vals ("X", x);
        std::vector<double> y;   network.read_contained_vals ("Y", y);
        std::vector<double> z;   network.read_contained_vals ("Z", z);
        std::vector<double> q;   network.read_contained_vals ("Q", q);  // NOTE: Added for RW project (should be done in a more general way)

        if((!(x.size()==N)) || (!(y.size()==N)) || (!(z.size()==N)) || (!(q.size()==N))){
            std::cout<<"X, Y, Z, Q, F not all the same size... all kinds of badness!"<<std::endl;
        }

        X.push_back(x);
        X.push_back(y);
        X.push_back(z);
        X.push_back(q); // NOTE: Added for RW project (should be done in a more general way)
        */

        // NOTE: Edited for RW project - now expects all input 1 vals then all input 2 vals etc. concatenated
        std::vector<double> x;   network.read_contained_vals ("X", x);
        int k=0;
        for(int i=0;i<floor(x.size()/N);i++){
            std::vector<double>tmp;
            for(int j=0;j<N;j++){
                tmp.push_back(x[k]);
                k++;
            }
            X.push_back(tmp);
        }

        minF = tools::getMin(F);
        maxF = tools::getMax(F);
        Fscaled = tools::normalize(F);

        for(int i=0;i<X.size();i++){
            maxX.push_back(tools::getMax(X[i]));
            minX.push_back(tools::getMin(X[i]));
        }

        quads = getQuads(X[0],X[1]);

    }

    Map(std::string filename){
        init(filename);
    }

    Map(std::string filename,int outputID){
        init(filename);
        this->outputID = outputID;
    }

    Map(std::string filename,int outputID, int contextID, double contextVal){
        init(filename);
        this->outputID = outputID;
        this->contextID = contextID;
        this->contextVal = contextVal;
    }
};



class RecurrentNetworkModel{

public:
    std::string logpath;
    std::ofstream logfile;
    std::vector<double> inputs, Error;//, response;
    std::vector<std::vector<double> > response;
    RecurrentNetwork P;
    std::vector<Map> M;
    Domain<double> domain;
    std::vector<int> inputID;
    std::vector<int> outputID;
    std::vector<int> contextIDs;
    std::vector<double> contextVals;
    int nContext;
    morph::ColourMapType colourMap;


    RecurrentNetworkModel(std::string logpath){

        // setup log file
        this->logpath = logpath;
        morph::Tools::createDir (logpath);
        { std::stringstream ss; ss << logpath << "/log.txt"; logfile.open(ss.str());}
        logfile<<"Hello."<<std::endl;

        // Read in network params
        Config conf;
        { std::stringstream ss; ss << logpath <<"/config.json"; conf.init (ss.str()); }
        float dt = conf.getFloat("dt",1.0);
        float tauW = conf.getFloat("tauW",2.0);
        float tauX = conf.getFloat("tauX",1.0);
        float tauY = conf.getFloat("tauY",1.0);
        float weightNudgeSize = conf.getFloat("weightNudgeSize",0.001);
        float divergenceThreshold = conf.getFloat("divergenceThreshold",0.000001);
        int maxConvergenceSteps = conf.getInt("maxConvergenceSteps",400);

        float dx = conf.getFloat("dx",0.02);
        float yAspect = conf.getFloat("yAspect",0.75);
        float scaleDomain = conf.getFloat("scaleDomain",1.5);


        // Read in map info
        const Json::Value maps = conf.getArray("maps");
        for(int i=0;i<maps.size();i++){
            std::string fn = maps[i].get("filename", "unknown map").asString();
            std::stringstream ss; ss << logpath <<"/"<<fn;
            logfile<<"Map["<<i<<"]:"<<ss.str()<<std::endl;
            int oID = maps[i].get("outputID",-1).asInt();
            int cID = maps[i].get("contextID",-1).asInt();
            float cVal = maps[i].get("contextVal",0.0).asFloat();
            M.push_back(Map(ss.str(),oID,cID,cVal));
        }

        for(int i=0;i<M.size();i++){
            if(M[i].outputID!=-1){
                outputID.push_back(M[i].outputID);
            }
        }
        outputID = tools::getUnique(outputID);

        inputID.resize(2,0);
        inputID[1] = 1;
        const Json::Value inp = conf.getArray("inputID");
        for(int i=0;i<inp.size();i++){
            inputID.push_back(inp[i].asInt());
        }
        inputID = tools::getUnique(inputID);

        // Setup network connectivity
        std::vector<int> pre, post;
        std::stringstream ss; ss << logpath << "/network.h5"; HdfData network(ss.str(),1);
        network.read_contained_vals ("pre", pre);
        network.read_contained_vals ("post", post);

        if(pre.size()!=post.size()){ logfile<<"Pre/Post different sizes ("<<pre.size()<<"/"<<post.size()<<")"<<std::endl; exit(0);}
        if(pre.size()<1){ logfile<<"No connections in network!"<<std::endl; exit(0);}

        int N = tools::getMax(pre);
        if(tools::getMax(post)>N){
            N=tools::getMax(post);
        }
        N++;

        // Initiate network
        P.init (N,dt,tauW,tauX,tauY,weightNudgeSize,divergenceThreshold,maxConvergenceSteps);
        for(int i=0;i<pre.size();i++){ P.connect(pre[i],post[i]); }
        P.addBias();
        P.setNet();
        inputs.resize(inputID.size());

        // Define the domain over which it can be evaluated
        //{ std::stringstream ss; ss << logpath <<"/domain.svg"; domain.svgpath = ss.str(); }
        domain.init();
        domain.setEllipse(1.0,yAspect,dx);
        domain.allocate();

        double maxX=-1e9;
        double maxY=-1e9;
        double minX= 1e9;
        double minY= 1e9;
        for(int i=0;i<M.size();i++){
            if(M[i].maxX[0]>maxX){maxX=M[i].maxX[0];};
            if(M[i].maxX[1]>maxY){maxY=M[i].maxX[1];};
            if(M[i].minX[0]<minX){minX=M[i].minX[0];};
            if(M[i].minX[1]<minY){minY=M[i].minX[1];};
        }

        double scale = (maxX-minX)/(2.0*domain.ellipseA) * scaleDomain;
        domain.setAxes(scale, scale, minX+(maxX-minX)*0.5, minY+(maxY-minY)*0.5);

        // identify the unique context conditions
        std::vector<int> allContextIDs;
        std::vector<double> allContextVals;
        for(int i=0;i<M.size();i++){
            if(M[i].contextID!=-1){
                allContextIDs.push_back(M[i].contextID);
                allContextVals.push_back(M[i].contextVal);
            }
        }

        for(int i=0;i<allContextIDs.size();i++){
            bool uni = true;
            for(int k=0;k<contextIDs.size();k++){
                if((allContextIDs[i]==contextIDs[k]) && (allContextVals[i]==contextVals[k])){
                    uni = false; break;
                }
            } if(uni){
                contextIDs.push_back(allContextIDs[i]);
                contextVals.push_back(allContextVals[i]);
            }
        }
        nContext = contextIDs.size();

        setColourMap(morph::ColourMapType::Viridis);

    }

    void saveOutputs(void) { // log outputs
        std::stringstream fname; fname << logpath << "/outputs.h5";
        HdfData outdata(fname.str());
        outdata.add_contained_vals ("error", Error);
        outdata.add_contained_vals ("responses", response);
    }

    void saveWeights(void){ // log weights
        std::stringstream fname; fname << logpath << "/weights.h5";
        HdfData weightdata(fname.str());
        weightdata.add_contained_vals ("weights", P.W);
        std::vector<double> flatweightmat = P.getWeightMatrix();
        weightdata.add_contained_vals ("weightmat", flatweightmat);
    }

    void loadWeights(void) { // log outputs
        std::stringstream fname; fname << logpath << "/weights.h5";
        HdfData loaded(fname.str(),1);
        loaded.read_contained_vals ("weights", P.W);
        P.Wbest = P.W;
    }

    ~RecurrentNetworkModel(void){
        logfile<<"Goodbye."<<std::endl;
        logfile.close();
    }

    void setInput(int mapID, int locID){
        P.reset();
        for(int i=0; i<inputID.size(); i++){
            P.Input[inputID[i]] = M[mapID].X[i][locID]; // Training on the supplied x-values
        }
        if(M[mapID].contextID != -1){
            P.Input[M[mapID].contextID] = M[mapID].contextVal;
        }
    }

    std::vector<int> setRandomInput(void){
        // first is mapID, second is location ID
        std::vector<int> sample(2,floor(morph::Tools::randDouble()*M.size()));
        sample[1] = floor(morph::Tools::randDouble()*M[sample[0]].N);;
        return sample;
    }

    std::vector<std::vector<double> > testMap(int mapID){
        std::vector<std::vector<double> > response(P.N,std::vector<double>(M[mapID].N,0.));
        for(int j=0;j<M[mapID].N;j++){
            setInput(mapID,j);
            P.convergeForward(-1,false);
            for(int k=0;k<P.N;k++){
                response[k][j] = P.X[k];
            }
        }
        return response;
    }


    std::vector<std::vector<double> > testDomainContext(int i){

        std::vector<std::vector<double> > R(P.N,std::vector<double>(domain.nhex,0.));

        for (unsigned int j=0; j<domain.nhex; ++j) {
            P.reset();
            P.Input[inputID[0]] = domain.X[j];
            P.Input[inputID[1]] = domain.Y[j];
            P.Input[contextIDs[i]] = contextVals[i];
            P.convergeForward(-1,false);
            for(int k=0;k<P.N;k++){
                R[k][j] = P.X[k];
            }
        }
        return R;
    }


/*  ********  */

    std::vector<std::vector<std::vector<double> > > testDomains(void){

        std::vector<std::vector<std::vector<double> > > R;
        for(int i=0;i<nContext;i++){
            R.push_back(testDomainContext(i));
        }
        return R;
    }

    void run(int K, int errorSamplePeriod, int errorSampleSize, bool resetWeights){

        P.randomizeWeights(-1.0, +1.0);

        double errMin = 1e9;
        for(int k=0;k<K;k++){
            if(k%errorSamplePeriod){

                std::vector<int> sample = setRandomInput();
                setInput(sample[0],sample[1]);
                P.convergeForward(-1,true);
                P.setError(std::vector<int> (1,M[sample[0]].outputID), std::vector<double> (1,M[sample[0]].F[sample[1]]));
                P.convergeBackward(-1,false);
                P.weightUpdate();

            } else {

                double err = 0.;
                for(int j=0;j<errorSampleSize;j++){

                    std::vector<int> sample = setRandomInput();
                    setInput(sample[0],sample[1]);
                    P.convergeForward(-1,false);
                    P.setError(std::vector<int> (1,M[sample[0]].outputID), std::vector<double> (1,M[sample[0]].F[sample[1]]));
                    err += P.getError();

                }
                err /= (double)errorSampleSize;
                if(err<errMin){
                    errMin = err;
                    P.Wbest = P.W;
                } else if (resetWeights) {
                    P.W = P.Wbest;
                }
                Error.push_back(errMin);
            }

            if(fmod(k,K/100)==0){
                logfile<<"steps: "<<(int)(100*(float)k/(float)K)<<"% ("<<k<<")"<<std::endl;
            }
        }
        P.W = P.Wbest;

    }


    /*
     PLOTTING
     */

    void setColourMap(morph::ColourMapType cmap){
        colourMap = cmap;
    }

    void plotMapValues(std::vector<double> F, std::string fname, int mapIndex){
        if(M[mapIndex].N != F.size()){ std::cout<<"Field supplied not correct size (Map)."<<std::endl;}
        Visual v (500, 500, "Map");
        v.backgroundWhite();
        v.zNear = 0.001;
        v.zFar = 20;
        v.fov = 45;
        v.sceneLocked = false;
        v.setZDefault(-3.7f);
        v.setSceneTransXY (0.0f,0.0f);
        Vector<float, 3> offset  = { 0., 0., 0.0 };
        Scale<float> scale;
        scale.do_autoscale = true;
        std::vector<float> fFlt;
        for (unsigned int i=0; i<M[mapIndex].N; i++){ fFlt.push_back (static_cast<float>(F[i])); }
        v.addVisualModel (new QuadsVisual<float> (v.shaderprog, &M[mapIndex].quads, offset, &fFlt, scale, colourMap));
        v.render();
        v.render();
        v.saveImage(fname);
    }


    void plotDomainValues(std::vector<double> F, std::string fname){
        if(domain.nhex != F.size()){ std::cout<<"Field supplied not correct size (domain)"<<std::endl;}
        Visual v (500, 500, "Response");
        v.backgroundWhite();
        v.zNear = 0.001;
        v.zFar = 20;
        v.fov = 45;
        v.sceneLocked = false;
        v.setZDefault(-2.7f);
        v.setSceneTransXY (0.0f,0.0f);
        Vector<float, 3> offset  = { 0., 0., 0.0 };
        Scale<float> scale;
        scale.do_autoscale = true;
        Scale<float> zscale; zscale.setParams (0.0f, 0.0f);
        std::vector<float> fFlt;
        for (unsigned int k=0; k<domain.nhex; k++){ fFlt.push_back (static_cast<float>(F[k])); }
        v.addVisualModel (new HexGridVisual<float> (v.shaderprog, domain.hg, offset, &fFlt, zscale, scale, colourMap));
        v.render();
        v.render();
        v.saveImage(fname);
    }


    // **************************************************** //

    void plotMapTargets(void){
        for(int i=0;i<M.size();i++){
            std::stringstream ss; ss<< logpath << "/targ_map_" << i << ".png";
            plotMapTarget(i, ss.str().c_str());
        }
    }

    void plotMapTarget(int i, std::string fname){
        plotMapValues(M[i].Fscaled, fname, i);
    }

    void plotMapResponsesAllMaps(void){
        for(int i=0;i<M.size();i++){
            plotMapResponses(i);
        }
    }

    void plotMapResponses(int i){
        std::vector<std::vector<double> > R = testMap(i);
        for(int j=0;j<R.size();j++){
            std::vector<double> F = normalize(R[j]);
            std::stringstream ss; ss<< logpath << "/resp_map_" << i << "_node_" << j << ".png";
            plotMapValues(F, ss.str().c_str(), i);
        }
    }

    void plotDomainContext(int i){
        std::vector<std::vector<double> > R = testDomainContext(i);
        R = tools::normalize(R);
        for(int j=0;j<R.size();j++){
            std::stringstream ss; ss<< logpath << "/context_" << contextIDs[i] << "_val_" << contextVals[i] << "_Node_" << j << "_(indivNorm).png";
            plotDomainValues(R[j],ss.str().c_str());
        }
    }

    void plotDomainsAllContexts(void){
        std::vector<std::vector<std::vector<double> > > R = testDomains();
        R = tools::normalize(R);
        for(int i=0;i<R.size();i++){
            for(int j=0;j<R[i].size();j++){
                std::stringstream ss; ss<< logpath << "/context_" << contextIDs[i] << "_val_" << contextVals[i] << "_Node_" << j << "_(jointNorm).png";
                plotDomainValues(R[i][j],ss.str().c_str());
            }
        }
    }

    void plotDomainNodeDiff(int contextIndex, int nodeA, int nodeB){

        if(contextIndex>=nContext){std::cout<<"Invalid context ID "<<contextIndex<<". Only "<<nContext<<"contexts."<<std::endl;}
        if(nodeA>=P.N){std::cout<<"Invalid node ID (A) "<<nodeA<<". Only "<<P.N<<"nodes."<<std::endl;}
        if(nodeB>=P.N){std::cout<<"Invalid node ID (B) "<<nodeB<<". Only "<<P.N<<"nodes."<<std::endl;}

        std::vector<std::vector<double> >  A = testDomainContext(contextIndex);
        std::vector<double> diff = A[nodeA];
        for(int i=0;i<domain.nhex;i++){
            diff[i] -= A[nodeB][i];
        }
        diff = tools::normalize(diff);

        std::stringstream ss; ss<< logpath << "/DIFF_node_"<<nodeA<<"_minus_node_"<<nodeB<<"_context_" << contextIDs[contextIndex] << "_val_" << contextVals[contextIndex]<<".png";
        plotDomainValues(diff,ss.str().c_str());

    }

    void plotDomainContextDiff(int nodeIndex, int contextA, int contextB){

        if(contextA>=nContext){std::cout<<"Invalid context ID (A) "<<contextA<<". Only "<<nContext<<"contexts."<<std::endl;}
        if(contextB>=nContext){std::cout<<"Invalid context ID (B) "<<contextB<<". Only "<<nContext<<"contexts."<<std::endl;}
        if(nodeIndex>=P.N){std::cout<<"Invalid node ID "<<nodeIndex<<". Only "<<P.N<<"nodes."<<std::endl;}

        std::vector<std::vector<double> >  A = testDomainContext(contextA);
        std::vector<std::vector<double> >  B = testDomainContext(contextB);
        std::vector<double> diff = A[nodeIndex];
        for(int i=0;i<domain.nhex;i++){
            diff[i] -= B[nodeIndex][i];
        }
        diff = tools::normalize(diff);

        std::stringstream ss; ss<< logpath << "/DIFF_context_("<<contextIDs[contextA]<<","<<contextVals[contextA]<<")_minus_context_("<<contextIDs[contextB]<<","<<contextVals[contextB]<<")_node"<<nodeIndex<<".png";
        plotDomainValues(diff,ss.str().c_str());

    }

    void plotDomainContextDiffOutputNodes(int contextA, int contextB){
        for(int i=0;i<outputID.size();i++){
            plotDomainContextDiff(outputID[i], contextA, contextB);
        }

    }

};

