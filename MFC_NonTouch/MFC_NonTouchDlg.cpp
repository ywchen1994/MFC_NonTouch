
// MFC_NonTouchDlg.cpp : 實作檔
//

#include "stdafx.h"
#include "MFC_NonTouch.h"
#include "MFC_NonTouchDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMFC_NonTouchDlg 對話方塊
#pragma region GobalInitial
CamShiftPatch CMFC_NonTouchDlg::Cmft;
IplImage *CMFC_NonTouchDlg::frame_Shoulder = cvCreateImage(cvSize(640, 480), IPL_DEPTH_8U, 3);
CvCapture *CMFC_NonTouchDlg::cap;
CvCapture *CMFC_NonTouchDlg::cap2;
CvPoint CMFC_NonTouchDlg::LBtnDown = { 0 };
CvPoint CMFC_NonTouchDlg::LBtnUp = {640,480};
CascadeClassifier CMFC_NonTouchDlg::face_cascade;
Mat CMFC_NonTouchDlg::HistDataSelect;
Mat CMFC_NonTouchDlg::Img_ROI_Face;
Mat CMFC_NonTouchDlg::frame;
SYSTEMTIME  CurTime;
bool CMFC_NonTouchDlg::f_getFace = false;
bool CMFC_NonTouchDlg::f_getFromCam = false;
#pragma endregion



CMFC_NonTouchDlg::CMFC_NonTouchDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_MFC_NONTOUCH_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMFC_NonTouchDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_Image_Shoulder, m_Image_Shoulder);
	DDX_Control(pDX, IDC_Image_Face, m_Image_Face);
	DDX_Control(pDX, IDC_LIST_HR, m_LIST_HR);
	DDX_Control(pDX, IDC_LIST_BR, m_LIST_BR);
}

BEGIN_MESSAGE_MAP(CMFC_NonTouchDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_Start, &CMFC_NonTouchDlg::OnBnClickedButtonStart)
	ON_BN_CLICKED(IDC_BUTTON_Detection, &CMFC_NonTouchDlg::OnBnClickedButtonDetection)
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_BN_CLICKED(IDC_Btn_Close, &CMFC_NonTouchDlg::OnBnClickedBtnClose)
	ON_BN_CLICKED(IDC_Btn_Reset, &CMFC_NonTouchDlg::OnBnClickedBtnReset)
END_MESSAGE_MAP()


// CMFC_NonTouchDlg 訊息處理常式

BOOL CMFC_NonTouchDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 設定此對話方塊的圖示。當應用程式的主視窗不是對話方塊時，
	// 框架會自動從事此作業
	SetIcon(m_hIcon, TRUE);			// 設定大圖示
	SetIcon(m_hIcon, FALSE);		// 設定小圖示

	// TODO: 在此加入額外的初始設定
	m_Image_Face.SetWindowPos(NULL, 640, 0, 480, 360, SWP_SHOWWINDOW);
	m_Image_Shoulder.SetWindowPos(NULL, 0, 0, 640, 480, SWP_SHOWWINDOW);

	

	m_LIST_BR.ModifyStyle(0, LVS_REPORT);
	m_LIST_BR.SetExtendedStyle( LVS_EX_GRIDLINES );
	m_LIST_BR.InsertColumn(0, _T("time"));
	m_LIST_BR.InsertColumn(1, _T("BreathRate"));

	m_LIST_HR.ModifyStyle(0, LVS_REPORT);
	m_LIST_HR.SetExtendedStyle( LVS_EX_GRIDLINES );
	CRect rect;
	m_LIST_BR.GetClientRect(rect);
	m_LIST_BR.SetColumnWidth(0, rect.Width() / 2);
	m_LIST_BR.SetColumnWidth(1, rect.Width() / 2);
	
	
	m_LIST_HR.InsertColumn(0, _T("time"));
	m_LIST_HR.InsertColumn(1, _T("HeartRate"));
	m_LIST_HR.GetClientRect(rect);
	m_LIST_HR.SetColumnWidth(0, rect.Width() / 2);
	m_LIST_HR.SetColumnWidth(1, rect.Width() / 2);

	GetLocalTime(&CurTime);
	unsigned int Time = CurTime.wMonth*1000000+ CurTime.wDay*10000+ CurTime.wHour*100+ CurTime.wMinute;
	sprintf(fileNameBR, ".\\Data\\%dBR.txt", Time);
	sprintf(fileNameHR, ".\\Data\\%dHR.txt", Time);
	return TRUE;  // 傳回 TRUE，除非您對控制項設定焦點
}

