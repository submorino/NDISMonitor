#if !defined(AFX_SETFILTERDLG_H__7C09318B_0249_4F54_B61E_FE3B6CAFDD21__INCLUDED_)
#define AFX_SETFILTERDLG_H__7C09318B_0249_4F54_B61E_FE3B6CAFDD21__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SetFilterDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSetFilterDlg dialog

class CSetFilterDlg : public CDialog
{
// Construction
public:
	CSetFilterDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSetFilterDlg)
	enum { IDD = IDD_DIALOG_FILT };
	CIPAddressCtrl	m_ipaddress_src;
	CIPAddressCtrl	m_ipaddress_dest;
	CComboBox	m_combo_prottype;
	UINT	m_port_dest;
	UINT	m_port_src;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSetFilterDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
public:
	UINT m_ipAddr_dest;
	UINT m_ipAddr_src;
	BYTE m_prottype;
	USHORT m_uport_dest;
	USHORT m_uport_src;

protected:

	// Generated message map functions
	//{{AFX_MSG(CSetFilterDlg)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SETFILTERDLG_H__7C09318B_0249_4F54_B61E_FE3B6CAFDD21__INCLUDED_)
