/*
 * A wrapper around JSON code for saving and retrieving parameters
 */
#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <json/json.h>
#include <fstream>
using std::ifstream;
using std::ios;
#include <sstream>
using std::stringstream;
#include <string>
using std::string;

namespace morph {

    /*!
     * A configuration file class to help read simulation parameters from a JSON file.
     *
     * This reads a config file which should be arranged as a JSON file. The format is fairly
     * free-form; getters and setters are used to access the parameters stored in the config file.
     *
     * This class also provides code for updating the JSON config and writing out the updated config
     * into the log directory to make a record of the parameters used to generate a set of
     * simulation data.
     */
    class Config {
    public:
        Config (const string& configfile) {
            stringstream ess;
            // Test for existence of the JSON file.
            ifstream jsonfile_test;
            int srtn = system ("pwd");
            if (srtn) {
                ess << "system call returned " << srtn;
                this->emsg = ess.str();
                this->ready = false;
                return;
            }
            jsonfile_test.open (configfile, ios::in);
            if (jsonfile_test.is_open()) {
                // Good, file exists.
                jsonfile_test.close();
            } else {
                ess << "json config file " << configfile << " not found.";
                this->emsg = ess.str();
                this->ready = false;
                return;
            }

            // Parse the JSON
            ifstream jsonfile (configfile, ifstream::binary);
            string errs;
            Json::CharReaderBuilder rbuilder;
            rbuilder["collectComments"] = false;
            bool parsingSuccessful = Json::parseFromStream (rbuilder, jsonfile, &this->root, &errs);
            if (!parsingSuccessful) {
                // report to the user the failure and their locations in the document.
                ess << "Failed to parse JSON: " << errs;
                this->emsg = ess.str();
                this->ready = false;
                return;
            }
            // JSON is open and parsed
            this->ready = true;
        }

        //! Write out the JSON to file.
        void write (const string& outfile) {
            ofstream configout;
            configout.open (outfile.c_str(), ios::out|ios::trunc);
            if (configout.is_open()) {
                configout << root;
                configout.close();
            } else {
                this->emsg = "Failed to open file '" + outfile + "' for writing";
                this->ready = false;
            }
        }

        //! Wrappers around gets
        //@{
        bool getBool (const string& thing, bool defaultval) {
            return this->root.get (thing, defaultval).asBool();
        }
        int getInt (const string& thing, int defaultval) {
            return this->root.get (thing, defaultval).asInt();
        }
        unsigned int getUInt (const string& thing, unsigned int defaultval) {
            return this->root.get (thing, defaultval).asUInt();
        }
        float getFloat (const string& thing, float defaultval) {
            return this->root.get (thing, defaultval).asFloat();
        }
        double getDouble (const string& thing, double defaultval) {
            return this->root.get (thing, defaultval).asDouble();
        }
        string getString (const string& thing, const string& defaultval) {
            return this->root.get (thing, defaultval).asString();
        }
        Json::Value getArray (const string& arrayname) {
            return this->root[arrayname];
        }
        //@}

        //! Setters
        //@{
        void set (const string& thing, bool value) {
            this->root[thing] = value;
        }
        void set (const string& thing, int value) {
            this->root[thing] = value;
        }
        void set (const string& thing, unsigned int value) {
            this->root[thing] = value;
        }
        void set (const string& thing, float value) {
            this->root[thing] = value;
        }
        void set (const string& thing, double value) {
            this->root[thing] = value;
        }
        void set (const string& thing, const string& value) {
            this->root[thing] = value;
        }
        //@}

        //! Set true when config object ready to be used
        bool ready = false;

        //! If !ready, error message is here.
        string emsg = "";

        // The root object which is set up in the constructor
        Json::Value root;
    };
} // namespace

#endif // _CONFIG_H_
