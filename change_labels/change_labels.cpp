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
	if (!fp.is_open()) {
		printf("ERROR reading map.txt\n");
		return 1;
	}
	string line;
	string dl = ":";
	map<int, int> colors;
	while (getline(fp, line)) {
		size_t pos = line.find(dl);
		if (pos == string::npos) {
			printf("ERROR parsing map.txt\n");
			return 1;
		}
		int key = atoi(line.substr(0, pos).c_str());
		int val = atoi(line.substr(pos + 1, line.length()).c_str());
		colors[key] = val;
		//printf("Key: %d, val: %d\n",key,val);
	}

	//Now we need the start num, the video folder, and the number of images
	int ann_num = 2;
	if (start % 3 == 0)
		ann_num = 0;
	else if ((start + 22) % 3 == 0)
		ann_num = 1;
	//printf("ann_num is: %d\n",ann_num);


	//Now we go through all of the images and update everything
	for (int i = 0; i < num; i++) {
		char fileName[500];
		sprintf(fileName, "/home/steve/cs7616/annotations/rendered/frame%05d.png", i);
		//printf("Filename: %s\n",fileName);
		Mat img = imread(fileName);
		if (img.empty()) {
			printf("ERROR opening image\n");
			return 1;
		}
		Mat m = Mat::zeros(img.rows, img.cols, CV_8UC1);
		//imshow("window",img);
		//waitKey(33);
		Mat_<Vec3b>::const_iterator pI = img.begin<Vec3b>();
		Mat_<uchar>::iterator pO = m.begin<uchar>();
		while (pI != img.end<Vec3b>()) {
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
		sprintf(outFile, "/home/steve/cs7616/annotations/%s/%d_%d.png", video.c_str(), frame_num, ann_num);
		imwrite(outFile, m);
	}
	return 0;
}

int vote_labels(string video, int start, int end) {
	//For every frame, lets grab the different versions and do per-pixel voting
	for (int i = start; i <= end; i++) {
		char fileName[500];
		sprintf(fileName, "/home/steve/cs7616/annotations/%s/%d_0.png", video.c_str(), i);
		Mat img0 = imread(fileName, 0);
		sprintf(fileName, "/home/steve/cs7616/annotations/%s/%d_1.png", video.c_str(), i);
		Mat img1 = imread(fileName, 0);
		sprintf(fileName, "/home/steve/cs7616/annotations/%s/%d_2.png", video.c_str(), i);
		//printf("Filename: %s\n",fileName);
		Mat img2 = imread(fileName, 0);

		bool use[3] = { !img0.empty(), !img1.empty(), !img2.empty() };
		//printf("Use: %d, %d, %d\n", (int)use[0], (int)use[1], (int)use[2]);
		Mat_<uchar>::iterator p0, p1, p2, pF;
		int rows, cols;
		if (use[0]) {
			rows = img0.rows;
			cols = img0.cols;
		}
		else if (use[1]) {
			rows = img1.rows;
			cols = img1.cols;
		}
		else if (use[2]) {
			rows = img2.rows;
			cols = img2.cols;
		}
		else {
			printf("ERROR no images\n");
			return 1;
		}
		if (use[0])
			p0 = img0.begin<uchar>();
		if (use[1])
			p1 = img1.begin<uchar>();
		if (use[2])
			p2 = img2.begin<uchar>();
		Mat out = Mat(rows, cols, CV_8UC1);
		pF = out.begin<uchar>();
		while (pF != out.end<uchar>()) {
			vector<int> counts;
			counts.resize(20);
			if (use[0])
				counts[*p0++]++;
			if (use[1])
				counts[*p1++]++;
			if (use[2])
				counts[*p2++]++;
			//One problem with this is the 2 person case. 
			int result = distance(counts.begin(), max_element(counts.begin(), counts.end()));
			*pF++ = result;
		}
		char outFile[500];
		sprintf(outFile, "/home/steve/cs7616/annotations/final/%s/%d.png", video.c_str(), i);
		imwrite(outFile, out);
	}
	return 0;
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
	*pColor++ = Vec3b(0, 0, 0);
	for (int i = 1; i < numSegs; i++)
	{
		cv::Vec3b color;
		random_rgb2(color);
		*pColor++ = color;
	}

	for (int i = start; i <= end; i++) {
		char fileName[500];
		sprintf(fileName, "/home/steve/cs7616/annotations/final/%s/%d.png", video.c_str(), i);
		Mat img = imread(fileName, 0);

		Mat out = Mat(img.rows, img.cols, CV_8UC3);
		Mat_<Vec3b>::iterator pF = out.begin<Vec3b>();
		Mat_<uchar>::iterator pI = img.begin<uchar>();
		while (pF != out.end<Vec3b>()) {
			*pF = m_colors[int(*pI)];
			++pF; ++pI;
		}

		char outFile[500];
		sprintf(outFile, "/home/steve/cs7616/annotations/visualized/%s/%d.png", video.c_str(), i);
		imwrite(outFile, out);
	}

	free(m_colors);
	return 0;
}