// 如果將最小化按鈕加入您的對話方塊，您需要下列的程式碼，
// 以便繪製圖示。對於使用文件/檢視模式的 MFC 應用程式，
// 框架會自動完成此作業。

void CMFC_NonTouchDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 繪製的裝置內容

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 將圖示置中於用戶端矩形
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 描繪圖示
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 當使用者拖曳最小化視窗時，
// 系統呼叫這個功能取得游標顯示。
HCURSOR CMFC_NonTouchDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

#pragma region Thread
UINT CMFC_NonTouchDlg::threadFun(LPVOID LParam)
{
	CthreadParam* para = (CthreadParam*)LParam;
	CMFC_NonTouchDlg* lpview = (CMFC_NonTouchDlg*)(para->m_lpPara);
	para->m_blthreading = TRUE;

	switch (para->m_case)
	{
	case 0:
		lpview->Thread_Image_RGB(LParam);
		break;
	case 1:

		break;
	case 2:

	default:
		break;
	}

	para->m_blthreading = FALSE;
	para->m_case = 0xFF;
	return 0;

}
void CMFC_NonTouchDlg::Thread_Image_RGB(LPVOID lParam)
{
	CthreadParam * Thread_Info = (CthreadParam *)lParam;
	CMFC_NonTouchDlg * hWnd = (CMFC_NonTouchDlg *)CWnd::FromHandle((HWND)Thread_Info->hWnd);
	IplImage * img_buffer, *img_buffer2;
	img_buffer = cvCreateImage(cvSize(480, 360), IPL_DEPTH_8U, 3);
	Mat img_show;


	while (f_getFromCam)
	{
		img_buffer = cvQueryFrame(cap);
		img_buffer2 = cvQueryFrame(cap2);
		if (img_buffer2 != nullptr)
		{
			cvCopy(img_buffer2, frame_Shoulder);
			IplImage* show = cvCreateImage(cvSize(640, 480), IPL_DEPTH_8U, 3);
			cvCopy(img_buffer2, show);
			cvRectangle(show, LBtnDown, LBtnUp, CV_RGB(255, 0, 0), 2);
			hWnd->ShowImage(show, hWnd->GetDlgItem(IDC_Image_Shoulder));
			cvReleaseImage(&show);
		}
		if (img_buffer != nullptr) {
			frame = img_buffer;
			frame.copyTo(img_show);
			if (f_getFace)
			{
				RotatedRect trackBox = Cmft.GetTrackBox(frame, HistDataSelect);
				Rect r = trackBox.boundingRect();
				Mat out;
				frame.copyTo(out);
				ellipse(out, trackBox, Scalar(255, 255, 255), 3, CV_FILLED);
				if (r.y < 0)r.y = 0; if (r.x < 0)r.x = 0;
				if (r.x + r.width > frame.cols)r.width = frame.cols - r.x;
				if (r.y + r.height > frame.rows)r.height = frame.rows - r.y;
				Img_ROI_Face = frame(r);
				cv::rectangle(img_show, r, Scalar(0, 0, 255), 1);
			}
			hWnd->ShowImage(img_show, hWnd->GetDlgItem(IDC_Image_Face));
		}
	}

}
#pragma endregion


