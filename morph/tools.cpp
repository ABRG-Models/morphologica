#include "tools.h"
#include <math.h>
#ifdef __ICC__
# define ARMA_ALLOW_FAKE_GCC 1
#endif
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

extern "C" {
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
}

/*
 * Make external process calls to insert git information (current
 * revision, whether or not the git repo has any modified or untracked
 * files) into the given Json::Value object.
 */
void
morph::Tools::insertGitInfo (Json::Value& root, const std::string& codedir)
{
    ProcessData pD;
    ToolsProcessCallbacks cb(&pD);
    Process p;
    std::string command ("/usr/bin/git");

    std::list<std::string> args1;
    args1.push_back ("git");
    args1.push_back ("rev-parse");
    args1.push_back ("HEAD");

    try {
        p.setCallbacks (&cb);
        p.start (command, args1);
        p.probeProcess ();
        if (!p.waitForStarted()) {
            throw std::runtime_error ("Process failed to start");
        }
        while (p.running() == true) {
            p.probeProcess();
        }

        std::stringstream theOutput;
        theOutput << p.readAllStandardOutput();
        std::string line = "";
        int nlines = 0;
        while (getline (theOutput, line, '\n')) {
            std::cout << "Current git HEAD: " << line << std::endl;
            if (nlines++ > 0) {
                break;
            }
            root["git_head"] = line; // Should be one line only
        }

    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        root["git_head"] = "unknown";
    }

    // Reset Process with arg true to keep callbacks
    p.reset (true);

    std::list<std::string> args2;
    args2.push_back ("git");
    args2.push_back ("status");

    try {
        p.start (command, args2);
        p.probeProcess ();
        if (!p.waitForStarted()) {
            throw std::runtime_error ("Process failed to start");
        }
        while (p.running() == true) {
            p.probeProcess();
        }

        std::stringstream theOutput;
        theOutput << p.readAllStandardOutput();
        std::string line = "";
        bool lm = false;
        bool ut = false;
        while (getline (theOutput, line, '\n')) {
            if (line.find("modified:") != std::string::npos) {
                if (line.find(codedir) != std::string::npos) {
                    if (!lm) {
                        root["git_modified_sim"] = true;
                        std::cout << "Repository has local modifications in " << codedir << " dir\n";
                    }
                    lm = true;
                }
            }
            if (line.find("Untracked files:") != std::string::npos) {
                if (line.find(codedir) != std::string::npos) {
                    if (!ut) {
                        root["git_untracked_sim"] = true;
                        std::cout << "Repository has untracked files present in " << codedir << " dir\n";
                    }
                    ut = true;
                }
            }
        }

    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        root["git_status"] = "unknown";
    }

    // Reset for third call
    p.reset (true);

    // This gets the git branch name
    std::list<std::string> args3;
    args3.push_back ("git");
    args3.push_back ("rev-parse");
    args3.push_back ("--abbrev-ref");
    args3.push_back ("HEAD");

    try {
        p.start (command, args3);
        p.probeProcess ();
        if (!p.waitForStarted()) {
            throw std::runtime_error ("Process failed to start");
        }
        while (p.running() == true) {
            p.probeProcess();
        }

        std::stringstream theOutput;
        theOutput << p.readAllStandardOutput();
        std::string line = "";
        int nlines = 0;
        while (getline (theOutput, line, '\n')) {
            std::cout << "Current git branch: " << line << std::endl;
            if (nlines++ > 0) {
                break;
            }
            root["git_branch"] = line; // Should be one line only
        }

    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        root["git_branch"] = "unknown";
    }
}

/// @param gray gray value from 0.0 to 1.0
/// @returns RGB value in jet colormap
std::array<float,3>
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

    std::array<float,3> col;
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

std::vector<double>
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

    std::vector<double> col;
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

std::vector<double>
morph::Tools::getGrayScaleColor(double gray)
{
    std::vector<double> col;
    for (int i=0; i<3; i++) {
        col.push_back(1.0 - gray);
    }
    return col;
}

std::array<float,3>
morph::Tools::HSVtoRGB(double h,double s,double v) // all in range 0,1
{
    double r=0.0, g=0.0, b=0.0;
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
    std::array<float, 3> rgb;
    rgb[0] = r;
    rgb[1] = g;
    rgb[2] = b;
    return rgb;
}

/*!
 * Random number generation functions
 */
//@{
unsigned int
morph::Tools::mix (unsigned int a, unsigned int b, unsigned int c)
{
    a=a-b;  a=a-c;  a=a^(c >> 13);
    b=b-c;  b=b-a;  b=b^(a << 8);
    c=c-a;  c=c-b;  c=c^(b >> 13);
    a=a-b;  a=a-c;  a=a^(c >> 12);
    b=b-c;  b=b-a;  b=b^(a << 16);
    c=c-a;  c=c-b;  c=c^(b >> 5);
    a=a-b;  a=a-c;  a=a^(c >> 3);
    b=b-c;  b=b-a;  b=b^(a << 10);
    c=c-a;  c=c-b;  c=c^(b >> 15);
    return c;
}

unsigned int
morph::Tools::randomSeed (void)
{
    unsigned int rsd = morph::Tools::mix(clock(), time(NULL), getpid());
    return rsd;
}

