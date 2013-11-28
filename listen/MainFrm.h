// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__FB575F32_C083_46CE_A254_2F00E996FBA2__INCLUDED_)
#define AFX_MAINFRM_H__FB575F32_C083_46CE_A254_2F00E996FBA2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CMainFrame : public CFrameWnd
{
	
protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
public:
	BOOL m_bPaused;
	BOOL m_bStarted;
	HANDLE m_hThreadPause;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	void WINAPI MyGetLastError();
	static ULONG _stdcall RecvPacketPro(LPVOID lpParam);
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CStatusBar  m_wndStatusBar;
	CToolBar    m_wndToolBar;

	HANDLE	m_hDevice;		// 设备句柄
	HANDLE	m_hEvent;		// 与驱动通信的事件对象
	PVOID	m_psharedmemory;	// 共享内存地址
//	int		list_count;		// 列表控件当前行数
	bool	Monitor_flag;	// 监视标志
	HANDLE m_hThread1;

// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnMenuStart();
	afx_msg void OnMenuStop();
	afx_msg void OnUpdateMenuStart(CCmdUI* pCmdUI);
	afx_msg void OnUpdateMenuStop(CCmdUI* pCmdUI);
	afx_msg void OnMenuPause();
	afx_msg void OnUpdateMenuPause(CCmdUI* pCmdUI);
	afx_msg void OnMenuSetfilter();
	afx_msg void OnFileOpen1();
	afx_msg void OnFileSave1();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__FB575F32_C083_46CE_A254_2F00E996FBA2__INCLUDED_)
