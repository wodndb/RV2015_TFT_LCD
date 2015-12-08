#pragma once

#include <iostream>
#include <cv.h>
#include <highgui.h>
#include <deque>

#define MAX(a, b) ((a > b) ? (a) : (b))
#define MIN(a, b) ((a < b) ? (a) : (b))
#define XOR_SWAP(a, b) ((a)^=(b)^=(a)^=(b))

using namespace std;
using namespace cv;

class DefeatImage {
public:
	int top;
	int bottom;
	int left;
	int right;

	DefeatImage(void) {
		this->top = 0;
		this->bottom = 0;
		this->left = 0;
		this->right = 0;
	}
	
	DefeatImage(int _top, int _bottom, int _left, int _right) {
		this->top = _top;
		this->bottom = _bottom;
		this->left = _left;
		this->right = _right;
	}

	void setDefeatImage(int _top, int _bottom, int _left, int _right) {
		this->top = _top;
		this->bottom = _bottom;
		this->left = _left;
		this->right = _right;
	}
};

class PanelImage {
public:
	Mat srcImage;
	Mat resultImage;
	int cellSizeX;
	int cellSizeY;

	deque<DefeatImage> defeatInfo;

	PanelImage(Mat _srcImage, Mat _resultImage, int cellSizeX, int CellSizeY, deque<DefeatImage> _defeatInfo) {
		this->srcImage = _srcImage.clone();
		this->resultImage = _resultImage.clone();
		this->cellSizeX = 0;
		this->cellSizeY = 0;
		this->defeatInfo = _defeatInfo;
	}

	void openImage(string &srcImageName);
	void getCellSize(void);
	void autoDefeatSearch(void);
};
