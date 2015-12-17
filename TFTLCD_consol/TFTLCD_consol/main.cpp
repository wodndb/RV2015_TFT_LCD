#include "tftlcd.h"
#include <conio.h>

int main(void) {
	//Mat tempImage = imread("000012.bmp");
	//Mat grayImage;
	//Mat dst;
	//cvtColor(tempImage, grayImage, CV_BGR2GRAY);
	//Mat resultImage = grayImage.clone();
	////threshold(grayImage, resultImage, 200, 255, 0);
	//Canny( grayImage, resultImage, 100, 300, 3 );
	//
	//addWeighted(grayImage, 0.5, resultImage, 0.5, 0.0, dst);

	//imshow("test", dst);

	//cvWaitKey(0);

	
	PanelImage pi;
	pi.openImage(String("000035.bmp"));
	pi.autoDefectSearch();
	pi.findCellPoint();
	pi.findDefectCell();
	
	return 0;
}