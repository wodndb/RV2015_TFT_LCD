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
///	file	: tftlcd.cpp
///	brief	: Librarys for search defect at TFT-LCD panel images
//

#include "tftlcd.h"

////////////////////////////////////////////////////////
//
//	Public member functions of PanelImage class
//

///
//	PanelImage::openImage
//
//	Open TFT-LCD image and save to member variable srcImage.
//
/// string &srcImageName	: Image name
//
void PanelImage::openImage(string &srcImageName)
{
	this->srcImage = imread(srcImageName);
	this->getCellSize();
}

///
//	PanelImage::getCellSize
//
//	Find cell size save to member variable cellSizeX and cellSizeY 
//
void PanelImage::getCellSize(void)
{
	if(this->srcImage.empty())
	{
		printf("Image is empty!\n");
	}
	else
	{
		Mat grayImage = this->srcImage.clone();				// Gray Image for find cell size
		cvtColor(this->srcImage, grayImage, CV_RGB2GRAY);	// Convert source image to gray scale image
		uchar maxValueX = 0;								// Max value on X Axis at middle point of height
		uchar maxValueY = 0;								// Max value on Y axis at middle point of width
		int middlePX = this->srcImage.cols / 2;				// Middle point of width
		int middlePY = this->srcImage.rows / 2;				// Middle point of height
		deque<int> pitchPointSampleX;
		deque<int> pitchPointSampleY;

		// Find max value (that is also called pitch value) on X axis and Y axis
		for(int x = 0; x < grayImage.cols; x++)
		{
			maxValueX = MAX(maxValueX, *(grayImage.ptr<uchar>(middlePY, x)));
		}
		for(int y = 0; y < grayImage.rows; y++)
		{
			maxValueY = MAX(maxValueY, *(grayImage.ptr<uchar>(y, middlePX)));
		}

		// Find points about max value on X axis and Y axis
		for(int x = 1; x < grayImage.cols; x++) {
			//�ִ밪 - 10���� ũ�ų�, ���� �ȼ����� ���� 10 �̻� ���̳��� ��� pitch���̴�.
			if(*(grayImage.ptr<uchar>(middlePY, x)) > maxValueX - 10 &&
				*(grayImage.ptr<uchar>(middlePY, x)) > *(grayImage.ptr<uchar>(middlePY, x - 1)) + 10)
			{
				pitchPointSampleX.push_back(x);
			}
		}
		for(int y = 1; y < grayImage.rows; y++) {
			//�ִ밪 - 10���� ũ�ų�, ���� �ȼ����� ���� 10 �̻� ���̳��� ��� pitch���̴�.
			if(*(grayImage.ptr<uchar>(y, middlePX)) > maxValueY - 10 &&
				*(grayImage.ptr<uchar>(y, middlePX)) > *(grayImage.ptr<uchar>(y - 1, middlePX)) + 10)
			{
				pitchPointSampleY.push_back(y);
			}
		}
		
		int maxPitchLengthX = 0;
		int maxPitchLengthY = 0;

		// Sampling pitch points to find cell size
		for(int i = 0; i < pitchPointSampleX.size() - 1; i++) {
			maxPitchLengthX = MAX(maxPitchLengthX, pitchPointSampleX.at(i + 1) - pitchPointSampleX.at(i));
		}
		for(int i = 0; i < pitchPointSampleY.size() - 1; i++) {
			maxPitchLengthY = MAX(maxPitchLengthY, pitchPointSampleY.at(i + 1) - pitchPointSampleY.at(i));
		}

		// Set cell size
		this->cellSizeX = maxPitchLengthX;
		this->cellSizeY = maxPitchLengthY;
	}
}

