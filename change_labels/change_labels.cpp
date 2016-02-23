#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <fstream>
#include "FHSegmentation.h"

using namespace std;
using namespace cv;

void random_rgb2(cv::Vec3b &c)
{
    c[0] = rand() % 255 + 1;
    c[1] = rand() % 255 + 1;
    c[2] = rand() % 255 + 1;
}

int global_labels(int start, string video, int num) {
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
    int ann_num = 2;
    if(start % 3 == 0)
        ann_num = 0;
    else if((start + 22) % 3 == 0)
        ann_num = 1;
    //printf("ann_num is: %d\n",ann_num);


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

int vote_labels(string video, int start, int end) {
    //For every frame, lets grab the different versions and do per-pixel voting
    for(int i = start; i <= end; i++) {
        char fileName[500];
        sprintf(fileName, "/home/steve/cs7616/annotations/%s/%d_0.png",video.c_str(),i);
        Mat img0 = imread(fileName,0);
        sprintf(fileName, "/home/steve/cs7616/annotations/%s/%d_1.png",video.c_str(),i);
        Mat img1 = imread(fileName,0);
        sprintf(fileName, "/home/steve/cs7616/annotations/%s/%d_2.png",video.c_str(),i);
        //printf("Filename: %s\n",fileName);
        Mat img2 = imread(fileName,0);

        bool use[3] = { !img0.empty(), !img1.empty(), !img2.empty()};
        //printf("Use: %d, %d, %d\n", (int)use[0], (int)use[1], (int)use[2]);
        Mat_<uchar>::iterator p0, p1, p2, pF;
        int rows, cols;
        if(use[0]) {
            rows = img0.rows;
            cols = img0.cols;
        } else if(use[1]) {
            rows = img1.rows;
            cols = img1.cols;
        } else if(use[2]) {
            rows = img2.rows;
            cols = img2.cols;
        } else {
            printf("ERROR no images\n");
            return 1;
        }
        if(use[0])
            p0 = img0.begin<uchar>(); 
        if(use[1])
            p1 = img1.begin<uchar>(); 
        if(use[2])
            p2 = img2.begin<uchar>(); 
        Mat out = Mat(rows,cols,CV_8UC1);
        pF = out.begin<uchar>();
        while(pF != out.end<uchar>()) {
            vector<int> counts;
            counts.resize(20);
            if(use[0])
                counts[*p0++]++;
            if(use[1])
                counts[*p1++]++;
            if(use[2])
                counts[*p2++]++;
            //One problem with this is the 2 person case. 
            int result = distance(counts.begin(),max_element(counts.begin(),counts.end())); 
            *pF++ = result;
        }
        char outFile[500];
        sprintf(outFile, "/home/steve/cs7616/annotations/final/%s/%d.png",video.c_str(),i);
        imwrite(outFile,out);
    }
}

/*void random_rgb(cv::Vec3b &c)
{
    c[0] = rand() % 255 + 1;
    c[1] = rand() % 255 + 1;
    c[2] = rand() % 255 + 1;
}*/

int visualize_labels(string video, int start, int end) {
    int numSegs = 20;
    cv::Vec3b *m_colors = (cv::Vec3b *)malloc(numSegs*sizeof(cv::Vec3b));
    cv::Vec3b *pColor = m_colors;
    *pColor++ = Vec3b(0,0,0);
    for (int i = 1; i < numSegs; i++)
    {
        cv::Vec3b color;
        random_rgb2(color);
        *pColor++ = color;
    }

    for(int i = start; i <= end; i++) {
        char fileName[500];
        sprintf(fileName, "/home/steve/cs7616/annotations/final/%s/%d.png",video.c_str(),i);
        Mat img = imread(fileName,0);

        Mat out = Mat(img.rows,img.cols,CV_8UC3);
        Mat_<Vec3b>::iterator pF = out.begin<Vec3b>();
        Mat_<uchar>::iterator pI = img.begin<uchar>();
        while(pF != out.end<Vec3b>()) {
            *pF = m_colors[int(*pI)];
            ++pF; ++pI;
        }

        char outFile[500];
        sprintf(outFile, "/home/steve/cs7616/annotations/visualized/%s/%d.png",video.c_str(),i);
        imwrite(outFile,out);
    }

    free(m_colors);
}

class Features {
public:
    int size, imgNum, segNum, label;
    Rect bbox;
    Point centroid;
    Mat b_hist, g_hist, r_hist;
    Features() : size(0), imgNum(0), segNum(0), centroid(-1,-1) { }
    ~Features() { }
};

int create_features(string video, int start, int end) {
    ofstream fp;
    fp.open("/home/steve/cs7616/features/features.csv");
    for(int i = start; i <= end; i++) {
        char fileName[500];
        sprintf(fileName, "/home/steve/cs7616/videos/%s/%d.png",video.c_str(),i);
        Mat img = imread(fileName,0);
        Mat seg, segDisp;
        int numSegs = segmentation(img, seg, segDisp, 1.5f, 400, 400, 0);
        //printf("Numsegs: %d\n", numSegs);
        //imshow("window",segDisp);
        //waitKey(0);
        // Go through each segment and generate features
        vector<Features> feats;
        vector<Mat> segmentMasks;
        feats.resize(numSegs);
        segmentMasks.resize(numSegs);
        vector<Moments> mu = moments( seg, false ); }
        Mat_<uchar>::iterator pS = seg.begin<uchar>();
        for(int y = 0; y < seg.rows; y++) {
            for(int x = 0; x < seg.cols; x++) {
                // Let's start by getting the centroid, size, and bounding box
                ++pS;
            }
        }
    }
}

int main(int argc, char* argv[]) {
    if(argc != 5) {
        printf("ERROR, wrong number of arguments\n");
        return 1;
    }
    int method = atoi(argv[1]);
    if(method == 0) {
        int start = atoi(argv[2]);
        string video = string(argv[3]);
        int num = atoi(argv[4]);
        return global_labels(start,video,num);
    } else if(method == 1) {
        int start = atoi(argv[2]);
        int end = atoi(argv[3]);
        string video = string(argv[4]);
        return vote_labels(video,start,end);
    } else if(method == 2) {
        int start = atoi(argv[2]);
        int end = atoi(argv[3]);
        string video = string(argv[4]);
        return visualize_labels(video,start,end);
    } else if(method == 3) {
        int start = atoi(argv[2]);
        int end = atoi(argv[3]);
        string video = string(argv[4]);
        return create_features(video,start,end);
    }
    return 0;
}
