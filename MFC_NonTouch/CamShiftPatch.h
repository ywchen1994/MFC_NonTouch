#pragma once
#include"cv.h"
#include"highgui.h"
using namespace cv;
class CamShiftPatch
{
public:
	CamShiftPatch();
	~CamShiftPatch();
	Rect TrackWindow = cv::Rect(0, 0, 50, 50);
	int vmin = 10;
	int vmax = 256;
	int smin = 30;
	Mat CreatHist(Mat image, Rect ROIwindow, OutputArray histData);
	RotatedRect GetTrackBox(Mat image, InputArray histData);
};

