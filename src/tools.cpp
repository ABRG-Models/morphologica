#include "tools.h"
#include <math.h>
#include <vector>
#include <armadillo>
#include <stdlib.h>
#include <stdexcept>

#include <stdio.h>
#ifdef __WIN__
# include <direct.h>
# define GetCurrentDir _getcwd
#else
# include <unistd.h>
# define GetCurrentDir getcwd
#endif

using namespace std;
using namespace arma;

/// @param gray gray value from 0.0 to 1.0
/// @returns RGB value in jet colormap
array<float,3>
morph::Tools::getJetColorF (double gray)
{
    float color_table[][3] = {
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

    array<float,3> col;
    float ivl = 1.0/8.0;
    for (int i=0; i<8; i++) {
        double llim = (i==0)?0.:(double)i/8.;
        double ulim = (i==7)?1.:((double)i+1.)/8.;
        if (gray >= llim && gray <= ulim) {
            for (int j=0; j<3; j++) {
                float c = static_cast<float>(gray - llim);
                col[j] = (color_table[i][j]*(ivl-c)/ivl + color_table[i+1][j]*c/ivl);
            }
            break;
        }
    }
    return col;
}

vector<double>
morph::Tools::getJetColor (double gray)
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
    double ivl = 1.0/8.0;
    for (int i=0; i<8; i++) {
        double llim = (i==0)?0.:(double)i/8.;
        double ulim = (i==7)?1.:((double)i+1.)/8.;
        if (gray >= llim && gray <= ulim) {
            for (int j=0; j<3; j++) {
                double c = gray - llim;
                col.push_back (color_table[i][j]*(ivl-c)/ivl + color_table[i+1][j]*c/ivl);
            }
            break;
        }
    }
    return col;
}

vector<double>
morph::Tools::getGrayScaleColor(double gray)
{
    vector<double> col;
    for (int i=0; i<3; i++) {
        col.push_back(1.0 - gray);
    }
    return col;
}

//vector<double>
array<float,3>
morph::Tools::HSVtoRGB(double h,double s,double v) // all in range 0,1
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
    //vector<double> rgb;
    array<float, 3> rgb;
    rgb[0] = r;
    rgb[1] = g;
    rgb[2] = b;
    //rgb.push_back(r);
    //rgb.push_back(g);
    //rgb.push_back(b);
    return rgb;
}

#if 0 // This was confusingly named to return a double, rather than a float.
double
morph::Tools::randFloat(void)
{
    return ((double) rand())/(double)RAND_MAX;
}
#endif

double
morph::Tools::randDouble (void)
{
    return static_cast<double>(rand()) / static_cast<double>(RAND_MAX);
}

float
morph::Tools::randSingle (void)
{
    return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
}

double
morph::Tools::normalDistributionValue(void)
{
    return sqrt(-2. * log(randDouble())) * cos(2. * M_PI * randDouble());
}

double
morph::Tools::wrapAngle(double a)
{
    return a-6.283185307179586*floor(a/6.283185307179586);
}

