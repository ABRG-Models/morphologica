#include <vector>
using std::vector;

namespace tools{

    template <class T>
    vector<T> getUnique(vector<T> x){
        vector<T> unique;
        for(int i=0;i<x.size();i++){
            bool uni = true;
            for(int k=0;k<unique.size();k++){
                if(x[i]==unique[k]){ uni = false; break; }
            } if(uni){ unique.push_back(x[i]);}
        }
        return unique;
    }

    template <class T>
    vector<int> getUniqueID(vector<T> x){
        vector<int> uniqueID;
        vector<T> unique;
        for(int i=0;i<x.size();i++){
            bool uni = true;
            for(int k=0;k<unique.size();k++){
                if(x[i]==unique[k]){ uni = false; break; }
            } if(uni){ unique.push_back(x[i]); uniqueID.push_back(i);}
        }
        return uniqueID;
    }

    template<typename T, size_t N> std::vector<T> makeVector(const T (&data)[N]){return std::vector<T>(data,data+N);}

    int getArgmax(vector<double> q){
        double maxV = -1e9;
        int maxI = 0;
        for(int i=0;i<q.size();i++){
            if(q[i]>maxV){
                maxV = q[i];
                maxI = i;
            }
        }
        return maxI;
    }

    int getArgmin(vector<double> q){
        double minV = 1e9;
        int minI = 0;
        for(int i=0;i<q.size();i++){
            if(q[i]<minV){
                minV = q[i];
                minI = i;
            }
        }
        return minI;
    }

    template <class T>
    T getMin(vector<T> x){
        T min = 1e9;
        for(int i=0;i<x.size();i++){
            if(x[i]<min){
                min = x[i];
            }
        }
        return min;
    }

    template <class T>
    T getMax(vector<T> x){
        T max = -1e9;
        for(int i=0;i<x.size();i++){
            if(x[i]>max){
                max = x[i];
            }
        }
        return max;
    }

    template <class T>
    vector<T> normalize(vector<T> X){
        T minX = getMin(X);
        T maxX = getMax(X);
        T norm = 1./(maxX-minX);
        for(int i=0;i<X.size();i++){
            X[i] = (X[i]-minX)*norm;
        }
        return X;
    }

    template <class T>
    vector<vector<T> > normalize(vector<vector<T> > X){
        T minX = 1e9;
        T maxX = -1e9;
        for(int i=0;i<X.size();i++){
            for(int j=0;j<X[i].size();j++){
                if(X[i][j]>maxX){ maxX = X[i][j]; }
                if(X[i][j]<minX){ minX = X[i][j]; }
            }
        }
        T norm = 1./(maxX-minX);
        for(int i=0;i<X.size();i++){
            for(int j=0;j<X[i].size();j++){
                X[i][j] = (X[i][j]-minX)*norm;
            }
        }
        return X;
    }

    template <class T>
    vector<vector<vector<T> > > normalize(vector<vector<vector<T> > > X){
        T minX = 1e9;
        T maxX = -1e9;
        for(int i=0;i<X.size();i++){
            for(int j=0;j<X[i].size();j++){
                for(int k=0;k<X[i][j].size();k++){
                    if(X[i][j][k]>maxX){ maxX = X[i][j][k]; }
                    if(X[i][j][k]<minX){ minX = X[i][j][k]; }
                }
            }
        }
        T norm = 1./(maxX-minX);
        for(int i=0;i<X.size();i++){
            for(int j=0;j<X[i].size();j++){
                for(int k=0;k<X[i][j].size();k++){
                    X[i][j][k] = (X[i][j][k]-minX)*norm;
                }
            }
        }
        return X;
    }

};