void CMFC_NonTouchDlg::LoadData()
{
	std::fstream FilterReader_a, FilterReader_b;
	FilterReader_a.open(".\\Parameter\\FilterParameter_a.txt", std::ios::in); FilterReader_b.open(".\\Parameter\\FilterParameter_b.txt", std::ios::in);
	for (unsigned int i = 0; i <19; i++) {
		double temp;
		FilterReader_a >> temp;
		a_coeff.push_back(temp);
		FilterReader_b >> temp;
		b_coeff.push_back(temp);
	} 
	FilterReader_a.close(); FilterReader_b.close();
}
void CMFC_NonTouchDlg::WriteTxt(vectord FilterData, const char filename[])
{
	std::fstream fp;
	fp.open(filename, std::ios::out| std::ios::app);

	if (fp) {
		for (int i = 0; i < FilterData.size(); i++)
		{
			fp << FilterData[i] << "\n";
		}
	}

	fp.close();
}
void CMFC_NonTouchDlg::WriteTxt(vector<Point> Time_Data, const char filename[])
{

	std::fstream fp;
	fp.open(filename, std::ios::out | std::ios::app);

	if (fp) {
		for (int i = 0; i < Time_Data.size(); i++)
		{
			fp << Time_Data[i].x <<" "<< Time_Data[i].y<< "\n";
		}
	}

	fp.close();
}
#pragma region 視窗事件
void CMFC_NonTouchDlg::ShowImage(cv::Mat Image, CWnd* pWnd)
{
	//Windows中顯示圖像存在一個4位元組對齊的問題, 也就是每一行的位元組數必須是4的倍數.
	//而Mat的資料是連續存儲的.一般Mat的資料格式為BGR, 也就是一個圖元3個位元組, 假設我的圖片一行有5個圖元, 那一行就是15個位元組, 這不符合MFC的資料對齊方式,
	//如果我們直接把Mat的data加個資料頭再顯示出來就可能會出錯.
	//手動4位元組對齊, 就是計算每行的位元組是不是4的倍數, 不是的話, 在後面補0
	//但是我們把圖片轉成RGBA之後, 一個圖元就是4個位元組, 不管你一行幾個圖元, 一直都是對齊的.

	cv::Mat imgTmp;
	CRect rect;
	pWnd->GetClientRect(&rect);
	cv::resize(Image, imgTmp, cv::Size(rect.Width(), rect.Height()));

	switch (imgTmp.channels())
	{
	case 1:
		cv::cvtColor(imgTmp, imgTmp, CV_GRAY2BGRA);
		break;
	case 3:
		cv::cvtColor(imgTmp, imgTmp, CV_BGR2BGRA);
		break;
	default:
		break;
	}
	int pixelBytes = imgTmp.channels()*(imgTmp.depth() + 1);
	BITMAPINFO bitInfo;
	bitInfo.bmiHeader.biBitCount = 8 * pixelBytes;
	bitInfo.bmiHeader.biWidth = imgTmp.cols;
	bitInfo.bmiHeader.biHeight = -imgTmp.rows;
	bitInfo.bmiHeader.biPlanes = 1;
	bitInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bitInfo.bmiHeader.biCompression = BI_RGB;
	bitInfo.bmiHeader.biClrImportant = 0;
	bitInfo.bmiHeader.biClrUsed = 0;
	bitInfo.bmiHeader.biSizeImage = 0;
	bitInfo.bmiHeader.biXPelsPerMeter = 0;
	bitInfo.bmiHeader.biYPelsPerMeter = 0;

	CDC *pDC = pWnd->GetDC();
	::StretchDIBits(
		pDC->GetSafeHdc(),
		0, 0, rect.Width(), rect.Height(),
		0, 0, rect.Width(), rect.Height(),
		imgTmp.data,
		&bitInfo,
		DIB_RGB_COLORS,
		SRCCOPY
	);
	ReleaseDC(pDC);
}
void CMFC_NonTouchDlg::OnBnClickedButtonStart()
{
	face_cascade.load("haarcascade_frontalface_alt.xml");
	cap = cvCaptureFromCAM(0);
	cap2 = cvCaptureFromCAM(1);
	f_getFromCam = true;
	m_threadPara.m_case = 0;
	m_threadPara.hWnd = m_hWnd;
	m_lpThread = AfxBeginThread(&CMFC_NonTouchDlg::threadFun, (LPVOID)&m_threadPara);
}

void CMFC_NonTouchDlg::OnBnClickedButtonDetection()
{
	LoadData();
	//多媒體計時器參數設定
	int SampleTime = 1000 / HeartRateSampleFreq;
	UINT uDelay = int(SampleTime);//m_SampleTime 為自訂的取樣時間 單位:毫秒

	UINT uResolution = 1;
	DWORD dwUser = (DWORD)this;
	UINT fuEvent = TIME_PERIODIC; //You also choose TIME_ONESHOT;

	timeBeginPeriod(uResolution); //精度1ms
	FTimerID = timeSetEvent(uDelay, uResolution, TimeProc, dwUser, fuEvent);
	UINT uDelay2 = 1000 / BreathRateSampleFreq;
	FTimerID2 = timeSetEvent(uDelay2, uResolution, TimeProc2, dwUser, fuEvent);
	LoadData();

}
void CMFC_NonTouchDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (0 <= point.x && point.x <= 640 && 0 <= point.y && point.y < 480)
	{
		LBtnDown = { point.x,point.y };
	}

	CDialogEx::OnLButtonDown(nFlags, point);
}
void CMFC_NonTouchDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	if (0 <= point.x && point.x <= 640 && 0 <= point.y && point.y < 480 && nFlags == MK_LBUTTON)
	{
		LBtnUp = { point.x ,point.y };
	}

	CDialogEx::OnMouseMove(nFlags, point);
}

