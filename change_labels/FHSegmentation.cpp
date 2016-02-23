#include "FHSegmentation.h"

/* make filters */
#define MAKE_FILTER(name, fun)                                \
    std::vector<float> make_ ## name(float sigma)       \
{                                                           \
    sigma = std::max(sigma, 0.01F);                                     \
    int len = (int)std::ceil(sigma * WIDTH) + 1;                     \
    std::vector<float> mask(len);                               \
for (int i = 0; i < len; i++)                               \
{                                                           \
    mask[i] = fun;                                              \
}                                                           \
    return mask;                                                \
}

MAKE_FILTER(fgauss, (float)expf(-0.5*square(i / sigma)));

void normalize(std::vector<float> &mask)
{
    int len = mask.size();
    float sum = 0;
    int i;
    for (i = 1; i < len; i++)
    {
        sum += std::fabs(mask[i]);
    }
    sum = 2 * sum + std::fabs(mask[0]);
    for (i = 0; i < len; i++)
    {
        mask[i] /= sum;
    }
}

/* convolve src with mask.  dst is flipped! */
void convolve_even(const cv::Mat& src, cv::Mat &dst, std::vector<float> &mask)
{
    int width = src.cols;
    int height = src.rows;
    int len = mask.size();
    dst = cv::Mat(src.rows, src.cols, src.type());
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            //cout << x << ", " << y << endl;
            float sum = mask[0] * src.at<float>(y, x);
            for (int i = 1; i < len; i++) {
                sum += mask[i] * (src.at<float>(y, std::max(x - i, 0)) + src.at<float>(y, std::min(x + i, width - 1)));
            }
            dst.at<float>(y, x) = sum;
        }
    }
}

void random_rgb(cv::Vec3b &c)
{
    c[0] = rand() % 255 + 1;
    c[1] = rand() % 255 + 1;
    c[2] = rand() % 255 + 1;
}

void iExtractRGBColorSpace(const cv::Mat& in, cv::Mat &B, cv::Mat &G, cv::Mat &R) {
  B = cv::Mat(in.rows, in.cols, CV_32F);
  G = cv::Mat(in.rows, in.cols, CV_32F);
  R = cv::Mat(in.rows, in.cols, CV_32F);
  cv::Mat_<cv::Vec3b>::const_iterator pI = in.begin<cv::Vec3b>();
  cv::Mat_<float>::iterator pB = B.begin<float>(), pG = G.begin<float>(), pR = R.begin<float>();
  while (pI != in.end<cv::Vec3b>()) {
      *pB = (float)(*pI)[0];
      *pG = (float)(*pI)[1];
      *pR = (float)(*pI)[2];
      pI++; pB++; pG++; pR++;
  }
}

void iSmooth(const cv::Mat &src, float sigma, cv::Mat &out) {
    std::vector<float> mask = make_fgauss(sigma);
    normalize(mask);
    cv::Mat tmp(src.rows, src.cols, src.type());
    convolve_even(src, tmp, mask);
    convolve_even(tmp, out, mask);
}

void iBuildGraph(const cv::Mat &in,
    float sigma,
    Edge *&edges,
    int *num_edges)
{
    int width = in.cols;
    int height = in.rows;
    int num = 0;
    int x, y, xp, ym, yp;
    int safeWidth = width - 1, safeHeight = height - 1;
    int reserve_size = in.rows * in.cols * 8;
    //printf("Reserve size = %d\n",reserve_size);
    edges = (Edge*)malloc(reserve_size*sizeof(Edge));
    if (edges == NULL) {
        printf("Error, could not malloc\n");
        return;
    }
    cv::Mat R, G, B, smooth_r, smooth_g, smooth_b;
    iExtractRGBColorSpace(in, B, G, R);
    iSmooth(B, sigma, smooth_b);
    iSmooth(G, sigma, smooth_g);
    iSmooth(R, sigma, smooth_r);

    //Normalize

    Edge *p = edges;
    cv::Mat_<float>::const_iterator pR = smooth_r.begin<float>(), pG = smooth_g.begin<float>(), pB = smooth_b.begin<float>();
    cv::Mat_<float>::const_iterator pRBegin = pR, pGBegin = pG, pBBegin = pB;
    for (y = 0, ym = -1, yp = 1; y < height; y++, ym++, yp++)
    {
        for (x = 0, xp = 1; x < width; x++, xp++)
        {
            //cout << x << ", " << y << endl;
            if (x < safeWidth)
            {
                Edge edge;
                edge.a = y * width + x;
                edge.b = y * width + xp;
                edge.w = sqrtf(square(*pR - *(pRBegin + edge.b)) + square(*pG - *(pGBegin + edge.b)) + square(*pB - *(pBBegin + edge.b)));
                //edge.valid = true;
                *p++ = edge;
                num++;
            }
            if (y < safeHeight)
            {
                Edge edge;
                edge.a = y * width + x;
                edge.b = yp * width + x;
                edge.w = sqrtf(square(*pR - *(pRBegin + edge.b)) + square(*pG - *(pGBegin + edge.b)) + square(*pB - *(pBBegin + edge.b)));
                //edge.valid = true;
                *p++ = edge;
                num++;
            }
            if ((x < safeWidth) && (y < safeHeight))
            {
                Edge edge;
                edge.a = y * width + x;
                edge.b = yp * width + xp;
                edge.w = sqrtf(square(*pR - *(pRBegin + edge.b)) + square(*pG - *(pGBegin + edge.b)) + square(*pB - *(pBBegin + edge.b)));
                //edge.valid = true;
                *p++ = edge;
                num++;
            }
            if ((x < safeWidth) && (y > 0))
            {
                Edge edge;
                edge.a = y * width + x;
                edge.b = ym * width + xp;
                edge.w = sqrtf(square(*pR - *(pRBegin + edge.b)) + square(*pG - *(pGBegin + edge.b)) + square(*pB - *(pBBegin + edge.b)));
                //edge.valid = true;
                *p++ = edge;
                num++;
            }
            pR++; pG++; pB++;
        }
    }
    B.release();
    G.release();
    R.release();
    smooth_b.release();
    smooth_g.release();
    smooth_r.release();
    *num_edges = num;
}

