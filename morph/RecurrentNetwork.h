/*!
 * \file
 *
 * \brief Provides a recurrent neural network class
 *
 * \author Stuart Wilson
 * \date 2020
 */

#include <vector>
#include <algorithm>

class RecurrentNetwork {

/*!
 * \brief The first line is marked brief and acts as a summary
 *
 * Further lines, a longer description
 * Use \a somearg to refer to function arguments in your descriptive summary
 * You can then describe each function arg like this:
 *
 * \param somearg An argument to do something
 *
 * Template params:
 *
 * \tparam T Some type
 *
 * Return values are:
 * \return 0 on success and a bomb on failure
 */

public:
    int N, Nweight, Nplus1, maxConvergenceSteps;
    std::vector<double> W, X, Input, U, Wbest, Y, F, V, Fprime, J;
    double dt, dtOverTauX, dtOverTauY, dtOverTauW;
    std::vector<int> Pre, Post;
    double zero, divergenceThreshold;
    std::vector<double*> Wptr;

    RecurrentNetwork(void){

    }

    RecurrentNetwork(int N, double dt, double tauW, double tauX, double tauY, double divergenceThreshold, int maxConvergenceSteps){

        init(N, dt, tauW, tauX, tauY, divergenceThreshold, maxConvergenceSteps);
    }

    void init(int N, double dt, double tauW, double tauX, double tauY, double divergenceThreshold, int maxConvergenceSteps){

        this->N=N;
        X.resize(N,0.);
        U.resize(N,0.);
        Y.resize(N,0.);
        F.resize(N,0.);
        J.resize(N,0.);
        Fprime.resize(N,0.);
        Nplus1 = N; // overwrite if bias
        this->divergenceThreshold= divergenceThreshold * N;
        this->maxConvergenceSteps= maxConvergenceSteps;
        zero = 0.0;
        this->dt = dt;
        dtOverTauW = dt/tauW;
        dtOverTauX = dt/tauX;
        dtOverTauY = dt/tauY;
    }

    ~RecurrentNetwork(void){

    }

    void addBias(void){
        for(int i=0;i<N;i++){
            W.push_back(0.);
            Pre.push_back(N);
            Post.push_back(i);
        }
        X.push_back(1.0);
        Nplus1 = N+1;
        V.resize(Nplus1,0.);
        Input.resize(Nplus1,0.);
    }

    void connect(int pre, int post){
        W.push_back(0.);
        Pre.push_back(pre);
        Post.push_back(post);
    }

    void randomizeWeights(double weightMin, double weightMax){
        double weightRange = weightMax-weightMin;
        for(int i=0;i<W.size();i++){
            W[i] = morph::Tools::randDouble()*weightRange+weightMin;
        }
    }

    void setNet(void){
        Nweight = W.size();
        Wbest = W;
        Wptr.resize(Nplus1*Nplus1,&zero);
        for(int i=0;i<Nweight;i++){
            Wptr[Pre[i]*Nplus1+Post[i]] = &W[i];
        }
    }

    void randomizeState(void){
        for(int i=0;i<N;i++){
            X[i] = morph::Tools::randDouble()*2.0-1.0;
        }
    }

    void reset(void){
        std::fill(X.begin(),X.end()-1,0.);
        std::fill(Y.begin(),Y.end()-1,0.);
        std::fill(Input.begin(),Input.end(),0.);
    }


   void forward(void){

        std::fill(U.begin(),U.end(),0.);

        // Don't OMP this loop - buggy!
        for(int k=0;k<Nweight;k++){
            U[Post[k]] += X[Pre[k]] * W[k];
        }
        //#pragma omp parallel for
        for(int i=0;i<N;i++){
            F[i] = 1./(1.+exp(-U[i]));
        }

        //#pragma omp parallel for
        for(int i=0;i<N;i++){
            X[i] +=dtOverTauX* ( -X[i] + F[i] + Input[i] );
        }

    }

    void setError(std::vector<int> oID, std::vector<double> targetOutput){
        std::fill(J.begin(),J.end(),0.);
        for(int i=0;i<oID.size();i++){
            J[oID[i]] = targetOutput[i]-X[oID[i]];
        }
    }


