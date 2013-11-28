// Listen.h : main header file for the LISTEN application
//

#if !defined(AFX_LISTEN_H__54ADCEF0_DADD_4947_BE98_95C4D06171AF__INCLUDED_)
#define AFX_LISTEN_H__54ADCEF0_DADD_4947_BE98_95C4D06171AF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CListenApp:
// See Listen.cpp for the implementation of this class
//

class CListenApp : public CWinApp
{
public:
	CListenApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CListenApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CListenApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LISTEN_H__54ADCEF0_DADD_4947_BE98_95C4D06171AF__INCLUDED_)
