#include "stdafx.h"
#include "CamShiftPatch.h"

CamShiftPatch::CamShiftPatch()
{
}

CamShiftPatch::~CamShiftPatch()
{
}
Mat CamShiftPatch::CreatHist(Mat image, Rect ROIwindow, OutputArray histData)
{
	Mat Image_hsv;
	cvtColor(image, Image_hsv, COLOR_BGR2HSV);

	Mat mask;
	//�ت����� S�Ȥp(����զ�) �M V�Ȥp(����¦�) ����Ӧa���o��
	inRange(
		Image_hsv,
		Scalar(0, smin, MIN(vmin, vmax)),
		Scalar(180, 256, MAX(vmin, vmax)),
		mask);

	Mat hueForHis;
	int ch[] = { 0, 0 };
	hueForHis.create(Image_hsv.size(), Image_hsv.depth());
	mixChannels(&Image_hsv, 1, &hueForHis, 1, ch, 1);//��Image_hsv����Hue�Ȯ��X�ӵ�"hue"

													 //�إ� roi
	Mat roi(hueForHis, ROIwindow);
	Mat maskroi(mask, ROIwindow);
	int hsize = 16;

	Mat histData_out;
	float hranges[] = { 0,180 };
	const float* phranges = hranges;
	calcHist(&roi, 1, 0, maskroi, histData_out, 1, &hsize, &phranges);
	normalize(histData_out, histData_out, 0, 255, CV_MINMAX);

	//�e�����
	Mat histImg = Mat::zeros(200, 320, CV_8UC3);
	histImg = Scalar::all(0);
	int binW = histImg.cols / hsize;
	Mat buf(1, hsize, CV_8UC3);
	for (int i = 0; i < hsize; i++)
		buf.at<Vec3b>(i) = Vec3b(saturate_cast<uchar>(i*180. / hsize), 255, 255);
	cvtColor(buf, buf, CV_HSV2BGR);

	for (int i = 0; i < hsize; i++)
	{
		int val = saturate_cast<int>(histData_out.at<float>(i)*histImg.rows / 255);
		rectangle(histImg, Point(i*binW, histImg.rows),
			Point((i + 1)*binW, histImg.rows - val),
			Scalar(buf.at<Vec3b>(i)), -1, 8);
	}


	histData_out.copyTo(histData);
	return histImg;
}
RotatedRect CamShiftPatch::GetTrackBox(Mat image, InputArray histData)
{
	//1. ��image�����XHue���Ϲ�
	Mat Image_hsv;
	cvtColor(image, Image_hsv, COLOR_BGR2HSV);

	Mat hueForTrack;
	int ch[] = { 0, 0 };
	hueForTrack.create(Image_hsv.size(), Image_hsv.depth());
	mixChannels(&Image_hsv, 1, &hueForTrack, 1, ch, 1);//��Image_hsv����Hue�Ȯ��X�ӵ�"hue"

													   //2. ��Hue�Ϲ� �� histData���X backproject
	Mat backproject;
	float hranges[] = { 0,180 };
	const float* phranges = hranges;
	calcBackProject(&hueForTrack, 1, 0, histData, backproject, &phranges);

	//3. backproject �n���h�@�ǭ�ϱ���¦�Υզ⪺�a��
	Mat mask;
	inRange(	//�ت����� S�Ȥp(����զ�) �M V�Ȥp(����¦�) ����Ӧa���o��
		Image_hsv,
		Scalar(0, smin, MIN(vmin, vmax)),
		Scalar(180, 256, MAX(vmin, vmax)),
		mask);
	backproject &= mask;//�ۥ[��backproject ->�N�O�Χ�backproject��Wmask

						//4. ��CamShift(�|�ݭnpackproject�A�|�o��trackWindow)
	RotatedRect trackBox = CamShift(backproject, TrackWindow,
		TermCriteria(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 10, 1));

	//�p�G���Ӥp�ΨS������
	if (TrackWindow.area() <= 1)
	{
		int cols = backproject.cols, rows = backproject.rows, r = (MIN(cols, rows) + 5) / 6;
		TrackWindow = Rect(TrackWindow.x - r, TrackWindow.y - r,
			TrackWindow.x + r, TrackWindow.y + r) &
			Rect(0, 0, cols, rows);
	}

	return trackBox;
}