    void backward(void){

        //#pragma omp parallel for
        for(int i=0;i<N;i++){
            Fprime[i] = F[i]*(1.0-F[i]);
        }

        std::fill(V.begin(), V.end()-1,0.);

        //#pragma omp parallel for
        for(int k=0;k<Nweight;k++){
            V[Pre[k]] += Fprime[Post[k]] * W[k] * Y[Post[k]];
        }

        //#pragma omp parallel for
        for(int i=0;i<N;i++){
            Y[i] +=dtOverTauY * (V[i] - Y[i] + J[i]);
        }

    }

    void weightUpdate(void){

        /*
            Weight update. Note that large weight updates are rejected, which seems to fix a stability issue causing weights (and thus error) to jump to very large values when learning starts to converge.
        */

        //#pragma omp parallel for
        double delta;
        for(int k=0;k<Nweight;k++){
            delta = (X[Pre[k]] * Y[Post[k]] * Fprime[Post[k]]);
            if(delta<-1.0){
                W[k] -= dtOverTauW;
            } else if (delta>1.0) {
                W[k] += dtOverTauW;
            } else {
                W[k] += dtOverTauW*delta;
            }
        }
    }

    double getError(void){
        double error = 0.;
        for(int i=0;i<N;i++){
            error += J[i]*J[i];
        }
        return error * 0.5;

    }

    std::vector<double> getWeightMatrix(void){
        std::vector<double> flatweightmat(Wptr.size());
        for(int i=0;i<Wptr.size();i++){
            flatweightmat[i] = *Wptr[i];
        }
        return flatweightmat;
    }


    bool convergeForward(void){
        std::vector<double> Xpre(N,0.);
        double total = N;
        for(int t=0;t<maxConvergenceSteps;t++){
            if(total>divergenceThreshold){
                Xpre=X;
                forward();
                total = 0.0;
                for(int i=0;i<N;i++){ total +=(X[i]-Xpre[i])*(X[i]-Xpre[i]); }
            } else {
                return true;
            }
        }
        return false;
    }

    bool convergeBackward(void){
        std::vector<double> Ypre(N,0.);
        double total = N;
        for(int t=0;t<maxConvergenceSteps;t++){
            if(total>divergenceThreshold){
                Ypre=Y;
                backward();
                total = 0.0;
                for(int i=0;i<N;i++){ total +=(Y[i]-Ypre[i])*(Y[i]-Ypre[i]); }
            } else {
                return true;
            }
        }
        return false;
    }

   void convergeForward(double weightNudgeSize){
        bool converged = convergeForward();
        if(!converged){
            W = Wbest;
            for(int k=0;k<Nweight;k++){
                W[k] += (morph::Tools::randDouble()*2-1)*weightNudgeSize;
            }
        }
    }

    void convergeBackward(double weightNudgeSize){
        bool converged = convergeBackward();
        if(!converged){
            W = Wbest;
            for(int k=0;k<Nweight;k++){
                W[k] += (morph::Tools::randDouble()*2-1)*weightNudgeSize;
            }
        }
    }







/*
    void convergeForward(int ko, bool nudge){
        bool knockout = (ko>=0);
        std::vector<double> Xpre(N,0.);
        double total = N;
        for(int t=0;t<maxConvergenceSteps;t++){
            if(total>divergenceThreshold){
                Xpre=X;
                if(knockout){ X[ko]=0; }
                forward();
                total = 0.0;
                for(int i=0;i<N;i++){
                    total +=(X[i]-Xpre[i])*(X[i]-Xpre[i]);
                }
            } else {
                if(knockout){ X[ko]=0.; }
                return;
            }
        }
        if(nudge){
            W = Wbest;
            for(int k=0;k<Nweight;k++){
                W[k] += (morph::Tools::randDouble()*2-1)*weightNudgeSize;
            }
        }
    if(knockout){ X[ko]=0.; }
    }

    void convergeBackward(int ko, bool nudge){
        bool knockout = (ko>=0);
        std::vector<double> Ypre(N,0.);
        double total = N;
        for(int t=0;t<maxConvergenceSteps;t++){
            if(total>divergenceThreshold){
                Ypre=Y;
                if(knockout){ Y[ko]=0.; }
                backward();
                total = 0.0;
                for(int i=0;i<N;i++){
                    total +=(Y[i]-Ypre[i])*(Y[i]-Ypre[i]);
                }
            } else {
                if(knockout){ Y[ko]=0.; }
                return;
            }
        }
        if(nudge){
            W = Wbest;
            for(int k=0;k<Nweight;k++){
                W[k] += (morph::Tools::randDouble()*2-1)*weightNudgeSize;
            }
        }
        if(knockout){ Y[ko]=0.; }
    }
    */
};
