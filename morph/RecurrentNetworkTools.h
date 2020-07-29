
namespace tools{

    template <class T>
    std::vector<T> getUnique(std::vector<T> x){
        std::vector<T> unique;
        for(int i=0;i<x.size();i++){
            bool uni = true;
            for(int k=0;k<unique.size();k++){
                if(x[i]==unique[k]){ uni = false; break; }
            } if(uni){ unique.push_back(x[i]);}
        }
        return unique;
    }

    template <class T>
    std::vector<int> getUniqueID(std::vector<T> x){
        std::vector<int> uniqueID;
        std::vector<T> unique;
        for(int i=0;i<x.size();i++){
            bool uni = true;
            for(int k=0;k<unique.size();k++){
                if(x[i]==unique[k]){ uni = false; break; }
            } if(uni){ unique.push_back(x[i]); uniqueID.push_back(i);}
        }
        return uniqueID;
    }

    //template<typename T, size_t N>
    //std::vector<T> std::vector(const T (&data)[N]){return std::vector<T>(data,data+N);}

    int getArgmax(std::vector<double> q){
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

    int getArgmin(std::vector<double> q){
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
    T getMin(std::vector<T> x){
        T min = 1e9;
        for(int i=0;i<x.size();i++){
            if(x[i]<min){
                min = x[i];
            }
        }
        return min;
    }

    template <class T>
    T getMax(std::vector<T> x){
        T max = -1e9;
        for(int i=0;i<x.size();i++){
            if(x[i]>max){
                max = x[i];
            }
        }
        return max;
    }

    template <class T>
    std::vector<T> normalize(std::vector<T> X){
        T minX = getMin(X);
        T maxX = getMax(X);
        T norm = 1./(maxX-minX);
        for(int i=0;i<X.size();i++){
            X[i] = (X[i]-minX)*norm;
        }
        return X;
    }

    template <class T>
    std::vector<std::vector<T> > normalize(std::vector<std::vector<T> > X){
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
    std::vector<std::vector<std::vector<T> > > normalize(std::vector<std::vector<std::vector<T> > > X){
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


    std::vector<std::array<float, 12>> getQuads(std::vector<double> X, std::vector<double> Y){
        std::vector<std::array<float, 12>> quads;
        double xRange = getMax(X) - getMin(X);
        double yRange = getMax(Y) - getMin(Y);
        double xOff = -0.5*xRange;
        double yOff = -0.5*yRange;
        double maxDim = xRange;
        if(yRange>maxDim){ maxDim = yRange; }
        double xScale = xRange/maxDim;
        double yScale = yRange/maxDim;
        std::vector<double> uniqueX = getUnique(X);
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
            colRange[i] = getMax(yByCol[i])-getMin(yByCol[i]);
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


};
