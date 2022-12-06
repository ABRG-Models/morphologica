/*!
 * \file
 *
 * \brief Implementation of recurrent backprop algorithm following Pineda (1987)
 *
 * \author Stuart Wilson
 * \date 2020
 */

#include <vector>
#include <string>
#include <array>
#include <fstream>
#include <sstream>
#include <iostream>

#include <morph/HdfData.h>
#include <morph/Visual.h>
#include <morph/QuadsVisual.h>
#include <morph/HexGridVisual.h>
#include <morph/ColourMap.h>
#include <morph/tools.h>
#include <morph/Random.h>
#include <morph/Config.h>
#include <morph/Scale.h>
#include <morph/vec.h>
#include <morph/RecurrentNetworkTools.h>
#include <morph/RecurrentNetwork.h>
#include <morph/ReadCurves.h>
#include <morph/RD_Base.h>

namespace morph {
    namespace recurrentnet {

        template <class Flt>
        class Domain : public morph::RD_Base<Flt> {

            /*
             * Provides a domain (a hexagonal lattice within a boundary) of 2D
             * coordinates, to be used as input combinations for a 2-input network whose
             * output is to be evaluated at each co-ordinate to obtain a
             * color-map. Inherets from morph::RD_Base.
             */

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

                // set size and shape of elliptical domain boundary (prior to memory allocation)

