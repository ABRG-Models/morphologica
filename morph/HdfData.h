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
#include <stdexcept>
#include "morph/Vector.h"

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
         * Operate in read mode? If true, open the HDF5 file in read-only mode. If
         * false, open and truncate the HDF5 file. If read-write access becomes useful
         * or necessary in the future, then this will need to become an enumerated class
         * or type, with read_only/read_write/write_only options.
         */
        bool read_mode = false;

        /*!
         * If there's an error in status, output a context (given by
         * emsg) sensible message and throw an exception.
         */
        void handle_error (const herr_t& status, const std::string& emsg) const;

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

        // add_contained_vals for morph::Vector<T, N>
        template<typename T, size_t N>
        void add_contained_vals (const char* path, const std::vector<morph::Vector<T, N>>& vals)
        {
            this->process_groups (path);
            hsize_t dim_vecNdcoords[N]; // N Dims
            dim_vecNdcoords[0] = vals.size();
            dim_vecNdcoords[1] = N; // N doubles in each Vector<T,N>
            // Note 2 dims (1st arg, which is rank = 2)
            hid_t dataspace_id = H5Screate_simple (2, dim_vecNdcoords, NULL);
            // Now determine width of T and select the most suitable H5Dcreate2 call
            hid_t dataset_id = 0;
            herr_t status = 0;
            if constexpr (std::is_same<std::decay_t<T>, double>::value == true) {
                dataset_id = H5Dcreate2 (this->file_id, path, H5T_IEEE_F64LE, dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
                status = H5Dwrite (dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &(vals[0]));

            } else if constexpr (std::is_same<std::decay_t<T>, float>::value == true) {
                dataset_id = H5Dcreate2 (this->file_id, path, H5T_IEEE_F64LE, dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
                status = H5Dwrite (dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, &(vals[0]));

            } else if constexpr (std::is_same<std::decay_t<T>, int>::value == true) {
                dataset_id = H5Dcreate2 (this->file_id, path, H5T_STD_I64LE, dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
                status = H5Dwrite (dataset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, &(vals[0]));

            } else if constexpr (std::is_same<std::decay_t<T>, long long int>::value == true) {
                dataset_id = H5Dcreate2 (this->file_id, path, H5T_STD_I64LE, dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
                status = H5Dwrite (dataset_id, H5T_NATIVE_LLONG, H5S_ALL, H5S_ALL, H5P_DEFAULT, &(vals[0]));

            } else if constexpr (std::is_same<std::decay_t<T>, unsigned int>::value == true) {
                dataset_id = H5Dcreate2 (this->file_id, path, H5T_STD_U64LE, dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
                status = H5Dwrite (dataset_id, H5T_NATIVE_UINT, H5S_ALL, H5S_ALL, H5P_DEFAULT, &(vals[0]));

            } else if constexpr (std::is_same<typename std::decay<T>::type, unsigned long long int>::value == true) {
                dataset_id = H5Dcreate2 (this->file_id, path, H5T_STD_U64LE, dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
                status = H5Dwrite (dataset_id, H5T_NATIVE_ULLONG, H5S_ALL, H5S_ALL, H5P_DEFAULT, &(vals[0]));

            } else {
                throw std::runtime_error ("HdfData::add_contained_vals: Don't know how to store that type");
            }
            this->handle_error (status, "Error. status after H5Dwrite: ");
            status = H5Dclose (dataset_id);
            this->handle_error (status, "Error. status after H5Dclose: ");
            status = H5Sclose (dataspace_id);
            this->handle_error (status, "Error. status after H5Sclose: ");
        }

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
