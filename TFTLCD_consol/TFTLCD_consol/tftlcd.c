#include "tftlcd.h"

void PanelImage::openImage(string &srcImageName)
{
	this->srcImage = imread(srcImageName);
	this->getCellSize();
}

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
			//최대값 - 10보다 크거나, 이전 픽셀보다 값이 10 이상 차이나는 경우 pitch값이다.
			if(*(grayImage.ptr<uchar>(middlePY, x)) > maxValueX - 10 &&
				*(grayImage.ptr<uchar>(middlePY, x)) > *(grayImage.ptr<uchar>(middlePY, x - 1)) + 10)
			{
				pitchPointSampleX.push_back(x);
			}
		}
		for(int y = 1; y < grayImage.rows; y++) {
			//최대값 - 10보다 크거나, 이전 픽셀보다 값이 10 이상 차이나는 경우 pitch값이다.
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
			maxPitchLengthX = MAX(maxPitchLengthX, pitchPointSampleX.at(i));
		}
		for(int i = 0; i < pitchPointSampleY.size() - 1; i++) {
			maxPitchLengthY = MAX(maxPitchLengthY, pitchPointSampleY.at(i));
		}

		// Set cell size
		this->cellSizeX = maxPitchLengthX;
		this->cellSizeY = maxPitchLengthY;
	}
}

void PanelImage::autoDefeatSearch(void) {
	// 1. 원 영상 획득
	Mat sourceImage = this->srcImage.clone();
	Mat grayImage = sourceImage.clone();
	cvtColor(sourceImage, grayImage, CV_RGB2GRAY);

	// 2. Rmax, Rmin 영상 생성
	Mat Rmax, Rmin;
	Mat element = getStructuringElement(MORPH_RECT, Size(3, 3), Point(1, 1));
	dilate(grayImage, Rmax, element);
	erode(grayImage, Rmin, element);

	// 3. 차영상 D(x, y) 생성
	Mat Dimage(sourceImage.rows, sourceImage.cols, CV_8UC1);

	// 4. 유령 결함 제거
	
	// 5. 영상 이진화
	
	// 6. 이진 닫힘 모폴로지 영상
	
	// 7. 인접한 결함들을 결합
	
	// 8. 결함 정보 획득

}