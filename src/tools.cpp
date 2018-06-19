#include "tools.h"
#include <math.h>
#include <vector>
#include <armadillo>
#include <stdlib.h>

using namespace std;
using namespace arma;

namespace tools
{
    /// @param gray gray value from 0.0 to 1.0
    /// @returns RGB value in jet colormap
    vector<double> getJetColor(double gray)
    {
        double color_table[][3] = {
            {0.0, 0.0, 0.5}, // #00007F
            {0.0, 0.0, 1.0}, // blue
            {0.0, 0.5, 1.0}, // #007FFF
            {0.0, 1.0, 1.0}, // cyan
            {0.5, 1.0, 0.5}, // #7FFF7F
            {1.0, 1.0, 0.0}, // yellow
            {1.0, 0.5, 0.0}, // #FF7F00
            {1.0, 0.0, 0.0}, // red
            {0.5, 0.0, 0.0}, // #7F0000
        };

        vector<double> col;
        double ivl = 1./8.;
        for (int i=0; i<8; i++) {
            double llim = (i==0)?0.:(double)i/8.;
            double ulim = (i==7)?1.:((double)i+1.)/8.;
            if (gray >= llim && gray <= ulim) {
                for (int j=0; j<3; j++) {
                    double c = gray - llim;
                    col.push_back(color_table[i][j]*(ivl-c)/ivl + color_table[i+1][j]*c/ivl);
                }
                break;
            }
        }
        return col;
    }

    vector<double> getGrayScaleColor(double gray)
    {
        vector<double> col;
        for (int i=0; i<3; i++) {
            col.push_back(1.0 - gray);
        }
        return col;
    }

    vector<double> HSVtoRGB(double h,double s,double v) // all in range 0,1
    {
        double r, g, b;
        int i = floor(h * 6);
        double f = h * 6. - i;
        double p = v * (1. - s);
        double q = v * (1. - f * s);
        double t = v * (1. - (1. - f) * s);

        switch(i % 6){
            case 0: r = v, g = t, b = p; break;
            case 1: r = q, g = v, b = p; break;
            case 2: r = p, g = v, b = t; break;
            case 3: r = p, g = q, b = v; break;
            case 4: r = t, g = p, b = v; break;
            case 5: r = v, g = p, b = q; break;
        }
        vector<double> rgb;
        rgb.push_back(r);
        rgb.push_back(g);
        rgb.push_back(b);
        return rgb;
    }

    double randFloat(void)
    {
        return ((double) rand())/(double)RAND_MAX;
    }

    double normalDistributionValue(void)
    {
        return sqrt(-2. * log(randFloat())) * cos(2. * M_PI * randFloat());
    }

    double wrapAngle(double a)
    {
        return a-6.283185307179586*floor(a/6.283185307179586);
    }

    vector < vector <double> > rotateCloud (vector < vector <double> > cloud, double Rx, double Ry, double Rz)
    {
        // expects a n by 3 cloud as input
        arma::mat Cloud (3,static_cast<unsigned int>(cloud.size()));
        for (int p=0; p<cloud.size(); p++){
            Cloud(0,p) = cloud[p][0];
            Cloud(1,p) = cloud[p][1];
            Cloud(2,p) = cloud[p][2];
        }
        // make rotation matrix
        arma::mat RX, RY, RZ;
        RX <<1.0<<0.0<<0.0<<endr<<0.0<<cos(Rx)<<-sin(Rx)<<endr<<0.0<<sin(Rx)<<cos(Rx)<<endr;
        RY <<cos(Ry)<<0.0<<sin(Ry)<<endr<<0.0<<1.0<<0.0<<endr<<-sin(Ry)<<0.0<<cos(Ry)<<endr;
        RZ <<cos(Rz)<<-sin(Rz)<<0.0<<endr<<sin(Rz)<<cos(Rz)<<0.0<<endr<<0.0<<0.0<<1.0<<endr;

        // do rotation
        Cloud = RX*Cloud; Cloud = RY*Cloud; Cloud = RZ*Cloud;

        // copy back into vector
        for (int p=0; p<cloud.size(); p++){
            cloud[p][0] = Cloud(0,p);
            cloud[p][1] = Cloud(1,p);
            cloud[p][2] = Cloud(2,p);
        }
        return cloud;
    }

#ifdef SPHERE_ATTEMPT
    vector <vector <float> > sphere(int n, double rad)
    {
        vector <vector <float> > S (n);
        float golden_angle = acos(-1.0) * (3. - sqrt(5.));
        for (int i=0; i<n; i++){
            S[i].resize(3);
            float theta = golden_angle * (float)i;
            float z = (1.0-1.0/n)*(1.0-(2.0*(float)i/(n-1)));
            float r = sqrt(1.0 - z * z);
            S[i][0] =  rad * r * cos(theta);
            S[i][1] =  rad * r * sin(theta);
            S[i][2] =  rad * z;
        } return S;
    }
#endif

