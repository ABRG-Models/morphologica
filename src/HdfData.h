/*!
 * Wrappers around the HDF5 C API for use in morphologica simulations.
 */
#ifndef _HDFDATA_H_
#define _HDFDATA_H_

#include <hdf5.h>
#include <vector>
#include <string>

using std::vector;
using std::string;

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
         * Read the data at path into vals.
         */
        template <typename F>
        void read_fpoint_vector (const char* path, vector<F>& vals);

        // WRITEME: Add read_double, read_float and read_float_vector.

        //@} // reading methods

        /*!
         * Writing methods
         */
        //@{

        /*!
         * Makes necessary calls to add a double to an HDF5 file store,
         * using path as the name of the variable. Path could be /myvar or
         * /somegroup/myvar, though I think you'd have to have created the
         * group for the latter.
         */
        template <typename F>
        void add_fpoint (const char* path, const double& val);

        /*!
         * Makes necessary calls to add a vector of floating point
         * values to an HDF5 file store, using path as the name of the
         * variable.
         */
        template <typename F>
        void add_fpoint_vector (const char* path, const vector<F>& vals);

        /*!
         * Add nvals floating point values from the F* array vals.
         */
        template <typename F>
        void add_fpoint_star (const char* path, F*& vals, const unsigned int nvals);

        //@} // writing methods

    }; // class hdf5

} // namespace morph

// THIS is where I will DEFINITELY need template/function specialization
template <typename F>
void
morph::HdfData::read_fpoint_vector (const char* path, vector<F>& vals)
{
    hid_t dataset_id = H5Dopen2 (this->file_id, path, H5P_DEFAULT);

    // Get number of elements in the dataset at path, and resize vals
    // so it's ready to receive the data.
    hid_t space_id = H5Dget_space (dataset_id);
    hsize_t dims[1] = {0};
    int ndims = H5Sget_simple_extent_dims (space_id, dims, NULL);
    if (ndims != 1) {
        stringstream ee;
        ee << "Error. Expected 1D data to be stored in " << path;
        throw runtime_error (ee.str());
    }
    vals.resize (dims[0], 0.0);

    // E.g HERE will need specialisation...
    herr_t status = H5Dread (dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &(vals[0]));
    if (status) {
        stringstream ee;
        ee << "Error. status after H5Dread: " << status;
        throw runtime_error (ee.str());
    }
    status = H5Dclose (dataset_id);
    if (status) {
        stringstream ee;
        ee << "Error. status after H5Dclose: " << status;
        throw runtime_error (ee.str());
    }
}

template <typename F>
void
morph::HdfData::add_fpoint (const char* path, const F& val)
{
    hsize_t dim_singleparam[1];
    dim_singleparam[0] = 1;
    hid_t dataspace_id = H5Screate_simple (1, dim_singleparam, NULL);
    // NB: Always use H5T_IEEE_F64LE to save the data in the file, so this line doesn't need specialisation:
    hid_t dataset_id = H5Dcreate2 (this->file_id, path, H5T_IEEE_F64LE, dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    // This lines needs specialisation
    herr_t status = H5Dwrite (dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &val);
    if (status) {
        stringstream ee;
        ee << "Error. status after H5Dwrite: " << status;
        throw runtime_error (ee.str());
    }
    status = H5Dclose (dataset_id);
    if (status) {
        stringstream ee;
        ee << "Error. status after H5Dclose: " << status;
        throw runtime_error (ee.str());
    }
    status = H5Sclose (dataspace_id);
    if (status) {
        stringstream ee;
        ee << "Error. status after H5Sclose: " << status;
        throw runtime_error (ee.str());
    }
}

template <typename F>
void
morph::HdfData::add_fpoint_star (const char* path, F*& vals, const unsigned int nvals)
{
    hsize_t dim_singleparam[1];
    herr_t status;
    dim_singleparam[0] = nvals;
    hid_t dataspace_id = H5Screate_simple (1, dim_singleparam, NULL);
    hid_t dataset_id = H5Dcreate2 (this->file_id, path, H5T_IEEE_F64LE, dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    // Specialise this line in some way:
    status = H5Dwrite (dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, vals);
    if (status) {
        stringstream ee;
        ee << "Error. status after H5Dwrite: " << status;
        throw runtime_error (ee.str());
    }
    status = H5Dclose (dataset_id);
    if (status) {
        stringstream ee;
        ee << "Error. status after H5Dclose: " << status;
        throw runtime_error (ee.str());
    }
    status = H5Sclose (dataspace_id);
    if (status) {
        stringstream ee;
        ee << "Error. status after H5Sclose: " << status;
        throw runtime_error (ee.str());
    }
}

template <typename F>
void
morph::HdfData::add_fpoint_vector (const char* path, const vector<F>& vals)
{
    hsize_t dim_singleparam[1];
    herr_t status;
    dim_singleparam[0] = vals.size();
    hid_t dataspace_id = H5Screate_simple (1, dim_singleparam, NULL);
    hid_t dataset_id = H5Dcreate2 (this->file_id, path, H5T_IEEE_F64LE, dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    // Specialise this line:
    status = H5Dwrite (dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &(vals[0]));
    if (status) {
        stringstream ee;
        ee << "Error. status after H5Dwrite: " << status;
        throw runtime_error (ee.str());
    }
    status = H5Dclose (dataset_id);
    if (status) {
        stringstream ee;
        ee << "Error. status after H5Dclose: " << status;
        throw runtime_error (ee.str());
    }
    status = H5Sclose (dataspace_id);
    if (status) {
        stringstream ee;
        ee << "Error. status after H5Sclose: " << status;
        throw runtime_error (ee.str());
    }
}

#endif // _HDFDATA_H_