///
//	PanelImage::autoDefectSearch
//
//	Search defect area in source image
//
void PanelImage::autoDefectSearch(void) {
	// 1. �� ���� ȹ��
	Mat sourceImage = this->srcImage.clone();
	Mat grayImage = sourceImage.clone();
	this->binImage = Mat(sourceImage.rows, sourceImage.cols, CV_8UC1);
	Mat dstImage = binImage.clone();
	cvtColor(sourceImage, grayImage, CV_RGB2GRAY);

	// 2. Rmax, Rmin ���� ����
	Mat rMax, rMin;
	Mat element = getStructuringElement(MORPH_RECT, Size(3, 3), Point(1, 1));
	dilate(grayImage, rMax, element);
	erode(grayImage, rMin, element);

	// 3. ������ D(x, y) ����
	// +
	// 4. ���� ���� ����
	Mat dImage(sourceImage.rows, sourceImage.cols, CV_8UC1);	// �ʱ�ȭ

	for(int y = 0; y < sourceImage.rows; y++) {
		// ����
		for(int x = 0; x < this->cellSizeX; x++) {
			if(*grayImage.ptr<uchar>(y, x) < *rMin.ptr<uchar>(y, x + this->cellSizeX)
				&& *grayImage.ptr<uchar>(y, x) < *rMin.ptr<uchar>(y, x + (this->cellSizeX * 2)))
			{
				*dImage.ptr<uchar>(y, x) = MIN(*rMin.ptr<uchar>(y, x + this->cellSizeX) - *grayImage.ptr<uchar>(y, x),
												*rMin.ptr<uchar>(y, x + this->cellSizeX * 2) - *grayImage.ptr<uchar>(y, x));
			}
			else if(*grayImage.ptr<uchar>(y, x) > *rMax.ptr<uchar>(y, x + this->cellSizeX)
				&& *grayImage.ptr<uchar>(y, x) > *rMax.ptr<uchar>(y, x + (this->cellSizeX * 2)))
			{
				*dImage.ptr<uchar>(y, x) = MIN(*grayImage.ptr<uchar>(y, x) - *rMax.ptr<uchar>(y, x + this->cellSizeX),
												*grayImage.ptr<uchar>(y, x) - *rMax.ptr<uchar>(y, x + this->cellSizeX * 2));
			}
			else
			{
				*dImage.ptr<uchar>(y, x) = 0;
			}
		}
		// �߰�
		for(int x = this->cellSizeX; x < sourceImage.cols - this->cellSizeX; x++) {
			if(*grayImage.ptr<uchar>(y, x) < *rMin.ptr<uchar>(y, x + this->cellSizeX)
				&& *grayImage.ptr<uchar>(y, x) < *rMin.ptr<uchar>(y, x - this->cellSizeX))
			{
				*dImage.ptr<uchar>(y, x) = MIN(*rMin.ptr<uchar>(y, x + this->cellSizeX) - *grayImage.ptr<uchar>(y, x),
												*rMin.ptr<uchar>(y, x - this->cellSizeX) - *grayImage.ptr<uchar>(y, x));
			}
			else if(*grayImage.ptr<uchar>(y, x) > *rMax.ptr<uchar>(y, x + this->cellSizeX)
				&& *grayImage.ptr<uchar>(y, x) > *rMax.ptr<uchar>(y, x - this->cellSizeX))
			{
				*dImage.ptr<uchar>(y, x) = MIN(*grayImage.ptr<uchar>(y, x) - *rMax.ptr<uchar>(y, x + this->cellSizeX),
												*grayImage.ptr<uchar>(y, x) - *rMax.ptr<uchar>(y, x - this->cellSizeX));
			}
			else
			{
				*dImage.ptr<uchar>(y, x) = 0;
			}
		}
		// ����
		for(int x = srcImage.cols - this->cellSizeX - 1; x < sourceImage.cols; x++) {
			if(*grayImage.ptr<uchar>(y, x) < *rMin.ptr<uchar>(y, x - this->cellSizeX)
				&& *grayImage.ptr<uchar>(y, x) < *rMin.ptr<uchar>(y, x - (this->cellSizeX * 2)))
			{
				*dImage.ptr<uchar>(y, x) = MIN(*rMin.ptr<uchar>(y, x - this->cellSizeX) - *grayImage.ptr<uchar>(y, x),
												*rMin.ptr<uchar>(y, x - this->cellSizeX * 2) - *grayImage.ptr<uchar>(y, x));
			}
			else if(*grayImage.ptr<uchar>(y, x) > *rMax.ptr<uchar>(y, x - this->cellSizeX)
				&& *grayImage.ptr<uchar>(y, x) > *rMax.ptr<uchar>(y, x - (this->cellSizeX * 2)))
			{
				*dImage.ptr<uchar>(y, x) = MIN(*grayImage.ptr<uchar>(y, x) - *rMax.ptr<uchar>(y, x - this->cellSizeX), 
												*grayImage.ptr<uchar>(y, x) - *rMax.ptr<uchar>(y, x - this->cellSizeX * 2));
			}
			else
			{
				*dImage.ptr<uchar>(y, x) = 0;
			}
		}
	}

	
	// 5. ���� ����ȭ
	threshold(dImage, binImage, 14, 255, CV_THRESH_BINARY);
	imshow("BinImage", binImage);
	
	// 6. ���� ���� �������� ����
	morphologyEx(binImage, dstImage, MORPH_CLOSE, element);

	// 7. ������ ���Ե��� ����
	findDefectArea();		// �����ִ� ������ ���簢������ �߻�ȭ
	mergeDefectArea(20);	// ���������� ������ ������ �ִ� �߻�ȭ�� ���� ������ �Ժ�
	
	// 7-2 �����

	deque<DefectImage>::iterator DAIterPos = this->DefectInfo.begin();
	while(DAIterPos != this->DefectInfo.end()) {
		Point a = Point(DAIterPos->left, DAIterPos->top);
		Point b = Point(DAIterPos->right, DAIterPos->bottom);
		rectangle(sourceImage, a, b, cv::Scalar(0.0, 0.0, 255.0), 1);
		printf("%d\n", DAIterPos->top + DAIterPos->bottom);
		DAIterPos++;
	}
	
	imshow("BinCloseImage", dstImage);
	imshow("ResultImage", sourceImage);

	cvWaitKey(0);

	// 8. ���� ���� ȹ��
	this->DefectInfo = DefectInfo;
}

