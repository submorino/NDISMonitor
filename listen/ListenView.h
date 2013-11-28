// ListenView.h : interface of the CListenView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_LISTENVIEW_H__D1CFEB31_5835_4FAE_843A_E01099A6C78F__INCLUDED_)
#define AFX_LISTENVIEW_H__D1CFEB31_5835_4FAE_843A_E01099A6C78F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CListenView : public CFormView
{
protected: // create from serialization only
	CListenView();
	DECLARE_DYNCREATE(CListenView)

public:
	//{{AFX_DATA(CListenView)
	enum { IDD = IDD_LISTEN_FORM };
	CListCtrl	m_listshow;
	//}}AFX_DATA

// Attributes
public:
	CListenDoc* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CListenView)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnInitialUpdate(); // called first time after construct
	//}}AFX_VIRTUAL

// Implementation
public:
	void UpdataList(BYTE * pNextPacket,int count);
	void DeleteListAll();	
	virtual ~CListenView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CListenView)
	afx_msg void OnDblclkListShow(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in ListenView.cpp
inline CListenDoc* CListenView::GetDocument()
   { return (CListenDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LISTENVIEW_H__D1CFEB31_5835_4FAE_843A_E01099A6C78F__INCLUDED_)
