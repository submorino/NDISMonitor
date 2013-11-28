// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "Listen.h"

#include "MainFrm.h"
#include "ListenDoc.h"
#include "ListenView.h"
#include "SetFilterDlg.h"

#include <winioctl.h>
#include "..\common\ioctl.h"
#include "..\common\EthernetHeader.h"
#include "MyDefine.h"
#include <fstream.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_COMMAND(ID_MENU_START, OnMenuStart)
	ON_COMMAND(ID_MENU_STOP, OnMenuStop)
	ON_UPDATE_COMMAND_UI(ID_MENU_START, OnUpdateMenuStart)
	ON_UPDATE_COMMAND_UI(ID_MENU_STOP, OnUpdateMenuStop)
	ON_COMMAND(ID_MENU_PAUSE, OnMenuPause)
	ON_UPDATE_COMMAND_UI(ID_MENU_PAUSE, OnUpdateMenuPause)
	ON_COMMAND(ID_MENU_SETFILTER, OnMenuSetfilter)
	ON_COMMAND(ID_FILE_OPEN1, OnFileOpen1)
	ON_COMMAND(ID_FILE_SAVE1, OnFileSave1)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
	m_bStarted = false;
	m_bPaused = false;
	m_hThreadPause = ::CreateEvent( NULL, TRUE, FALSE, NULL );
}

CMainFrame::~CMainFrame()
{
	::CloseHandle(m_hThreadPause);
	::CloseHandle(m_hThread1);
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	// TODO: Delete these three lines if you don't want the toolbar to
	//  be dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.style   &=   ~WS_THICKFRAME; 
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
	

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers


void CMainFrame::OnMenuStart() 
{
	// TODO: Add your command handler code here
	TCHAR szOutputBuffer[20]; 
	SetEvent(m_hThreadPause);
	if(!m_bPaused)
	{
	m_hDevice = CreateFile("\\\\.\\IMDKernel", 
							GENERIC_READ | GENERIC_WRITE,
							FILE_SHARE_READ,
							NULL,
							OPEN_EXISTING,
							FILE_ATTRIBUTE_NORMAL,
							NULL);
	
	if(m_hDevice == INVALID_HANDLE_VALUE)
	{
		MyGetLastError();
		return;
	}

	DWORD dwReturn;
	
	//设置过滤条件
	OnMenuSetfilter();

	// 创建 Event 对象，初始化状态为FALSE。
	m_hEvent = CreateEvent(NULL, false, false, NULL);
	
	// 把 Event 对象传递给驱动程序
	DeviceIoControl(m_hDevice, 
					IO_REFERENCE_EVENT, 
					(LPVOID)m_hEvent,
					0, 
					NULL, 
					0, 
					&dwReturn, 
					NULL);
	
	// 取得共享内存的地址
	if(!DeviceIoControl(m_hDevice, 
						IO_GET_SHAREDMEMORY_ADDR, 
						NULL, 
						NULL, 
						szOutputBuffer, 
						sizeof(szOutputBuffer), 
						&dwReturn, 
						NULL 
						))
		MyGetLastError();
	m_psharedmemory = *((PVOID *)szOutputBuffer);

	// 启动监视线程
	unsigned long dwnThreadID;
    m_hThread1= CreateThread(NULL, 
		0, 
		RecvPacketPro, 
		this, 
		0, 
		&dwnThreadID);
	}
	m_bStarted = true;
}

void CMainFrame::OnMenuStop() 
{
	// TODO: Add your command handler code here
	ULONG dwReturn;
	Monitor_flag = false;
	CListenView *pView = (CListenView*)GetActiveView();
	CListenDoc *pDoc = (CListenDoc*)GetActiveDocument();

	
	// 关闭设备句柄，清除事件对象。
	if(m_hDevice)
	{
		if(m_hEvent)
		{
			DeviceIoControl(m_hDevice, 
				IO_STOP_MONITOR_EVENT, 
				NULL,
				0, 
				NULL, 
				0, 
				&dwReturn, 
				NULL);
			CloseHandle(m_hEvent);
		}
		CloseHandle(m_hDevice);
	}
	pDoc->m_storedPacket.RemoveAll();
	pDoc->m_storedPacket.SetSize(0);
	pView->DeleteListAll();
	m_bStarted = false;
	m_bPaused = false;
}

void CMainFrame::OnMenuPause() 
{
	// TODO: Add your command handler code here
	::ResetEvent(m_hThreadPause);
	m_bPaused = true;
	m_bStarted = false;	
}

void CMainFrame::OnUpdateMenuStart(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(!m_bStarted);
}

void CMainFrame::OnUpdateMenuStop(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(m_bStarted || m_bPaused);
}


void CMainFrame::OnUpdateMenuPause(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(m_bStarted);
}


ULONG _stdcall CMainFrame::RecvPacketPro(LPVOID lpParam)
{
	DWORD dwReturn;
	CMainFrame* pThis = (CMainFrame*)lpParam;
	CListenView *pView = (CListenView*)pThis->GetActiveView();
	CListenDoc *pDoc = pView->GetDocument();
	BYTE *pPacket = NULL;
	while(pThis->Monitor_flag != false)
	{
		::WaitForSingleObject(pThis->m_hThreadPause,INFINITE);
		if(pThis->m_hEvent)
		{
			WaitForSingleObject(pThis->m_hEvent, INFINITE);

			pPacket = new BYTE [MAX_PACKET_SIZE];
			memset(pPacket,0,MAX_PACKET_SIZE);
			memcpy(pPacket, pThis->m_psharedmemory, *(USHORT*)pThis->m_psharedmemory);

			pDoc->m_storedPacket.Add(pPacket);
			pView->UpdataList(pPacket,pDoc->m_storedPacket.GetSize());

			// 设置事件对象为非信号态
			DeviceIoControl(pThis->m_hDevice, 
				IO_RESET_EVENT, 
				NULL,
				0, 
				NULL, 
				0, 
				&dwReturn, 
				NULL);
		}
	}
	
	return true;

}

void WINAPI CMainFrame::MyGetLastError()
{
	LPVOID lpMsgBuf = NULL; 
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM | 
		FORMAT_MESSAGE_IGNORE_INSERTS, 
		NULL, 
		GetLastError(), 
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR) &lpMsgBuf, 
		0, 
		NULL 
		); 
		
	MessageBox((LPCTSTR)lpMsgBuf, "Error", MB_OK | MB_ICONINFORMATION ); 
		
	LocalFree(lpMsgBuf);
}