///
//	PanelImage::findDefectArea
//
//	Find defect area in binary defect image
//
void PanelImage::findDefectArea(void) {
	Mat checkImage = this->binImage.clone();	// üũ�� �ҷ�ȭ�Ҹ� ������ �̹��� ����
	deque<DefectImage> resultDA;			// �ҷ�ȭ�� �������� ������ ������ ����
	
	// �ҷ�ȭ�� Ž��
	for(int y = 0; y < checkImage.rows; y++) {
		for(int x = 0; x < checkImage.cols; x++) {
			// �ҷ�ȭ�Ұ� �ִ��� Ž��
			if(*checkImage.ptr<uchar>(y, x) != 0) {
				// �˻� ��� ȭ���� ��ġ ����
				deque<int> dx;
				deque<int> dy;
				
				// �ҷ� ȭ�� ������ ������ �ӽ� ����
				DefectImage tempDefect(y, y, x, x);

				// ó�� �߰��� �ҷ�ȭ�Ҹ� �˻� ��� �߰�
				dx.push_back(x);
				dy.push_back(y);

				// �ҷ� ȭ�� ���� �˻�
				while(!dx.empty() && !dy.empty()) {
					if(*checkImage.ptr<uchar>(dy.front(), dx.front()) != 0) {
						// �ҷ�ȭ�� ���� üũ (255�� 0���� ����)
						*checkImage.ptr<uchar>(dy.front(), dx.front()) = 0;

						// �ҷ�ȭ�� ���� ����
						if(tempDefect.top > dy.front()) tempDefect.top = dy.front();
						if(tempDefect.bottom < dy.front()) tempDefect.bottom = dy.front();
						if(tempDefect.left > dx.front()) tempDefect.left = dx.front();
						if(tempDefect.right < dx.front()) tempDefect.right = dx.front();

						// �ֺ� ȭ�Ҹ� �˻� ������� �߰�
						if(dx.front() - 1 >= 0)					{dx.push_back(dx.front() - 1); dy.push_back(dy.front());}
						if(dx.front() + 1 < checkImage.cols)	{dx.push_back(dx.front() + 1); dy.push_back(dy.front());}
						if(dy.front() - 1 >= 0)					{dx.push_back(dx.front()); dy.push_back(dy.front() - 1);}
						if(dy.front() + 1 < checkImage.rows)	{dx.push_back(dx.front()); dy.push_back(dy.front() + 1);}
					}

					//�˻��� ���� ���� ȭ�Ҹ� �˻� ��󿡼� ����
					dx.pop_front(); dy.pop_front();
				}
				
				//Ž���� ���� �ҷ� ȭ�� ������ ť�� ����.
				resultDA.push_back(tempDefect);
			}
		}
	}

	this->DefectInfo = resultDA;
}