vector < vector <double> > rotateCloud (vector < vector <double> > cloud, double Rx, double Ry, double Rz)
{
    // expects a n by 3 cloud as input
    arma::mat Cloud (3,static_cast<unsigned int>(cloud.size()));
    for (unsigned int p=0; p<cloud.size(); p++){
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
    for (unsigned int p=0; p<cloud.size(); p++){
        cloud[p][0] = Cloud(0,p);
        cloud[p][1] = Cloud(1,p);
        cloud[p][2] = Cloud(2,p);
    }
    return cloud;
}

#ifdef SPHERE_ATTEMPT
vector<vector<float> >
morph::Tools::sphere (int n, double rad)
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
vector<vector<int> >
morph::Tools::graph (vector<vector<int> > agg)
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

vector<int>
morph::Tools::sort (vector<double> unsorted)
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
vector<vector<double> >
morph::Tools::matrixMultiply (vector < vector <double> > a, vector < vector <double> > b)
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

int
morph::Tools::ensureUnixNewlines (std::string& input)
{
    int num = 0;

    for (unsigned int i=0; i<input.size(); i++) {
        if (input[i] == '\r') {
            input.erase(i,1);
            num++;
        }
    }

    return num; // The number of \r characters we found in the string.
}

string
morph::Tools::getPwd (void)
{
    char b[FILENAME_MAX];
    GetCurrentDir (b, FILENAME_MAX);
    return string(b);
}

int
morph::Tools::stripTrailingCarriageReturn (std::string& input)
{
    if (input[input.size()-1] == '\r') {
        input.erase(input.size()-1, 1);
        return 1;
    }
    return 0;
}

int
morph::Tools::stripTrailingWhitespace (std::string& input)
{
    char c;
    string::size_type len = input.size(), pos = len;
    while (pos > 0 &&
           ((c = input[pos-1]) == ' '
            || c == '\t'
            || c == '\n'
            || c == '\r')) {
        pos--;
    }
    input.erase (pos);
    return (len - pos);
}

int
morph::Tools::stripChars (std::string& input, const std::string& charList)
{
    int rtn(0);
    string::size_type pos(0);
    while ((pos = input.find_last_of (charList)) != string::npos) {
        input.erase (pos, 1);
        ++rtn;
    }
    return rtn;
}

int
morph::Tools::stripChars (std::string& input, const char charList)
{
    int rtn(0);
    string::size_type pos(0);
    while ((pos = input.find_last_of (charList)) != string::npos) {
        input.erase (pos, 1);
        ++rtn;
    }
    return rtn;
}

int
morph::Tools::convertCHexCharSequences (std::string& input)
{
    // This converts a string containing C style hex sequences
    // like "\x41\x42\x43" into the corresponding characters
    // ("ABC" for the example).

    string::iterator readPos = input.begin();
    string::iterator writePos = input.begin();
    string::size_type newSize = 0;
    char n1 = '\0', n2 = '\0'; // two Ns in "1xNN"
    char c = 0;
    int count = 0;

    while (readPos != input.end()) {

        c = *readPos;

        if (*readPos == '\\') {
            // We have a possible hex escape sequence.
            ++readPos;
            if (readPos != input.end() && *readPos == 'x') {
                // We have a hex escape sequence. Read in next two chars
                ++readPos;
                if (readPos != input.end()) {
                    n1 = *readPos;
                    ++readPos;
                    if (readPos != input.end()) {
                        n2 = *readPos;
                        ++count;
                        // Now create the replacement for c.
                        c = 0;
                        switch (n1) {
                        case '0':
                            // c |= 0 << 4;
                            break;
                        case '1':
                            c |= 1 << 4;
                            break;
                        case '2':
                            c |= 2 << 4;
                            break;
                        case '3':
                            c |= 3 << 4;
                            break;
                        case '4':
                            c |= 4 << 4;
                            break;
                        case '5':
                            c |= 5 << 4;
                            break;
                        case '6':
                            c |= 6 << 4;
                            break;
                        case '7':
                            c |= 7 << 4;
                            break;
                        case '8':
                            c |= 8 << 4;
                            break;
                        case '9':
                            c |= 9 << 4;
                            break;
                        case 'a':
                        case 'A':
                            c |= 10 << 4;
                            break;
                        case 'b':
                        case 'B':
                            c |= 11 << 4;
                            break;
                        case 'c':
                        case 'C':
                            c |= 12 << 4;
                            break;
                        case 'd':
                        case 'D':
                            c |= 13 << 4;
                            break;
                        case 'e':
                        case 'E':
                            c |= 14 << 4;
                            break;
                        case 'f':
                        case 'F':
                            c |= 15 << 4;
                            break;
                        default:
                            break;
                        }

                        switch (n2) {
                        case '0':
                            // c |= 0;
                            break;
                        case '1':
                            c |= 1;
                            break;
                        case '2':
                            c |= 2;
                            break;
                        case '3':
                            c |= 3;
                            break;
                        case '4':
                            c |= 4;
                            break;
                        case '5':
                            c |= 5;
                            break;
                        case '6':
                            c |= 6;
                            break;
                        case '7':
                            c |= 7;
                            break;
                        case '8':
                            c |= 8;
                            break;
                        case '9':
                            c |= 9;
                            break;
                        case 'a':
                        case 'A':
                            c |= 10;
                            break;
                        case 'b':
                        case 'B':
                            c |= 11;
                            break;
                        case 'c':
                        case 'C':
                            c |= 12;
                            break;
                        case 'd':
                        case 'D':
                            c |= 13;
                            break;
                        case 'e':
                        case 'E':
                            c |= 14;
                            break;
                        case 'f':
                        case 'F':
                            c |= 15;
                            break;
                        default:
                            break;
                        }

                    } else {
                        // Nothing following "\xN", step back 3.
                        --readPos;
                        --readPos;
                        --readPos;
                    }
                } else {
                    // Nothing following "\x", step back 2.
                    --readPos;
                    --readPos;
                }

            } else {
                // Not an escape sequence, just a '\' character. Step back 1.
                --readPos;
            }

        } else {
            // We already set writePos to readPos and c to *readPos.
        }

        // if need to write
        *writePos = c;
        ++writePos;
        ++newSize;

        ++readPos;
    }

    // Terminate the now possibly shorter string:
    input.resize (newSize);

    return count;
}

int
morph::Tools::stripTrailingSpaces (std::string& input)
{
    return Tools::stripTrailingChars (input);
}

int
morph::Tools::stripTrailingChars (std::string& input, const char c)
{
    int i = 0;
    while (input.size()>0 && input[input.size()-1] == c) {
        input.erase (input.size()-1, 1);
        i++;
    }
    return i;
}

int
morph::Tools::stripLeadingWhitespace (std::string& input)
{
    char c;
    string::size_type pos = 0;
    while (pos<input.size() &&
           ((c = input[pos]) == ' '
            || c == '\t'
            || c == '\n'
            || c == '\r')) {
        pos++;
    }
    input.erase (0, pos);
    return pos;
}

int
morph::Tools::stripWhitespace (std::string& input)
{
    int n = Tools::stripLeadingWhitespace (input);
    n += Tools::stripTrailingWhitespace (input);
    return n;
}

int
morph::Tools::stripLeadingSpaces (std::string& input)
{
    return Tools::stripLeadingChars (input);
}

int
morph::Tools::stripLeadingChars (std::string& input, const char c)
{
    int i = 0;
    while (input.size()>0 && input[0] == c) {
        input.erase (0, 1);
        i++;
    }
    return i;
}

int
morph::Tools::searchReplace (const string& searchTerm,
                            const string& replaceTerm,
                            std::string& data,
                            const bool replaceAll)
{
    int count = 0;
    string::size_type pos = 0;
    string::size_type ptr = string::npos;
    string::size_type stl = searchTerm.size();
    if (replaceAll) {
        pos = data.size();
        while ((ptr = data.rfind (searchTerm, pos)) != string::npos) {
            data.erase (ptr, stl);
            data.insert (ptr, replaceTerm);
            count++;
            if (ptr >= stl) {
                // This is a move backwards along the
                // string far enough that we don't
                // match a substring of the last
                // replaceTerm in the next search.
                pos = ptr - stl;
            } else {
                break;
            }
        }
    } else {
        // Replace first only
        if ((ptr = data.find (searchTerm, pos)) != string::npos) {
            data.erase (ptr, stl);
            data.insert (ptr, replaceTerm);
            count++;
        }
    }

    return count;
}

void
morph::Tools::conditionAsXmlTag (std::string& str)
{
    // 1) Replace chars which are disallowed in an XML tag
    string::size_type ptr = string::npos;

    // We allow numeric and alpha chars, the underscore and the
    // hyphen. colon strictly allowed, but best avoided.
    while ((ptr = str.find_last_not_of (CHARS_NUMERIC_ALPHA"_-", ptr)) != string::npos) {
        // Replace the char with an underscore:
        str[ptr] = '_';
        ptr--;
    }

    // 2) Check first 3 chars don't spell xml (in any case)
    string firstThree = str.substr (0,3);
    transform (firstThree.begin(), firstThree.end(),
               firstThree.begin(), morph::to_lower());
    if (firstThree == "xml") {
        // Prepend 'A'
        string newStr("_");
        newStr += str;
        str = newStr;
    }

    // 3) Prepend an '_' if initial char begins with a numeral or hyphen
    if (str[0] > 0x29 && str[0] < 0x3a) {
        // Prepend '_'
        string newStr("_");
        newStr += str;
        str = newStr;
    }
}

unsigned int
morph::Tools::countChars (const std::string& line, const char c)
{
    unsigned int count(0);
    string::const_iterator i = line.begin();
    while (i != line.end()) {
        if (*i++ == c) { ++count; }
    }
    return count;
}
