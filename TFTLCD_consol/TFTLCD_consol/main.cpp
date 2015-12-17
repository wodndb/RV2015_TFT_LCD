#include "tftlcd.h"
#include <conio.h>

int main(void) {
	Mat tempImage = imread("000012.bmp");
	Mat grayImage;
	cvtColor(tempImage, grayImage, CV_BGR2GRAY);
	Mat resultImage = grayImage.clone();
	//threshold(grayImage, resultImage, 200, 255, 0);
	Canny( grayImage, resultImage, 100, 300, 3 );
	
	imshow("test", resultImage);

	cvWaitKey(0);

	/*
	PanelImage pi;
	pi.openImage(String("000012.bmp"));
	pi.autoDefectSearch();
	*/
	return 0;
}