///
//	PanelImage::mergeDefectArea
//
//	Merge defect information using AABB Collision Algorithm
//	Actually, merge defect area in range.
//
/// int range	: Range for merge defect area.
//
void PanelImage::mergeDefectArea(int range) {
	// �ʱ�ȭ
	deque<DefectImage> mergedAreaInfo;

	// this->DefectInfo�� �� ������ �պ��Ѵ�.
	// �պ��� �Ϸ�� ������ mergedAreaInfo�� �����Ѵ�.
	while(!this->DefectInfo.empty()) {
		DefectImage tempDA = this->DefectInfo.front();
		this->DefectInfo.pop_front();
		int num = this->DefectInfo.size();
		// Compare all of area for merge Defect
		for(int i = 0; i < num; i++) {
			// AABB Collosion Check
			if(AABBCollision(tempDA, this->DefectInfo.front(), range)) {
				// If true, merge Defect area
				if(tempDA.top > this->DefectInfo.front().top)
					tempDA.top = this->DefectInfo.front().top;
				if(tempDA.bottom < this->DefectInfo.front().bottom)
					tempDA.bottom = this->DefectInfo.front().bottom;
				if(tempDA.left > this->DefectInfo.front().left)
					tempDA.left = this->DefectInfo.front().left;
				if(tempDA.right < this->DefectInfo.front().right)
					tempDA.right = this->DefectInfo.front().right;
			}
			else {
				// If not true, push front area to back
				this->DefectInfo.push_back(this->DefectInfo.front());
			}
			// pop front data
			this->DefectInfo.pop_front();
		}
		// When end to compare area, push merged area to deque (mergedAreaInfo)
		mergedAreaInfo.push_back(tempDA);
	}

	this->DefectInfo = mergedAreaInfo;
}

//void PanelImage::findCellPoint(void) {
//	//�ڳ� ������ ���� ���� �غ�
//	Mat grayImage = this->srcImage.clone();
//	cvtColor(this->srcImage, grayImage, CV_RGB2GRAY);
//	Mat eigImage = this->srcImage.clone();
//	Mat tempImage = this->srcImage.clone();
//	vector<Point2f> corners;
//	int cornerCount = 1024;		//�ڳ��� �ִ� ������ ����
//	goodFeaturesToTrack(grayImage, corners, 1024, 0.01, 10, Mat(), 3, false, 0.04);
//
//	for (int i = 0; i < cornerCount; i++)
//	{
//		cvCircle(this->srcImage, cvPointFrom32f(corners[i]), 2, CV_RGB(0, 255, 0));
//	}
//	cvShowImage("oriImage", oriImage);
//	cvWaitKey(0);
//}

////////////////////////////////////////////////////////
//
//	Public functions
//

// AABB Collision is to compare two rectangle for detect overlap between them.
// This function is modified for Auto defect search algorithm.
// It is same to compare two areas, but area A is expanded and expand range is inputted from parameter.
bool AABBCollision(DefectImage a, DefectImage b, int exA_range) {
	if( a.right + exA_range < b.left || 
		b.right < a.left - exA_range || 
		a.bottom + exA_range < b.top || 
		b.bottom < a.top - exA_range )
		return false;
	else
		return true;
}