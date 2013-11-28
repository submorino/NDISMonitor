// SetFilterDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Listen.h"
#include "SetFilterDlg.h"
#include "MyDefine.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSetFilterDlg dialog


CSetFilterDlg::CSetFilterDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSetFilterDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSetFilterDlg)

	m_port_dest = 0;
	m_port_src = 0;
	//}}AFX_DATA_INIT
}


void CSetFilterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSetFilterDlg)
	DDX_Control(pDX, IDC_IPADDRESS_SRC, m_ipaddress_src);
	DDX_Control(pDX, IDC_IPADDRESS_DEST, m_ipaddress_dest);
	DDX_Control(pDX, IDC_COMBO_PROTTYPE, m_combo_prottype);
	DDX_Text(pDX, IDC_PORT_DEST, m_port_dest);
	DDX_Text(pDX, IDC_PORT_SRC, m_port_src);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSetFilterDlg, CDialog)
	//{{AFX_MSG_MAP(CSetFilterDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSetFilterDlg message handlers

void CSetFilterDlg::OnOK() 
{
	// TODO: Add extra validation here
	UpdateData(true);
	CString csTmp;
	DWORD dwAddress;
	m_combo_prottype.GetWindowText(csTmp);
	m_prottype = QueryProtocolNum(csTmp);
	
	m_ipaddress_src.GetAddress(dwAddress);
	m_ipAddr_src = dwAddress;
	
	m_uport_src = m_port_src;
	m_uport_dest = m_port_dest;
	
	m_ipaddress_dest.GetAddress(dwAddress);
	m_ipAddr_dest = dwAddress;
	
	CDialog::OnOK();
}

BOOL CSetFilterDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	m_ipaddress_src.SetAddress(0,0,0,0);
	m_ipaddress_dest.SetAddress(0,0,0,0);
	m_combo_prottype.SetCurSel(0);
	// TODO: Add extra initialization here

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
