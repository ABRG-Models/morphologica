/*!
 * \file
 *
 * \brief A class for loading the MNIST handwritten numerals
 *
 * Mnist allows you to load in the Mnist database of handwritten numerals.
 *
 * \author Seb James
 * \date May 2020
 */
#pragma once

// TRAINING SET IMAGE FILE (train-images-idx3-ubyte):
// Format:
// [offset] [type]          [value]          [description]
// 0000     32 bit integer  0x00000803(2051) magic number
// 0004     32 bit integer  60000            number of images
// 0008     32 bit integer  28               number of rows
// 0012     32 bit integer  28               number of columns
// 0016     unsigned byte   ??               pixel
// 0017     unsigned byte   ??               pixel
// ........
// xxxx     unsigned byte   ??               pixel

// TRAINING SET LABEL FILE (train-labels-idx1-ubyte):
// [offset] [type]          [value]          [description]
// 0000     32 bit integer  0x00000801(2049) magic number (MSB first)
// 0004     32 bit integer  60000            number of items
// 0008     unsigned byte   ??               label
// 0009     unsigned byte   ??               label
// ........
// xxxx     unsigned byte   ??               label
// The labels values are 0 to 9.

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <utility>
#include <tuple>
#include <stdexcept>
#include <morph/vvec.h>

namespace morph {

    //! Mnist images are 28x28=784 pixels
    constexpr int mnlen = 784;

    enum class fixlabels { yes, no };

    //! A class to read, and then manage the data of, the Mnist database.
    struct Mnist
    {
        Mnist() { this->init(); }

        Mnist (const std::string& path, morph::fixlabels fl = fixlabels::no)
        {
            this->apply_label_cleaning = fl;
            this->basepath = path;
            this->init();
        }

        void init()
        {
            // Read data. From two files, in sequence
            this->load_data ("train", this->training_f);
            this->loading_test = true;
            this->load_data ("t10k", this->test_f);
            this->loading_test = false;
        }

