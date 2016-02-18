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
    
    return 0;
}
