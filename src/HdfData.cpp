/*!
 * Implementation of HdfData
 */

#include "HdfData.h"
#include <vector>
#include <string>
#include <sstream>
#include <stdexcept>

using std::vector;
using std::string;
using std::runtime_error;
using std::stringstream;

#include <iostream>
using std::cout;
using std::endl;

morph::HdfData::HdfData (const string fname, const bool read_data)
{
    this->read_mode = read_data;
    if (this->read_mode == true) {
        this->file_id = H5Fopen (fname.c_str(), H5F_ACC_RDWR, H5P_DEFAULT);
    } else {
        this->file_id = H5Fcreate (fname.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    }
    // Check it's open, if not throw exception
    if ((int)this->file_id < 0) {
        stringstream ee;
        ee << "Error opening HDF5 file '" << fname << "'";
        throw runtime_error (ee.str());
    }
}

morph::HdfData::~HdfData()
{
    herr_t status = H5Fclose (this->file_id);
    if (status) {
        //stringstream ee;
        //ee << "Error closing HDF5 file; status: " << status;
        //throw runtime_error (ee.str());
    }
}

void
morph::HdfData::read_double_vector (const char* path, vector<double>& vals)
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

void
morph::HdfData::add_double (const char* path, const double& val)
{
    hsize_t dim_singleparam[1];
    dim_singleparam[0] = 1;
    hid_t dataspace_id = H5Screate_simple (1, dim_singleparam, NULL);
    hid_t dataset_id = H5Dcreate2 (this->file_id, path, H5T_IEEE_F64LE, dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
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

void
morph::HdfData::add_float (const char* path, const float& val)
{
    hsize_t dim_singleparam[1];
    herr_t status;
    dim_singleparam[0] = 1;
    hid_t dataspace_id = H5Screate_simple (1, dim_singleparam, NULL);
    hid_t dataset_id = H5Dcreate2 (this->file_id, path, H5T_IEEE_F64LE, dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    status = H5Dwrite (dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, &val);
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

void
morph::HdfData::add_double_vector (const char* path, const vector<double>& vals)
{
    hsize_t dim_singleparam[1];
    herr_t status;
    dim_singleparam[0] = vals.size();
    hid_t dataspace_id = H5Screate_simple (1, dim_singleparam, NULL);
    hid_t dataset_id = H5Dcreate2 (this->file_id, path, H5T_IEEE_F64LE, dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
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

void
morph::HdfData::add_float_vector (const char* path, const vector<float>& vals)
{
    hsize_t dim_singleparam[1];
    herr_t status;
    dim_singleparam[0] = vals.size();
    hid_t dataspace_id = H5Screate_simple (1, dim_singleparam, NULL);
    hid_t dataset_id = H5Dcreate2 (this->file_id, path, H5T_IEEE_F64LE, dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    status = H5Dwrite (dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, &(vals[0]));
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
