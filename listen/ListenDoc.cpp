// ListenDoc.cpp : implementation of the CListenDoc class
//

#include "stdafx.h"
#include "Listen.h"

#include "ListenDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CListenDoc

IMPLEMENT_DYNCREATE(CListenDoc, CDocument)

BEGIN_MESSAGE_MAP(CListenDoc, CDocument)
	//{{AFX_MSG_MAP(CListenDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CListenDoc construction/destruction

CListenDoc::CListenDoc()
{
	// TODO: add one-time construction code here

}

CListenDoc::~CListenDoc()
{
}

BOOL CListenDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CListenDoc serialization

void CListenDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CListenDoc diagnostics

#ifdef _DEBUG
void CListenDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CListenDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CListenDoc commands
