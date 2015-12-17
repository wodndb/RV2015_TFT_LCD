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
	int cellNum;
	this->srcImage = imread(srcImageName);
	this->getCellSize();
	cellNum = (this->srcImage.rows / this->cellSizeY + 2) * (this->srcImage.cols / this->cellSizeX + 2);
	printf("%d\n", cellNum);
	this->defectCellIdx = (bool*)malloc(sizeof(bool) * cellNum);
	for(int i = 0; i < cellNum; i++) {
		this->defectCellIdx[i] = false;
	}
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
		Mat sourceImage = this->srcImage.clone();
		Mat grayImage = sourceImage.clone();				// Gray Image for find cell size
		Mat cannyImage = sourceImage.clone();
		cvtColor(sourceImage, grayImage, CV_RGB2GRAY);	// Convert source image to gray scale image
		uchar maxValueX = 0;								// Max value on X Axis at middle point of height
		uchar maxValueY = 0;								// Max value on Y axis at middle point of width
		int middlePX = this->srcImage.cols / 2;				// Middle point of width
		int middlePY = this->srcImage.rows / 2;				// Middle point of height
		deque<int> pitchPointSampleX;
		deque<int> pitchPointSampleY;

		Canny(grayImage, cannyImage, 100, 300, 3);

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
		for(int x = 1; x < grayImage.cols - 1; x++) {
			//최대값 - 10보다 크거나, 이전 픽셀보다 값이 10 이상 차이나는 경우 pitch값이다.
			if( *(grayImage.ptr<uchar>(middlePY, x)) > maxValueX - 10 &&
				*(grayImage.ptr<uchar>(middlePY, x)) > *(grayImage.ptr<uchar>(middlePY, x - 1)) &&
				*(grayImage.ptr<uchar>(middlePY, x)) >= *(grayImage.ptr<uchar>(middlePY, x + 1)))
			{
				pitchPointSampleX.push_back(x);
				//circle(srcImage, Point(x, middlePY), 10, cv::Scalar(255. ), 2);
				//printf("%5d %4d %4d\n", x, *(grayImage.ptr<uchar>(middlePY, x)),
				//	*(grayImage.ptr<uchar>(middlePY, x - 1)) - *(grayImage.ptr<uchar>(middlePY, x)));
			}
		}
		
		int maxPitchLengthX = 0;
		int maxPitchNumX = 0;
		int maxPitchLengthY = 0;
		int maxPitchNumY = 0;

		map<int, int> lengthMapX;
		map<int, int> lengthMapY;
		map<int, int>::iterator findIter;
		map<int, int>::iterator iterPos;

		// Sampling pitch points to find cell size
		for(int i = 0; i < pitchPointSampleX.size() - 1; i++) {
			findIter = lengthMapX.find(pitchPointSampleX.at(i + 1) - pitchPointSampleX.at(i));
			if(findIter != lengthMapX.end()) {
				findIter->second++;
			}
			else {
				lengthMapX.insert(map<int, int>::value_type(pitchPointSampleX.at(i + 1) - pitchPointSampleX.at(i), 1));
			}
		}

		for(iterPos = lengthMapX.begin(); iterPos != lengthMapX.end(); ++iterPos) {
			if(maxPitchNumX < iterPos->second) {
				maxPitchNumX = iterPos->second;
				maxPitchLengthX = iterPos->first;
			}
		}

		int pitchSizeX = pitchPointSampleX.size();

		for(int i = 0; i < pitchSizeX - 1; i++) {
			if( maxPitchLengthX == pitchPointSampleX.at(1) - pitchPointSampleX.at(0) ) {
				pitchPointSampleX.push_back(pitchPointSampleX.front());
			}
			pitchPointSampleX.pop_front();
		}
		pitchPointSampleX.pop_front();

		maxPitchLengthX = pitchPointSampleX.at(1) - pitchPointSampleX.at(0);
		pitchSizeX = pitchPointSampleX.size();

		for(int i = 1; i < pitchSizeX - 2; i++) {
			maxPitchLengthX = MIN(maxPitchLengthX, pitchPointSampleX.at(i + 1) - pitchPointSampleX.at(i));
		}

		maxPitchLengthY = maxPitchLengthX * 3;

		// Set cell size
		this->cellSizeX = maxPitchLengthX;
		this->cellSizeY = maxPitchLengthY;
		printf("%d %d\n", maxPitchLengthX, maxPitchLengthY);
	}
}