                this->ellipseA = ellipseA;
                this->ellipseB = ellipseB;
                this->hextohex_d = hextohex_d;
            }
            virtual void allocate (void) {

                // set elliptical domain boundary and allocate memory

                this->hg = new morph::HexGrid (this->hextohex_d, this->hexspan, 0);
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


        class Context{

            /*
             * Structure for storing a context (array of context node identities and
             * corresponding input values for those nodes)
             */
        public:
            std::string name;
            std::vector<int> nodeIDs;
            std::vector<double> Vals;

            Context(std::string name, std::vector<int> nodeIDs, std::vector<double> Vals){
                this->name = name;
                this->nodeIDs = nodeIDs;
                this->Vals = Vals;
            }
        };

        /*!
         * Structure for storing a map (pre-defined X and F vectors in a HdfData
         * file). The number of map points N is determined by the length of the suppled
         * F vector, and the length of X should be an integer multiple of N. This
         * integer multiplier M should correspond to the number of input nodes, e.g., if
         * M=3*N it is assumed the the first N values of X are the input values for the
         * first input node, the second N values are for the second input node, and the
         * third N values are for the third input node. The length of input node
         * identities supplied via the 'inputID' array in config.json should be of
         * length M. When using a common 'domain' onto which to project the network
         * outputs the assumption is that the first two 'inputID' values correspond to X
         * and Y coordinate, i.e., specifying locations on a 2D sheet.
         */
        class Map{

        public:

            // N - number of map points
            // X - (number of inputs) x (number of map points) matrix of input values
            // maxX, minX - min and max values of the map points for each input
            // F - target output value corresponding to the input combination at each map point
            // Fscaled - copy of output values scaled between 0 and 1 (for plotting). SWHACKALERT: maybe redundant given that plotting functions do autoscaling (check not used elsewhere).
            // minF, maxF - SWHACKALERT: looks to be unused.
            // outputID - integer identity of the network node to which this map is assigned
            // contextID - integer identity of the context (in the vector<int> C of RecurrentNetworkModel) in which training data from this map are to be sampled
            // contextVal - SWHACKALERT: looks to now be unused.
            // quads - specification of the quad information required to plot values from this map as a 2D image

            int N;
            std::vector<std::vector<double> > X;
            std::vector<double> maxX, minX;
            std::vector<double> F, Fscaled;
            double minF, maxF;
            int outputID, contextID;
            double contextVal;
            std::vector<std::array<float, 12>> quads;

            /*!
             * Initialize a map from a .h5 file (specified by filename), which should
             * contain X and F vectors (both 1D).
             */
            void init(std::string filename){

                outputID = -1; // flag for not set
                contextID = -1; // flag for not set
                contextVal = 0.0; // default

                HdfData network(filename,1);

                network.read_contained_vals ("F", F);
                N = F.size();

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

                quads = tools::getQuads(X[0],X[1]);

            }

            Map(std::string filename){
                init(filename);
            }

            Map(std::string filename,int outputID, int contextID){
                init(filename);
                this->outputID = outputID;
                this->contextID = contextID;
            }

        };



        /*!
         * Combines the recurrent network algorithm (P) with a vector of input-output
         * training 'maps' (M), a vector of training 'contexts' (C), and a domain for 2D
         * color-map plotting, and provides methods for saving and plotting the
         * responses of the network, and saving and loading the weights. Contains the
         * run() method, which trains the network and keeps track of the error.
         */
        class RecurrentNetworkModel{


        public:

            // logpath - path to directory containing config.json and to be populated with log.txt
            // logfile - file object for logging details
            // inputs - SWHACKALERT: maybe not actually used! consider removing
            // Error - vector of (mean) error, updated during run()
            // P - recurrent network object
            // M - vector of input/output 'map' objects
            // C - vector of training 'contexts' (each a combination of additional inputs for 'context nodes')
            // domain - hexagonal lattice for constructing 2D network response plots
            // inputID - integer identity of the input nodes
            // outputID - SWHACKALERT: essentially redundant (each maps has an outputID) - update plotDomainContextDiffOutputNodes and remove.
            // nContext - number of different contexts
            // colorMap - global colorMap object to set before each plotting call.

            std::string logpath;
            std::ofstream logfile;
            std::vector<double> inputs, Error;
            morph::recurrentnet::RecurrentNetwork P;
            std::vector<Map> M;
            std::vector<Context> C;
            morph::recurrentnet::Domain<double> domain;
            std::vector<int> inputID;
            std::vector<int> outputID;
            int nContext;
            morph::ColourMapType colourMap;
            morph::RandUniform<double> rng;

            /*!
             * Initialize the RecurrentNetworkModel. Logpath should be a folder
             * containing a config.h which should contain 'contexts' and 'maps' arrays,
             * and a network.h5 file containing the 'pre' and 'post' arrays that specify
             * the network connectivity.
             *
             * SWHACKALERT: The 'pre' and 'post' vectors could be specified in the
             * config to simplify things, now that I know that its straightforward to
             * write these into a config file from python.
             */
            RecurrentNetworkModel(std::string logpath){

                // setup log file
                this->logpath = logpath;
                morph::Tools::createDir (logpath);
                { std::stringstream ss; ss << logpath << "/log.txt"; logfile.open(ss.str());}
                logfile<<"Hello."<<std::endl;

                // Read in network params
                morph::Config conf;
                { std::stringstream ss; ss << logpath <<"/config.json"; conf.init (ss.str()); }

                float dt = conf.getFloat("dt",1.0);
                float tauW = conf.getFloat("tauW",32.0);
                float tauX = conf.getFloat("tauX",1.0);
                float tauY = conf.getFloat("tauY",1.0);
                //float weightNudgeSize = conf.getFloat("weightNudgeSize",0.001);
                float divergenceThreshold = conf.getFloat("divergenceThreshold",0.000001);
                int maxConvergenceSteps = conf.getInt("maxConvergenceSteps",400);

                float dx = conf.getFloat("dx",0.02);
                float yAspect = conf.getFloat("yAspect",0.75);
                float scaleDomain = conf.getFloat("scaleDomain",1.5);

                // Read in contexts info
                const nlohmann::json ctx = conf.get("contexts");
                nContext = static_cast<unsigned int>(ctx.size());
                for (unsigned int i = 0; i < ctx.size(); ++i) {
                    nlohmann::json ctxi = ctx[i];
                    std::string name = ctxi["name"].get<std::string>();
                    nlohmann::json cid = ctxi["ID"];
                    std::vector<int> cID;
                    for (unsigned int j = 0; j < cid.size(); ++j) {
                        cID.push_back(cid[j].get<int>());
                    }
                    nlohmann::json cval = ctxi["Val"];
                    std::vector<double> cVal;
                    for (unsigned int j = 0; j < cval.size(); ++j) {
                        cVal.push_back(cval[j].get<double>());
                    }
                    C.push_back(Context(name, cID, cVal));
                }
                if(nContext==0){
                    C.push_back(Context("null",std::vector<int>(1,0), std::vector<double>(1,0.0))); ////////CHECK THIS *******!!!!!
                }

                // Read in maps info
                const nlohmann::json maps = conf.get("maps");
                for(int i=0;i<maps.size();i++){
                    std::string fn = maps[i].contains("filename") ? maps[i]["filename"].get<std::string>() : std::string("unknown_map");
                    std::stringstream ss; ss << logpath <<"/"<<fn;
                    logfile<<"Map["<<i<<"]:"<<ss.str()<<std::endl;
                    int oID = maps[i].contains("outputID") ? maps[i]["outputID"].get<int>() : -1;
                    int cID = maps[i].contains("contextID") ? maps[i]["contextID"].get<int>() : 0;
                    M.push_back(Map(ss.str(),oID,cID));
                }

                for(int i=0;i<M.size();i++){
                    if(M[i].outputID!=-1){
                        outputID.push_back(M[i].outputID);
                    }
                }
                outputID = tools::getUnique(outputID);

                inputID.resize(2,0);
                inputID[1] = 1;
                const nlohmann::json inp = conf.get("inputID");
                for(int i=0;i<inp.size();i++){
                    inputID.push_back(inp[i].get<int>());
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
                P.init (N,dt,tauW,tauX,tauY,divergenceThreshold,maxConvergenceSteps);
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

                setColourMap(morph::ColourMapType::Viridis);

            }

            //! save the response of the network to all points in the map indexed (in M) by mapID.
            void saveMapResponse(int mapID) {

                std::vector<std::vector<double> > r = testMap(mapID);
                std::vector<double> response;
                for(int i=0;i<r.size();i++){
                    for(int j=0;j<r[i].size();j++){
                        response.push_back(r[i][j]);
                    }
                }
                std::stringstream fname; fname << logpath << "/responseForMap_"<<mapID<<".h5";
                HdfData outdata(fname.str());
                outdata.add_contained_vals ("response", response);
            }

            //! save the error (over time) into error.h5
            void saveError(void) {

                std::stringstream fname; fname << logpath << "/error.h5";
                HdfData errordata(fname.str());
                errordata.add_contained_vals ("error", Error);
            }

            //! save the weights into weights.h5
            void saveWeights(void){

                std::stringstream fname; fname << logpath << "/weights.h5";
                HdfData weightdata(fname.str());
                weightdata.add_contained_vals ("weights", P.W);
                std::vector<double> flatweightmat = P.getWeightMatrix();
                weightdata.add_contained_vals ("weightmat", flatweightmat);
            }

            //! load the weights from weights.h5
            void loadWeights(void) {

                std::stringstream fname; fname << logpath << "/weights.h5";
                HdfData loaded(fname.str(),1);
                loaded.read_contained_vals ("weights", P.W);
                P.Wbest = P.W;
            }

            //! Destructor.
            ~RecurrentNetworkModel(void){

                logfile<<"Goodbye."<<std::endl;
                logfile.close();
            }

            /*!
             * Sets the values of P.Input corresponding to map mapID and map point
             * locID, as well as the input values of the appropriate context nodes.
             */
            void setInput(int mapID, int locID){

                P.reset();
                for(int i=0; i<inputID.size(); i++){
                    P.Input[inputID[i]] = M[mapID].X[i][locID];
                }
                for(int i=0;i<C[M[mapID].contextID].nodeIDs.size();i++){
                    P.Input[C[M[mapID].contextID].nodeIDs[i]] += C[M[mapID].contextID].Vals[i];
                }
            }

            /*!
             * Rerurns a randomly chosen index into the vector of maps M (sample[0]),
             * and a randomly chosen index into its map points (sample[1]).
            */
            std::vector<int> setRandomInput(void){

                std::vector<int> sample(2,floor(this->rng.get()*M.size()));
                sample[1] = floor(this->rng.get()*M[sample[0]].N);
                return sample;
            }


            /*!
             * Tests the network by supplying each input value combination specified in
             * the map indexed by mapID.
             *
             * Returns a (num. nodes) x (num. map values) array of settled node response
             * values.
             */
            std::vector<std::vector<double> > testMap(int mapID){

                std::vector<std::vector<double> > response(P.N,std::vector<double>(M[mapID].N,0.));
                for(int j=0;j<M[mapID].N;j++){
                    setInput(mapID,j);
                    P.convergeForward();
                    for(int k=0;k<P.N;k++){
                        response[k][j] = P.X[k];
                    }
                }
                return response;
            }


            /*!
             * Evaluates input coordinates on the domain for context i. Return a
             * (num. nodes) x (num. domain points) matrix of settled activation values.
             */
            std::vector<std::vector<double> > testDomainContext(int i){

                std::vector<std::vector<double> > R(P.N,std::vector<double>(domain.nhex,0.));

                for (unsigned int j=0; j<domain.nhex; ++j) {
                    P.reset();
                    P.Input[inputID[0]] = domain.X[j];  // SWHACKALERT: THIS ASSUMES FIRST 2 INPUTS ARE X and Y
                    P.Input[inputID[1]] = domain.Y[j];

                    for(int k=0;k<C[i].nodeIDs.size();k++){
                        P.Input[C[i].nodeIDs[k]] = C[i].Vals[k];
                    }

                    P.convergeForward();
                    for(int k=0;k<P.N;k++){
                        R[k][j] = P.X[k];
                    }
                }
                return R;
            }


            /*  ********  */

            /*!
             * Evaluates input coordinates on the domain for each context. Return a
             * (num. contexts) x (num. nodes) x (num. domain points) matrix of settled
             * activation values.
             */
            std::vector<std::vector<std::vector<double> > > testDomains(void){

                std::vector<std::vector<std::vector<double> > > R;
                for(int i=0;i<nContext;i++){
                    R.push_back(testDomainContext(i));
                }
                return R;
            }

            /*!
             * The top-level algorithm for training the network. Supply the number of
             * training iterations K, and the number of iterations between sampling the
             * error (across all map points in all maps).
             *
             * Network weights are initialized to uniform random values in the range -1
             * and +1.
             *
             * If the total error at a given sample exceeds twice the running minimum
             * total error, the weights are reset to the value at which that running
             * minimum error was obtained.
             */
            void run(int K, int errorSamplePeriod){

                P.randomizeWeights(-1.0, +1.0);
                double errMin = 1e9;
                for(int k=0;k<K;k++){
                    if(k%errorSamplePeriod){
                        std::vector<int> sample = setRandomInput();
                        setInput(sample[0],sample[1]);
                        P.convergeForward();
                        P.setError(std::vector<int> (1,M[sample[0]].outputID), std::vector<double> (1,M[sample[0]].F[sample[1]]));
                        P.convergeBackward();
                        P.weightUpdate();
                    } else {
                        double err = 0.;
                        int count = 0;
                        for(int i=0;i<M.size();i++){
                            for(int j=0;j<M[i].N;j++){
                                setInput(i,j);
                                P.convergeForward();
                                P.setError(std::vector<int> (1,M[i].outputID), std::vector<double> (1,M[i].F[j]));
                                err += P.getError();
                                count++;
                            }
                        }
                        err /= (double)count;
                        if(err<errMin){ errMin = err; }
                        if (err>2.0*errMin) { P.W = P.Wbest; }
                        else { P.Wbest = P.W; }
                        Error.push_back(err);
                    }
                    if(fmod(k,K/100)==0){ logfile<<"steps: "<<(int)(100*(float)k/(float)K)<<"% ("<<k<<")"<<std::endl; }
                }
                P.W = P.Wbest;
            }

            /*
             * PLOTTING
             */

            void setColourMap(morph::ColourMapType cmap){

                // set the colour map

                colourMap = cmap;
            }

            /*!
             * plot values from F over the map indexed (in M) by mapIndex, i.e., using
             * its quads structure, and save the result in file fname.  F is assumed to
             * be derived from a function that iterated over the map
             * locations. SWHACKALERT: Shoulf really make this like plotDomainValues
             * below so that if supplied color_min == color_max then the caxis is
             * autoscaled.
             */
            void plotMapValues(std::vector<double> F, std::string fname, int mapIndex){

                if(M[mapIndex].N != F.size()){ std::cout<<"Field supplied not correct size (Map)."<<std::endl;}
                morph::Visual v (500, 500, "Map");
                v.backgroundWhite();
                v.zNear = 0.001;
                v.zFar = 20;
                v.fov = 45;
                v.sceneLocked = false;
                v.setZDefault(-3.7f);
                v.setSceneTransXY (0.0f,0.0f);
                morph::vec<float, 3> offset  = { 0., 0., 0.0 };
                morph::Scale<float> scale;
                scale.do_autoscale = true;
                std::vector<float> fFlt;
                for (unsigned int i=0; i<M[mapIndex].N; i++){ fFlt.push_back (static_cast<float>(F[i])); }
                v.addVisualModel (new QuadsVisual<float> (v.shaderprog, &M[mapIndex].quads, offset, &fFlt, scale, colourMap));
                v.render();
                v.render();
                v.saveImage(fname);
            }

            /*!
             * SWHACKALERT: Should probably remove this one, as the overloaded version
             * below is equivalent when color_min == color_max
             */
            void plotDomainValues(std::vector<double> F, std::string fname){

                if(domain.nhex != F.size()){ std::cout<<"Field supplied not correct size (domain)"<<std::endl;}
                morph::Visual v (500, 500, "Response");
                v.backgroundWhite();
                v.zNear = 0.001;
                v.zFar = 20;
                v.fov = 45;
                v.sceneLocked = false;
                v.setZDefault(-2.7f);
                v.setSceneTransXY (0.0f,0.0f);
                morph::vec<float, 3> offset  = { 0., 0., 0.0 };
                morph::Scale<float> scale;
                scale.do_autoscale = true;
                morph::Scale<float> zscale; zscale.setParams (0.0f, 0.0f);
                std::vector<float> fFlt;
                for (unsigned int k=0; k<domain.nhex; k++){ fFlt.push_back (static_cast<float>(F[k])); }
                v.addVisualModel (new HexGridVisual<float> (v.shaderprog, v.tshaderprog, domain.hg, offset, &fFlt, zscale, scale, colourMap));
                v.render();
                v.render();
                v.saveImage(fname);
            }

            /*!
             * plot values from F over the domain (assumed to be derived from a function
             * that iterated over the domain values) and save the result in file
             * fname. If color_min == color_max then the caxis is autoscaled.
             */
            void plotDomainValues(std::vector<double> F, std::string fname, double color_min, double color_max){

                if(domain.nhex != F.size()){ std::cout<<"Field supplied not correct size (domain)"<<std::endl;}
                morph::Visual v (500, 500, "Response");
                v.backgroundWhite();
                v.zNear = 0.001;
                v.zFar = 20;
                v.fov = 45;
                v.sceneLocked = false;
                v.setZDefault(-2.7f);
                v.setSceneTransXY (0.0f,0.0f);
                morph::vec<float, 3> offset  = { 0., 0., 0.0 };
                morph::Scale<float> scale;

                if(color_min==color_max){
                    scale.do_autoscale = true;
                } else {
                    scale.do_autoscale = false;
                    float color_grad = 1.0f/(color_max-color_min);
                    scale.setParams(color_grad,-(color_grad*color_min));
                }
                morph::Scale<float> zscale; zscale.setParams (0.0f, 0.0f);
                std::vector<float> fFlt;
                for (unsigned int k=0; k<domain.nhex; k++){ fFlt.push_back (static_cast<float>(F[k])); }
                v.addVisualModel (new HexGridVisual<float> (v.shaderprog,v.tshaderprog, domain.hg, offset, &fFlt, zscale, scale, colourMap));
                v.render();
                v.render();
                v.saveImage(fname);
            }

            // **************************************************** //

            /*!
             * plot the supplied data for all maps in M, saving as
             * targ_map_i.png. SWHACKALERT: Probably don't need to supply Fscaled
             * (plotting will autoscale anyway).
             */
            void plotMapTargets(void){

                for(int i=0;i<M.size();i++){
                    std::stringstream ss; ss<< logpath << "/targ_map_" << i << ".png";
                    plotMapTarget(i, ss.str().c_str());
                }
            }

            /*!
             * plot the supplied data for map indexed by i (in M), and save as
             * fname. SWHACKALERT: Probably don't need to supply Fscaled (plotting will
             * autoscale anyway).
             */

            void plotMapTarget(int i, std::string fname){
                plotMapValues(M[i].Fscaled, fname, i);
            }

            void plotMapResponsesAllMaps(void){
                for(int i=0;i<M.size();i++){
                    plotMapResponses(i);
                }
            }

            /*!
             * test the response of all nodes to input combinations specified by map i,
             * plot, and save. Normalizes the responses across all nodes first.
             */
            void plotMapResponses(int i){

                std::vector<std::vector<double> > R = testMap(i);
                for(int j=0;j<R.size();j++){
                    std::vector<double> F = tools::normalize(R[j]);
                    std::stringstream ss; ss<< logpath << "/resp_map_" << i << "_node_" << j << ".png";
                    plotMapValues(F, ss.str().c_str(), i);
                }
            }

            void plotDomainContext(int i){

                std::vector<std::vector<double> > R = testDomainContext(i);
                R = tools::normalize(R);
                for(int j=0;j<R.size();j++){
                    std::stringstream ss; ss<< logpath << "/context_" << C[i].name << "_Node_" << j << "_(indivNorm).png";
                    plotDomainValues(R[j],ss.str().c_str());
                }
            }

            void plotDomainsAllContexts(void){
                std::vector<std::vector<std::vector<double> > > R = testDomains();
                R = tools::normalize(R);
                for(int i=0;i<R.size();i++){
                    for(int j=0;j<R[i].size();j++){
                        std::stringstream ss; ss<< logpath << "/context_" << C[i].name << "_Node_" << j << "_(jointNorm).png";
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
                double minVal = tools::getMin(diff);
                double maxVal = tools::getMax(diff);
                diff = tools::normalize(diff);

                std::stringstream ss; ss<< logpath << "/DIFF_node_"<<nodeA<<"_minus_node_"<<nodeB<<"_context_" << C[contextIndex].name << ".png";
                plotDomainValues(diff,ss.str().c_str());

                // print out the colormap axis values
                std::cout<<logpath<<std::endl;
                std::cout<<"Min: "<<minVal<<std::endl;
                std::cout<<"Max: "<<maxVal<<std::endl;
                if((minVal<0) && (maxVal>0)){
                    std::cout<<"0 at: "<<(-minVal)/(maxVal-minVal)<<std::endl;
                } else {
                    std::cout<<"0 at: off the scale."<<std::endl;
                }


            }

            void plotDomainContextDiff(int nodeIndex, int contextA, int contextB, double cmin, double cmax){

                if(contextA>=nContext){std::cout<<"Invalid context ID (A) "<<contextA<<". Only "<<nContext<<"contexts."<<std::endl;}
                if(contextB>=nContext){std::cout<<"Invalid context ID (B) "<<contextB<<". Only "<<nContext<<"contexts."<<std::endl;}
                if(nodeIndex>=P.N){std::cout<<"Invalid node ID "<<nodeIndex<<". Only "<<P.N<<"nodes."<<std::endl;}

                std::vector<std::vector<double> >  A = testDomainContext(contextA);
                std::vector<std::vector<double> >  B = testDomainContext(contextB);
                std::vector<double> diff = A[nodeIndex];
                for(int i=0;i<domain.nhex;i++){
                    diff[i] -= B[nodeIndex][i];
                }
                double minVal = tools::getMin(diff);
                double maxVal = tools::getMax(diff);

                std::stringstream ss; ss<< logpath << "/DIFF_context_("<<C[contextA].name<<")_minus_context_("<< C[contextB].name<<")_node"<<nodeIndex<<".png";
                plotDomainValues(diff,ss.str().c_str(),cmin,cmax);

                std::cout<<logpath<<std::endl;
                std::cout<<"Min: "<<minVal<<std::endl;
                std::cout<<"Max: "<<maxVal<<std::endl;
                if((minVal<0) && (maxVal>0)){
                    std::cout<<"0 at: "<<(-minVal)/(maxVal-minVal)<<std::endl;
                } else {
                    std::cout<<"0 at: off the scale."<<std::endl;
                }


            }

            void plotDomainContextDiffOutputNodes(int contextA, int contextB){
                for(int i=0;i<outputID.size();i++){
                    plotDomainContextDiff(outputID[i], contextA, contextB, 0.0, 0.0);
                }
            }

        };

    } // namespace recurrentnet
} // namespace morph