void CMainFrame::OnMenuSetfilter() 
{
	// TODO: Add your command handler code here
	CSetFilterDlg setfilterdlg;
	BYTE filt[13] = {0};
	BYTE *pfilt = filt;
	CString csTmp;
	DWORD dwReturn;

	if(setfilterdlg.DoModal() == IDOK)
	{
		*filt = setfilterdlg.m_prottype;

		*(UINT*)(filt+1) = setfilterdlg.m_ipAddr_src;

		*(USHORT*)(pfilt+5) = setfilterdlg.m_uport_src;

		*(DWORD*)(filt+7) = setfilterdlg.m_ipAddr_dest;

		*(USHORT*)(pfilt+11) = setfilterdlg.m_uport_dest;

		DeviceIoControl(m_hDevice, 
			IO_SET_FILTER, 
			(LPVOID)filt,
			13, 
			NULL, 
			0, 
			&dwReturn, 
			NULL);
	}
}

void CMainFrame::OnFileOpen1() 
{
	// TODO: Add your command handler code here
	CFileDialog fdlg(1,"txt","untitled 1",OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,"*.txt||",NULL);
	if(fdlg.DoModal() == IDOK)
	{
		CString csPathName;
		CString csFileName;
		csPathName = fdlg.GetPathName();
		csFileName = fdlg.GetFileName();
		csPathName = csPathName.Left(csPathName.ReverseFind('\\'));
		ShellExecute(NULL,NULL,csFileName,NULL,csPathName,SW_SHOW);
	}

}

void CMainFrame::OnFileSave1() 
{
	// TODO: Add your command handler code here
	CListenDoc *pDoc = (CListenDoc*)GetActiveDocument();
	CFileDialog fdlg(0,"txt","untitled 1",OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,"*.txt||",NULL);
	
	if(fdlg.DoModal() == IDOK)
	{
		CString csPath;
		csPath = fdlg.GetPathName();
		ofstream of(csPath,ios::out);
		
		int lines,i;
		CString tmp,csData,csDatatoChar,csShowData;
		
		for(int j = 0; j < pDoc->m_storedPacket.GetSize(); j++)
		{
			BYTE *pdata = pDoc->m_storedPacket[j] + 2;
			int dwpacketLength = *(USHORT*)(pDoc->m_storedPacket[j]);
			lines = dwpacketLength/16 + (dwpacketLength%16?1:0);
			
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
				csShowData+=csData+csDatatoChar+"\n";
				of<<csShowData;
				
				csData.Empty();
				csDatatoChar.Empty();
				csShowData.Empty();
			}
			of<<"\n\n";
		}
	}
}