///
//	PanelImage::autoDefectSearch
//
//	Search defect area in source image
//
void PanelImage::autoDefectSearch(void) {
	// 1. 원 영상 획득
	Mat sourceImage = this->srcImage.clone();
	Mat grayImage = sourceImage.clone();
	this->binImage = Mat(sourceImage.rows, sourceImage.cols, CV_8UC1);
	Mat dstImage = this->binImage.clone();
	cvtColor(sourceImage, grayImage, CV_RGB2GRAY);

	// 2. Rmax, Rmin 영상 생성
	Mat rMax, rMin;
	Mat element = getStructuringElement(MORPH_RECT, Size(3, 3), Point(1, 1));
	dilate(grayImage, rMax, element);
	erode(grayImage, rMin, element);

	// 3. 차영상 D(x, y) 생성
	// +
	// 4. 유령 결함 제거
	Mat dImage(sourceImage.rows, sourceImage.cols, CV_8UC1);	// 초기화

	for(int y = 0; y < sourceImage.rows; y++) {
		// 좌측
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
		// 중간
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
		// 우측
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

	
	// 5. 영상 이진화
	threshold(dImage, this->binImage, 14, 255, CV_THRESH_BINARY);
	//imshow("BinImage", binImage);
	
	// 6. 이진 닫힘 모폴로지 영상
	morphologyEx(this->binImage, this->binImage, MORPH_CLOSE, element);

	// 7. 인접한 결함들을 결합
	findDefectArea();		// 뭉쳐있는 결함을 직사각형으로 추상화
	mergeDefectArea(20);	// 뭉쳐있지는 않지만 가까이 있는 추상화된 결함 정보를 함병
	
	// 7-2 디버깅

	deque<DefectImage>::iterator DAIterPos = this->DefectInfo.begin();
	while(DAIterPos != this->DefectInfo.end()) {
		Point a = Point(DAIterPos->left, DAIterPos->top);
		Point b = Point(DAIterPos->right, DAIterPos->bottom);
		rectangle(sourceImage, a, b, cv::Scalar(0.0, 0.0, 255.0), 1);
		//printf("%d\n", DAIterPos->top + DAIterPos->bottom);
		DAIterPos++;
	}
	
	imshow("BinCloseImage", this->binImage);
	imshow("ResultImage", sourceImage);

	cvWaitKey(0);

	// 8. 결함 정보 획득
	this->DefectInfo = DefectInfo;
}

///
//	PanelImage::findDefectArea
//
//	Find defect area in binary defect image
//
void PanelImage::findDefectArea(void) {
	Mat checkImage = this->binImage.clone();	// 체크할 불량화소를 저장할 이미지 변수
	deque<DefectImage> resultDA;			// 불량화소 영역들의 정보를 저장할 변수
	
	// 불량화소 탐색
	for(int y = 0; y < checkImage.rows; y++) {
		for(int x = 0; x < checkImage.cols; x++) {
			// 불량화소가 있는지 탐색
			if(*checkImage.ptr<uchar>(y, x) != 0) {
				// 검색 대상 화소의 위치 저장
				deque<int> dx;
				deque<int> dy;
				
				// 불량 화소 영역을 저장할 임시 변수
				DefectImage tempDefect(y, y, x, x);

				// 처음 발견한 불량화소를 검색 대상에 추가
				dx.push_back(x);
				dy.push_back(y);

				// 불량 화소 영역 검색
				while(!dx.empty() && !dy.empty()) {
					if(*checkImage.ptr<uchar>(dy.front(), dx.front()) != 0) {
						// 불량화소 영역 체크 (255를 0으로 변경)
						*checkImage.ptr<uchar>(dy.front(), dx.front()) = 0;

						// 불량화소 영역 갱신
						if(tempDefect.top > dy.front()) tempDefect.top = dy.front();
						if(tempDefect.bottom < dy.front()) tempDefect.bottom = dy.front();
						if(tempDefect.left > dx.front()) tempDefect.left = dx.front();
						if(tempDefect.right < dx.front()) tempDefect.right = dx.front();

						// 주변 화소를 검색 대상으로 추가
						if(dx.front() - 1 >= 0)					{dx.push_back(dx.front() - 1); dy.push_back(dy.front());}
						if(dx.front() + 1 < checkImage.cols)	{dx.push_back(dx.front() + 1); dy.push_back(dy.front());}
						if(dy.front() - 1 >= 0)					{dx.push_back(dx.front()); dy.push_back(dy.front() - 1);}
						if(dy.front() + 1 < checkImage.rows)	{dx.push_back(dx.front()); dy.push_back(dy.front() + 1);}
					}

					//검색이 끝난 현재 화소를 검색 대상에서 제외
					dx.pop_front(); dy.pop_front();
				}
				
				//탐색이 끝난 불량 화소 영역을 큐에 저장.
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
	// 초기화
	deque<DefectImage> mergedAreaInfo;

	// this->DefectInfo가 빌 때까지 합병한다.
	// 합병이 완료된 영역은 mergedAreaInfo에 저장한다.
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

void PanelImage::findCellPoint(void) {
	//코너 검출을 위한 영상 준비
	Mat subImage = this->srcImage(Rect((this->srcImage.cols - this->cellSizeX) / 2, 
										(this->srcImage.rows - this->cellSizeY) / 2,
										this->cellSizeX, this->cellSizeY));
	Mat grayImage = subImage.clone();
	Mat cannyImage = subImage.clone();
	Mat dst = subImage.clone();
	cvtColor(subImage, grayImage, CV_RGB2GRAY);
	Canny(grayImage, cannyImage, 50, 200, 3);

	int *edgeAccDataX = (int*)malloc(sizeof(int) * cannyImage.cols);
	int *edgeAccDataY = (int*)malloc(sizeof(int) * cannyImage.rows);

	for(int i = 0; i < cannyImage.cols; i++) {edgeAccDataX[i] = 0;}
	for(int i = 0; i < cannyImage.rows; i++) {edgeAccDataY[i] = 0;}

	for(int y = 0; y < cannyImage.rows; y++) {
		for(int x = 0; x < cannyImage.cols; x++) {
			if(*cannyImage.ptr<uchar>(y, x) == 255) {
				edgeAccDataX[x]++;
				edgeAccDataY[y]++;
			}
		}
	}

	////////////////////
	//	최대값 탐색

	//	초기화
	int maxValIndexX = 0;
	int maxValX = edgeAccDataX[grayImage.cols - 1] + edgeAccDataX[0] + edgeAccDataX[1];
	int maxValIndexY = 0;
	int maxValY = edgeAccDataY[grayImage.rows - 1] + edgeAccDataY[0] + edgeAccDataY[1];
	int tempSum = 0;

	////
	//	Y축
	printf("<Y>\n");

	for(int i = 1; i < grayImage.rows - 1; i++) {
		tempSum = edgeAccDataY[i - 1] + edgeAccDataY[i] + edgeAccDataY[i + 1];
		if(maxValY < tempSum) {
			maxValIndexY = i;
			maxValY = tempSum;
		}
		//printf("%d\t%d\n", edgeAccDataY[i], tempSum);
	}
	
	tempSum = edgeAccDataY[grayImage.rows - 2] + edgeAccDataY[grayImage.rows - 1] + edgeAccDataY[0];
	if(maxValY < tempSum) {
		maxValIndexY = grayImage.rows - 1;
		maxValY = tempSum;
	}

	////
	//	X축

	printf("<X>\n");
	for(int i = 1; i < grayImage.cols - 1; i++) {
		tempSum = edgeAccDataX[i - 1] + edgeAccDataX[i] + edgeAccDataX[i + 1];
		if(maxValX < tempSum) {
			maxValIndexX = i;
			maxValX = tempSum;
		}
		//printf("%d\t%d\n", edgeAccDataX[i], tempSum);
	}

	tempSum = edgeAccDataX[grayImage.cols - 2] + edgeAccDataX[grayImage.cols - 1] + edgeAccDataX[0];
	if(maxValX < tempSum) {
		maxValIndexX = grayImage.cols - 1;
		maxValX = tempSum;
	}

	//printf("%d %d\n", maxValIndexX, maxValIndexY);

	maxValIndexX += this->cellSizeY / 100;
	maxValIndexY += this->cellSizeX / 100;

	//line(subImage, Point(0, maxValIndexY), Point(cannyImage.cols, maxValIndexY), Scalar(255, 0, 0));
	//line(subImage, Point(maxValIndexX, 0), Point(maxValIndexX, cannyImage.rows), Scalar(255, 0, 0));

	//printf("%d %d\n", maxValIndexX, maxValIndexY);

	addWeighted(grayImage, 0.5, cannyImage, 0.5, 0.0, dst);

	this->cellPointX = ((this->srcImage.cols - this->cellSizeX) / 2 + maxValIndexX) % this->cellSizeX;
	this->cellPointY = ((this->srcImage.rows - this->cellSizeY) / 2 + maxValIndexY) % this->cellSizeY;

	//printf("%d %d\n", this->cellPointX, this->cellPointY);
	
	//imshow("test1", subImage);
	//imshow("test2", dst);

	cvWaitKey(0);
}

void PanelImage::findDefectCell(void) {
	Mat sourceImage = this->srcImage.clone();
	Mat dstImage = this->binImage.clone();

	int cellNumX = this->srcImage.cols / this->cellSizeX + 1;
	int cellNumY = this->srcImage.rows / this->cellSizeY + 1;

	deque<DefectImage>::iterator iterPos;

	for(iterPos = this->DefectInfo.begin(); iterPos != this->DefectInfo.end(); ++iterPos) {
		DefectImage tempImage;
		DefectImage range(0, 0, 0, 0);

		range.right	= (((*iterPos).right - this->cellPointX) / this->cellSizeX) + 1;
		range.left = (((*iterPos).left - this->cellPointX) / this->cellSizeX) + 1;
		range.bottom = (((*iterPos).bottom - this->cellPointY) / this->cellSizeY) + 1;
		range.top = (((*iterPos).top - this->cellPointY) / this->cellSizeY) + 1;

		tempImage.right = this->cellSizeX * range.right + this->cellPointX;
		tempImage.left = this->cellSizeX * range.left + this->cellPointX;
		tempImage.bottom = this->cellSizeY * range.bottom + this->cellPointY;
		tempImage.top = this->cellSizeY * range.top + this->cellPointY;

		if((*iterPos).top <= this->cellPointY) {
			tempImage.top = 0;
			range.top = 0;
		}
		if((*iterPos).bottom >= this->srcImage.rows){
			tempImage.bottom = this->srcImage.rows - 1;
			range.bottom = this->srcImage.rows / this->cellSizeY;
		}
		if((*iterPos).left <= this->cellPointX){
			tempImage.left = 0;
			range.left = 0;
		}
		if((*iterPos).right >= this->srcImage.cols) {
			tempImage.right = this->srcImage.cols - 1;
			range.right = this->srcImage.cols / this->cellSizeX;
		}
		else if((*iterPos).right <= this->cellPointX) {
			tempImage.right = this->cellPointX;
			range.right = 0;
		}

		for(int i = range.top; i <= range.bottom; i++) {
			for(int j = range.left; j <= range.right; j++) {
				this->defectCellIdx[i * cellNumX + j] = true;
				printf("%d %d %d\n", i, j, i * cellNumX + j);
				rectangle(sourceImage, 
						  Point(MAX(this->cellPointX * (j > 0 ? 1 : 0) + (j - 1) * this->cellSizeX, 0), 
								MAX(this->cellPointY * (i > 0 ? 1 : 0) + (i - 1) * this->cellSizeY, 0)),
						  Point(MIN(this->cellPointX + (j) * this->cellSizeX, this->srcImage.cols), 
								MIN(this->cellPointY + (i) * this->cellSizeY, this->srcImage.rows)),
						  cv::Scalar(0.0, 0.0, 255.0), 1);
			}
		}

		//Point a = Point(tempImage.left, tempImage.top);
		//Point b = Point(tempImage.right, tempImage.bottom);
		//rectangle(sourceImage, a, b, cv::Scalar(0.0, 0.0, 255.0), 1);
		//printf("%d\n", DAIterPos->top + DAIterPos->bottom);
	}

	imshow("defect cell area", sourceImage);

	cvWaitKey(0);
}

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