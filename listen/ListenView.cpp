// ListenView.cpp : implementation of the CListenView class
//

#include "stdafx.h"
#include "Listen.h"

#include "MyDefine.h"
#include "ListenDoc.h"
#include "ListenView.h"
#include "PacketAnalysisDlg.h"

#include <winioctl.h>
#include "..\common\ioctl.h"
#include "..\common\EthernetHeader.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CListenView

IMPLEMENT_DYNCREATE(CListenView, CFormView)

BEGIN_MESSAGE_MAP(CListenView, CFormView)
	//{{AFX_MSG_MAP(CListenView)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_SHOW, OnDblclkListShow)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CListenView construction/destruction

CListenView::CListenView()
	: CFormView(CListenView::IDD)
{
	//{{AFX_DATA_INIT(CListenView)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// TODO: add construction code here

}

CListenView::~CListenView()
{
}

void CListenView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CListenView)
	DDX_Control(pDX, IDC_LIST_SHOW, m_listshow);
	//}}AFX_DATA_MAP
}

BOOL CListenView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CFormView::PreCreateWindow(cs);
}

void CListenView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();
	GetParentFrame()->RecalcLayout();
	ResizeParentToFit();
	RECT rc;
	m_listshow.GetClientRect(&rc);

	int nWidth = rc.right - rc.left - 110;
	int nLength = nWidth/6;

	m_listshow.InsertColumn(0, "时间", LVCFMT_LEFT, 125, -1);
	m_listshow.InsertColumn(1, "协议类型", LVCFMT_CENTER, 75, -1);
	m_listshow.InsertColumn(2, "源MAC", LVCFMT_LEFT, 115, -1);
	m_listshow.InsertColumn(3, "目的MAC", LVCFMT_LEFT, 115, -1);
	m_listshow.InsertColumn(4, "源IP", LVCFMT_LEFT, 105, -1);
	m_listshow.InsertColumn(5, "目的IP", LVCFMT_LEFT, 105, -1);
	m_listshow.InsertColumn(6, "包大小", LVCFMT_LEFT,80, -1);
	m_listshow.SetExtendedStyle(LVS_EX_FULLROWSELECT);


}

/////////////////////////////////////////////////////////////////////////////
// CListenView diagnostics

#ifdef _DEBUG
void CListenView::AssertValid() const
{
	CFormView::AssertValid();
}

void CListenView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}

CListenDoc* CListenView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CListenDoc)));
	return (CListenDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CListenView message handlers

void CListenView::UpdataList(BYTE *pNextPacket, int count)
{
	CString sNowTime=GetNowTime();
	CString str_srcMAC,str_dstMAC;
	CString srcIP,dstIP;
	CString csPacketSize,csProtoclType;
	int curItem = count-1;
	BYTE *ppacket = (BYTE *)pNextPacket+2;

	BYTE srcMAC[6],dstMAC[6];
	for(int i=0;i<6;i++)
	{
		dstMAC[i] = *(ppacket+i);
		srcMAC[i] = *(ppacket+6+i);
	}	
	str_dstMAC = BytestoString(dstMAC,6*2+5);
	str_dstMAC.MakeUpper();
	str_srcMAC = BytestoString(srcMAC,6*2+5);
	str_srcMAC.MakeUpper();
	
	ppacket += 12;
	csPacketSize.Format("%dbyte(s)",*(USHORT*)pNextPacket);//pnextpacket->dwDataLength;
	if((*ppacket)==0x08 && *(ppacket+1) == 0x00)
	{
		srcIP = GetIPFromBytes(ppacket+14,4);
		dstIP = GetIPFromBytes(ppacket+18,4);
		csProtoclType+=QueryProtocol(*(ppacket+11));	
	}
	else if((*ppacket)==0x08 && *(ppacket+1)==0x06)
	{
		csProtoclType+="ARP";
		srcIP="?.?.?.?";
		dstIP="?.?.?.?";
	}
	else if((*ppacket)==0x08 && *(ppacket+1)==0x35)
	{
		csProtoclType+="RARP";
		srcIP="?.?.?.?";
		dstIP="?.?.?.?";
	}
	else if((*ppacket)==0x81 && *(ppacket+1)==0x37)
	{
		csProtoclType+="Novell IPX";
		srcIP="?.?.?.?";
		dstIP="?.?.?.?";
	}
	else if((*ppacket)==0x80 && *(ppacket+1)==0x9B)
	{
		csProtoclType+="Apple Talk";
		srcIP="?.?.?.?";
		dstIP="?.?.?.?";
	}
	else
	{
		csProtoclType+="其它帧格式";
		srcIP="?.?.?.?";
		dstIP="?.?.?.?";
	}
	
	m_listshow.InsertItem(curItem,sNowTime);
	m_listshow.SetItemText(curItem,1,csProtoclType);
	m_listshow.SetItemText(curItem,2,str_srcMAC);
	m_listshow.SetItemText(curItem,3,str_dstMAC);
	m_listshow.SetItemText(curItem,4,srcIP);
	m_listshow.SetItemText(curItem,5,dstIP);
	m_listshow.SetItemText(curItem,6,csPacketSize);
}

void CListenView::DeleteListAll()
{
	m_listshow.DeleteAllItems();
}

void CListenView::OnDblclkListShow(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
 	CListenDoc *pDoc = (CListenDoc *)GetDocument();
 	int sel = m_listshow.GetSelectionMark();
	if(sel>=0)
 	{
	CPacketAnalysisDlg pktAnalysis;
	pktAnalysis.m_pPacketSelected = pDoc->m_storedPacket[sel];
 	pktAnalysis.DoModal();
 	}	
	*pResult = 0;
}
