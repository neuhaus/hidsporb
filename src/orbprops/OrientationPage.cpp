// OrientationPage.cpp : implementation file
//

#include "stdafx.h"
#include "OrbProps.h"
#include "OrientationPage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COrientationPage property page

IMPLEMENT_DYNCREATE(COrientationPage, CPropertyPage)

COrientationPage::COrientationPage() : CPropertyPage(COrientationPage::IDD)
{
	//{{AFX_DATA_INIT(COrientationPage)
	m_iOrientation = -1;
	//}}AFX_DATA_INIT
}

COrientationPage::~COrientationPage()
{
}

void COrientationPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COrientationPage)
	DDX_Radio(pDX, IDC_VERTICAL, m_iOrientation);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COrientationPage, CPropertyPage)
	//{{AFX_MSG_MAP(COrientationPage)
	ON_BN_CLICKED(IDC_HORIZONTAL, OnHorizontal)
	ON_BN_CLICKED(IDC_VERTICAL, OnVertical)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COrientationPage message handlers

void COrientationPage::OnHorizontal() 
{
    axesopts->SetOrientation(ORIENT_HORIZONTAL);
    buttonsopts->m_iOrientation = ORIENT_HORIZONTAL;
    gainopts->m_iOrientation = ORIENT_HORIZONTAL;
    precisionopts->m_iOrientation = ORIENT_HORIZONTAL;
    sensopts->m_iOrientation = ORIENT_HORIZONTAL;
    // turn on Apply button
    SetModified(TRUE);
}

void COrientationPage::OnVertical() 
{
    axesopts->SetOrientation(ORIENT_VERTICAL);
    buttonsopts->m_iOrientation = ORIENT_VERTICAL;
    gainopts->m_iOrientation = ORIENT_VERTICAL;
    precisionopts->m_iOrientation = ORIENT_VERTICAL;
    sensopts->m_iOrientation = ORIENT_VERTICAL;
    // turn on Apply button
    SetModified(TRUE);
}


// give this page references to pages that need to be notified when user switches orientation in the dialog
void COrientationPage::SetPageRefs(CAxesPage *a, CButtonsPage *b, CGainPage *g, CPrecisionPage *p, CSensitivityPage *s)
{
    axesopts = a;
    buttonsopts = b;
    gainopts = g;
    precisionopts = p;
    sensopts = s;
}
