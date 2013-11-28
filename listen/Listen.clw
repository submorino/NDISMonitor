; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CMainFrame
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "Listen.h"
LastPage=0

ClassCount=7
Class1=CListenApp
Class2=CListenDoc
Class3=CListenView
Class4=CMainFrame

ResourceCount=5
Resource1=IDD_LISTEN_FORM
Resource2=IDR_MAINFRAME
Class5=CAboutDlg
Resource3=IDD_DIALOG_FILT
Class6=CSetFilterDlg
Resource4=IDD_ABOUTBOX
Class7=CPacketAnalysisDlg
Resource5=IDD_DIALOG_ANALYSIS

[CLS:CListenApp]
Type=0
HeaderFile=Listen.h
ImplementationFile=Listen.cpp
Filter=N

[CLS:CListenDoc]
Type=0
HeaderFile=ListenDoc.h
ImplementationFile=ListenDoc.cpp
Filter=N

[CLS:CListenView]
Type=0
HeaderFile=ListenView.h
ImplementationFile=ListenView.cpp
Filter=D
BaseClass=CFormView
VirtualFilter=VWC
LastObject=CListenView


[CLS:CMainFrame]
Type=0
HeaderFile=MainFrm.h
ImplementationFile=MainFrm.cpp
Filter=T
BaseClass=CFrameWnd
VirtualFilter=fWC
LastObject=ID_FILE_OPEN1




[CLS:CAboutDlg]
Type=0
HeaderFile=Listen.cpp
ImplementationFile=Listen.cpp
Filter=D

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg
ControlCount=4
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308480
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889

[MNU:IDR_MAINFRAME]
Type=1
Class=CMainFrame
Command1=ID_FILE_OPEN1
Command2=ID_FILE_SAVE1
Command3=ID_APP_EXIT
Command4=ID_MENU_SETFILTER
Command5=ID_MENU_START
Command6=ID_MENU_PAUSE
Command7=ID_MENU_STOP
Command8=ID_APP_ABOUT
CommandCount=8

[ACL:IDR_MAINFRAME]
Type=1
Class=CMainFrame
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_SAVE
Command4=ID_EDIT_UNDO
Command5=ID_EDIT_CUT
Command6=ID_EDIT_COPY
Command7=ID_EDIT_PASTE
Command8=ID_EDIT_UNDO
Command9=ID_EDIT_CUT
Command10=ID_EDIT_COPY
Command11=ID_EDIT_PASTE
Command12=ID_NEXT_PANE
Command13=ID_PREV_PANE
CommandCount=13

[DLG:IDD_LISTEN_FORM]
Type=1
Class=CListenView
ControlCount=1
Control1=IDC_LIST_SHOW,SysListView32,1342242821

[TB:IDR_MAINFRAME]
Type=1
Class=?
Command1=ID_FILE_OPEN1
Command2=ID_FILE_SAVE1
Command3=ID_MENU_START
Command4=ID_MENU_PAUSE
Command5=ID_MENU_STOP
Command6=ID_APP_ABOUT
CommandCount=6

[DLG:IDD_DIALOG_FILT]
Type=1
Class=CSetFilterDlg
ControlCount=14
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_STATIC,button,1342177287
Control4=IDC_STATIC,static,1342308352
Control5=IDC_IPADDRESS_SRC,SysIPAddress32,1342242816
Control6=IDC_STATIC,static,1342308352
Control7=IDC_PORT_SRC,edit,1350631552
Control8=IDC_STATIC,button,1342177287
Control9=IDC_STATIC,static,1342308352
Control10=IDC_IPADDRESS_DEST,SysIPAddress32,1342242816
Control11=IDC_STATIC,static,1342308352
Control12=IDC_PORT_DEST,edit,1350631552
Control13=IDC_STATIC,static,1342308352
Control14=IDC_COMBO_PROTTYPE,combobox,1344339970

[CLS:CSetFilterDlg]
Type=0
HeaderFile=SetFilterDlg.h
ImplementationFile=SetFilterDlg.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC
LastObject=CSetFilterDlg

[DLG:IDD_DIALOG_ANALYSIS]
Type=1
Class=CPacketAnalysisDlg
ControlCount=2
Control1=IDC_EDIT_DATASHOW,edit,1353779204
Control2=IDC_LIST_ANALYSIS,listbox,1352728833

[CLS:CPacketAnalysisDlg]
Type=0
HeaderFile=PacketAnalysisDlg.h
ImplementationFile=PacketAnalysisDlg.cpp
BaseClass=CDialog
Filter=D
LastObject=IDC_LIST_ANALYSIS
VirtualFilter=dWC

