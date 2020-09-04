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
#include <sstream>
#include <stdexcept>
#include "morph/Vector.h"

namespace morph {

    /*!
     * Encodes the action to take on a read error. e.g. if you try to read /vars/A but
     * /vars/A does not exist inside the opened Hdf5 file. Choices are throw an
     * exception; output some information to stdout, output a warning to stderr or just
     * carry on and accept that you can't read that thing.
     */
    enum class ReadErrorAction {
        Exception,
        Warning,
        Info,
        Continue
    };

    /*!
     * Very simple data access class, wrapping around the HDF5 C API. Operates either in
     * write mode (the default) or read mode. Choose which when constructing.
     */
    class HdfData
    {
    private:
        //! The HDF5 file id.
        hid_t file_id = -1;

        /*!
         * Operate in read mode? If true, open the HDF5 file in read-only mode. If
         * false, open and truncate the HDF5 file. If read-write access becomes useful
         * or necessary in the future, then this will need to become an enumerated class
         * or type, with read_only/read_write/write_only options.
         */
        bool read_mode = false;

        /*!
         * If there's an error in status, output a context (given by emsg) sensible
         * message and throw an exception.
         */
        void handle_error (const herr_t& status, const std::string& emsg) const;

        /*!
         * Check the dataset_id, perform correct action and return a value which says
         * whether the calling function should itself return.
         *
         * \param dataset_id the return value from call to H5Dopen2()
         * \param path The path of the thing opened; used to construct info message
         *
         * \return 0 if dataset_id was ok; -1 if dataset_id was -1 and caller should
         * return.
         */
        int check_dataset_id (const hid_t dataset_id, const char* path) const;

    public:
        /*!
         * Construct, creating open file_id. If read_data is true, then open in read
         * mode; if read_data is false, then truncate on opening. Note that this HDF5
         * wrapper library doesn't attempt to carry out HDF5 file modification
         * operations - it's all-or-nothing - a file is truncated and recreated if it's
         * going to have data written into it at all. Finally, set
         * show_hdf_internal_errors to true to switch on libhdf's verbose error output.
         */
        HdfData (const std::string fname, const bool read_data = false, const bool show_hdf_internal_errors = false);
        //! Deconstruct, closing the file_id
        ~HdfData();

        /*!
         * When reading a field in a file that doesn't exist, client code could want the
         * library to either gracefully handle its missing-ness and carry on -
         * i.e. read_contained_vals("/non-existent/path", vals) should just return
         * without changing the content of \a vals. Client code could instead need it to
         * throw an exception in such a case. Finally, it might be desirable to output a
         * message to stdout or stderr, but carry on anyway.
         *
         * When *writing* I think you always want an error if the write action fails,
         * which is why this is a ReadErrorAction.
         */
        ReadErrorAction read_error_action = ReadErrorAction::Info;