#pragma endregion

#pragma region Timer
void CALLBACK TimeProc(UINT uTimerID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
	CMFC_NonTouchDlg *pointer = (CMFC_NonTouchDlg *)dwUser;
	pointer->DoEvent();
}
void CALLBACK TimeProc2(UINT uTimerID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
	CMFC_NonTouchDlg *pointer = (CMFC_NonTouchDlg *)dwUser;
	pointer->DoEvent2();
}
void CMFC_NonTouchDlg::DoEvent()
{
	if (f_getFace == false)
	{
		std::vector<Rect> faces;
		Mat frame_gray;
		cvtColor(frame, frame_gray, CV_BGR2GRAY);
		equalizeHist(frame_gray, frame_gray);
		face_cascade.detectMultiScale(frame_gray, faces, 1.2, 3, 0 | CV_HAAR_SCALE_IMAGE, cv::Size(30, 30));//FPS下降
		if (faces.size() > 0)
		{
			Cmft.CreatHist(frame, faces[0], HistDataSelect);
			Cmft.TrackWindow = faces[0];
			f_getFace = true;
		   Img_ROI_Face = frame(faces[0]);
		}
	}
	if (f_getFace)
	{
		G_Signal.push_back(MeanofGreen(Img_ROI_Face));
		CString str;
		str.Format(_T("%.1f"), (double)(G_Signal.size() * 100) / (double)(HeartRateSampleFreq*DataSec));
		SetDlgItemText(IDC_HR_Percentage, str);
		if (G_Signal.size() == (HeartRateSampleFreq*DataSec))
		{
			
			vector<complx>G_DFT(G_Signal.size());
			vector<double>G_filtfilt_out;
			filter.filtfilt(b_coeff, a_coeff, G_Signal, G_filtfilt_out);
		
			DFT(G_Signal, G_DFT);
			for (uint i = 0; i < G_DFT.size(); i++)
			{
				G_DFT[i].mag = G_DFT[i].GetMagnitude();
				G_DFT[i].frequency = (double)(i*HeartRateSampleFreq) / (double)G_DFT.size();
			}
			double HR = MaxMagititude(G_DFT, 0.8, 2.5);
			HR = HR * 60;
			GetLocalTime(&CurTime);
			unsigned int Time = CurTime.wHour * 10000 + CurTime.wMinute * 100 + CurTime.wSecond;

		
			if (HR <= 50)
			{
			 char strff[100];
			 sprintf(strff, ".\\errorData\\%.f_%d.txt", HR, Time);
			 WriteTxt(G_Signal,strff);
			}

		    str.Format(_T("%d"), Time);
			m_LIST_HR.InsertItem(0, str);
			str.Format(_T("%.f"), HR);
			m_LIST_HR.SetItemText(0,1,str);
		
			time_HR.push_back(Point(Time, HR));
			if (time_HR.size()%2==0)
			{
				
				WriteTxt(time_HR, fileNameHR);
				time_HR.resize(0); 
				m_LIST_HR.DeleteItem(3);
				m_LIST_HR.DeleteItem(2);
			}
		
			for (uint i = 0; i < G_Signal.size() - 200; i++)
				G_Signal[i] = G_Signal[i + 200];
			G_Signal.resize(G_Signal.size() - 200);
		}
	}

}
void CMFC_NonTouchDlg::DoEvent2()
{
	Rect roi_shoulder = Rect(LBtnDown.x, LBtnDown.y, LBtnUp.x - LBtnDown.x, LBtnUp.y - LBtnDown.y);
	Mat tmp = frame_Shoulder;
	Mat image_gray = tmp(roi_shoulder);
	cvtColor(image_gray, image_gray, CV_BGR2GRAY);

	threshold(image_gray, image_gray, 200, 255, CV_THRESH_OTSU);
	S_Signal.push_back(PixelCalculate(image_gray));
	CString str;
	str.Format(_T("%.1f"), (double)(S_Signal.size() * 100) / (double)(BreathRateSampleFreq*DataSec));
	SetDlgItemText(IDC_BR_Percentage, str);
	if (S_Signal.size() == (BreathRateSampleFreq*DataSec))
	{
		vector<double>threePointAverage;
		threePointAverage.push_back(S_Signal[0] / 3);
		threePointAverage.push_back((S_Signal[0] + S_Signal[1]) / 3);
		for (int i = 2; i < S_Signal.size(); i++)
		{
			double value = S_Signal[i] + S_Signal[i - 1] + S_Signal[i - 2];
			threePointAverage.push_back(value);
		}
		vector<complx> DFT_out;
		DFT(threePointAverage, DFT_out);
		for (uint i = 0; i < DFT_out.size(); i++)
		{
			DFT_out[i].mag = DFT_out[i].GetMagnitude();
			DFT_out[i].frequency = (double)(i*HeartRateSampleFreq) / (double)DFT_out.size();
		}
		double BR = MaxMagititude(DFT_out, 0.25, 0.666666);
		BR = BR * 60;
		GetLocalTime(&CurTime);
		unsigned int Time = CurTime.wHour * 10000 + CurTime.wMinute * 100 + CurTime.wSecond;
		CString str; str.Format(_T("%d"), Time);
		m_LIST_BR.InsertItem(0, str);
		str.Format(_T("%.f"), BR);
		m_LIST_BR.SetItemText(0, 1, str);

		time_BR.push_back(Point(Time,BR));
		if (time_BR.size()%2==0)
		{
			WriteTxt(time_BR, fileNameBR);
			time_BR.resize(0);
			m_LIST_BR.DeleteItem(3);
			m_LIST_BR.DeleteItem(2);

		}
		for (uint i = 0; i < S_Signal.size() - 200; i++)
		{
			S_Signal[i] = S_Signal[i + 200];
		}
		S_Signal.resize(S_Signal.size() - 200);
	}

}
#pragma endregion

