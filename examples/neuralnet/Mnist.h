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
#include <string>
#include <map>
#include <vector>
#include <stdexcept>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <morph/vVector.h>

//! Mnist images are 28x28=784 pixels
constexpr size_t mnlen = 784;

//! To hold an image, with a flag recording whether or not it has been used.
struct MnistImage
{
};

//! A class to read, and then manage the data of, the Mnist database.
struct Mnist
{
    Mnist() {
        this->init();
    }

    Mnist (const std::string& path) {
        this->basepath = path;
        this->init();
    }

    void init() {
        // Read data. From two files, in sequence
        this->loadData ("train", this->training, this->training_f);
        this->loadData ("t10k", this->test, this->test_f);
    }

    void loadData (const std::string& tag,
                   std::multimap<unsigned char, cv::Mat>& theMats,
                   std::multimap<unsigned char, morph::vVector<float>>& vecFloats) {

        // Training data
        std::ifstream img_f;
        std::ifstream lbl_f;

        std::string img_p = basepath + tag + "-images-idx3-ubyte";
        std::string lbl_p = basepath + tag + "-labels-idx1-ubyte";
        img_f.open (img_p.c_str(), std::ios::in | std::ios::binary);
        lbl_f.open (lbl_p.c_str(), std::ios::in | std::ios::binary);

        if (!img_f.is_open() || !lbl_f.is_open()) {
            throw std::runtime_error ("Mnist: File access error opening MNIST data files");
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
            // Read one label...
            lbl_f.read (cbuf, 1);
            unsigned char lbl = static_cast<unsigned char>(cbuf[0]);
            //std::cout << "Label is " << (unsigned int)lbl << std::endl;

            // ...and 28x28 pixels, and stick this into the training multimap.
            cv::Mat oneimg(this->nr, this->nc, CV_32F, cv::Scalar(0));
            //cv::Mat oneimg(nr, nc, CV_8UC1, cv::Scalar(0));
            for (int r = 0; r < this->nr; ++r) {
                for (int c = 0; c < this->nc; ++c) {
                    // Read pixel and stick it in Mat
                    img_f.read (cbuf, 1);
                    //oneimg.at<float>(r, c, 0) = static_cast<float>(cbuf[0])/255.0f;
                    unsigned char uc = cbuf[0];
                    float val = (float)uc/256.0f;
                    //std::cout << "Value: " << val << std::endl;
                    oneimg.at<float>(r, c) = val;
                    //std::cout << "Value in omeimg: " << oneimg.at<float>(r, c)  << std::endl;
                }
            }
            cv::Mat tmp(this->nr, this->nc, CV_32F, cv::Scalar(0)); // create new empty mat
            auto ii = theMats.insert ({lbl, tmp}); // create a new mat header in the list
            oneimg.copyTo (ii->second);

            morph::vVector<float> ar(nr*nc);
            size_t i = 0;
            for (int r = 0; r < this->nr; ++r) {
                for (int c = 0; c < this->nc; ++c) {
                    ar[i++] = oneimg.at<float>(r, c);
                }
            }
            vecFloats.insert ({lbl, ar});
        }
    }

    // Show all the training images
    void showall() {
        const std::string winName = "Mnist";
        namedWindow (winName, cv::WINDOW_AUTOSIZE);
        for (auto im : this->training) {
            cv::Mat m (this->nr, this->nc, CV_32F, cv::Scalar(0));

            im.second.copyTo (m);
#if 0
            // How to get a copy of array into a vector<float>
            std::vector<float> ar(this->nr*this->nc, 0.0f);
            size_t i = 0;
            for (int r = 0; r < this->nr; ++r) {
                for (int c = 0; c < this->nc; ++c) {
                    ar[i++] = m.at<float>(r, c);
                }
            }
#endif
#if 0
            if (m.isContinuous()) {
                // Don't like this:
                ar.assign (m.data, m.data + m.total());
                for (auto ari : ar) {
                    std::cout << ari << ",";
                }
                std::cout << std::endl << std::endl;
            } else {
                throw std::runtime_error ("non continuous cv::Mat");
            }
#endif
            cv::imshow (winName, m);
            std::cout << "Label: " << (unsigned int)im.first << std::endl;
            cv::waitKey(0);
        }
    }

    size_t num_training() {
        return this->training.size();
    }

    int chars_to_int (const char* buf) {
        int rtn = (buf[3]&0xff) | (buf[2]&0xff)<<8 | (buf[1]&0xff)<<16 | (buf[0]&0xff)<<24;
        return rtn;
    }

    // Number of rows and cols. These will be 28 and 28.
    int nr = 0;
    int nc = 0;

    std::string basepath = "mnist/";

    //! The training data. The key to this multimap is the label, the Mat (or vVector)
    //! contains each training image. This is to be 50000 out of 60000 examples.
    std::multimap<unsigned char, cv::Mat> training;
    // Same data extracted into vectors of floats, rather than Mats.
    std::multimap<unsigned char, morph::vVector<float>> training_f;

    //! Validation - 10000 out of the 60000
    //std::multimap<unsigned char, cv::Mat> validation;
    //std::multimap<unsigned char, morph::Vector<float, mnlen>> validation_f;

    //! The training data. The key to this multimap is the label; the Mat (or vVector)
    //! contains each test image
    std::multimap<unsigned char, cv::Mat> test;
    std::multimap<unsigned char, morph::vVector<float>> test_f;
};