        /*!
         * Templated version of read_contained_vals, for vector/list/deque (but not map)
         * and whatever simple value (int, double, float, etc) is contained. Use this to
         * read, for example: std::vector<double> or std::deque<unsigned int> or
         * std::list<float>. Can also do std::vector<cv::Point> or
         * std::deque<cv::Point2d> etc.
         */
        template < template <typename, typename> typename Container,
                   typename T,
                   typename Allocator=std::allocator<T> >
        void read_contained_vals (const char* path, Container<T, Allocator>& vals)
        {
            hid_t dataset_id = H5Dopen2 (this->file_id, path, H5P_DEFAULT);
            if (this->check_dataset_id (dataset_id, path) == -1) { return; }

            hid_t space_id = H5Dget_space (dataset_id);
            // Regardless of read_error_action, throw exception here; missing paths
            // should cause missing dataset_id
            if (space_id < 0) {
                std::stringstream ee;
                ee << "Error: Failed to get a dataset space_id for dataset_id " << dataset_id;
                throw std::runtime_error (ee.str());
            }

            // If cv::Point like. Could add pair<float, float> and pair<double, double>,
            // also container of array<T, 2> also.
            if constexpr (std::is_same<std::decay_t<T>, cv::Point2i>::value == true
                          || std::is_same<std::decay_t<T>, cv::Point2f>::value == true
                          || std::is_same<std::decay_t<T>, cv::Point2d>::value == true
                          || std::is_same<std::decay_t<T>, std::array<float, 2>>::value == true
                          || std::is_same<std::decay_t<T>, std::array<double, 2>>::value == true) {
                hsize_t dims[2] = {0,0}; // 2D
                int ndims = H5Sget_simple_extent_dims (space_id, dims, NULL);
                if (ndims != 2) {
                    std::stringstream ee;
                    ee << "In:\n" << __PRETTY_FUNCTION__
                       << "\nError: Expected 2D data to be stored in " << path << ". ndims=" << ndims;
                    throw std::runtime_error (ee.str());
                }
                if (dims[1] != 2) {
                    std::stringstream ee;
                    ee << "In:\n" << __PRETTY_FUNCTION__
                       << ":\nError: Expected 2 coordinates to be stored in each cv::Point of " << path;
                    throw std::runtime_error (ee.str());
                }
                vals.resize (dims[0]);
            } else {
                // If standard thing like double, float, int etc:
                hsize_t dims[1] = {0}; // 1D

                int ndims = H5Sget_simple_extent_dims (space_id, dims, NULL);
                if (ndims != 1) {
                    std::stringstream ee;
                    ee << "In:\n" << __PRETTY_FUNCTION__
                       << ":\nError: Expected 1D data to be stored in " << path << ". ndims=" << ndims;
                    throw std::runtime_error (ee.str());
                }
                vals.resize (dims[0], T{0});
            }

            herr_t status = 0;
            if constexpr (std::is_same<std::decay_t<T>, double>::value == true) {
                status = H5Dread (dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &(vals[0]));

            } else if constexpr (std::is_same<std::decay_t<T>, float>::value == true) {
                status = H5Dread (dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, &(vals[0]));

            } else if constexpr (std::is_same<std::decay_t<T>, int>::value == true) {
                status = H5Dread (dataset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, &(vals[0]));

            } else if constexpr (std::is_same<std::decay_t<T>, long long int>::value == true) {
                status = H5Dread (dataset_id, H5T_NATIVE_LLONG, H5S_ALL, H5S_ALL, H5P_DEFAULT, &(vals[0]));

            } else if constexpr (std::is_same<std::decay_t<T>, unsigned int>::value == true) {
                status = H5Dread (dataset_id, H5T_NATIVE_UINT, H5S_ALL, H5S_ALL, H5P_DEFAULT, &(vals[0]));

            } else if constexpr (std::is_same<typename std::decay<T>::type, unsigned long long int>::value == true) {
                status = H5Dread (dataset_id, H5T_NATIVE_ULLONG, H5S_ALL, H5S_ALL, H5P_DEFAULT, &(vals[0]));

            } else if constexpr (std::is_same<typename std::decay<T>::type, cv::Point2i>::value == true) {
                status = H5Dread (dataset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, &(vals[0]));

            } else if constexpr (std::is_same<typename std::decay<T>::type, cv::Point2d>::value == true) {
                status = H5Dread (dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &(vals[0]));

            } else if constexpr (std::is_same<typename std::decay<T>::type, cv::Point2f>::value == true) {
                status = H5Dread (dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, &(vals[0]));

            } else if constexpr (std::is_same<typename std::decay<T>::type, std::array<float,2>>::value == true) {
                status = H5Dread (dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, &(vals[0]));

            } else if constexpr (std::is_same<typename std::decay<T>::type, std::array<double,2>>::value == true) {
                status = H5Dread (dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &(vals[0]));

            } else {
                throw std::runtime_error ("HdfData::read_contained_vals<T>: Don't know how to read that type");
            }

            this->handle_error (status, "Error. status after H5Dread: ");
            status = H5Dclose (dataset_id);
            this->handle_error (status, "Error. status after H5Dclose: ");
        }

