// PacketAnalysisDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Listen.h"
#include "PacketAnalysisDlg.h"

#include "..\common\EthernetHeader.h"
#include "MyDefine.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPacketAnalysisDlg dialog


CPacketAnalysisDlg::CPacketAnalysisDlg(CWnd* pParent /*=NULL*/)
: CDialog(CPacketAnalysisDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPacketAnalysisDlg)
	// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CPacketAnalysisDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPacketAnalysisDlg)
	DDX_Control(pDX, IDC_LIST_ANALYSIS, m_list_analysis);
	DDX_Control(pDX, IDC_EDIT_DATASHOW, m_edit_dataShow);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPacketAnalysisDlg, CDialog)
//{{AFX_MSG_MAP(CPacketAnalysisDlg)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPacketAnalysisDlg message handlers

void CPacketAnalysisDlg::DataShow()
{
	BYTE *pdata = (BYTE *)m_pPacketSelected+2;
	int lines,i;
	CString tmp,csData,csDatatoChar,csShowData;
	int dwpacketLength;
	dwpacketLength = *(USHORT*)m_pPacketSelected;
	lines = dwpacketLength/16+(dwpacketLength%16?1:0);
	
	for(int rows=0;rows<lines;rows++)
	{
		tmp.Format("%6.6X  ", rows*16);
		csShowData+=tmp;
		for(int cols=0;cols<16;cols++)
		{
			i = rows*16+cols;
			if(i>=dwpacketLength)
			{
				for(int j=0;j<16-cols;j++)
				{
					csData+="   ";
				}
				if(cols<8)
				{
					csData+=" ";
				}
				csData+="  ";
				break;
			}
			tmp.Format("%2.2X ",(unsigned char)pdata[i]);
			if(cols == 7)
				tmp+=" ";
			else if(cols == 15)
				tmp+="  ";
			csData+=tmp;
			
			if(pdata[i]>=32 && pdata[i]<127)
				tmp.Format("%c", (char)pdata[i]);
			else
				tmp.Format(".");
			csDatatoChar+=tmp;
		}
		csShowData+=csData+csDatatoChar+"\r\n";
		//		m_edit1.SetWindowText(csShowData);
		csData.Empty();
		csDatatoChar.Empty();
	}
	//	m_data.SetWindowText(csShowData);
	m_edit_dataShow.SetWindowText(csShowData);
	
}

