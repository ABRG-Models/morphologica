/*
 * A small class to allocate space for some text and to then read it
 * from a file. Originally written for SpineML_PreFlight.
 *
 * Author: Seb James
 * Date: Oct 2014
 */

#pragma once

#include <string>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <stdlib.h>
#include <string.h>
#include "rapidxml.hpp"

namespace morph
{
    /*!
     * Allocate storage and read in the data from the file at
     * filepath.
     */
    class AllocAndRead {
    public:
        /*!
         * Construct an empty AllocAndRead object.
         */
        AllocAndRead ()
            : filepath ("")
            , data_((char*)0)
        {
        }
        /*!
         * Construct an AllocAndRead object and read the content of
         * the file at @param path
         */
        AllocAndRead (const std::string& path)
            : filepath (path)
            , data_((char*)0)
        {
            this->read();
        }
        /*!
         * Destructor needs to free up @see data_.
         */
        ~AllocAndRead ()
        {
            if (this->data_) {
                free (this->data_);
            }
        }

        /*!
         * A copy constructor - we have to make a copy of @see data_
         */
        AllocAndRead (const AllocAndRead& other)
        {
            this->filepath = other.filepath;
            this->sz = other.getsize();
            this->data_ = static_cast<char*>(calloc (this->sz, sizeof(char)));
            // Now copy contents of others' data
            size_t i = 0;
            while (i < this->sz) {
                this->data_[i] = other.datachar(i);
                ++i;
            }
            // no need to null-terminate as we used calloc.
        }

        /*!
         * Obtain an indexed character from @see data_.
         * @param i index into @see data_.
         * @return the character at data_[i].
         */
        char datachar (size_t i) const
        {
            char c = this->data_[i];
            return c;
        }

        /*!
         * Get a pointer to @see data_.
         * @return @see data_
         */
        char* data (void)
        {
            return this->data_;
        }

        /*!
         * Get the size of @see data_. This is stored in the member
         * @see sz.
         * @return @see sz.
         */
        size_t getsize (void) const
        {
            return this->sz;
        }

        /*!
         * Reads the file at @param path into @see data_. Allocates
         * memory as required.
         */
        void read (const std::string& path)
        {
            this->filepath = path;
            this->read();
        }

    private:
        /*!
         * Read the file, allocating memory as required.
         */
        void read (void)
        {
            std::ifstream f;
            f.open (this->filepath.c_str(), std::ios::in);
            if (!f.is_open()) {
                std::stringstream ee;
                ee << "AllocAndRead: Failed to open file " << this->filepath << " for reading";
                throw std::runtime_error (ee.str());
            }

            // Work out how much memory to allocate - seek to the end
            // of the file and find its size.
            f.seekg (0, std::ios::end);
            this->sz = f.tellg();
            f.seekg (0);
            this->data_ = static_cast<char*>(calloc (++this->sz, sizeof(char))); // ++ for trailing null

            char* textpos = this->data_;
            std::string line("");
            size_t llen = 0;   // line length (chars)
            size_t curpos = 0;
            while (getline (f, line)) {
                // Restore the newline
                line += "\n";
                // Get line length
                llen = line.size();
                // Restore textpos pointer in the reallocated memory:
                textpos = this->data_ + curpos;
                // copy line to textpos:
                strncpy (textpos, line.c_str(), llen);
                // Update current character position
                curpos += llen;
            }
            f.close();
            // Note: text is already null terminated as we used calloc.
        }

        //! The path from which to read data.
        std::string filepath;

        //! The character data.
        char* data_;

        //! The size in bytes of the character data @data_
        size_t sz;
    };

} // namespace morph
