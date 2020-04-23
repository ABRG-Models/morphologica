/*!
 * Wrappers around the HDF5 C API for use in morphologica simulations.
 */
#pragma once

#include <opencv2/opencv.hpp>
#include <hdf5.h>
#include <vector>
#include <array>
#include <list>
#include <string>
#include <utility>
#include <bitset>

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
        void handle_error (const herr_t& status, const std::string& emsg);

    public:
        /*!
         * Construct, creating open file_id. If read_data is
         */
        HdfData (const std::string fname, const bool read_data = false);

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
        void read_contained_vals (const char* path, std::vector<double>& vals);
        void read_contained_vals (const char* path, std::vector<float>& vals);
        void read_contained_vals (const char* path, std::vector<int>& vals);
        void read_contained_vals (const char* path, std::vector<unsigned int>& vals);
        void read_contained_vals (const char* path, std::vector<long long int>& vals);
        void read_contained_vals (const char* path, std::vector<unsigned long long int>& vals);

        /*!
         * Read pairs
         */
        //@{
        void read_contained_vals (const char* path, std::pair<float, float>& vals);
        void read_contained_vals (const char* path, std::pair<double, double>& vals);
        //@}

        /*!
         * Read lists of pairs
         */
        //@{
        void read_contained_vals (const char* path, std::list<std::pair<float, float>>& vals);
        void read_contained_vals (const char* path, std::list<std::pair<double, double>>& vals);
        //@}

        /*!
         * 2D coordinates (or other pairs of values)
         */
        //@{
        void read_contained_vals (const char* path, std::vector<std::array<float, 2>>& vals);
        void read_contained_vals (const char* path, std::vector<std::array<double, 2>>& vals);
        //@}

        /*!
         * Vector of 3D coordinates
         */
        //@{
        void read_contained_vals (const char* path, std::vector<std::array<float, 3>>& vals);
        //! 3D coordinates collected into groups of 4 (each specifying a quad)
        void read_contained_vals (const char* path, std::vector<std::array<float, 12>>& vals);
        //! OpenCV-friendly overloads
        //@{
        void read_contained_vals (const char* path, std::vector<cv::Point2i>& vals);
        void read_contained_vals (const char* path, std::vector<cv::Point2f>& vals);
        void read_contained_vals (const char* path, std::vector<cv::Point2d>& vals);
        //@}
        //@}

        void read_val (const char* path, double& val);
        void read_val (const char* path, float& val);
        void read_val (const char* path, int& val);
        void read_val (const char* path, unsigned int& val);
        void read_val (const char* path, long long int& val);
        void read_val (const char* path, unsigned long long int& val);
        void read_val (const char* path, bool& val);

        //! Read a string of chars
        void read_string (const char* path, std::string& str);

        //! Templated read_val for bitsets
        template <size_t N>
        void read_val (const char* path, std::bitset<N>& val) {
            unsigned long long int bs_ullong = 0ULL;
            this->read_val (path, bs_ullong);
            std::bitset<N> val_ (bs_ullong);
            val = val_;
        }

        //! Read an OpenCV Matrix that was stored with the sister add_contained_vals
        //! function (which also stores some necessary metadata).
        void read_contained_vals (const char* path, cv::Mat& vals);

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
        void verify_group (const std::string& path);
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

        /*!
         * Templated add_val for bitset, so that the code can handle all sizes of
         * bitset. Note the limit on N here is the size of an unsigned long long int,
         * so that's a bit of a FIXME for the future.
         */
        template <size_t N>
        void add_val (const char* path, const std::bitset<N>& val) {
            unsigned long long int bs_ullong = val.to_ullong();
            this->add_val (path, bs_ullong);
        }
        //@}

        //! Add a string of chars
        void add_string (const char* path, const std::string& str);

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
        void add_contained_vals (const char* path, const std::vector<double>& vals);
        void add_contained_vals (const char* path, const std::vector<float>& vals);
        void add_contained_vals (const char* path, const std::vector<int>& vals);
        void add_contained_vals (const char* path, const std::vector<unsigned int>& vals);
        void add_contained_vals (const char* path, const std::vector<long long int>& vals);
        void add_contained_vals (const char* path, const std::vector<unsigned long long int>& vals);
        //@}

        /*!
         * Containers of coordinates
         */
        //@{
        //! 2D coordinates
        void add_contained_vals (const char* path, const std::vector<std::array<float, 2>>& vals);
        void add_contained_vals (const char* path, const std::vector<std::array<double, 2>>& vals);
        //! 3D coordinates
        void add_contained_vals (const char* path, const std::vector<std::array<float, 3>>& vals);
        //! Sets of 4 3D coordinates (if you like, or anything else that requires arrays of 12 floats)
        void add_contained_vals (const char* path, const std::vector<std::array<float, 12>>& vals);
        //! OpenCV Point objects
        void add_contained_vals (const char* path, const std::vector<cv::Point2i>& vals);
        void add_contained_vals (const char* path, const std::vector<cv::Point2d>& vals);
        void add_contained_vals (const char* path, const std::vector<cv::Point2f>& vals);

        //! Write out cv::Mat matrix data, along with the data type and the channels
        //! as metadata.
        void add_contained_vals (const char* path, const cv::Mat& vals);
        //@}

        /*!
         * Add pairs
         */
        //@{
        void add_contained_vals (const char* path, const std::pair<float, float>& vals);
        void add_contained_vals (const char* path, const std::pair<double, double>& vals);
        //@}

        /*!
         * Add lists of pairs
         */
        //@{
        void add_contained_vals (const char* path, const std::list<std::pair<float, float>>& vals);
        void add_contained_vals (const char* path, const std::list<std::pair<double, double>>& vals);
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