        void load_data (const std::string& tag,
                        std::multimap<unsigned char, std::pair<int, morph::vvec<float>>>& vecFloats)
        {
            // Ifstreams for images and labels.
            std::ifstream img_f;
            std::ifstream lbl_f;

            std::string img_p = basepath + tag + "-images-idx3-ubyte";
            std::string lbl_p = basepath + tag + "-labels-idx1-ubyte";
            img_f.open (img_p.c_str(), std::ios::in | std::ios::binary);
            lbl_f.open (lbl_p.c_str(), std::ios::in | std::ios::binary);

            if (!img_f.is_open() || !lbl_f.is_open()) {
                std::stringstream ee;
                ee << "Mnist: File access error opening MNIST data files: "
                   << img_p << " (images) and " << lbl_p << " (labels)";
                throw std::runtime_error (ee.str());
            }

            // Process training data magic number, stuff
            char buf[4];
            img_f.read (buf, 4);
            int magic_imgs = chars_to_int (buf);
            img_f.read (buf, 4);
            int n_imgs = (buf[3]&0xff) | (buf[2]&0xff)<<8 | (buf[1]&0xff)<<16 | (buf[0]&0xff)<<24;
            img_f.read (buf, 4);
            this->nr = (buf[3]&0xff) | (buf[2]&0xff)<<8 | (buf[1]&0xff)<<16 | (buf[0]&0xff)<<24;
            img_f.read (buf, 4);
            this->nc = (buf[3]&0xff) | (buf[2]&0xff)<<8 | (buf[1]&0xff)<<16 | (buf[0]&0xff)<<24;

            if (nr * nc != mnlen) { throw std::runtime_error ("Mnist: Expecting 28x28 images in Mnist!"); }

            // Check images magic number
            if (magic_imgs != 2051) { throw std::runtime_error ("Mnist: data, images magic number is wrong"); }

            // Process labels magic number etc
            lbl_f.read (buf, 4);
            int magic_lbls = (buf[3]&0xff) | (buf[2]&0xff)<<8 | (buf[1]&0xff)<<16 | (buf[0]&0xff)<<24;
            lbl_f.read (buf, 4);
            int n_lbls = (buf[3]&0xff) | (buf[2]&0xff)<<8 | (buf[1]&0xff)<<16 | (buf[0]&0xff)<<24;

            // Check labels magic number
            if (magic_lbls != 2049) { throw std::runtime_error ("Mnist: data, labels magic number is wrong"); }

            // Check reported number of images == number of labels
            if (n_lbls != n_imgs) {
                throw std::runtime_error ("Mnist: Training data, num labels != num images");
            }

            // Should now be able to read through each file, pulling in the data.
            char cbuf[1];
            for (int inum = 0; inum < n_imgs; ++inum) {
                morph::vvec<float> ar(nr*nc, 0.0f);
                // Read one label...
                lbl_f.read (cbuf, 1);
                unsigned char lbl = static_cast<unsigned char>(cbuf[0]);
                for (int r = 0; r < this->nr; ++r) {
                    for (int c = 0; c < this->nc; ++c) {
                        // Read pixel and stick it in the array
                        img_f.read (cbuf, 1);
                        unsigned char uc = static_cast<unsigned char>(cbuf[0]);
                        float numf = (float)uc/256.0f;
                        // Fill array as cartgrids are displayed: bottom row first.
                        ar[(this->nr-r-1)*28+c] = numf;
                    }
                }

                if (this->apply_label_cleaning == morph::fixlabels::yes && this->loading_test == true) {
                    // If inum in bad set, then fix lbl here (or ignore example)
                    try {
                        auto badlab = badlabels_test.at (inum);
                        if (badlab[0] != lbl) {
                            std::cout << "BAD: label for ID" << inum << " is expected to be "
                                      << (int)badlab[0] << ", not "  << (int)lbl << std::endl;
                        } else {
                            if (badlab[1] == 255) {
                                // Omit
                                std::cout << "Omit ambiguous example ID " << inum << std::endl;
                            } else {
                                // Insert with fixed label
                                std::cout << "Fixed label for example ID " << inum << "( from "
                                          << static_cast<int>(badlab[0]) << " to )" << static_cast<int>(badlab[1]) << std::endl;
                                vecFloats.insert ({badlab[1], std::make_pair(inum, ar)});
                            }
                        }
                    } catch (std::out_of_range& e) {
                        // inum is not a bad label example, so add
                        vecFloats.insert ({lbl, std::make_pair(inum, ar)});
                    }
                } else {
                    // In this case we're not label cleaning.
                    vecFloats.insert ({lbl, std::make_pair(inum, ar)});
                }
            }
        }

        //! Get the number of training examples
        size_t num_training() { return this->training_f.size(); }
        //! Number of test examples
        size_t num_test() { return this->test_f.size(); }

        //! Extract an integer from a character array in \a buf by 're-joining' the 4 bytes together
        int chars_to_int (const char* buf)
        {
            int rtn = (buf[3]&0xff) | (buf[2]&0xff)<<8 | (buf[1]&0xff)<<16 | (buf[0]&0xff)<<24;
            return rtn;
        }

        // Select one of each numeral from training_f and return in a multimap (or fewer, if you set num to say, 1)
        std::multimap<unsigned char, std::pair<int, morph::vvec<float>> >
        training_oneshot (unsigned int num = 10, unsigned int firstnum = 0)
        {
            std::multimap<unsigned char, std::pair<int, morph::vvec<float>> > rtn;
            for (unsigned char numeral = firstnum; numeral < num+firstnum; ++numeral) {
                unsigned char modnum = numeral%10;
                auto range = this->training_f.equal_range (modnum);
                // select one from range
                size_t rsz = 0;
                for (auto i = range.first; i != range.second; ++i, ++rsz) {}
                //std::cout << "range contains " << rsz << " elements\n";
                // Choose random number between 0 and rsz. Select that one.
                morph::RandUniform<size_t> rng(0, rsz);
                size_t tgt = rng.get();
                auto i = range.first;
                size_t j = 0;
                for (j = 0; j<tgt; ++i, ++j) {}
                //std::cout << "j = " << j << " tgt is " << tgt << std::endl;
                rtn.insert (*i);
            }
            return rtn;
        }