double
morph::Tools::randDouble (void)
{
    // FIXME: Have this (and randSingle) instantiate a singleton RandUniform container,
    // and then get instances from that.
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
//@}

double
morph::Tools::wrapAngle(double a)
{
    return a-6.283185307179586*floor(a/6.283185307179586);
}

std::vector < std::vector <double> > rotateCloud (std::vector < std::vector <double> > cloud, double Rx, double Ry, double Rz)
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
    RX <<1.0<<0.0<<0.0<<arma::endr<<0.0<<cos(Rx)<<-sin(Rx)<<arma::endr<<0.0<<sin(Rx)<<cos(Rx)<<arma::endr;
    RY <<cos(Ry)<<0.0<<sin(Ry)<<arma::endr<<0.0<<1.0<<0.0<<arma::endr<<-sin(Ry)<<0.0<<cos(Ry)<<arma::endr;
    RZ <<cos(Rz)<<-sin(Rz)<<0.0<<arma::endr<<sin(Rz)<<cos(Rz)<<0.0<<arma::endr<<0.0<<0.0<<1.0<<arma::endr;

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
std::vector<std::vector<float> >
morph::Tools::sphere (int n, double rad)
{
    std::vector <std::vector <float> > S (n);
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
std::vector<std::vector<int> >
morph::Tools::graph (std::vector<std::vector<int> > agg)
{
    int N = agg.size();

    std::vector<std::vector<int> > conn;
    std::vector<int> con (2,0);
    for(int i=0;i<N;i++){
        for(int j=0;j<N;j++){
            if(agg[i][j]){
                con[0]=i;
                con[1]=j;
                conn.push_back(con);
            }
        }
    }

    std::vector<std::vector<int> > graph;
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
            std::vector<int> isolate(1,k);
            graph.push_back(isolate);
        }
    }

    // sort by descending group size
    std::vector<std::vector<int> > graphSorted;
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

std::vector<int>
morph::Tools::sort (std::vector<double> unsorted)
{
    std::vector<int> unsortID;
    for(int i=0;i<static_cast<int>(unsorted.size());i++){
        unsortID.push_back(i);
    }
    std::vector<int> sortID;
    std::vector<double> sorted;

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
std::vector<std::vector<double> >
morph::Tools::matrixMultiply (std::vector < std::vector <double> > a, std::vector < std::vector <double> > b)
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

std::string
morph::Tools::getPwd (void)
{
    char b[FILENAME_MAX];
    GetCurrentDir (b, FILENAME_MAX);
    return std::string(b);
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
    std::string::size_type len = input.size(), pos = len;
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
    std::string::size_type pos(0);
    while ((pos = input.find_last_of (charList)) != std::string::npos) {
        input.erase (pos, 1);
        ++rtn;
    }
    return rtn;
}

int
morph::Tools::stripChars (std::string& input, const char charList)
{
    int rtn(0);
    std::string::size_type pos(0);
    while ((pos = input.find_last_of (charList)) != std::string::npos) {
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

    std::string::iterator readPos = input.begin();
    std::string::iterator writePos = input.begin();
    std::string::size_type newSize = 0;
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
    std::string::size_type pos = 0;
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

bool
morph::Tools::containsOnlyWhitespace (std::string& input)
{
    bool rtn = true;
    for (std::string::size_type i = 0; i < input.size(); ++i) {
        if (input[i] == ' ' || input[i] == '\t' || input[i] == '\n' || input[i] == '\r') {
            // continue.
        } else {
            rtn = false;
            break;
        }
    }
    return rtn;
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
morph::Tools::searchReplace (const std::string& searchTerm,
                             const std::string& replaceTerm,
                             std::string& data,
                             const bool replaceAll)
{
    int count = 0;
    std::string::size_type pos = 0;
    std::string::size_type ptr = std::string::npos;
    std::string::size_type stl = searchTerm.size();
    if (replaceAll) {
        pos = data.size();
        while ((ptr = data.rfind (searchTerm, pos)) != std::string::npos) {
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
        if ((ptr = data.find (searchTerm, pos)) != std::string::npos) {
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
    std::string::size_type ptr = std::string::npos;

    // We allow numeric and alpha chars, the underscore and the
    // hyphen. colon strictly allowed, but best avoided.
    while ((ptr = str.find_last_not_of (CHARS_NUMERIC_ALPHA"_-", ptr)) != std::string::npos) {
        // Replace the char with an underscore:
        str[ptr] = '_';
        ptr--;
    }

    // 2) Check first 3 chars don't spell xml (in any case)
    std::string firstThree = str.substr (0,3);
    transform (firstThree.begin(), firstThree.end(),
               firstThree.begin(), morph::to_lower());
    if (firstThree == "xml") {
        // Prepend 'A'
        std::string newStr("_");
        newStr += str;
        str = newStr;
    }

    // 3) Prepend an '_' if initial char begins with a numeral or hyphen
    if (str[0] > 0x29 && str[0] < 0x3a) {
        // Prepend '_'
        std::string newStr("_");
        newStr += str;
        str = newStr;
    }
}

unsigned int
morph::Tools::countChars (const std::string& line, const char c)
{
    unsigned int count(0);
    std::string::const_iterator i = line.begin();
    while (i != line.end()) {
        if (*i++ == c) { ++count; }
    }
    return count;
}

/*!
 * Implementation of file access methods
 */
//@{
bool
morph::Tools::fileExists (const std::string& path)
{
    struct stat * buf = NULL;

    buf = static_cast<struct stat*> (malloc (sizeof (struct stat)));
    if (buf == NULL) {
        //cerr << "Memory allocation error in futil::fileExists";
        return false;
    }
    memset (buf, 0, sizeof (struct stat));

    if (stat (path.c_str(), buf)) {
        free (buf);
        return false;
    }

    if (S_ISREG (buf->st_mode)
        || S_ISBLK (buf->st_mode)
        || S_ISSOCK (buf->st_mode)
        || S_ISFIFO (buf->st_mode)
        || S_ISLNK (buf->st_mode)
        || S_ISCHR (buf->st_mode)) {
        free (buf);
        return true;
    }

    free (buf);
    return false;
}

bool
morph::Tools::regfileExists (const std::string& path)
{
    struct stat * buf = NULL;

    buf = static_cast<struct stat*> (malloc (sizeof (struct stat)));
    memset (buf, 0, sizeof (struct stat));

    if (stat (path.c_str(), buf)) {
        free (buf);
        return false;
    }

    if (S_ISREG (buf->st_mode)) {
        free (buf);
        return true;
    }

    free (buf);
    return false;
}

bool
morph::Tools::userExefileExists (const std::string& path)
{
    struct stat * buf = NULL;

    buf = static_cast<struct stat*> (malloc (sizeof (struct stat)));
    memset (buf, 0, sizeof (struct stat));

    if (stat (path.c_str(), buf)) {
        free (buf);
        return false;
    }

    if (S_ISREG (buf->st_mode) && (S_IXUSR & buf->st_mode) ) {
        free (buf);
        return true;
    }

    free (buf);
    return false;
}

bool
morph::Tools::blockdevExists (const std::string& path)
{
    struct stat * buf = NULL;

    buf = static_cast<struct stat*> (malloc (sizeof (struct stat)));
    memset (buf, 0, sizeof (struct stat));

    if (stat (path.c_str(), buf)) {
        free (buf);
        return false;
    }

    if (S_ISBLK (buf->st_mode)) {
        free (buf);
        return true;
    }

    free (buf);
    return false;
}

bool
morph::Tools::socketExists (const std::string& path)
{
    struct stat * buf = NULL;

    buf = static_cast<struct stat*> (malloc (sizeof (struct stat)));
    memset (buf, 0, sizeof (struct stat));

    if (stat (path.c_str(), buf)) {
        free (buf);
        return false;
    }

    if (S_ISSOCK (buf->st_mode)) {
        free (buf);
        return true;
    }

    free (buf);
    return false;
}

bool
morph::Tools::fifoExists (const std::string& path)
{
    struct stat * buf = NULL;

    buf = static_cast<struct stat*> (malloc (sizeof (struct stat)));
    memset (buf, 0, sizeof (struct stat));

    if (stat (path.c_str(), buf)) {
        free (buf);
        return false;
    }

    if (S_ISFIFO (buf->st_mode)) {
        free (buf);
        return true;
    }

    free (buf);
    return false;
}

bool
morph::Tools::chardevExists (const std::string& path)
{
    struct stat * buf = NULL;

    buf = static_cast<struct stat*> (malloc (sizeof (struct stat)));
    memset (buf, 0, sizeof (struct stat));

    if (stat (path.c_str(), buf)) {
        free (buf);
        return false;
    }

    if (S_ISCHR (buf->st_mode)) {
        free (buf);
        return true;
    }

    free (buf);
    return false;
}

bool
morph::Tools::linkExists (const std::string& path)
{
    struct stat * buf = NULL;

    buf = static_cast<struct stat*> (malloc (sizeof (struct stat)));
    memset (buf, 0, sizeof (struct stat));

    if (stat (path.c_str(), buf)) {
        free (buf);
        return false;
    }

    if (S_ISLNK (buf->st_mode)) {
        free (buf);
        return true;
    }

    free (buf);
    return false;
}

bool
morph::Tools::dirExists (const std::string& path)
{
    DIR* d;
    if (!(d = opendir (path.c_str()))) {
        // Dir doesn't exist.
        return false;
    } else {
        // Dir does exist.
        (void) closedir (d);
        return true;
    }
}

void
morph::Tools::createDir (const std::string& path,
                         const mode_t mode,
                         const int uid, const int gid)
{
    if (path.empty()) {
        // Create no directory. Just return.
        return;
    }

    // Set to true if we are provided with an absolute filepath
    bool pathIsAbsolute(false);

    // Set umask to 0000 to stop it interfering with mode
    int oldUmask = umask (0000);
    std::string::size_type pos, lastPos = path.size()-1;
    std::vector<std::string> dirs;
    if ((pos = path.find_last_of ('/', lastPos)) == std::string::npos) {
        // Path is single directory.
        dirs.push_back (path);
    } else {
        // Definitely DO have a '/' in the path somewhere:
        if (path[0] == '/') {
            pathIsAbsolute = true;
            while ((pos = path.find_last_of ('/', lastPos)) != 0) {
                dirs.push_back (path.substr(pos+1, lastPos-pos));
                lastPos = pos-1;
            }
            dirs.push_back (path.substr(1, lastPos));
        } else {
            // Non absolute...
            while ((pos = path.find_last_of ('/', lastPos)) != 0) {
                dirs.push_back (path.substr(pos+1, lastPos-pos));
                lastPos = pos-1;
                if (pos == std::string::npos) {
                    break;
                }
            }
        }
    }

    std::vector<std::string>::reverse_iterator i = dirs.rbegin();
    std::string prePath("");
    bool first(true);
    while (i != dirs.rend()) {
        if (first && !pathIsAbsolute) {
            prePath += "./" + *i;
            first = false;
        } else {
            prePath += "/" + *i;
        }
        int rtn = mkdir (prePath.c_str(), mode);
        if (rtn) {
            int e = errno;
            std::stringstream emsg;
            emsg << "createDir(): mkdir() set error: ";
            switch (e) {
            case EACCES:
                emsg << "Permission is denied";
                break;
            case EEXIST:
                // Path exists, though maybe not as a directory.
                // Set mode/ownership before moving on:
                if (uid>-1 && gid>-1) {
                    chown (prePath.c_str(), static_cast<uid_t>(uid), static_cast<gid_t>(gid));
                    chmod (prePath.c_str(), mode);
                }
                i++;
                continue;
                break;
            case EFAULT:
                emsg << "Bad address";
                break;
            case ELOOP:
                emsg << "Too many symlinks in " << prePath;
                break;
            case ENAMETOOLONG:
                emsg << "File name (" << prePath << ") too long";
                break;
            case ENOENT:
                emsg << "Path '" << prePath << "' invalid (part or all of it)";
                break;
            case ENOMEM:
                emsg << "Out of kernel memory";
                break;
            case ENOSPC:
                emsg << "Out of storage space/quota exceeded.";
                break;
            case ENOTDIR:
                emsg << "component of the path '" << prePath << "' is not a directory";
                break;
            case EPERM:
                emsg << "file system doesn't support directory creation";
                break;
            case EROFS:
                emsg << "path '" << prePath << "' refers to location on read only filesystem";
                break;
            default:
                emsg << "unknown error";
                break;
            }
            throw std::runtime_error (emsg.str());
        }
        if (uid>-1 && gid>-1) {
            chown (prePath.c_str(), static_cast<uid_t>(uid), static_cast<gid_t>(gid));
        }
        i++;
    }

    // Reset umask
    umask (oldUmask);
}

void
morph::Tools::removeDir (const std::string& path)
{
    int rtn = rmdir (path.c_str());
    if (rtn) {
        int e = errno;
        std::stringstream emsg;
        emsg << "setPermissions(): chmod() set error: ";
        switch (e) {
        case EACCES:
            emsg << "Permission is denied";
            break;
        case EBUSY:
            emsg << "Path in use";
            break;
        case EFAULT:
            emsg << "Bad address";
            break;
        case EINVAL:
            emsg << "Path has . as last component";
            break;
        case ELOOP:
            emsg << "Too many symlinks";
            break;
        case ENAMETOOLONG:
            emsg << "File name too long";
            break;
        case ENOENT:
            emsg << "Path invalid (part or all of it)";
            break;
        case ENOMEM:
            emsg << "Out of kernel memory";
            break;
        case ENOTDIR:
            emsg << "component of the path is not a directory";
            break;
        case EPERM:
            emsg << "file system doesn't support directory creation";
            break;
        case EROFS:
            emsg << "path refers to location on read only filesystem";
            break;
        default:
            emsg << "unknown error";
            break;
        }
        throw std::runtime_error (emsg.str());
    }
}

void
morph::Tools::setPermissions (const std::string& filepath, const mode_t mode)
{
    int rtn = chmod (filepath.c_str(), mode);
    if (rtn) {
        int e = errno;
        std::stringstream emsg;
        emsg << "setPermissions(): chmod() set error: ";
        switch (e) {
        case EACCES:
            emsg << "Permission is denied";
            break;
        case EFAULT:
            emsg << "Bad address";
            break;
        case ELOOP:
            emsg << "Too many symlinks";
            break;
        case ENAMETOOLONG:
            emsg << "File name too long";
            break;
        case ENOENT:
            emsg << "Path invalid (part or all of it)";
            break;
        case ENOMEM:
            emsg << "Out of kernel memory";
            break;
        case ENOTDIR:
            emsg << "component of the path is not a directory";
            break;
        case EPERM:
            emsg << "file system doesn't support directory creation";
            break;
        case EROFS:
            emsg << "path refers to location on read only filesystem";
            break;
        case EBADF:
            emsg << "file descriptor is not valid";
            break;
        case EIO:
            emsg << "an i/o error occurred";
            break;
        default:
            emsg << "unknown error";
            break;
        }
        throw std::runtime_error (emsg.str());
    }
}

bool
morph::Tools::checkAccess (const std::string& filepath,
                           const std::string& accessType)
{
    if (accessType.find("r") != std::string::npos) {
        std::ifstream in;
        in.open (filepath.c_str(), std::ios::in);
        if (!in.is_open()) {
            return false;
        }
        in.close();
    }
    if (accessType.find("w") != std::string::npos) {
        std::ofstream out;
        out.open (filepath.c_str(), std::ios::out);
        if (!out.is_open()) {
            return false;
        }
        out.close();
    }
    return true;
}

void
morph::Tools::setOwnership (const std::string& filepath, const int uid, const int gid)
{
    int rtn = chown (filepath.c_str(), uid, gid);
    if (rtn) {
        int e = errno;
        std::stringstream emsg;
        emsg << "setOwnership(): chown() set error: ";
        switch (e) {
        case EACCES:
            emsg << "Permission is denied";
            break;
        case EFAULT:
            emsg << "Bad address";
            break;
        case ELOOP:
            emsg << "Too many symlinks";
            break;
        case ENAMETOOLONG:
            emsg << "File name too long";
            break;
        case ENOENT:
            emsg << "Path invalid (part or all of it)";
            break;
        case ENOMEM:
            emsg << "Out of kernel memory";
            break;
        case ENOTDIR:
            emsg << "component of the path is not a directory";
            break;
        case EPERM:
            emsg << "file system doesn't support directory creation";
            break;
        case EROFS:
            emsg << "path refers to location on read only filesystem";
            break;
        case EBADF:
            emsg << "file descriptor is not valid";
            break;
        case EIO:
            emsg << "an i/o error occurred";
            break;
        default:
            emsg << "unknown error";
            break;
        }
        throw std::runtime_error (emsg.str());
    }
}

void
morph::Tools::touchFile (const std::string& path)
{
    std::ofstream f;
    f.open (path.c_str(), std::ios::out|std::ios::app);
    if (!f.is_open()) {
        f.open (path.c_str(), std::ios::out|std::ios::trunc);
        if (!f.is_open()) {
            std::string emsg = "Failed to create file '" + path + "'";
            throw std::runtime_error (emsg);
        } else {
            f.close();
        }
    } else {
        // File is open, was already there
        f.close();
    }
}

void
morph::Tools::copyFile (const std::string& from, const std::string& to)
{
    std::ofstream out;

    out.open (to.c_str(), std::ios::out|std::ios::trunc);
    if (!out.is_open()) {
        std::string emsg = "Tools::copyFile(): Couldn't open TO file '" + to + "'";
        throw std::runtime_error (emsg);
    }

    Tools::copyFile (from, out);

    out.close();
}

void
morph::Tools::copyFile (const std::string& from, std::ostream& to)
{
    std::ifstream in;

    // Test that "from" is a regular file
    if (!Tools::regfileExists (from)) {
        std::stringstream ee;
        ee << "Tools::copyFile(): FROM file '"
           << from << "' is not a regular file";
        throw std::runtime_error (ee.str());
    }

    in.open (from.c_str(), std::ios::in);
    if (!in.is_open()) {
        throw std::runtime_error ("Tools::copyFile(): Couldn't open FROM file");
    }

    if (!to) {
        throw std::runtime_error ("Tools::copyFile(): Error occurred in TO stream");
    }

    char buf[64];
    while (!in.eof()) {
        in.read (buf, 63);
        // Find out how many were read
        unsigned int bytes = in.gcount();
        // and write that many to the output stream
        to.write (buf, bytes);
    }

    // Make sure output buffer is flushed.
    to.flush();

    // Finally, close the input.
    in.close();
}

#define COPYFILE_BUFFERSIZE    32768
#define COPYFILE_BUFFERSIZE_MM 32767 // MM: Minus Minus
void
morph::Tools::copyFile (FILE * from, const std::string& to)
{
    FILE * ofp = NULL;
    long pos;
    int bytes=0, output=0;
    char inputBuffer[COPYFILE_BUFFERSIZE];

    // Get posn of from as we will return the file pointer there when done
    pos = ftell (from);

    ofp = fopen (to.c_str(), "w");
    if (!ofp) {
        throw std::runtime_error ("Tools::copyFile(): Can't open output for writing");
    }
    while ((bytes = fread (inputBuffer, 1, COPYFILE_BUFFERSIZE_MM, from)) > 0) {
        output = fwrite (inputBuffer, 1, bytes, ofp);
        if (output != bytes) {
            fseek (from, pos, SEEK_SET); /* reset input */
            throw std::runtime_error ("Tools::copyFile(): Error writing data");
        }
    }
    fclose (ofp); /* close output */
    fseek (from, pos, SEEK_SET); /* reset input */
}

void
morph::Tools::copyFile (std::istream& from, const std::string& to)
{
    char buf[64];
    std::ofstream f;
    f.open (to.c_str(), std::ios::out|std::ios::trunc);
    if (!f.is_open()) {
        std::stringstream ee;
        ee << "Failed to open output file '" << to << "'";
        throw std::runtime_error (ee.str());
    }
    while (!from.eof()) {
        from.read (buf, 63);
        f.write (buf, from.gcount());
    }
}

void
morph::Tools::copyFile (const std::string& from, FILE * to)
{
    FILE* ifp = fopen (from.c_str(), "r");
    Tools::copyFile (ifp, to);
    fclose (ifp);
}

void
morph::Tools::copyFile (FILE * from, FILE * to)
{
    long pos;
    int bytes=0, output=0;
    char inputBuffer[COPYFILE_BUFFERSIZE];

    // Get posn of from as we will return the file pointer there when done
    pos = ftell (from);

    if (!to) {
        throw std::runtime_error ("Tools::copyFile(): output is not open for writing");
    }
    while ((bytes = fread (inputBuffer, 1, COPYFILE_BUFFERSIZE_MM, from)) > 0) {
        output = fwrite (inputBuffer, 1, bytes, to);
        if (output != bytes) {
            fseek (from, pos, SEEK_SET); /* reset input */
            throw std::runtime_error ("Tools::copyFile(): Error writing data");
        }
    }
    fseek (from, pos, SEEK_SET); /* reset input */
}

void
morph::Tools::copyFileToString (std::istream& from, std::string& to)
{
    char buf[64];
    while (!from.eof()) {
        from.read (buf, 63);
        to.append (buf, from.gcount());
    }
}

void
morph::Tools::appendFile (const std::string& from, std::ostream& appendTo)
{
    if (!appendTo.good()) {
        throw std::runtime_error ("Can't append to appendTo, it's not good()");
    }
    std::ifstream in;
    in.open (from.c_str(), std::ios::in);
    if (!in.is_open()) {
        throw std::runtime_error ("Tools::appendFile(): Couldn't open FROM file");
    }

    char buf[64];
    while (!in.eof() && appendTo.good()) {
        in.read (buf, 63);
        appendTo.write (buf, in.gcount());
    }

    in.close();
}

// AH - because USER opens the ostream, then the USER/client controls if copy or append.
// This would be the same as copyFile(istream&, ostream&
void
morph::Tools::appendFile (std::istream& from, std::ostream& appendTo)
{
    if (!appendTo.good()) {
        throw std::runtime_error ("Can't append to appendTo, it's not good()");
    }

    char buf[64];
    buf[63] = '\0';
    while (!from.eof() && appendTo.good()) {
        from.read (buf, 63);
        appendTo.write (buf, from.gcount());
    }
}

void
morph::Tools::appendFile (std::istream& from, const std::string& appendTo)
{
    std::ofstream f;
    f.open (appendTo.c_str(), std::ios::out|std::ios::app);
    if (!f.is_open()) {
        std::stringstream ee;
        ee << "Failed to open output file '" << appendTo << "'";
        throw std::runtime_error (ee.str());
    }

    char buf[64];
    buf[63] = '\0';
    while (!from.eof() && f.good()) {
        from.read (buf, 63);
        f.write (buf, from.gcount());
    }
}

void
morph::Tools::appendFile (const std::string& from, const std::string& appendTo)
{
    std::ifstream fin;
    fin.open (from.c_str(), std::ios::in);
    if (!fin.is_open()) {
        std::stringstream ee;
        ee << "Failed to open input file '" << from << "'";
        throw std::runtime_error (ee.str());
    }

    std::ofstream f;
    f.open (appendTo.c_str(), std::ios::out|std::ios::app);
    if (!f.is_open()) {
        std::stringstream ee;
        ee << "Failed to open output file '" << appendTo << "'";
        throw std::runtime_error (ee.str());
    }

    char buf[64];
    buf[63] = '\0';
    while (!fin.eof() && f.good()) {
        fin.read (buf, 63);
        f.write (buf, fin.gcount());
    }
}

void
morph::Tools::truncateFile (const std::string& from,
                            const std::string& to,
                            const unsigned int bytes)
{
    std::ofstream out;

    out.open (to.c_str(), std::ios::out|std::ios::trunc);
    if (!out.is_open()) {
        std::string emsg = "Tools::copyFile(): Couldn't open TO file '" + to + "'";
        throw std::runtime_error (emsg);
    }

    std::ifstream in;

    // Test that "from" is a regular file
    if (!Tools::regfileExists (from)) {
        std::stringstream ee;
        ee << "Tools::truncateFile(): FROM file '"
           << from << "' is not a regular file";
        throw std::runtime_error (ee.str());
    }

    in.open (from.c_str(), std::ios::in);
    if (!in.is_open()) {
        throw std::runtime_error ("Tools::truncateFile(): Couldn't open FROM file");
    }

    if (!out) {
        throw std::runtime_error ("Tools::truncateFile(): Error occurred in TO stream");
    }

    unsigned int loops(0);
    unsigned int maxLoops = bytes / 63;
    unsigned int remaining = bytes % 63;
    char buf[64];
    while (!in.eof() && loops < maxLoops) {
        in.read (buf, 63);
        // Find out how many were read
        unsigned int bytesCopied = in.gcount();
        // and write that many to the output stream
        out.write (buf, bytesCopied);
        ++loops;
    }
    // Copy remaining
    if (!in.eof()) {
        in.read (buf, remaining);
        // Find out how many were read
        unsigned int bytesCopied = in.gcount();
        if (bytesCopied != remaining) {
            throw std::runtime_error ("copy error bytesCopied != remaining");
        }
        // and write that many to the output stream
        out.write (buf, bytesCopied);
    }

    // Make sure output buffer is flushed.
    out.flush();

    // Finally, close the input and output
    in.close();
    out.close();
}

void
morph::Tools::moveFile (const std::string& from, const std::string& to)
{
    Tools::copyFile (from, to);
    Tools::unlinkFile (from);
}

void
morph::Tools::unlinkFile (const std::string& fpath)
{
    int rtn = unlink (fpath.c_str());
    if (rtn) {
        int theError = errno;
        std::string emsg;
        switch (theError) {
        case EPERM:
        case EACCES:
            emsg = "Write access to '" + fpath + "' is not allowed due to permissions";
            break;
        case EBUSY:
            emsg = "'" + fpath + "' cannot be removed as it is in use by another process";
            break;
        case EFAULT:
            emsg = "'" + fpath + "' points outside your accessible address space";
            break;
        case EIO:
            emsg = "I/O error occurred reading '" + fpath + "'";
            break;
        case EISDIR:
            emsg = "'" + fpath + "' is a directory";
            break;
        case ELOOP:
            emsg = "Too many symlinks encountered in '" + fpath + "'";
            break;
        case ENAMETOOLONG:
            emsg = "'" + fpath + "' is too long a name";
            break;
        case ENOENT:
            emsg = "'" + fpath + "' does not exist or is a dangling symlink";
            break;
        case ENOMEM:
            emsg = "In sufficient kernel memory to open '" + fpath + "'";
            break;
        case ENOTDIR:
            emsg = "'" + fpath + "' contains a component that is not a directory";
            break;
        case EROFS:
            emsg = "'" + fpath + "' is on a read-only filesystem";
            break;
        default:
            emsg = "Unknown error unlinking file '" + fpath + "'";
            break;
        }

        throw std::runtime_error (emsg);
    }
}

void
morph::Tools::clearoutDir (const std::string& dirPath,
                           const unsigned int olderThanSeconds,
                           const std::string& filePart)
{
    std::vector<std::string> files;
    try {
        Tools::readDirectoryTree (files, dirPath, olderThanSeconds);
    } catch (const std::exception& e) {
        //DBG ("Failed to read dir tree: " << e.what());
        return;
    }
    std::vector<std::string>::iterator i = files.begin();
    while (i != files.end()) {
        std::string fpath = dirPath + "/" + *i;
        try {
            if (filePart.empty()) {
                Tools::unlinkFile (fpath);
            } else {
                // Must find filePart to unlink
                if (i->find (filePart, 0) != std::string::npos) {
                    Tools::unlinkFile (fpath);
                } // else do nothing
            }

        } catch (const std::exception& e) {
            //DBG ("Failed to unlink " << *i << ": " << e.what());
        }
        ++i;
    }
}

void
morph::Tools::readDirectoryTree (std::vector<std::string>& vec,
                                 const std::string& dirPath,
                                 const unsigned int olderThanSeconds)
{
    Tools::readDirectoryTree (vec, dirPath, "", olderThanSeconds);
}

void
morph::Tools::readDirectoryTree (std::vector<std::string>& vec,
                                 const std::string& baseDirPath,
                                 const std::string& subDirPath,
                                 const unsigned int olderThanSeconds)
{
    DIR* d;
    struct dirent *ep;
    size_t entry_len = 0;

    std::string dirPath (baseDirPath);
    std::string sd (subDirPath);
    if (!sd.empty()) {
        dirPath += "/" + sd;
    }

    if (!(d = opendir (dirPath.c_str()))) {
        std::string msg = "Failed to open directory " + dirPath;
        throw std::runtime_error (msg);
    }

    struct stat buf;
    while ((ep = readdir (d))) {

        unsigned char fileType;
        std::string fileName = dirPath + "/" + (std::string)ep->d_name;

        if (ep->d_type == DT_LNK) {
            // Is it a link to a directory or a file?
            struct stat * buf = NULL;
            buf = (struct stat*) malloc (sizeof (struct stat));
            if (!buf) { // Malloc error.
                throw std::runtime_error ("Failed to malloc buf; "
                                     "could not stat link " + fileName);
            }
            memset (buf, 0, sizeof(struct stat));
            if (stat (fileName.c_str(), buf)) {
                throw std::runtime_error ("Failed to stat link " + fileName);
            } else {
                if (S_ISREG(buf->st_mode)) {
                    fileType = DT_REG;
                } else if (S_ISDIR(buf->st_mode)) {
                    fileType = DT_DIR;
                } else {
                    fileType = DT_UNKNOWN;
                }
            }
            if (buf) { free (buf); }
        } else {
            fileType = ep->d_type;
        }

        if (fileType == DT_DIR) {

            // Skip "." and ".." directories
            if ( ((entry_len = std::strlen (ep->d_name)) > 0 && ep->d_name[0] == '.') &&
                 (ep->d_name[1] == '\0' || ep->d_name[1] == '.') ) {
                continue;
            }

            // For all other directories, recurse.
            std::string newPath;
            if (sd.size() == 0) {
                newPath = ep->d_name;
            } else {
                newPath = sd + "/" + ep->d_name;
            }
            Tools::readDirectoryTree (vec, baseDirPath,
                                      newPath.c_str(), olderThanSeconds);
        } else {
            // Non-directories are simply added to the vector
            std::string newEntry;
            if (sd.size() == 0) {
                newEntry = ep->d_name;
            } else {
                newEntry = sd + "/" + ep->d_name;
            }

            // If we have to check the file age, do so here before the vec.push_back()
            if (olderThanSeconds > 0) {
                // Stat the file
                memset (&buf, 0, sizeof (struct stat));

                if (stat (fileName.c_str(), &buf)) {
                    // no file to stat
                    //DBG ("stat() error for '" << fileName << "'");
                    continue;
                }

                if (static_cast<unsigned int>(time(NULL)) - buf.st_mtime
                    <= olderThanSeconds) {
                    // The age of the last modification is less
                    // than olderThanSeconds, so skip
                    // (we're only returning the OLDER
                    // files)
                    //DBG ("File " << fileName << " is too new to include, continuing");
                    continue;
                } //else DBG ("File " << fileName << " is older than " << olderThanSeconds << " s");
            }
            vec.push_back (newEntry);
        }
    }

    (void) closedir (d);
}

void
morph::Tools::readDirectoryDirs (std::set<std::string>& dset,
                                 const std::string& dirPath)
{
    DIR* d;
    struct dirent *ep;
    size_t entry_len = 0;

    if (!(d = opendir (dirPath.c_str()))) {
        std::string msg = "Failed to open directory " + dirPath;
        throw std::runtime_error (msg);
    }

    while ((ep = readdir (d))) {

        if (ep->d_type == DT_DIR) {

            // Skip "." and ".." directories
            if ( ((entry_len = std::strlen (ep->d_name)) > 0 && ep->d_name[0] == '.') &&
                 (ep->d_name[1] == '\0' || ep->d_name[1] == '.') ) {
                continue;
            }

            // All other directories are added to vec
            dset.insert (ep->d_name);
        }
    }

    (void) closedir (d);
}

void
morph::Tools::readDirectoryEmptyDirs (std::set<std::string>& dset,
                                      const std::string& baseDirPath,
                                      const std::string& subDir)
{
    DIR* d;
    struct dirent *ep;
    size_t entry_len = 0;

    std::string dirPath (baseDirPath);
    if (!subDir.empty()) {
        dirPath += "/" + subDir;
    }

    if (!(d = opendir (dirPath.c_str()))) {
        std::string msg = "Failed to open directory " + dirPath;
        throw std::runtime_error (msg);
    }

    unsigned int levelDirCount = 0;
    while ((ep = readdir (d))) {

        if (ep->d_type == DT_DIR) {
            // Skip "." and ".." directories
            if ( ((entry_len = std::strlen (ep->d_name)) > 0 && ep->d_name[0] == '.') &&
                 (ep->d_name[1] == '\0' || ep->d_name[1] == '.') ) {
                continue;
            }

            ++levelDirCount;
            // Because we found a directory, this current
            // directory ain't empty - recurse with a new
            // directory in the subDir path:
            std::string newSubDir;
            if (subDir.empty()) {
                newSubDir = (const char*)ep->d_name;
            } else {
                newSubDir = subDir + "/" + (const char*)ep->d_name;
            }
            Tools::readDirectoryEmptyDirs (dset, baseDirPath, newSubDir);
        }
    }

    if (levelDirCount == 0) {
        // No directories found here, check for files
        std::vector<std::string> foundfiles;
        Tools::readDirectoryTree (foundfiles, dirPath);
        if (foundfiles.empty()) {
            dset.insert (subDir);
        } // else DBG ("NOT adding " << subDir << " as " << dirPath << " contains " << foundfiles.size() << " files");
    }

    (void) closedir (d);
}

void
morph::Tools::removeUnusedDirs (std::set<std::string>& dset,
                                const std::string& dirPath)
{
    std::set<std::string> onepass;
    do {
        onepass.clear();
        Tools::removeEmptySubDirs (onepass, dirPath);
        dset.insert (onepass.begin(), onepass.end());
    } while (!onepass.empty());
}

void
morph::Tools::removeEmptySubDirs (std::set<std::string>& dset,
                                  const std::string& baseDirPath,
                                  const std::string& subDir)
{
    DIR* d;
    struct dirent *ep;
    size_t entry_len = 0;

    std::string dirPath (baseDirPath);
    if (!subDir.empty()) {
        dirPath += "/" + subDir;
    }

    if (!(d = opendir (dirPath.c_str()))) {
        std::string msg = "Failed to open directory " + dirPath;
        throw std::runtime_error (msg);
    }

    unsigned int levelDirCount = 0;
    while ((ep = readdir (d))) {

        if (ep->d_type == DT_DIR) {
            // Skip "." and ".." directories
            if ( ((entry_len = std::strlen (ep->d_name)) > 0 && ep->d_name[0] == '.') &&
                 (ep->d_name[1] == '\0' || ep->d_name[1] == '.') ) {
                continue;
            }

            ++levelDirCount;
            // Because we found a directory, this current
            // directory ain't empty - recurse with a new
            // directory in the subDir path:
            std::string newSubDir;
            if (subDir.empty()) {
                newSubDir = (const char*)ep->d_name;
            } else {
                newSubDir = subDir + "/" + (const char*)ep->d_name;
            }
            Tools::removeEmptySubDirs (dset, baseDirPath, newSubDir);
        }
    }

    if (levelDirCount == 0) {
        // No directories found here, check for files
        std::vector<std::string> foundfiles;
        Tools::readDirectoryTree (foundfiles, dirPath);

        if (foundfiles.empty()) {
            if (!subDir.empty()) {
                Tools::removeDir (dirPath);
                dset.insert (subDir);
            }
        } // else "NOT Removing " << dirPath << " which contains " << foundfiles.size() << " files";
    }

    (void) closedir (d);
}

std::string
morph::Tools::fileModDatestamp (const std::string& filename)
{
    struct stat * buf = NULL;
    std::stringstream datestamp;

    buf = (struct stat*) malloc (sizeof (struct stat));
    if (!buf) { // Malloc error.
        std::cout << "malloc error\n";
    }
    memset (buf, 0, sizeof(struct stat));
    if (stat (filename.c_str(), buf)) {
        datestamp << 0;
    } else {
        datestamp << buf->st_mtime;
    }
    if (buf) { free (buf); }

    std::string dstr = datestamp.str();
    return dstr;
}

bool
morph::Tools::filesDiffer (const std::string& first, const std::string& second)
{
    if (!(Tools::regfileExists (first)
          && Tools::regfileExists (second))) {
        throw std::runtime_error ("Error: expecting two regular files");
    }
    std::string diffcmd = "diff " + first + " " + second + " >/dev/null 2>&1";
    // diff returns zero if files are identical, non-zero if files
    // differ.
    return (system (diffcmd.c_str()) != 0);
}

void
morph::Tools::stripUnixPath (std::string& unixPath)
{
    std::string::size_type pos (unixPath.find_last_of ('/'));
    if (pos != std::string::npos) {
        unixPath = unixPath.substr (++pos);
    }
}

void
morph::Tools::stripUnixFile (std::string& unixPath)
{
    std::string::size_type pos (unixPath.find_last_of ('/'));
    if (pos != std::string::npos) {
        unixPath = unixPath.substr (0, pos);
    }
}

void
morph::Tools::stripFileSuffix (std::string& unixPath)
{
    std::string::size_type pos (unixPath.rfind('.'));
    if (pos != std::string::npos) {
        // We have a '.' character
        std::string tmp (unixPath.substr (0, pos));
        if (!tmp.empty()) {
            unixPath = tmp;
        }
    }
}
//@}

/*!
 * Implementation of date and time methods
 */
//@{
unsigned int
morph::Tools::yearNow (void)
{
    time_t curtime; // Current time
    struct tm * t;
    curtime = time(NULL);
    t = localtime (&curtime);
    unsigned int theYear = static_cast<unsigned int>(t->tm_year+1900);
    return theYear;
}

unsigned int
morph::Tools::monthNow (void)
{
    time_t curtime; // Current time
    struct tm * t;
    curtime = time(NULL);
    t = localtime (&curtime);
    unsigned int theMonth = static_cast<unsigned int>(t->tm_mon+1);
    return theMonth;
}

unsigned int
morph::Tools::dateNow (void)
{
    time_t curtime; // Current time
    struct tm * t;
    curtime = time(NULL);
    t = localtime (&curtime);
    unsigned int theDate = static_cast<unsigned int>(t->tm_mday);
    return theDate;
}

std::string
morph::Tools::monthStr (const int month, const bool shortFormat)
{
    std::string rtn("");

    if (shortFormat == true) {
        switch (month) {
        case 1:
            rtn = "Jan";
            break;
        case 2:
            rtn = "Feb";
            break;
        case 3:
            rtn = "Mar";
            break;
        case 4:
            rtn = "Apr";
            break;
        case 5:
            rtn = "May";
            break;
        case 6:
            rtn = "Jun";
            break;
        case 7:
            rtn = "Jul";
            break;
        case 8:
            rtn = "Aug";
            break;
        case 9:
            rtn = "Sep";
            break;
        case 10:
            rtn = "Oct";
            break;
        case 11:
            rtn = "Nov";
            break;
        case 12:
            rtn = "Dec";
            break;
        default:
            rtn = "unk";
            break;
        }
    } else {
        switch (month) {
        case 1:
            rtn = "January";
            break;
        case 2:
            rtn = "February";
            break;
        case 3:
            rtn = "March";
            break;
        case 4:
            rtn = "April";
            break;
        case 5:
            rtn = "May";
            break;
        case 6:
            rtn = "June";
            break;
        case 7:
            rtn = "July";
            break;
        case 8:
            rtn = "August";
            break;
        case 9:
            rtn = "September";
            break;
        case 10:
            rtn = "October";
            break;
        case 11:
            rtn = "November";
            break;
        case 12:
            rtn = "December";
            break;
        default:
            rtn = "unknown";
            break;
        }
    }

    return rtn;
}

time_t
morph::Tools::dateToNum (const std::string& dateStr)
{
    char separator = '\0';

    if (dateStr.empty()) {
        return -2;
    }

    if (dateStr.size() < 8) {
        return -3;
    }

    bool bigEndian (true);

    if (dateStr[2] < '0'
        || dateStr[2] > '9') {
        separator = dateStr[2];
        bigEndian = false;
    } else if (dateStr[4] < '0'
               || dateStr[4] > '9') {
        separator = dateStr[4];
    }
    if (separator != '\0' && dateStr.size() < 10) {
        return -4;
    }

    std::string year;
    std::string month;
    std::string day;
    unsigned int yearN=0, monthN=0, dayN=0;

    if (bigEndian) {
        year = dateStr.substr (0,4);

        if (separator == '\0') {
            month = dateStr.substr (4,2);
            day = dateStr.substr (6,2);
        } else {
            month = dateStr.substr (5,2);
            day = dateStr.substr (8,2);
        }

    } else {
        day = dateStr.substr (0,2);

        if (separator == '\0') {
            month = dateStr.substr (2,2);
            year = dateStr.substr (4,4);
        } else {
            month = dateStr.substr (3,2);
            year = dateStr.substr (6,4);
        }
    }

    std::stringstream yearss, monthss, dayss;
    yearss << year;
    yearss.width(4);
    yearss.fill ('0');
    yearss >> yearN;

    monthss << month;
    monthss.width(2);
    monthss.fill ('0');
    monthss >> monthN;

    dayss << day;
    dayss.width(2);
    dayss.fill ('0');
    dayss >> dayN;

    struct tm * t;
    t = (struct tm*) malloc (sizeof (struct tm));
    t->tm_year = yearN-1900;
    t->tm_mon = monthN-1;
    t->tm_mday = dayN;
    t->tm_hour = 0;
    t->tm_min = 0;
    t->tm_sec = 0;
    t->tm_isdst = -1;
    time_t rtnTime = mktime (t);
    if (rtnTime == -1) {
        throw std::runtime_error ("mktime() returned -1");
    }
    free (t);

    return rtnTime;
}

time_t
morph::Tools::dateTimeToNum (const std::string &dateTimeStr)
{
    char dateSeparator = '\0';
    char timeSeparator = '\0';

    if (dateTimeStr.empty()) {
        return -2;
    }

    if (dateTimeStr.size() < 8) {
        return -3;
    }

    if (dateTimeStr[4] < '0'
        || dateTimeStr[4] > '9') {
        dateSeparator = dateTimeStr[4];
        if (dateTimeStr.size() < 10) {
            return -4;
        }
    }

    std::string year;
    std::string month;
    std::string day;
    unsigned int yearN=0, monthN=0, dayN=0;

    year = dateTimeStr.substr (0,4);

    if (dateSeparator == '\0') {
        month = dateTimeStr.substr (4,2);
        day = dateTimeStr.substr (6,2);
    } else {
        month = dateTimeStr.substr (5,2);
        day = dateTimeStr.substr (8,2);
    }

    std::stringstream yearss, monthss, dayss;
    yearss << year;
    yearss.width(4);
    yearss.fill ('0');
    yearss >> yearN;

    monthss << month;
    monthss.width(2);
    monthss.fill ('0');
    monthss >> monthN;

    dayss << day;
    dayss.width(2);
    dayss.fill ('0');
    dayss >> dayN;

    std::string hour;
    std::string min;
    std::string sec;
    unsigned int hourN=0, minN=0, secN=0;

    std::string::size_type spacePos = dateTimeStr.find (" ", 0);
    if (spacePos != std::string::npos) {
        if (dateTimeStr[spacePos+3] < '0'
            || dateTimeStr[spacePos+3] > '9') {
            timeSeparator = dateTimeStr[spacePos+3];
        }

        hour = dateTimeStr.substr (spacePos+1, 2);

        if (timeSeparator != '\0') {
            min = dateTimeStr.substr (spacePos+4, 2);
            sec = dateTimeStr.substr (spacePos+7, 2);
        } else {
            min = dateTimeStr.substr (spacePos+3, 2);
            sec = dateTimeStr.substr (spacePos+5, 2);
        }

        //DBG ("hour: " << hour << " min: " << min << " sec: " << sec);

        std::stringstream hourss, minss, secss;
        hourss << hour;
        hourss.width(2);
        hourss.fill ('0');
        hourss >> hourN;

        minss << min;
        minss.width(2);
        minss.fill ('0');
        minss >> minN;

        secss << sec;
        secss.width(2);
        secss.fill ('0');
        secss >> secN;
    }

    struct tm * t;
    t = (struct tm*) malloc (sizeof (struct tm));
    t->tm_year = yearN-1900;
    t->tm_mon = monthN-1;
    t->tm_mday = dayN;
    t->tm_hour = hourN;
    t->tm_min = minN;
    t->tm_sec = secN;
    t->tm_isdst = -1;
    time_t rtnTime = mktime (t);
    if (rtnTime == -1) {
        throw std::runtime_error ("mktime() returned -1");
    }
    free (t);

    return rtnTime;
}

std::string
morph::Tools::numToDateTime (const time_t epochSeconds,
                             const char dateSeparator,
                             const char timeSeparator)
{
    if (epochSeconds == 0) {
        return "unknown";
    }

    struct tm * t;
    time_t es = epochSeconds;
    t = (struct tm*) malloc (sizeof (struct tm));
    t = localtime_r (&es, t);
    int theDay = t->tm_mday;
    int theMonth = t->tm_mon+1;
    int theYear = t->tm_year+1900;
    int theHour = t->tm_hour;
    int theMin = t->tm_min;
    int theSec = t->tm_sec;
    free (t);

    std::stringstream rtn;

    // Date part
    rtn.width(4);
    rtn.fill('0');
    rtn << theYear;
    if (dateSeparator != '\0') {
        rtn << dateSeparator;
    }
    rtn.width(2);
    rtn.fill('0');
    rtn << theMonth;
    if (dateSeparator != '\0') {
        rtn << dateSeparator;
    }
    rtn.width(2);
    rtn.fill('0');
    rtn << theDay;

    rtn << " ";

    // Time part
    rtn.width(2);
    rtn.fill('0');
    rtn << theHour;
    if (timeSeparator != '\0') {
        rtn << timeSeparator;
    }
    rtn.width(2);
    rtn.fill('0');
    rtn << theMin;
    if (timeSeparator != '\0') {
        rtn << timeSeparator;
    }
    rtn.width(2);
    rtn.fill('0');
    rtn << theSec;

    return rtn.str();
}

std::string
morph::Tools::numToDate (const time_t epochSeconds,
                         const char separator)
{
    struct tm * t;
    time_t es = epochSeconds;
    t = (struct tm*) malloc (sizeof (struct tm));
    t = localtime_r (&es, t);
    int theDay = t->tm_mday;
    int theMonth = t->tm_mon+1;
    int theYear = t->tm_year+1900;
    free (t);

    std::stringstream rtn;
    if (separator == '\0') {
        rtn.width(4);
        rtn.fill('0');
        rtn << theYear;
        rtn.width(2);
        rtn.fill('0');
        rtn << theMonth;
        rtn.width(2);
        rtn.fill('0');
        rtn << theDay;
    } else {
        rtn.width(4);
        rtn.fill('0');
        rtn << theYear << separator;
        rtn.width(2);
        rtn.fill('0');
        rtn << theMonth << separator;
        rtn.width(2);
        rtn.fill('0');
        rtn << theDay;
    }

    return rtn.str();
}

std::string
morph::Tools::timeNow (void)
{
    time_t curtime;
    struct tm *loctime;
    curtime = time (NULL);
    loctime = localtime (&curtime);
    return asctime(loctime);
}

// Similiar to futil::splitString() but FASTER.
std::vector<std::string>
morph::Tools::stringToVector (const std::string& s, const std::string& separator,
                              const bool ignoreTrailingEmptyVal)
{
    if (separator.empty()) {
        throw std::runtime_error ("Can't split the string; the separator is empty.");
    }
    std::vector<std::string> theVec;
    std::string entry("");
    std::string::size_type sepLen = separator.size();
    std::string::size_type a=0, b=0;
    while (a < s.size()
           && (b = s.find (separator, a)) != std::string::npos) {
        entry = s.substr (a, b-a);
        theVec.push_back (entry);
        a=b+sepLen;
    }
    // Last one has no separator
    if (a < s.size()) {
        b = s.size();
        entry = s.substr (a, b-a);
        theVec.push_back (entry);
    } else {
        if (!ignoreTrailingEmptyVal) {
            theVec.push_back ("");
        }
    }

    return theVec;
}

//@}
