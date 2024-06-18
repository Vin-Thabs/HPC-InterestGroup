/* File: ip_main.cpp
 * Purpose: Perform simple intensity transform on a PPM image
 * Compile: g++ -g -Wall -fopenmp ip_main.cpp -o ip_main
 * Run: ./ip_main
 *
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <omp.h>
using namespace std;

struct img_info {string format; string width; string height; string RGB_max;};
struct attrib {int no_cols; int no_rows; int rgb_max;};

void par_convert (int *p, struct attrib im);
void seq_convert (int *p, struct attrib im);
void rgb_convert (string infile, string outfile1, string outfile2);

#include "convert.hpp"

int main(int argc, char *argv[]) {
    string im_in = "data/building.ppm";
    string im_out1 = "data/new_im_s.ppm", im_out2 = "data/new_im_p.ppm"; 

    rgb_convert(im_in, im_out1, im_out2); 

    return 0;
}

void rgb_convert (string infile, string outfile1, string outfile2) {
    
    // the number of times to be repeated 
    // in order to get more accurate timing
    int iter = 3;
    
    ifstream image;
    ofstream newimage_s, newimage_p;
    struct img_info info;
    struct attrib im_a;
    string red = "", green = "", blue = "";
    int r = 0, g = 0, b = 0; 
    int c; 
    int *p1, *p2, *img; 
    double start_time, elapsed_time_s, elapsed_time_p; 
    
    // Lines 45 - 107 read in basic information about the 
    // input image including the format and size etc., and 
    // the image pixel values in a 1D array
    
    // open ppm image files
    image.open(infile);
    newimage_s.open(outfile1);
    newimage_p.open(outfile2);
    
    // get header information
    image >> info.format;
    image >> info.width; 
    image >> info.height;
    image >> info.RGB_max;
        
    cout << info.format << " " << info.width << " " << info.height <<  " " << info.RGB_max << endl; 
    
    // copy header information over to the new image
    newimage_s << info.format << endl;
    newimage_s << info.width << " " << info.height << endl;
    newimage_s << info.RGB_max << endl;
    
    // copy header information over to the new image
    newimage_p << info.format << endl;
    newimage_p << info.width << " " << info.height << endl;
    newimage_p << info.RGB_max << endl;
        
    im_a.rgb_max = stoi(info.RGB_max);
    im_a.no_cols = stoi(info.width);
    im_a.no_rows = stoi(info.height);
    
    //cout << im_a.rgb_max << " " << im_a.no_cols << " " << im_a.no_rows << endl; 
    
    // creating memory space for output images, p1 for sequential processing
    // p2 for parallel processing 
    //im_in = new int[im_a.no_cols * im_a.no_rows * 3];
    p1 = new int[im_a.no_cols * im_a.no_rows * 3];
    p2 = new int[im_a.no_cols * im_a.no_rows * 3];
    img = new int[im_a.no_cols * im_a.no_rows * 3];
    
    if (p1 == nullptr || p2 == nullptr || img == nullptr)
        exit(EXIT_FAILURE);
    else
    {
        // read in the same image for both p1 and p2 in order to 
        // do in-place processing
        c = 0;
        for(int i = 0; i < im_a.no_rows; i++) {
            for(int j = 0; j < im_a.no_cols; j++){
                image >> red;
                image >> green;
                image >> blue;            
                stringstream redstream(red);
                stringstream greenstream(green); 
                stringstream bluestream(blue);
                redstream >> r;
                greenstream >> g;
                bluestream >> b;
                img[c] = r;
                c++;
                img[c] = g;
                c++;
                img[c] = b;
                c++;
            }
        }
    }
    
    // It is expected the serial run and parallel run to produce the same outputs as 
    // long as they are called for the same number of times
   
    // call the sequential function multiple times and take the average time
    elapsed_time_s = 0.0;
    for(int r = 0; r < iter; r++) {
        // read in the same image for both p1 in order to 
        // do in-place processing
        c = 0;
        for(int i = 0; i < im_a.no_rows; i++) {
            for(int j = 0; j < im_a.no_cols; j++) {
                p1[c] = img[c];
                c++;
                p1[c] = img[c];
                c++;
                p1[c] = img[c];
                c++;
            }
        } 
        start_time = omp_get_wtime();               
        seq_convert(p1, im_a);
        elapsed_time_s += (omp_get_wtime() - start_time); 
    }
    elapsed_time_s /= iter; 
    
    // call the parallel function multiple times and take the average time
    elapsed_time_p = 0.0;
    for(int r = 0; r < iter; r++){
        // read in the same image for both p2 in order to 
        // do in-place processing
        c = 0;
        for(int i = 0; i < im_a.no_rows; i++) {
            for(int j = 0; j < im_a.no_cols; j++) {
                p2[c] = img[c];
                c++;
                p2[c] = img[c];
                c++;
                p2[c] = img[c];
                c++;
            }
        } 
        start_time = omp_get_wtime();   
        par_convert(p2, im_a);
        elapsed_time_p += (omp_get_wtime() - start_time);
    }
    elapsed_time_p /= iter; 
    
    // print out the run times and the ratio 
    // (serial time)/(parallel time) as speedup
    cout << "Repeats the run " << iter << " times" << endl;
    cout << "Sequential elapsed time: \t" << elapsed_time_s << endl;     
    cout << "Parallel elapsed time: \t\t" << elapsed_time_p << endl; 
    cout << "The speedup is:\t\t\t" << elapsed_time_s/elapsed_time_p << endl;    
    
    // write the output image from the serial run to file 
    for(int i = 0; i < im_a.no_rows; i++) {
        for(int j = 0; j < im_a.no_cols; j++){
            int num = (i * im_a.no_cols + j)*3;
            for (int cnt = 0; cnt < 3; cnt ++) {
                newimage_s << p1[num+cnt] << " "; 
            }
            newimage_s << endl;
        }
    }
    // write the output image from the parallel run to file
    for(int i = 0; i < im_a.no_rows; i++) {
        for(int j = 0; j < im_a.no_cols; j++){
            int num = (i * im_a.no_cols + j)*3;
            for (int cnt = 0; cnt < 3; cnt ++) {
                newimage_p << p2[num+cnt] << " "; 
            }
            newimage_p << endl;
        }
    }
    // clean up
    image.close();
    newimage_s.close();
    newimage_p.close();
    delete[] (img);
    delete[] (p1);
    delete[] (p2);
}


     