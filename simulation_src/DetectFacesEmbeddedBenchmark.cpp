/*
By downloading, copying, installing or using the software you agree to this license.
If you do not agree to this license, do not download, install,
copy or use the software.


                  License Agreement For libfacedetection
                     (3-clause BSD License)

Copyright (c) 2018-2020, Shiqi Yu, all rights reserved.
shiqi.yu@gmail.com

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.

  * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

  * Neither the names of the copyright holders nor the names of the contributors
    may be used to endorse or promote products derived from this software
    without specific prior written permission.

This software is provided by the copyright holders and contributors "as is" and
any express or implied warranties, including, but not limited to, the implied
warranties of merchantability and fitness for a particular purpose are disclaimed.
In no event shall copyright holders or contributors be liable for any direct,
indirect, incidental, special, exemplary, or consequential damages
(including, but not limited to, procurement of substitute goods or services;
loss of use, data, or profits; or business interruption) however caused
and on any theory of liability, whether in contract, strict liability,
or tort (including negligence or otherwise) arising in any way out of
the use of this software, even if advised of the possibility of such damage.
*/

#include <stdio.h>
#include <opencv2/opencv.hpp>
#include "facedetectcnn.h"
#include <fstream>

#define DETECT_BUFFER_SIZE 0x9000

using namespace std;
using namespace cv;

int main(int argc, char* argv[]){
    
    // Load list of images
    ifstream list_of_images("file_list.txt");
    if(!list_of_images){
        fprintf(stderr, "Can not load the list of images from file %s.\n", argv[1]);
        return -1; 
    }

    // String to store the name of the image
    string image_name;

    // Counter for the number of images analyzed
    int counter = 0;

    // For each image in the list
    while(list_of_images >> image_name){

        // Load an image and convert it to gray (single-channel)
        Mat image = imread("images/" + image_name); 

        if(image.empty()){
            fprintf(stderr, "Can not load the image file %s.\n", argv[1]);
            return -1;
        }

        int * pResults = NULL; 

        // Buffer used in the detection function
        unsigned char * pBuffer = (unsigned char *)malloc(DETECT_BUFFER_SIZE);

        if(!pBuffer){
            fprintf(stderr, "Can not alloc buffer.\n");
            return -1;
        }

        // Detect faces
        pResults = facedetect_cnn(pBuffer, (unsigned char*)(image.ptr(0)), image.cols, image.rows, (int)image.step);
    
        // Clone image to modify it
        Mat result_image = image.clone();

        //print the detection results
        for(int i = 0; i < (pResults ? *pResults : 0); i++){
            short * p = ((short*)(pResults + 1)) + 16*i;

            int confidence = p[0];
            int x = p[1];
            int y = p[2];
            int w = p[3];
            int h = p[4];
            
            //show the score of the face. Its range is [0-100]
            char sScore[256];
            snprintf(sScore, 256, "%d", confidence);
            cv::putText(result_image, sScore, cv::Point(x, y-3), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 0), 1);

            //draw face rectangle
            rectangle(result_image, Rect(x, y, w, h), Scalar(0, 255, 0), 2);

            //draw five face landmarks in different colors
            cv::circle(result_image, cv::Point(p[5], p[5 + 1]), 1, cv::Scalar(255, 0, 0), 2);
            cv::circle(result_image, cv::Point(p[5 + 2], p[5 + 3]), 1, cv::Scalar(0, 0, 255), 2);
            cv::circle(result_image, cv::Point(p[5 + 4], p[5 + 5]), 1, cv::Scalar(0, 255, 0), 2);
            cv::circle(result_image, cv::Point(p[5 + 6], p[5 + 7]), 1, cv::Scalar(255, 0, 255), 2);
            cv::circle(result_image, cv::Point(p[5 + 8], p[5 + 9]), 1, cv::Scalar(0, 255, 255), 2);
            
        }

        // Write image in results/ folder
        image_name = "results/" + image_name;
        imwrite(image_name, result_image);

        // Update counter and possibly write the result
        counter++;
        if(counter % 100 == 0) cout << "Number of images analyzed: " << counter << std::endl;

        //release the buffer
        free(pBuffer);
    }

    return 0;
}
