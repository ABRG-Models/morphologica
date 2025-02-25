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
#include <morph/rngd.h> // for morph::randDouble()

namespace morph {
    namespace nn {
        namespace recurrentnet {

            /*!
             *
             * \brief A Neural Network model, capable of learning with an arbitrary network
             * topology, with (attractor) activiation dynamics and (recurrent
             * backpropagation) learning rule as described by Pineda F.J., (1987) Physical
             * Review Letters, 59(19), p2229--2232.
             *
             * Feed-forward dynamics are given by (dx_i/dt)*tauX = -x_i + f(\sum_j x_j *
             * W_ij)+I_i, where f(x)=1/(1+exp(-x)) is a sigmoid and I_i is the external
             * input to input nodes.
             *
             * Feed-backward dynamics are given by (dy_i/dt)*tauY = -y_i + \sum_j(f_j' w_ji
             * y_j) + J_i, where f_j'=f_j(x)*(1-f_j(x)) is the derivative of the sigmoid,
             * and J_i=target_i-x_i is the discrepancy to be minimised
             *
             * Weight update is given by (dw_ij/dt)*tauW = x_i * y_j * f_j'.
             *
             * Individual connections (between pre and post units) need to be made by
             * calling connect before the network can be properly initialized, so typical
             * initialization would look like this:
             *
             *    ****
             *    RecurrentNetwork P;
             *    P.init (N,dt,tauW,tauX,tauY,divergenceThreshold,maxConvergenceSteps);
             *    for(int i=0;i<pre.size();i++){ P.connect(pre[i],post[i]); }
             *    P.addBias();
             *    P.setNet();
             *    ****
             */
            class RecurrentNetwork {

            public:

                // N  - number of nodes in the network
                // Nweight - number of connection weights in the network
                // Nplus1 - =N if nodes do not have a bias input, or =N+1 id nodes have a bias input
                // maxConvergenceSteps - maximum number of steps for the convergeForward and convergeBackward loops
                // W - vector of weights
                // X - vector of node activation values (forward pass)
                // Input - vector of input values for the input nodes
                // U - stores the feed-forward activity (before squashing)
                // Wbest - keep track of the current best weights (those yielding minimum reconstruction error)
                // Y - vector of node activation values (backward pass)
                // F - stores the feed-forward activity (after squahsing - sigmoid)
                // V - stores the backward-pass activity
                // Fprime - stores the derivative of the sigmoid
                // J - stores error term used in backward pass
                // dt - integration time constant
                // dtOverTauX - dt / tau_x (where tau_x is the time constant for the forward pass)
                // dtOverTauY - dt / tau_y (where tau_y is the time constant for the backward pass)
                // dtOverTauW - dt / tau_w (where tau_w is the time constant for the weight change)
                // Pre - vector of pre-synaptic node identities (should be same length as Post)
                // Post - vector of post-synaptic node identities (should be same length as Pre)
                // zero - useful for resetting the matrix of pointers Wptr
                // divergenceThreshold - threshold below which time-differences in total error signal convergence to a (point) attractor state
                // Wptr - pointers into the weight vector W, useful for efficiently constructing an N x N sparse weight matrix, for convenient inspection / saving

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
                    // Destructor
                }

                /*!
                 * Add a bias input (i.e., threshold) to each node by simulating an extra
                 * node with a constant activation of 1.0, and modifiable connection weights
                 * from this node to all others. Optional.
                 */
                void addBias(void){

                    for(int i=0;i<N;i++){
                        W.push_back(0.);
                        Pre.push_back(N);
                        Post.push_back(i);
                    }
                    X.push_back(1.0);
                    Nplus1 = N+1;
                    V.resize(Nplus1,0.);        // SPWHACKALERT: V.resize and Input.resize should maybe be called in setNet() in case addBias is not called - check/test?
                    Input.resize(Nplus1,0.);
                }

                //! adds a 0-weight connection between the pre-synaptic node identified by
                //! integer pre and the post-synaptic node identified by the integer post.
                void connect(int pre, int post){

                    W.push_back(0.);
                    Pre.push_back(pre);
                    Post.push_back(post);
                }

                //! set all network weights to random values from a uniform disribution in
                //! the range weightMin -- weightMax
                void randomizeWeights(double weightMin, double weightMax){

                    double weightRange = weightMax-weightMin;
                    for(int i=0;i<W.size();i++){
                        W[i] = morph::randDouble()*weightRange+weightMin;
                    }
                }

