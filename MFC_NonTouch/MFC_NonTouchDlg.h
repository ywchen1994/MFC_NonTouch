
// MFC_NonTouchDlg.h : ���Y��
//

#pragma once
#include"CFilter.h"
#define _USE_MATH_DEFINES
#include"math.h"
#include"CamShiftPatch.h"
#include"cv.h"
#include"highgui.h"
#include<iostream>
#include<fstream>
#define HeartRateSampleFreq 20
#define BreathRateSampleFreq 20

#pragma region �h�C��p�ɾ�
#include <windows.h>
#include <mmsystem.h>
#include "afxwin.h"
#include "afxcmn.h"
#pragma comment(lib, "winmm.lib")
void CALLBACK TimeProc(UINT uTimerID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2);
void CALLBACK TimeProc2(UINT uTimerID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2);

#pragma endregion
typedef unsigned int uint;
const uint DataSec = 30;
using namespace cv;
using namespace std;
struct CthreadParam
{
public:
	HWND hWnd;
	LPVOID m_lpPara;
	UINT   m_case;
	BOOL m_blthreading;
};
class complx
{
public:
	complx()
	{
		real = 0;
		image = 0;
	}
	complx(double re, double im)
	{
		real = re;
		image = im;
	}
	double real;
	double image;
	double mag;
	double frequency;

	double GetMagnitude()
	{
		return sqrt(pow(real, 2) + pow(image, 2));
	}
};
// CMFC_NonTouchDlg ��ܤ��
class CMFC_NonTouchDlg : public CDialogEx
{
// �غc
public:
	CMFC_NonTouchDlg(CWnd* pParent = NULL);	// �зǫغc�禡

// ��ܤ�����
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MFC_NONTOUCH_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV �䴩


// �{���X��@
protected:
	HICON m_hIcon;

	// ���ͪ��T�������禡
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:

	
#pragma region threadParameter
	CthreadParam m_threadPara;
	CWinThread*  m_lpThread;
	static UINT threadFun(LPVOID LParam);
	static CamShiftPatch Cmft;
	static CvCapture *cap, *cap2;
	static CascadeClassifier face_cascade;
	static IplImage *frame_Shoulder;
	static Mat HistDataSelect;
	static Mat Img_ROI_Face;
	static CvPoint LBtnDown, LBtnUp;
	void Thread_Image_RGB(LPVOID lParam);
#pragma endregion

#pragma region �����ƥ�
CStatic m_Image_Shoulder;
	CStatic m_Image_Face;
	afx_msg void OnBnClickedButtonStart();
	afx_msg void OnBnClickedButtonDetection();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	void ShowImage(cv::Mat Image, CWnd * pWnd);
#pragma endregion

	static Mat frame;
	vector<double> a_coeff, b_coeff;
	vector<double>G_Signal,S_Signal;
	CFilter filter;
	char fileNameBR[100];
	char fileNameHR[100];
	void LoadData();

	void WriteTxt(vectord FilterData, const char filename[]);

	void WriteTxt(vector<Point> Time_Data, const char filename[]);

	MMRESULT FTimerID;
	MMRESULT FTimerID2;
	void DoEvent();
	void DoEvent2();
	double MeanofGreen(Mat img);
	double PixelCalculate(Mat Image);
	double MaxMagititude(vector<complx> data, double lowbound, double upbound);
	void DFT(vector<double> signal, vector<complx>& out_data);
	static  bool f_getFace,f_getFromCam;
	vector<cv::Point> time_HR;
	vector<cv::Point> time_BR;
	CListCtrl m_LIST_HR;
	CListCtrl m_LIST_BR;
	afx_msg void OnBnClickedBtnClose();
	afx_msg void OnBnClickedBtnReset();
	
};
