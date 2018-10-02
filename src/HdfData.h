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
        void read_double_vector (const char* path, vector<double>& vals);

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
         * Add nvals doubles from the double array vals.
         */
        void add_double_star (const char* path, double*& vals, const unsigned int nvals);

        /*!
         * Makes necessary calls to add a vector of floats to an HDF5
         * file store, using path as the name of the variable.
         */
        void add_float_vector (const char* path, const vector<float>& vals);
        //@} // writing methods

    }; // class hdf5

} // namespace morph

#endif // _HDFDATA_H_