        // Read pairs
        void read_contained_vals (const char* path, std::pair<float, float>& vals);
        void read_contained_vals (const char* path, std::pair<double, double>& vals);
        // Read lists of pairs
        void read_contained_vals (const char* path, std::list<std::pair<float, float>>& vals);
        void read_contained_vals (const char* path, std::list<std::pair<double, double>>& vals);
        //! Read a vector of 3D coordinates
        void read_contained_vals (const char* path, std::vector<std::array<float, 3>>& vals);
        //! 3D coordinates collected into groups of 4 (each specifying a quad)
        void read_contained_vals (const char* path, std::vector<std::array<float, 12>>& vals);

        //! Read a simple value of type T
        template <typename T>
        void read_val (const char* path, T& val)
        {
            hid_t dataset_id = H5Dopen2 (this->file_id, path, H5P_DEFAULT);
            if (this->check_dataset_id (dataset_id, path) == -1) { return; }

            herr_t status = 0;
            if constexpr (std::is_same<std::decay_t<T>, double>::value == true) {
                status = H5Dread (dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &val);
            } else if constexpr (std::is_same<std::decay_t<T>, float>::value == true) {
                status = H5Dread (dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, &val);
            } else if constexpr (std::is_same<std::decay_t<T>, int>::value == true) {
                status = H5Dread (dataset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, &val);
            } else if constexpr (std::is_same<std::decay_t<T>, long long int>::value == true) {
                status = H5Dread (dataset_id, H5T_NATIVE_LLONG, H5S_ALL, H5S_ALL, H5P_DEFAULT, &val);
            } else if constexpr (std::is_same<std::decay_t<T>, unsigned int>::value == true) {
                status = H5Dread (dataset_id, H5T_NATIVE_UINT, H5S_ALL, H5S_ALL, H5P_DEFAULT, &val);
            } else if constexpr (std::is_same<typename std::decay<T>::type, unsigned long long int>::value == true) {
                status = H5Dread (dataset_id, H5T_NATIVE_ULLONG, H5S_ALL, H5S_ALL, H5P_DEFAULT, &val);
            } else if constexpr (std::is_same<typename std::decay<T>::type, bool>::value == true) {
                unsigned int uival = 0;
                status = H5Dread (dataset_id, H5T_NATIVE_UINT, H5S_ALL, H5S_ALL, H5P_DEFAULT, &uival);
                // Copy unsigned int value into bool:
                val = uival > 0 ? true : false;
            } else {
                throw std::runtime_error ("HdfData::read_val<T>: Don't know how to read that type");
            }
            this->handle_error (status, "Error. status after H5Dread: ");
            status = H5Dclose (dataset_id);
            this->handle_error (status, "Error. status after H5Dclose: ");
        }

        //! Read a string of chars
        void read_string (const char* path, std::string& str);

        //! Templated read_val for bitsets
        template <size_t N>
        void read_val (const char* path, std::bitset<N>& val)
        {
            unsigned long long int bs_ullong = 0ULL;
            this->read_val (path, bs_ullong);
            std::bitset<N> val_ (bs_ullong);
            val = val_;
        }

        /*!
         * Read an OpenCV Matrix that was stored with the sister add_contained_vals
         * function (which also stores some necessary metadata).
         */
        void read_contained_vals (const char* path, cv::Mat& vals);

        /*!
         * Given a path like /a/b/c, verify and if necessary create group a, then verify
         * and if necessary create group b so that the dataset c can be succesfully
         * created.
         */
        void process_groups (const char* path);
        void verify_group (const std::string& path);