                //! Register the number of connection weights and obtain pointers to them
                //! once all connections have been set.
                void setNet(void){

                    Nweight = W.size();
                    Wbest = W;
                    Wptr.resize(Nplus1*Nplus1,&zero);
                    for(int i=0;i<Nweight;i++){
                        Wptr[Pre[i]*Nplus1+Post[i]] = &W[i];
                    }
                }

                //! initialize values of x to random values in the range -1 -- +1
                void randomizeState(void){

                    for(int i=0;i<N;i++){
                        X[i] = morph::randDouble()*2.0-1.0;
                    }
                }

                //! reset forward activity (x=0), the backward activity (y=0), and the input (I=0)
                void reset(void){

                    std::fill(X.begin(),X.end()-1,0.);
                    std::fill(Y.begin(),Y.end()-1,0.);
                    std::fill(Input.begin(),Input.end(),0.);
                }

                //! Feed-forward dynamics are given by (dx_i/dt)*tauX = -x_i + f(\sum_j x_j
                //! * W_ij)+I_i, where f(x)=1/(1+exp(-x)) is a sigmoid and I_i is the
                //! external input to input nodes.
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

                //! Compute the discrepancy between target and output values.  Supply the
                //! integer identity of the output nodes, and their corresponding double
                //! target values.
                void setError(std::vector<int> oID, std::vector<double> targetOutput){

                    std::fill(J.begin(),J.end(),0.);
                    for(int i=0;i<oID.size();i++){
                        J[oID[i]] = targetOutput[i]-X[oID[i]];
                    }
                }

                //! Feed-backward dynamics are given by (dy_i/dt)*tauY = -y_i + \sum_j(f_j'
                //! w_ji y_j) + J_i, where f_j'=f_j(x)*(1-f_j(x)) is the derivative of the
                //! sigmoid, and J_i=target_i-x_i is the discrepancy to be minimised
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

                /*!
                 * Weight update is given by (dw_ij/dt)*tauW = x_i * y_j * f_j'.
                 *
                 * Note that large weight updates are rejected, fixing an instability issue
                 * where weights (and thus error) jump to very large values as learning
                 * converges.
                 */
                void weightUpdate(void){

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

                //! returns the error = 0.5 * sum_i (target_i - x_i)**2
                double getError(void){

                    double error = 0.;
                    for(int i=0;i<N;i++){
                        error += J[i]*J[i];
                    }
                    return error * 0.5;

                }

                //! returns a 1D vector of N**2 doubles (for saving) corresponding to the
                //! flattened NxN weight matrix
                std::vector<double> getWeightMatrix(void){

                    std::vector<double> flatweightmat(Wptr.size());
                    for(int i=0;i<Wptr.size();i++){
                        flatweightmat[i] = *Wptr[i];
                    }
                    return flatweightmat;
                }


                //! iteratively apply the feed-forward dynamics until either i) the dynamics
                //! have converged (sum_i (x_i(t)-x_i(t-1))^2<divergenceThreshold, or ii) a
                //! maximum number of settling steps has occurred. If i) return
                //! converged=true, else if ii) return converged=false
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

                //! iteratively apply the feed-backward dynamics until either i) the
                //! dynamics have converged (sum_i (y_i(t)-y_i(t-1))^2<divergenceThreshold,
                //! or ii) a maximum number of settling steps has occurred. If i) return
                //! converged=true, else if ii) return converged=false
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

                //! supply a weighNudgeSize to overload, adding a random 'nudge' of +/-
                //! weightNudgeSize to each weight when the forward dynamics fail to
                //! converge
                void convergeForward(double weightNudgeSize){

                    bool converged = convergeForward();
                    if(!converged){
                        W = Wbest;
                        for(int k=0;k<Nweight;k++){
                            W[k] += (morph::randDouble()*2-1)*weightNudgeSize;
                        }
                    }
                }

                //! supply a weighNudgeSize to overload, adding a random 'nudge' of +/-
                //! weightNudgeSize to each weight when the backward dynamics fail to
                //! converge
                void convergeBackward(double weightNudgeSize){

                    bool converged = convergeBackward();
                    if(!converged){
                        W = Wbest;
                        for(int k=0;k<Nweight;k++){
                            W[k] += (morph::randDouble()*2-1)*weightNudgeSize;
                        }
                    }
                }

            };

        } // namespace recurrentnet
    } // namespace nn
} // namespace morph
