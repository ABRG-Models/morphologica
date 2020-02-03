/*!
 * Wrappers around the HDF5 C API for use in morphologica simulations.
 */
#ifndef _HDFDATA_H_
#define _HDFDATA_H_

//#include <opencv2/opencv.hpp>
#include <hdf5.h>
#include <vector>
#include <array>
#include <list>
#include <string>
#include <utility>

using std::vector;
using std::array;
using std::list;
using std::string;
using std::pair;

namespace morph {

    /*!
     * Very simple data access class, wrapping around the HDF5 C
     * API. Operates either in write mode (the default) or read
     * mode. Choose which when constructing.
     */
    class HdfData
    {
    private:
        /*!
         * The HDF5 file id.
         */
        hid_t file_id = -1;

        /*!
         * Operate in read mode?
         */
        bool read_mode = false;

        /*!
         * If there's an error in status, output a context (given by
         * emsg) sensible message and throw an exception.
         */
        void handle_error (const herr_t& status, const string& emsg);

    public:
        /*!
         * Construct, creating open file_id. If read_data is
         */
        HdfData (const string fname, const bool read_data = false);

        /*!
         * Deconstruct, closing the file_id
         */
        ~HdfData();

        /*!
         * Reading methods
         */
        //@{

        /*!
         * Read the data at path into the container vals. Templating
         * worthwhile because of number of possible overloads? Quite
         * possibly.
         */
        void read_contained_vals (const char* path, vector<double>& vals);
        void read_contained_vals (const char* path, vector<float>& vals);
        void read_contained_vals (const char* path, vector<int>& vals);
        void read_contained_vals (const char* path, vector<unsigned int>& vals);
        void read_contained_vals (const char* path, vector<long long int>& vals);
        void read_contained_vals (const char* path, vector<unsigned long long int>& vals);

        /*!
         * Read pairs
         */
        //@{
        void read_contained_vals (const char* path, pair<float, float>& vals);
        void read_contained_vals (const char* path, pair<double, double>& vals);
        //@}

        /*!
         * Read lists of pairs
         */
        //@{
        void read_contained_vals (const char* path, list<pair<float, float>>& vals);
        void read_contained_vals (const char* path, list<pair<double, double>>& vals);
        //@}

        /*!
         * Vector of 3D coordinates
         */
        //@{
        void read_contained_vals (const char* path, vector<array<float, 3>>& vals);
        void read_contained_vals (const char* path, vector<array<float, 12>>& vals);
        //@}

        void read_val (const char* path, double& val);
        void read_val (const char* path, float& val);
        void read_val (const char* path, int& val);
        void read_val (const char* path, unsigned int& val);
        void read_val (const char* path, long long int& val);
        void read_val (const char* path, unsigned long long int& val);
        void read_val (const char* path, bool& val);
        //@} // reading methods

        /*!
         * Writing methods
         */
        //@{

        /*!
         * Given a path like /a/b/c, verify and if necessary create
         * group a, then verify and if necessary create group b so
         * that the dataset c can be succesfully created.
         */
        //@{
        void process_groups (const char* path);
        void verify_group (const string& path);
        //@}

        /*!
         * Makes necessary calls to add a double or float (or integer
         * types if the overloads are added) to an HDF5 file store,
         * using path as the name of the variable. Path could be
         * /myvar or /somegroup/myvar, though I think you'd have to
         * have created the group for the latter. I don't think
         * templating is worthwhile for these functions.
         */
        //@{
        void add_val (const char* path, const double& val);
        void add_val (const char* path, const float& val);
        void add_val (const char* path, const int& val);
        void add_val (const char* path, const unsigned int& val);
        void add_val (const char* path, const long long int& val);
        void add_val (const char* path, const unsigned long long int& val);
        void add_val (const char* path, const bool& val);
        //@}

        /*!
         * Makes necessary calls to add a container of values to an
         * HDF5 file store, using path as the name of the
         * variable. This might be a candidate for templating, where
         * it wasn't worth it for add_val(), because of the
         * possibility of vectors, lists, sets, deques etc, of floats,
         * doubles, integers, unsigned integers, etc. However, I can't
         * see the solution to the problem of the specialisation
         * required in just one line of each function, so it may be
         * that may overloaded copies of these functions is still the
         * best solution. It also compiles to linkable, unchanging
         * code in libmorphologica, rather than being header-only,
         */
        //@{
        void add_contained_vals (const char* path, const vector<double>& vals);
        void add_contained_vals (const char* path, const vector<float>& vals);
        void add_contained_vals (const char* path, const vector<int>& vals);
        void add_contained_vals (const char* path, const vector<unsigned int>& vals);
        void add_contained_vals (const char* path, const vector<long long int>& vals);
        void add_contained_vals (const char* path, const vector<unsigned long long int>& vals);
        //@}

        /*!
         * Containers of coordinates
         */
        //@{
        //! 3D coordinates
        void add_contained_vals (const char* path, const vector<array<float, 3>>& vals);
        //! Sets of 4 3D coordinates (if you like, or anything else that requires arrays of 12 floats)
        void add_contained_vals (const char* path, const vector<array<float, 12>>& vals);
        //! OpenCV Point objects
        //void add_contained_vals (const char* path, const vector<cv::Point>& vals);
        //void add_contained_vals (const char* path, const vector<cv::Point2d>& vals);
        //@}

        /*!
         * Add pairs
         */
        //@{
        void add_contained_vals (const char* path, const pair<float, float>& vals);
        void add_contained_vals (const char* path, const pair<double, double>& vals);
        //@}

        /*!
         * Add lists of pairs
         */
        //@{
        void add_contained_vals (const char* path, const list<pair<float, float>>& vals);
        void add_contained_vals (const char* path, const list<pair<double, double>>& vals);
        //@}

        /*!
         * Add nvals values from the pointer vals.
         */
        //@{
        void add_ptrarray_vals (const char* path, double*& vals, const unsigned int nvals);
        void add_ptrarray_vals (const char* path, float*& vals, const unsigned int nvals);
        //@}

        //@} // writing methods

    }; // class hdf5

} // namespace morph

#endif // _HDFDATA_H_
