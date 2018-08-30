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

    class HdfData
    {
    private:
        /*!
         * The HDF5 file id.
         */
        hid_t file_id = -1;

    public:
        /*!
         * Construct, creating open file_id
         */
        HdfData (const string fname);

        /*!
         * Deconstruct, closing the file_id
         */
        ~HdfData();

        /*!
         * Makes necessary calls to add a double to an HDF5 file store,
         * using path as the name of the variable. Path could be /myvar or
         * /somegroup/myvar, though I think you'd have to have created the
         * group for the latter.
         */
        void add_double (const char* path, const double& val);

        /*!
         * Makes necessary calls to add a float to an HDF5 file store,
         * using path as the name of the variable.
         */
        void add_float (const char* path, const float& val);

        /*!
         * Makes necessary calls to add a vector of doubles to an HDF5
         * file store, using path as the name of the variable.
         */
        void add_double_vector (const char* path, const vector<double>& vals);

        /*!
         * Makes necessary calls to add a vector of floats to an HDF5
         * file store, using path as the name of the variable.
         */
        void add_float_vector (const char* path, const vector<float>& vals);

    }; // class hdf5

} // namespace morph

#endif // _HDFDATA_H_
