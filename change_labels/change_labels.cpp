#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <fstream>

using namespace std;
using namespace cv;

int main(int argc, char* argv[]) {
    //First lets read the map.txt
    ifstream fp("/home/steve/cs7616/annotations/map.txt");
    if(fp == NULL) {
        printf("ERROR reading map.txt\n");
        return 1;
    }
    string line;
    string dl = ":";
    map<int,int> colors;
    while(getline(fp, line)) {
        size_t pos = line.find(dl);
        if(pos == string::npos) {
            printf("ERROR parsing map.txt\n");
            return 1;
        }
        int key = atoi(line.substr(0,pos).c_str());
        int val = atoi(line.substr(pos + 1, line.length()).c_str());
        colors[key] = val;
        //printf("Key: %d, val: %d\n",key,val);
    }

    //Now we need the start num, the video folder, and the number of images
    if(argc != 4) {
        printf("ERROR, wrong number of arguments\n");
        return 1;
    }
    int start = atoi(argv[1]);
    string video = string(argv[2]);
    int num = atoi(argv[3]);
    int ann_num = 2;
    if(start % 3 == 0)
        ann_num = 0;
    else if((start + 22) % 3 == 0)
        ann_num = 1;


    //Now we go through all of the images and update everything
    for(int i = 0; i < num; i++) {
        char fileName[500];
        sprintf(fileName, "/home/steve/cs7616/annotations/rendered/frame%05d.png",i);
        //printf("Filename: %s\n",fileName);
        Mat img = imread(fileName);
        if(img.empty()) {
            printf("ERROR opening image\n");
            return 1;
        }
        Mat m = Mat::zeros(img.rows,img.cols, CV_8UC1);
        //imshow("window",img);
        //waitKey(33);
        Mat_<Vec3b>::const_iterator pI = img.begin<Vec3b>();
        Mat_<uchar>::iterator pO = m.begin<uchar>();
        while(pI != img.end<Vec3b>()) {
            int id = int((*pI)[0]) * 255 * 255 + int((*pI)[1]) * 255 + int((*pI)[2]);
            *pO = colors[id];
            ++pI;
            ++pO;
        }
        //imshow("window",m);
        //waitKey(33);
        //Now save the images to the proper place
        int frame_num = start * 30 + i;
        char outFile[500];
        sprintf(outFile, "/home/steve/cs7616/annotations/%s/%d_%d.png",video.c_str(),frame_num,ann_num);
        imwrite(outFile,m);
    }
    return 0;
}
