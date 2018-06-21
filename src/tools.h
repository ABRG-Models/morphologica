/*
 * Utility functions
 */

#include <vector>

using std::vector;

namespace morph
{
    class Tools {
        Tools() {}
        ~Tools() {}

    public:
        static vector<double> getJetColor(double gray);
        static vector<double> getGrayScaleColor(double gray);
        static vector <double> HSVtoRGB(double,double,double);
        static double randFloat(void);
        static double normalDistributionValue(void);
        static double wrapAngle(double);
        static vector <vector <float> > rotateCloud (vector <vector <float> >, double, double, double);
#ifdef SPHERE_ATTEMPT
        static vector <vector <float> > sphere(int, double);
#endif
        static vector<vector<int> > graph(vector<vector<int> >);
        static vector<int> sort(vector<double>);
    };
}
