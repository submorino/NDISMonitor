#if !defined(AFX_PACKETANALYSISDLG_H__3EF4AECE_BB91_4A10_BA76_6F617DCDFD9D__INCLUDED_)
#define AFX_PACKETANALYSISDLG_H__3EF4AECE_BB91_4A10_BA76_6F617DCDFD9D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PacketAnalysisDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPacketAnalysisDlg dialog

class CPacketAnalysisDlg : public CDialog
{
// Construction
public:
	void PacketAnalysis();
	void DataShow();
	CPacketAnalysisDlg(CWnd* pParent = NULL);   // standard constructor
public:
	BYTE * m_pPacketSelected;
	

// Dialog Data
	//{{AFX_DATA(CPacketAnalysisDlg)
	enum { IDD = IDD_DIALOG_ANALYSIS };
	CListBox	m_list_analysis;
	CEdit	m_edit_dataShow;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPacketAnalysisDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPacketAnalysisDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PACKETANALYSISDLG_H__3EF4AECE_BB91_4A10_BA76_6F617DCDFD9D__INCLUDED_)