class Features {
public:
	int size, imgNum, segNum, label;
	float orientation, eccentricity, cov00, cov10, cov11;
	Rect bbox;
	Point2f centroid;
	Mat b_hist, g_hist, r_hist;
	Features() : size(0), imgNum(0), segNum(0), centroid(-1, -1) { 
		bbox.x = INT_MAX;
		bbox.y = INT_MAX;
		bbox.width = 0;
		bbox.height = 0;
	}
	~Features() { }

	void GenFromMoments(Moments &mu) {
		centroid = Point2f(mu.m10 / mu.m00, mu.m01 / mu.m00);
		cov00 = mu.mu20 / mu.m00;
		cov10 = mu.mu11 / mu.m00;
		cov11 = mu.mu02 / mu.m00;
		float tmp, tmp1, tmp2, eigen1, eigen2;
		tmp = cov00 - cov11;
		orientation = 0.5f * atan2f(tmp, 2 * cov10);
		tmp1 = cov11 + cov00;
		tmp2 = 0.5f * sqrtf(4 * cov10 * cov10 + tmp*tmp);
		eigen1 = tmp1 + tmp2;
		eigen2 = tmp1 - tmp2;
		eccentricity = sqrtf(1 - (eigen2 / eigen1));
	}

	void GenHistFromMask(Mat &img, Mat &mask) {
		const int bins = 20;
		const float range[] = { 0, 256 };
		const float* histRange = { range };

		// Separate the image in 3 places ( B, G and R )
		vector<Mat> bgr_planes;
		split(img, bgr_planes);

		/// Compute the histograms:
		calcHist(&bgr_planes[0], 1, 0, mask, b_hist, 1, &bins, &histRange, true, false);
		normalize(b_hist, b_hist, 1);
		calcHist(&bgr_planes[1], 1, 0, mask, g_hist, 1, &bins, &histRange, true, false);
		normalize(g_hist, g_hist, 1);
		calcHist(&bgr_planes[2], 1, 0, mask, r_hist, 1, &bins, &histRange, true, false);
		normalize(r_hist, r_hist, 1);
	}

	void WriteFeaturesToFile(ofstream &fp) {
		//size, centroid - x,y, bounding box - xmin, ymin, width, height, orientation, eccentricity, cov00, cov10, cov11, b_hist, g_hist, r_hist, imgNum, segNum, label
		fp << size << ",";
		fp << centroid.x << ",";
		fp << centroid.y << ",";
		bbox.width -= bbox.x;
		bbox.height -= bbox.y;
		fp << bbox.x << ",";
		fp << bbox.y << ",";
		fp << bbox.width << ",";
		fp << bbox.height << ",";
		fp << orientation << ",";
		fp << eccentricity << ",";
		fp << cov00 << ",";
		fp << cov10 << ",";
		fp << cov11 << ",";
		Mat_<float>::iterator p = b_hist.begin<float>();
		while (p != b_hist.end<float>())
			fp << *p++ << ",";
		p = g_hist.begin<float>();
		while (p != g_hist.end<float>())
			fp << *p++ << ",";
		p = r_hist.begin<float>();
		while (p != r_hist.end<float>())
			fp << *p++ << ",";
		fp << imgNum << ",";
		fp << segNum << ",";
		fp << label << "\n";
	}
};