    // TAKES A LIST OF PAIRS AND NUMBER OF PAIRS AND RETURNS A VECTOR OF CLUSTERS SORTED BY CLUSTER SIZE
    vector<vector<int> > graph(vector<vector<int> > agg)
    {
        int N = agg.size();

        vector<vector<int> > conn;
        vector<int> con (2,0);
        for(int i=0;i<N;i++){
            for(int j=0;j<N;j++){
                if(agg[i][j]){
                    con[0]=i;
                    con[1]=j;
                    conn.push_back(con);
                }
            }
        }

        vector<vector<int> > graph;
        int a, b, Ain, Bin;
        bool nov, ain, bin;
        for(unsigned int k=0;k<conn.size();k++){
            a = conn[k][0];
            b = conn[k][1];
            if(a != b){
                // reset flags
                Ain = -1;
                Bin = -1;
                nov = true;
                // check if conn k is in the graph
                int I = graph.size();
                for(int i=0;i<I;i++){
                    ain = false;
                    bin = false;
                    int J = graph[i].size();
                    for(int j=0;j<J;j++){
                        if(a == graph[i][j]){
                            ain = true;
                            Ain = i;
                        }
                        if(b == graph[i][j]){
                            bin = true;
                            Bin = i;
                        }
                    }
                    if(ain && !bin){
                        graph[i].push_back(b);
                    }
                    if(!ain && bin){
                        graph[i].push_back(a);
                    }
                    if(ain||bin){
                        nov=false;
                    }
                }
                // Add a new group
                if(nov){
                    graph.push_back(conn[k]);
                }
                // Join two existing groups
                if(Ain>-1 && Bin>-1 && Ain!=Bin){
                    graph[Ain].pop_back();
                    graph[Bin].pop_back();
                    for(unsigned int l=0;l<graph[Bin].size();l++){
                        graph[Ain].push_back(graph[Bin][l]);
                    }
                    graph.erase(graph.begin()+Bin);
                }
            }
        }

        for(int k=0;k<N;k++){
            bool isolated = true;
            int I = graph.size();
            for(int i=0;i<I;i++){
                int J = graph[i].size();
                for(int j=0;j<J;j++){
                    if(k==graph[i][j]){
                        isolated = false;
                        //break;
                    }
                }
            }
            if(isolated){
                vector<int> isolate(1,k);
                graph.push_back(isolate);
            }
        }

        // sort by descending group size
        vector<vector<int> > graphSorted;
        while(graph.size()){
            unsigned int maxVal = 0;
            int maxInd = 0;
            for(unsigned int i=0;i<graph.size();i++){
                if(graph[i].size()>maxVal){
                    maxVal = graph[i].size();
                    maxInd = i;
                }
            }
            graphSorted.push_back(graph[maxInd]);
            graph.erase(graph.begin()+maxInd);
        }

        return graphSorted;
    }

    /*
      return indices of descending value in unsorted
    */
    vector<int> sort(vector<double> unsorted)
    {
        vector<int> unsortID;
        for(int i=0;i<static_cast<int>(unsorted.size());i++){
            unsortID.push_back(i);
        }
        vector<int> sortID;
        vector<double> sorted;

        while(unsorted.size()){
            double maxVal = 0.;
            int maxInd = 0;
            for(int i=0;i<static_cast<int>(unsorted.size());i++){
                if(unsorted[i]>maxVal){
                    maxVal = unsorted[i];
                    maxInd = i;
                }
            }
            sorted.push_back(unsorted[maxInd]);
            sortID.push_back(unsortID[maxInd]);
            unsorted.erase(unsorted.begin()+maxInd);
            unsortID.erase(unsortID.begin()+maxInd);
        }
        return sortID;
    }

#ifdef MATRIX_MULTIPLY_ATTEMPT
    vector < vector <double> > matrixMultiply (vector < vector <double> > a, vector < vector <double> > b)
    {
        // expects a n by 3 cloud as input
        arma::mat A (3,a.size());
        for (int p=0; p<a.size(); p++){
            A(0,p) = a[p][0];
            A(1,p) = a[p][1];
            A(2,p) = a[p][2];
        }

        arma::mat B (3,a.size());
        for (int p=0; p<b.size(); p++){
            B(0,p) = b[p][0];
            B(1,p) = b[p][1];
            B(2,p) = b[p][2];
        }

        arma::mat C = A*B;

        // copy back into vector
        for (int p=0; p<a.size(); p++){
            a[p][0] = C(0,p);
            a[p][1] = C(1,p);
            a[p][2] = C(2,p);
        }

        return a;
    }
#endif

} // namespace tools