bool lessThan(const Edge& a, const Edge& b) {
    return a.w < b.w;
}

void iSegment_graph(int num_vertices, int num_edges, Edge*& edges, float c, Universe *u)
{
    Edge* pEdge = edges, *edgesEnd = pEdge + num_edges;
    // sort edges by weight
    std::sort(pEdge, edgesEnd);
    //thrustsort(pEdge,edgesEnd);

    // init thresholds
    float *threshold = new float[num_vertices];
    int i;
    float *pThresh = threshold;
    for (i = 0; i < num_vertices; i++)
        *pThresh++ = THRESHOLD(1, c);

    // for each edge, in non-decreasing weight order...
    while (pEdge != edgesEnd)
    {
        //if(pEdge->valid) {
        // components conected by this edge
        int a = u->find(pEdge->a);
        int b = u->find(pEdge->b);
        if (a != b /*&& a >= 0 && b>= 0 && a < num_vertices && b < num_vertices*/) {
            if ((pEdge->w <= threshold[a]) &&
                (pEdge->w <= threshold[b])) {
                u->join(a, b);
                a = u->find(a);
                if (a < num_vertices && a >= 0)
                    threshold[a] = pEdge->w + THRESHOLD(u->size(a), c);
                else
                    printf("a is %d, which is out of bounds\n", a);
            }
        }
        //}
        pEdge++;
    }

    // free up
    delete threshold;
}

inline void iJoin_graph(Edge *&edges, int num_edges, int min_size, Universe *u) {
    Edge *pEdge = edges, *edgesEnd = edges + num_edges;
    while (pEdge != edgesEnd)
    {
        int a = u->find(pEdge->a);
        int b = u->find(pEdge->b);
        if ((a != b) && ((u->size(a) < min_size) || (u->size(b) < min_size)))
        {
            u->join(a, b);
        }
        pEdge++;
    }
}

int FHGraphSegment(
    const cv::Mat &in,
    const float sigma,
    const float c,
    const int min_size,
    cv::Mat &out,
    cv::Mat &out_color,
    const int segStartNumber = 0)
{

    int i, size = in.rows * in.cols;
    Universe u(size);
    Edge* edges = NULL;
    int num_edges;

    iBuildGraph(in, sigma, edges, &num_edges);
    if (edges == NULL || num_edges == 0) {
        printf("Error, graph has no edges\n");
        return 0;
    }
    iSegment_graph(size, num_edges, edges, c, &u);
    iJoin_graph(edges, num_edges, min_size, &u);

    free(edges);

    int numSegs = u.num_sets();
    cv::Vec3b *m_colors = (cv::Vec3b *)malloc(numSegs*sizeof(cv::Vec3b));
    cv::Vec3b *pColor = m_colors;
    for (i = 0; i < numSegs; i++)
    {
        cv::Vec3b color;
        random_rgb(color);
        *pColor++ = color;
    }

    out = cv::Mat(in.rows, in.cols, CV_32SC1);
    out_color = cv::Mat(in.rows, in.cols, CV_8UC3);
    cv::Mat_<int>::iterator pO = out.begin<int>();
    cv::Mat_<cv::Vec3b>::iterator pPseudo = out_color.begin<cv::Vec3b>();
    i = 0;
    //We know there are u.num_sets() segments and size possible ids, and we want those ordered starting from 0.
    int *m_ids = (int*)malloc(size*sizeof(int));
    std::fill_n(m_ids, size, -1);
    int currSeg = segStartNumber;
    //std::map<int, int> segmentIdMap;
    while (pO != out.end<int>()) {
        int uId = u.find(i);
        /*if (segmentIdMap.find(uId) == segmentIdMap.end()) {
            segmentIdMap[uId] = currSeg;
            ++currSeg;
            }
            *pO = segmentIdMap[uId];*/
        if (m_ids[uId] == -1) {
            m_ids[uId] = currSeg;
            ++currSeg;
        }
        *pO = m_ids[uId];
        *pPseudo = m_colors[*pO - segStartNumber];
        ++pO; ++i; ++pPseudo;
    }
    free(m_colors);
    u.elts.clear();
    return numSegs;
}

/* END FH */
/* -----------------------------------------------------------------------------------------*/

int segmentation(const cv::Mat &in, cv::Mat &out, cv::Mat &out_color, const float sigma, const int k, const int size, const int segStartNumber) {
    /*SLIC slic(in.cols, in.rows);
    slic.ComputeSuperPixels(in, cv::Point(0, 0), s, m, iter);
    slic.DrawResults(out);*/
    int numSegs = FHGraphSegment(in, sigma, k, size, out, out_color, segStartNumber);
    return numSegs;
}