        /*!
         * Makes necessary calls to add a double or float (or integer types if the
         * overloads are added) to an HDF5 file store, using path as the name of the
         * variable. Path could be /myvar or /somegroup/myvar, though I think you'd have
         * to have created the group for the latter. I don't think templating is
         * worthwhile for these functions. (2020 update: Having discovered expression
         * sfinae and constexpr, I now think templating IS probably useful. See example:
         * void add_contained_vals (const char*, const std::vector<morph::Vector<T, N>>&)
         */
        template <typename T>
        void add_val (const char* path, const T& val)
        {
            this->process_groups (path);
            hsize_t dim_singleparam[1];
            dim_singleparam[0] = 1;
            hid_t dataspace_id = H5Screate_simple (1, dim_singleparam, NULL);
            hid_t dataset_id = 0;
            herr_t status = 0;
            if constexpr (std::is_same<std::decay_t<T>, double>::value == true) {
                dataset_id = H5Dcreate2 (this->file_id, path, H5T_IEEE_F64LE, dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
                status = H5Dwrite (dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &val);
            } else if constexpr (std::is_same<std::decay_t<T>, float>::value == true) {
                dataset_id = H5Dcreate2 (this->file_id, path, H5T_IEEE_F64LE, dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
                status = H5Dwrite (dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, &val);
            } else if constexpr (std::is_same<std::decay_t<T>, int>::value == true) {
                dataset_id = H5Dcreate2 (this->file_id, path, H5T_STD_I64LE, dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
                status = H5Dwrite (dataset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, &val);
            } else if constexpr (std::is_same<std::decay_t<T>, long long int>::value == true) {
                dataset_id = H5Dcreate2 (this->file_id, path, H5T_STD_I64LE, dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
                status = H5Dwrite (dataset_id, H5T_NATIVE_LLONG, H5S_ALL, H5S_ALL, H5P_DEFAULT, &val);
            } else if constexpr (std::is_same<std::decay_t<T>, unsigned int>::value == true) {
                dataset_id = H5Dcreate2 (this->file_id, path, H5T_STD_U64LE, dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
                status = H5Dwrite (dataset_id, H5T_NATIVE_UINT, H5S_ALL, H5S_ALL, H5P_DEFAULT, &val);
            } else if constexpr (std::is_same<typename std::decay<T>::type, unsigned long long int>::value == true) {
                dataset_id = H5Dcreate2 (this->file_id, path, H5T_STD_U64LE, dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
                status = H5Dwrite (dataset_id, H5T_NATIVE_ULLONG, H5S_ALL, H5S_ALL, H5P_DEFAULT, &val);
            } else if constexpr (std::is_same<typename std::decay<T>::type, bool>::value == true) {
                unsigned int uival = (val == true ? 1 : 0);
                dataset_id = H5Dcreate2 (this->file_id, path, H5T_STD_U64LE, dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
                status = H5Dwrite (dataset_id, H5T_NATIVE_UINT, H5S_ALL, H5S_ALL, H5P_DEFAULT, &uival);
            } else {
                throw std::runtime_error ("HdfData::add_val<T>: Don't know how to add that type");
            }
            this->handle_error (status, "Error. status after H5Dwrite: ");
            status = H5Dclose (dataset_id);
            this->handle_error (status, "Error. status after H5Dclose: ");
            status = H5Sclose (dataspace_id);
            this->handle_error (status, "Error. status after H5Sclose: ");
        }

        /*!
         * Templated add_val for bitset, so that the code can handle all sizes of
         * bitset. Note the limit on N here is the size of an unsigned long long int, so
         * that's a bit of a FIXME for the future.
         */
        template <size_t N>
        void add_val (const char* path, const std::bitset<N>& val)
        {
            unsigned long long int bs_ullong = val.to_ullong();
            this->add_val (path, bs_ullong);
        }

        //! Add a string of chars
        void add_string (const char* path, const std::string& str);

        /*!
         * Makes necessary calls to add a container of values to an HDF5 file store,
         * using path as the name of the variable. This might be a candidate for
         * templating, where it wasn't worth it for add_val(), because of the
         * possibility of vectors, lists, sets, deques etc, of floats, doubles,
         * integers, unsigned integers, etc. However, I can't see the solution to the
         * problem of the specialisation required in just one line of each function, so
         * it may be that may overloaded copies of these functions is still the best
         * solution. It also compiles to linkable, unchanging code in libmorphologica,
         * rather than being header-only,
         */
        template < template <typename, typename> typename Container,
                   typename T,
                   typename Allocator=std::allocator<T> >
        void add_contained_vals (const char* path, const Container<T, Allocator>& vals)
        {
            this->process_groups (path);

            hid_t dataspace_id = 0;
            if constexpr (std::is_same<std::decay_t<T>, cv::Point2i>::value == true
                          || std::is_same<std::decay_t<T>, cv::Point2f>::value == true
                          || std::is_same<std::decay_t<T>, cv::Point2d>::value == true
                          || std::is_same<std::decay_t<T>, std::array<float, 2>>::value == true
                          || std::is_same<std::decay_t<T>, std::array<double, 2>>::value == true) {
                hsize_t dim_vec2dcoords[2]; // 2 Dims
                dim_vec2dcoords[0] = vals.size();
                dim_vec2dcoords[1] = 2; // 2 ints in each cv::Point
                // Note 2 dims (1st arg, which is rank = 2)
                dataspace_id = H5Screate_simple (2, dim_vec2dcoords, NULL);
            } else {
                hsize_t dim_singleparam[1];
                dim_singleparam[0] = vals.size();
                dataspace_id = H5Screate_simple (1, dim_singleparam, NULL);
            }

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

            } else if constexpr (std::is_same<typename std::decay<T>::type, cv::Point2i>::value == true) {
                dataset_id = H5Dcreate2 (this->file_id, path, H5T_STD_I64LE, dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
                status = H5Dwrite (dataset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, &(vals[0]));

            } else if constexpr (std::is_same<typename std::decay<T>::type, cv::Point2d>::value == true) {
                dataset_id = H5Dcreate2 (this->file_id, path, H5T_IEEE_F64LE, dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
                status = H5Dwrite (dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &(vals[0]));

            } else if constexpr (std::is_same<typename std::decay<T>::type, cv::Point2f>::value == true) {
                dataset_id = H5Dcreate2 (this->file_id, path, H5T_IEEE_F64LE, dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
                status = H5Dwrite (dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, &(vals[0]));

            } else if constexpr (std::is_same<typename std::decay<T>::type, std::array<float,2>>::value == true) {
                dataset_id = H5Dcreate2 (this->file_id, path, H5T_IEEE_F64LE, dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
                status = H5Dwrite (dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, &(vals[0]));

            } else if constexpr (std::is_same<typename std::decay<T>::type, std::array<double,2>>::value == true) {
                dataset_id = H5Dcreate2 (this->file_id, path, H5T_IEEE_F64LE, dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
                status = H5Dwrite (dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &(vals[0]));

            } else {
                throw std::runtime_error ("HdfData::add_contained_vals<T>: Don't know how to store that type");
            }
            this->handle_error (status, "Error. status after H5Dwrite: ");
            status = H5Dclose (dataset_id);
            this->handle_error (status, "Error. status after H5Dclose: ");
            status = H5Sclose (dataspace_id);
            this->handle_error (status, "Error. status after H5Sclose: ");
        }

        //! 3D coordinates (array of floats)
        void add_contained_vals (const char* path, const std::vector<std::array<float, 3>>& vals);
        //! Sets of 4 3D coordinates (if you like, or anything else that requires arrays of 12 floats)
        void add_contained_vals (const char* path, const std::vector<std::array<float, 12>>& vals);

        //! add_contained_vals for morph::Vector<T, N>
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
                throw std::runtime_error ("HdfData::add_contained_vals<T,N>: Don't know how to store that type");
            }
            this->handle_error (status, "Error. status after H5Dwrite: ");
            status = H5Dclose (dataset_id);
            this->handle_error (status, "Error. status after H5Dclose: ");
            status = H5Sclose (dataspace_id);
            this->handle_error (status, "Error. status after H5Sclose: ");
        }

        //! Add a cv::Point_<T> to the HDF5 file
        template <typename T>
        void add_contained_vals (const char* path, const cv::Point_<T>& val)
        {
            this->process_groups (path);
            hsize_t dim_vec2dcoord[2];
            dim_vec2dcoord[0] = 1;
            dim_vec2dcoord[1] = 2; // 2 coordinates in a cv::Point_
            // Note 2 dims (1st arg, which is rank = 2)
            hid_t dataspace_id = H5Screate_simple (2, dim_vec2dcoord, NULL);
            // Now determine width of T and select the most suitable H5Dcreate2 call
            hid_t dataset_id = 0;
            herr_t status = 0;
            // Copy the data out of the point and into a nice contiguous array
            std::vector<T> data_array(2, 0);
            data_array[0] = val.x;
            //std::cout << "val: ("  << val.x << "," << val.y << ")" << std::endl;
            data_array[1] = val.y;
            if constexpr (std::is_same<std::decay_t<T>, double>::value == true) {
                dataset_id = H5Dcreate2 (this->file_id, path, H5T_IEEE_F64LE, dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
                status = H5Dwrite (dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &(data_array[0]));

            } else if constexpr (std::is_same<std::decay_t<T>, float>::value == true) {
                dataset_id = H5Dcreate2 (this->file_id, path, H5T_IEEE_F64LE, dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
                status = H5Dwrite (dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, &(data_array[0]));

            } else if constexpr (std::is_same<std::decay_t<T>, int>::value == true) {
                dataset_id = H5Dcreate2 (this->file_id, path, H5T_STD_I64LE, dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
                status = H5Dwrite (dataset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, &(data_array[0]));

            } else if constexpr (std::is_same<std::decay_t<T>, long long int>::value == true) {
                dataset_id = H5Dcreate2 (this->file_id, path, H5T_STD_I64LE, dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
                status = H5Dwrite (dataset_id, H5T_NATIVE_LLONG, H5S_ALL, H5S_ALL, H5P_DEFAULT, &(data_array[0]));

            } else if constexpr (std::is_same<std::decay_t<T>, unsigned int>::value == true) {
                dataset_id = H5Dcreate2 (this->file_id, path, H5T_STD_U64LE, dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
                status = H5Dwrite (dataset_id, H5T_NATIVE_UINT, H5S_ALL, H5S_ALL, H5P_DEFAULT, &(data_array[0]));

            } else if constexpr (std::is_same<typename std::decay<T>::type, unsigned long long int>::value == true) {
                dataset_id = H5Dcreate2 (this->file_id, path, H5T_STD_U64LE, dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
                status = H5Dwrite (dataset_id, H5T_NATIVE_ULLONG, H5S_ALL, H5S_ALL, H5P_DEFAULT, &(data_array[0]));

            } else {
                throw std::runtime_error ("HdfData::add_contained_vals: Don't know how to store a cv::Point_ of that type");
            }
            this->handle_error (status, "Error. status after H5Dwrite: ");
            status = H5Dclose (dataset_id);
            this->handle_error (status, "Error. status after H5Dclose: ");
            status = H5Sclose (dataspace_id);
            this->handle_error (status, "Error. status after H5Sclose: ");
        }

        //! Write out cv::Mat, along with the data type and the channels as metadata.
        void add_contained_vals (const char* path, const cv::Mat& vals);
        //! Add pair of floats
        void add_contained_vals (const char* path, const std::pair<float, float>& vals);
        //! Add pair of doubles
        void add_contained_vals (const char* path, const std::pair<double, double>& vals);
        //! Add a list of pairs of floats
        void add_contained_vals (const char* path, const std::list<std::pair<float, float>>& vals);
        //! Add a list of pairs of doubles
        void add_contained_vals (const char* path, const std::list<std::pair<double, double>>& vals);
        //! Add nvals values from the pointer (to doubles) vals.
        void add_ptrarray_vals (const char* path, double*& vals, const unsigned int nvals);
        //! Add nvals values from the pointer (to floats) vals.
        void add_ptrarray_vals (const char* path, float*& vals, const unsigned int nvals);

    }; // class HdfData

} // namespace morph