int create_features(string video, int start, int end) {
	ofstream fp;
	fp.open("/home/steve/cs7616/annotations/features.csv", std::ofstream::app);
	for (int i = start; i <= end; i++) {
        if(i % 10 != 0)
            continue;
		char fileName[500];
		sprintf(fileName, "/home/steve/cs7616/videos/%s/%d.png", video.c_str(), i);
		Mat img = imread(fileName);
        if(img.empty()) {
            printf("Empty image\n");
            return 1;
        }
		char fileName2[500];
		sprintf(fileName2, "/home/steve/cs7616/annotations/final/%s/%d.png", video.c_str(), i);
		Mat label = imread(fileName2,0);
        if(label.empty()) {
            printf("Empty label\n");
            return 1;
        }
		Mat seg, segDisp;
		int numSegs = segmentation(img, seg, segDisp, 1.5f, 800, 800, 0);
		//printf("Numsegs: %d\n", numSegs);
		//imshow("window",segDisp);
		//waitKey(0);
		// Go through each segment and generate features
		vector<Features> feats;
		feats.resize(numSegs);
		vector<Features>::iterator pF = feats.begin();
		//vector<Moments> mu = moments( seg, false ); }
		for (int c = 0; c < numSegs; c++, pF++) {
			pF->imgNum = i;
			pF->segNum = c;
			Mat mask = Mat::zeros(seg.rows, seg.cols, CV_8UC1);
			Mat_<int>::iterator pS = seg.begin<int>();
			Mat_<uchar>::iterator pM = mask.begin<uchar>();
			Mat_<uchar>::iterator pL = label.begin<uchar>();
			vector<int> labelScores;
			labelScores.resize(20);
			std::fill_n(labelScores.begin(), 20, 0);
			// Let's start by extracting the mask, size, and bounding box
			for (int y = 0; y < seg.rows; y++) {
				for (int x = 0; x < seg.cols; x++) {
					if (*pS == c) {
						*pM = 1;
						pF->size++;
						if (x < pF->bbox.x)
							pF->bbox.x = x;
						if (y < pF->bbox.y)
							pF->bbox.y = y;
						if (x > pF->bbox.width)
							pF->bbox.width = x;
						if (y > pF->bbox.height)
							pF->bbox.height = y;
						labelScores[int(*pL)]++;
					}
					++pS; ++pM; ++pL;
				}
			}
			pF->label = distance(labelScores.begin(), max_element(labelScores.begin(), labelScores.end()));
			Moments mu = moments(mask, true);
			pF->GenFromMoments(mu);
			//Get histograms
			pF->GenHistFromMask(img, mask);

			//Write features and save segmentations
			pF->WriteFeaturesToFile(fp);
			char outFile[500];
			sprintf(outFile, "/home/steve/cs7616/annotations/segmentation/%s/%d.png", video.c_str(), i);
			imwrite(outFile, seg);
		}
	}
	fp.close();
	return 0;
}

int main(int argc, char* argv[]) {
	if (argc != 5) {
		printf("ERROR, wrong number of arguments\n");
		return 1;
	}
	int method = atoi(argv[1]);
	if (method == 0) {
		int start = atoi(argv[2]);
		string video = string(argv[3]);
		int num = atoi(argv[4]);
		return global_labels(start, video, num);
	}
	else if (method == 1) {
		int start = atoi(argv[2]);
		int end = atoi(argv[3]);
		string video = string(argv[4]);
		return vote_labels(video, start, end);
	}
	else if (method == 2) {
		int start = atoi(argv[2]);
		int end = atoi(argv[3]);
		string video = string(argv[4]);
		return visualize_labels(video, start, end);
	}
	else if (method == 3) {
		int start = atoi(argv[2]);
		int end = atoi(argv[3]);
		string video = string(argv[4]);
		return create_features(video, start, end);
	}
	return 0;
}