#pragma region Calculate
void CMFC_NonTouchDlg::DFT(vector<double> signal, vector<complx> &out_data)
{
	int m, k;
	out_data.resize(signal.size());
	double w_cos, w_sin, angle_step, angle;

	angle_step = 2 * M_PI / signal.size();
	for (m = 0; m < signal.size(); m++)
	{
		out_data[m].real = signal[0];
		out_data[m].image = 0;
		for (k = 1; k < signal.size(); k++)
		{
			angle = (float)m*k*angle_step;
			w_cos = cos(angle);
			w_sin = sin(angle);
			out_data[m].real += signal[k] * w_cos;
			out_data[m].image += signal[k] * w_sin;
		}
	}
}
double CMFC_NonTouchDlg::MaxMagititude(vector<complx> data, double lowbound, double upbound)
{
	double maxMag_frq = 0;
	double Mag = 0;
	for (uint i = 0; i < data.size(); i++)
	{
		if ((lowbound <= data[i].frequency) && (data[i].frequency <= upbound))
		{
			if (Mag < data[i].mag)
			{
				Mag = data[i].mag;
				maxMag_frq = data[i].frequency;
			}
		}
	}
	return maxMag_frq;
}
double CMFC_NonTouchDlg::MeanofGreen(Mat img)
{
	uint sum = 0;
	double mean;
	for (uint j = 0; j < img.rows; j++)
	{
		uchar* data = img.ptr<uchar>(j);
		for (uint i = 0; i < img.cols; i++)
		{
			sum += data[i * 3 + 1];
		}
	}


	mean = (double)sum / (double)(img.rows*img.cols);
	return mean;

}
double CMFC_NonTouchDlg::PixelCalculate(Mat Image)
{
	int upstep = 0;
	int downstep = 0;
	upstep = cv::sum(Image).val[0];
	upstep = upstep / 255;
	downstep = Image.rows*Image.cols - upstep;
	return (double)(upstep - downstep) / (upstep + downstep);
}
#pragma endregion




void CMFC_NonTouchDlg::OnBnClickedBtnClose()
{
	timeKillEvent(FTimerID);
	timeKillEvent(FTimerID2);
	G_Signal.resize(0);
	S_Signal.resize(0);
	f_getFace = false;
	f_getFromCam = false;
	this->CloseWindow();
	exit(0);
	
}


void CMFC_NonTouchDlg::OnBnClickedBtnReset()
{
	f_getFace = false;
	G_Signal.resize(0);
	S_Signal.resize(0);
}