        // Select num examples of numeral chosen_numeral from training_f and return in a multimap
        std::multimap<unsigned char, std::pair<int, morph::vvec<float>> >
        debug_oneshot (unsigned int num = 10, unsigned int chosen_numeral = 0)
        {
            std::multimap<unsigned char, std::pair<int, morph::vvec<float>> > rtn;
            for (unsigned char numeral = 0; numeral < num; ++numeral) {
                unsigned char modnum = chosen_numeral%10;
                auto range = this->training_f.equal_range (modnum);
                // select one from range
                size_t rsz = 0;
                for (auto i = range.first; i != range.second; ++i, ++rsz) {}
                // Choose random number between 0 and rsz. Select that one.
                morph::RandUniform<size_t> rng(0, rsz);
                size_t tgt = rng.get();
                auto i = range.first;
                size_t j = 0;
                for (j = 0; j<tgt; ++i, ++j) {}
                rtn.insert (*i);
            }
            return rtn;
        }

        // Get the training example with ID _id out of training_f
        std::tuple<int, unsigned char, morph::vvec<float>> training_example (int _id)
        {
            for (auto ex : this->training_f) {
                if (ex.second.first == _id) {
                    return std::make_tuple (ex.second.first, ex.first, ex.second.second);
                }
            }
            morph::vvec<float> empty (mnlen, 0.0f);
            return std::make_tuple (_id, 255, empty);
        }

        // Get the training example with ID _id out of test_f
        std::tuple<int, unsigned char, morph::vvec<float>> test_example (int _id)
        {
            for (auto ex : this->test_f) {
                if (ex.second.first == _id) {
                    return std::make_tuple (ex.second.first, ex.first, ex.second.second);
                }
            }
            morph::vvec<float> empty (mnlen, 0.0f);
            return std::make_tuple (_id, 255, empty);
        }

        // Number of rows in the numeral image. Always 28. Set (and tested) during load.
        int nr = 0;
        // Number of cols in the image. Again, 28.
        int nc = 0;

        //! The basepath for finding the files that contain the numeral image data
        std::string basepath = "mnist/";

        //! Whether or not to fix labels thought to be bad (or omit those examples). See https://labelerrors.com/
        morph::fixlabels apply_label_cleaning = morph::fixlabels::no;

        //! Set true when we're loading test data, rather than training data.
        bool loading_test = false;

        //! Map of MNIST example index number and a pair of numbers that are 'bad
        //! label', 'good label'. If good label is 255, then that means ambiguous. I
        //! don't have an equivalent list for the MNIST training data.
        std::map<int, morph::vec<unsigned char,2>> badlabels_test = { // This list relates to test data only.
            {947,  {8,9}   },
            {6651, {0,6}   },
            {2597, {5,3}   },
            {2462, {2,255} },   // Cleanlab guessed 0, MTurk consensus 0. I think it's fully ambiguous.
            {3558, {5,0}   },
            {9729, {5,6}   },
            {3520, {6,255} },   // Cleanlab guess 4, but I think it looks like a 6 but a really bad 6, so will remove it.
            {1901, {9,255} },   // Could be 4 or 9 in my opinion.
            {2654, {6,255} },
            {1621, {0,255} },   // Cleanlab and MTurk think 6. I think 0 is plausible, but let's omit this one.
            {6783, {1,255} },   // Cleanlab guessed 6, but I think 1 is plausible.
            {5937, {5,3}   },   // Cleanlab & MTurk think 3, I agree.
            {9679, {6,255} } }; // Highly ambiguous

        //! The training data. The key to this multimap is the label, the value contains
        //! as its first element, the ID of the image (sequential order of appearance in
        //! the data file, counting from 0) and as its second element a vvec of the
        //! training image data. This is 60000 examples.
        std::multimap<unsigned char, std::pair<int, morph::vvec<float>> > training_f;

        //! The test data. 10000 examples. Key/Value same as in training_f.
        std::multimap<unsigned char, std::pair<int, morph::vvec<float>> > test_f;
    };

} // namespace morph
