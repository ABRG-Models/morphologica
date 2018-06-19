#include <vector>

namespace tools
{
    using std::vector;

    vector<double> getJetColor(double gray);
    vector<double> getGrayScaleColor(double gray);
    vector <double> HSVtoRGB(double,double,double);
    double randFloat(void);
    double normalDistributionValue(void);
    double wrapAngle(double);
    vector <vector <float> > rotateCloud (vector <vector <float> >, double, double, double);
#ifdef SPHERE_ATTEMPT
    vector <vector <float> > sphere(int, double);
#endif
    vector<vector<int> > graph(vector<vector<int> >);
    vector<int> sort(vector<double>);
}