BOOL CPacketAnalysisDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	DataShow();
	PacketAnalysis();
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CPacketAnalysisDlg::PacketAnalysis()
{
	CString cstemp;
	CString str_srcMAC,str_dstMAC;
	BYTE *pData = m_pPacketSelected+2;
	
	BYTE srcMAC[6],dstMAC[6];
	for(int i=0;i<6;i++)
	{
		dstMAC[i] = *(pData+i);
		srcMAC[i] = *(pData+6+i);
	}	
	str_dstMAC = BytestoString(dstMAC,6*2+5);
	str_dstMAC.MakeUpper();
	str_srcMAC = BytestoString(srcMAC,6*2+5);
	str_srcMAC.MakeUpper();
	m_list_analysis.AddString("Ethernet Base Header");
	m_list_analysis.AddString("   Dest MAC address = "+str_dstMAC);	
	m_list_analysis.AddString("   Src MAC address = "+str_srcMAC);
	
	pData += 12;
	if(ntohs(*(USHORT*)pData)>1500)
	{
		IP_HEADER *pIpHdr;
		m_list_analysis.AddString("Ethernet II Header");
		if((*pData)==0x08 && *(pData+1) == 0x00)
		{	
			m_list_analysis.AddString("   Proctocol type = IP(0x0800)");
			m_list_analysis.AddString("IP Network Protocol Header");
			pIpHdr = (IP_HEADER *)(pData+2);
			
			cstemp.Format("%d",(pIpHdr->h_verlen >> 4)&0xf);
			m_list_analysis.AddString("   IP version = "+cstemp);
			
			cstemp.Format("%2.2x",(pIpHdr->h_verlen)&0x7);
			cstemp.MakeUpper();
			m_list_analysis.AddString("   IHL = 0x"+cstemp);
			
			cstemp.Format("%d",(pIpHdr->tos >> 5)&0xf);
			m_list_analysis.AddString("   Tos Precedence = "+cstemp);
			
			cstemp = (pIpHdr->tos >> 4)&0x1?"1":"0";
			m_list_analysis.AddString("   Tos Delay = "+cstemp);
			
			cstemp = (pIpHdr->tos >> 3)&0x1?"1":"0";
			m_list_analysis.AddString("   Tos Throughput = "+cstemp);
			
			cstemp = (pIpHdr->tos >> 2)&0x1?"1":"0";
			m_list_analysis.AddString("   Tos Reliability = "+cstemp);
			
			cstemp = (pIpHdr->tos >> 1)&0x1?"1":"0";
			m_list_analysis.AddString("   Tos Monetary Cost = "+cstemp);
			
			cstemp.Format("%4.4x",ntohs(pIpHdr->total_len));
			cstemp.MakeUpper();
			m_list_analysis.AddString("   Total Length = 0x"+cstemp);
			
			cstemp.Format("%4.4x",ntohs(pIpHdr->ident));
			cstemp.MakeUpper();
			m_list_analysis.AddString("   Identification = 0x"+cstemp);
			
			cstemp = (pIpHdr->frag_and_flags >> 14)&0x1?"1":"0";
			m_list_analysis.AddString("   Don't Fragment = "+cstemp);
			
			cstemp = (pIpHdr->frag_and_flags >> 13)&0x1?"1":"0";
			m_list_analysis.AddString("   More Fragment = "+cstemp);
			
			cstemp.Format("%4.4x",ntohs(pIpHdr->frag_and_flags & 0x1fff));
			cstemp.MakeUpper();
			m_list_analysis.AddString("   Fragment Offset = 0x"+cstemp);
			
			cstemp.Format("%2.2x",pIpHdr->ttl);
			cstemp.MakeUpper();
			m_list_analysis.AddString("   TTL = 0x"+cstemp);
			
			cstemp = QueryProtocol(pIpHdr->proto);
			m_list_analysis.AddString("   Protocol = "+cstemp);
			
			cstemp.Format("%4.4x",ntohs(pIpHdr->checksum));
			cstemp.MakeUpper();
			m_list_analysis.AddString("   Head Checksum = 0x"+cstemp);
			
			cstemp.Format("%8.8x",ntohl(pIpHdr->sourceIP));
			cstemp.MakeUpper();
			m_list_analysis.AddString("   Source IP Address = "+GetIPFromBytes(pData+14,4)+"(0x"+cstemp+")");
			
			cstemp.Format("%8.8x",ntohl(pIpHdr->destIP));
			cstemp.MakeUpper();
			m_list_analysis.AddString("   Destination IP Address = "+GetIPFromBytes(pData+18,4)+"(0x"+cstemp+")");			
			// ICMP
			if(*(pData+11) == 1)
			{
				m_list_analysis.AddString("ICMP Transport Protocol Header");
				
				cstemp.Format("%d",*(pData+22));
				m_list_analysis.AddString("   Type = "+cstemp);
				
				cstemp.Format("%2.2x",*(pData+23));
				cstemp.MakeUpper();
				m_list_analysis.AddString("   Code = 0x"+cstemp);
				
				cstemp.Format("%4.4x",ntohs(*(unsigned short *)(pData+24)));
				cstemp.MakeUpper();
				m_list_analysis.AddString("   Checksum = 0x"+cstemp);
			}
			//IGMP
			else if(*(pData+11) == 2)
			{
				m_list_analysis.AddString("IGMP Transport Protocol Header");
				
				cstemp.Format("%2.2x",*(pData+22));
				cstemp.MakeUpper();
				m_list_analysis.AddString("   Type = 0x"+cstemp);
				
				cstemp.Format("%2.2x",*(pData+23));
				cstemp.MakeUpper();
				m_list_analysis.AddString("   Maximal Response Time = 0x"+cstemp);
				
				cstemp.Format("%4.4x",ntohs(*(unsigned short *)(pData+24)));
				cstemp.MakeUpper();
				m_list_analysis.AddString("   Checksum = 0x"+cstemp);
				
			}
			//TCP
			else if(*(pData+11) == 6)
			{
				TCP_HEADER *pTcpHead;
				pTcpHead = (TCP_HEADER *)(pData+22);
				
				m_list_analysis.AddString("TCP Transport Protocol Header");
				
				cstemp.Format("%4.4x",ntohs(pTcpHead->th_sport));
				cstemp.MakeUpper();
				m_list_analysis.AddString("   Source Port = 0x"+cstemp);
				
				cstemp.Format("%4.4x",ntohs(pTcpHead->th_dport));
				cstemp.MakeUpper();
				m_list_analysis.AddString("   Destination Port = 0x"+cstemp);
				
				cstemp.Format("%8.8x",ntohl(pTcpHead->th_seq));
				cstemp.MakeUpper();
				m_list_analysis.AddString("   Sequence Number = 0x"+cstemp);
				
				cstemp.Format("%8.8x",ntohl(pTcpHead->th_ack));
				cstemp.MakeUpper();
				m_list_analysis.AddString("   Acknowledgment Number = 0x"+cstemp);
				
				cstemp.Format("%d",ntohs(((pTcpHead->th_lenres) >> 12)&0xf));
				m_list_analysis.AddString("   Data Offset = "+cstemp);
				
				cstemp.Format("%4.4x",ntohs(((pTcpHead->th_flag) >> 6)&0x3f));
				cstemp.MakeUpper();
				m_list_analysis.AddString("   Reserved = 0x"+cstemp);
				
				m_list_analysis.AddString("   Control Bits");
				
				cstemp = ( pTcpHead->th_flag >> 5)&0x1?"1":"0";
				m_list_analysis.AddString("     -URG = "+cstemp);
				
				cstemp = ( pTcpHead->th_flag >> 4)&0x1?"1":"0";
				m_list_analysis.AddString("     -ACK = "+cstemp);
				
				cstemp = ( pTcpHead->th_flag >> 3)&0x1?"1":"0";
				m_list_analysis.AddString("     -PSH = "+cstemp);
				
				cstemp = ( pTcpHead->th_flag >> 2)&0x1?"1":"0";
				m_list_analysis.AddString("     -RST = "+cstemp);
				
				cstemp = ( pTcpHead->th_flag >> 1)&0x1?"1":"0";
				m_list_analysis.AddString("     -SYN = "+cstemp);
				
				cstemp = pTcpHead->th_flag&0x1?"1":"0";
				m_list_analysis.AddString("     -FIN = "+cstemp);
				
				cstemp.Format("%4.4x",ntohs(pTcpHead->th_win));
				cstemp.MakeUpper();
				m_list_analysis.AddString("   Window Size = 0x"+cstemp);
				
				cstemp.Format("%4.4x",ntohs(pTcpHead->th_checksum));
				cstemp.MakeUpper();
				m_list_analysis.AddString("   Checksum = 0x"+cstemp);
				
				cstemp.Format("%4.4x",ntohs(pTcpHead->th_urp));
				cstemp.MakeUpper();
				m_list_analysis.AddString("   Urgent Pointer = 0x"+cstemp);
			}
			//UDP
			else if(*(pData+11) == 17)
			{
				UDP_HEADER *pUdpHead;
				pUdpHead = (UDP_HEADER*)(pData+22);
				
				m_list_analysis.AddString("UDP Transport Protocol Header");
				
				cstemp.Format("%4.4x",ntohs(pUdpHead->uh_sport));
				cstemp.MakeUpper();
				m_list_analysis.AddString("   Source Port = 0x"+cstemp);
				
				cstemp.Format("%4.4x",ntohs(pUdpHead->uh_dport));
				cstemp.MakeUpper();
				m_list_analysis.AddString("   Destination Port = 0x"+cstemp);
				
				cstemp.Format("%4.4x",ntohs(pUdpHead->uh_len));
				cstemp.MakeUpper();
				m_list_analysis.AddString("   Total Length = 0x"+cstemp);
				
				cstemp.Format("%4.4x",ntohs(pUdpHead->uh_checksum));
				cstemp.MakeUpper();
				m_list_analysis.AddString("   Checksum = 0x"+cstemp);
			}
			//其它IP协议;
			else
			{
				m_list_analysis.AddString("   Proctocol type = 其它IP分组协议");
			}
		}
		//ARP
		else if((*pData)==0x08 && *(pData+1)==0x06)
		{
			m_list_analysis.AddString("   Proctocol type = ARP(0x0806)");
		}
		//RARP
		else if((*pData)==0x08 && *(pData+1)==0x35)
		{
			m_list_analysis.AddString("   Proctocol type = RARP(0x0835)");
		}
		//Novell IPX
		else if((*pData)==0x81 && *(pData+1)==0x37)
		{
			m_list_analysis.AddString("   Proctocol type = Novell IPX(0x8137)");
		}
		//Apple Talk
		else if((*pData)==0x80 && *(pData+1)==0x9B)
		{
			m_list_analysis.AddString("   Proctocol type = Apple Talk(0x809B)");
		}
		else;
	}
	else
	{
		if(ntohs(*(USHORT*)(pData+2)) == 0xFFFF)
		{
			m_list_analysis.AddString("802.3 RAW Header");
			cstemp.Format("0x%4.4x",ntohs(*(unsigned short *)pData));
			cstemp.MakeUpper();
			m_list_analysis.AddString("   Total Length = "+cstemp);
			//Novell ethernet header 待解析
		}
		else if(ntohs(*(USHORT*)(pData+2)) == 0xAAAA)
		{
			m_list_analysis.AddString("Ethernet SNAP Header");
			cstemp.Format("0x%4.4x",ntohs(*(unsigned short *)pData));
			cstemp.MakeUpper();
			m_list_analysis.AddString("   Total Length = "+cstemp);
			//Novell ethernet header 待解析
		}
		else
		{
			m_list_analysis.AddString("802.3 SAP Header" );
			cstemp.Format("0x%4.4x",ntohs(*(unsigned short *)pData));
			cstemp.MakeUpper();
			m_list_analysis.AddString("   Total Length = "+cstemp);
			
			cstemp.Format("0x%2.2x",*((BYTE *)pData+2));
			cstemp.MakeUpper();
			m_list_analysis.AddString("   DSAP = "+cstemp);
			
			cstemp.Format("0x%2.2x",*((BYTE *)pData+3));
			cstemp.MakeUpper();
			m_list_analysis.AddString("   SSAP = "+cstemp);
			
			cstemp.Format("0x%2.2x",*((BYTE *)pData+4));
			cstemp.MakeUpper();
			m_list_analysis.AddString("   Control = "+cstemp);
		}
	}
	
	
	

}
