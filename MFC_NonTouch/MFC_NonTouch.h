
// MFC_NonTouch.h : PROJECT_NAME ���ε{�����D�n���Y��
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�� PCH �]�t���ɮ׫e���]�t 'stdafx.h'"
#endif

#include "resource.h"		// �D�n�Ÿ�


// CMFC_NonTouchApp: 
// �аѾ\��@�����O�� MFC_NonTouch.cpp
//

class CMFC_NonTouchApp : public CWinApp
{
public:
	CMFC_NonTouchApp();

// �мg
public:
	virtual BOOL InitInstance();

// �{���X��@

	DECLARE_MESSAGE_MAP()
};

extern CMFC_NonTouchApp theApp;