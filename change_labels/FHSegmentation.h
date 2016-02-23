#ifndef FH_SEGMENTATION
#define FH_SEGMENTATION

#include <vector>
#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <math.h>

#define WIDTH 4.0
#define THRESHOLD(size, c) (c/size)
template <class T>
inline T square(const T &x) { return x*x; };

class Edge {
public:
    float w;
    int a, b;
    bool valid;
    Edge() : w(0), a(0), b(0), valid(false) { };

    bool operator< (const Edge &other) const {
        return (w < other.w);
    }
};

typedef struct
{
    int rank;
    int p;
    int size;
} uni_elt;

class Universe
{
public:
    Universe() : num(0) { }
    Universe(int elements)
    {
        num = elements;
        elts.resize(num);
        std::vector<uni_elt>::iterator p = elts.begin();
        int i = 0;
        while (p != elts.end()) {
            p->rank = 0;
            p->size = 1;
            p->p = i;
            p++;
            i++;
        }
    }
    ~Universe(){};
    int find(int x)
    {
        int y = x;
        while (y != elts[y].p)
            y = elts[y].p;
        elts[x].p = y;
        return y;
    };
    void join(int x, int y)
    {
        if (elts[x].rank > elts[y].rank)
        {
            elts[y].p = x;
            elts[x].size += elts[y].size;
        }
        else
        {
            elts[x].p = y;
            elts[y].size += elts[x].size;
            if (elts[x].rank == elts[y].rank)
                elts[y].rank++;
        }
        num--;
    }
    void release() {
        elts.clear();
    }
    int size(int x) const { return elts[x].size; }
    int num_sets() const { return num; }
    //should be private but I need to access some things
    std::vector<uni_elt>elts;
    int num;
};

int segmentation(const cv::Mat &in, cv::Mat &out, cv::Mat &out_color, const float sigma = 0.5f, const int k = 200, const int size = 200, const int segStartNumber = 0);

#endif //FH_SEGMENTATION
