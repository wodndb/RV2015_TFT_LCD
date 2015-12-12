////
//	KOREA UNIVERSITY OF TECHNOLOGY AND EDUCATION
//	SCHOOL OF COMPUTER SCIENCE AND ENGINEERING
//
//	ROBOT VISION AND PRACTICE 2015 AUTUMN TERM PROJECT
//	TFT-LCD Defect Auto Search Algorithm
//
//	URL:	http://www.koreatech.ac.kr		// Official Univ. Homepage
//			http://cse.koreatech.ac.kr		// Official Dept. Homepage
//
//	Author :
//		SCHOOL OF COMPUTER SCIENCE AND ENGINEERING
//		2012136116 JEONG, JAEU
//		wodndb@koreatech.ac.kr
//

//
///	file	: tftlcd.h
///	brief	: Librarys for search defect at TFT-LCD panel images
//

#pragma once

///
//	Includes
///
#include <iostream>
#include <cv.h>
#include <highgui.h>
#include <deque>

///
//	Macros
///
#define MAX(a, b) ((a > b) ? (a) : (b))
#define MIN(a, b) ((a < b) ? (a) : (b))
#define XOR_SWAP(a, b) ((a)^=(b)^=(a)^=(b))

///
//	Using namespaces
///
using namespace std;
using namespace cv;

////////////////////////////////////////////////////////
//
//	Classes
//

///
//	DefectImage
//
//	This class has information about defect area at image
//
class DefectImage {
public:
	///
	//	Member variables
	///
	int top;		// Top pixel coordinate
	int bottom;		// Bottom pixel coordinate
	int left;		// Left pixel coordinate
	int right;		// Right pixel coordinate
	
	///
	// Constructor
	///
	DefectImage(void) {
		this->top = 0;
		this->bottom = 0;
		this->left = 0;
		this->right = 0;
	}
	
	DefectImage(int _top, int _bottom, int _left, int _right) {
		this->top = _top;
		this->bottom = _bottom;
		this->left = _left;
		this->right = _right;
	}

	void setDefectImage(int _top, int _bottom, int _left, int _right) {
		this->top = _top;
		this->bottom = _bottom;
		this->left = _left;
		this->right = _right;
	}
};

///
//	PanelImage
//
//	This class has information TFT-LCD image
//
class PanelImage {
public:
	///
	// Member variables
	///
	Mat srcImage;		// Original source image
	Mat binImage;		// Binary image only defect area in source image
	Mat resultImage;	// Marked image about defect area
	int cellSizeX;		// Width of cell size
	int cellSizeY;		// Height of cell size

	deque<DefectImage> DefectInfo;	// Defect area information

	///
	// Constructor
	///
	PanelImage(void) {
		int cellSizeX = 0;
		int cellSizeY = 0;
	}

	PanelImage(Mat _srcImage, Mat _binImage, Mat _resultImage, int cellSizeX, int CellSizeY, deque<DefectImage> _DefectInfo) {
		this->srcImage = _srcImage.clone();
		this->binImage = _binImage.clone();
		this->resultImage = _resultImage.clone();
		this->cellSizeX = 0;
		this->cellSizeY = 0;
		this->DefectInfo = _DefectInfo;
	}

	///
	//	Public member functions
	///

	///
	//	openImage
	//
	//	Open TFT-LCD image and save to member variable srcImage.
	//
	void openImage(string &srcImageName);
	
	///
	//	getCellSize
	//
	//	Find cell size save to member variable cellSizeX and cellSizeY 
	//
	void getCellSize(void);

	///
	//	autoDefectSearch
	//
	//	Search defect area in source image
	//
	void autoDefectSearch(void);

	///
	//	findDefectArea
	//
	//	Find defect area in binary defect image
	//
	void findDefectArea(void);

	///
	//	mergeDefectArea
	//
	//	Merge defect information using AABB Collision Algorithm
	//	Actually, merge defect area in range.
	//
	/// int range	: Range for merge defect area.
	//
	void mergeDefectArea(int range);
};

////////////////////////////////////////////////////////
//
//	Public functions
//

///
//	AABBCollision
//
//	AABB Collision is to compare two rectangle for detect overlap between them.
//	This function is modified for Auto defect search algorithm.
//	It is same to compare two areas, but area A is expanded and expand range is inputted from parameter.
//
bool AABBCollision(DefectImage a, DefectImage b, int exA_range);