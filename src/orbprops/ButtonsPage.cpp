// ButtonsPage.cpp : implementation file
//

#include "stdafx.h"
#include "OrbProps.h"
#include "ButtonsPage.h"

static char* ChordNames[] = {"No Buttons", "Button A", "Button B", "Button A&B"};

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CButtonsPage property page

IMPLEMENT_DYNCREATE(CButtonsPage, CPropertyPage)

CButtonsPage::CButtonsPage() : CPropertyPage(CButtonsPage::IDD)
{
	//{{AFX_DATA_INIT(CButtonsPage)
	m_fEnableChording = FALSE;
	m_csSelectedChord = _T("No Buttons");
	m_csButtonA = _T("");
	m_csButtonB = _T("");
	m_csButtonC = _T("");
	m_csButtonD = _T("");
	m_csButtonE = _T("");
	m_csButtonF = _T("");
	//}}AFX_DATA_INIT
}

CButtonsPage::~CButtonsPage()
{
}

void CButtonsPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CButtonsPage)
	DDX_Check(pDX, IDC_ENABLE_CHORDING, m_fEnableChording);
	DDX_CBString(pDX, IDC_SelectedChord, m_csSelectedChord);
	DDX_CBString(pDX, IDC_BUTTONA, m_csButtonA);
	DDX_CBString(pDX, IDC_BUTTONB, m_csButtonB);
	DDX_CBString(pDX, IDC_BUTTONC, m_csButtonC);
	DDX_CBString(pDX, IDC_BUTTOND, m_csButtonD);
	DDX_CBString(pDX, IDC_BUTTONE, m_csButtonE);
	DDX_CBString(pDX, IDC_BUTTONF, m_csButtonF);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CButtonsPage, CPropertyPage)
	//{{AFX_MSG_MAP(CButtonsPage)
	ON_BN_CLICKED(IDC_ENABLE_CHORDING, OnEnableChording)
	ON_CBN_SELCHANGE(IDC_SelectedChord, OnChangeSelectedChord)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CButtonsPage message handlers

BOOL CButtonsPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
    SetOrientation(m_iOrientation);

    CComboBox *pCBChord = (CComboBox *)GetDlgItem(IDC_SelectedChord);
    pCBChord->AddString(ChordNames[0]);
    pCBChord->AddString(ChordNames[1]);
    pCBChord->AddString(ChordNames[2]);
    pCBChord->AddString(ChordNames[3]);

    SetButtonNames();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}



// changes orientation of axes and bitmap
void CButtonsPage::SetOrientation(int orientation)
{
    m_iOrientation = orientation;
    CStatic *pImage = (CStatic *)GetDlgItem(IDC_BUTTONS_IMAGE);
    ChangeStaticBitmap(pImage, (m_iOrientation == ORIENT_VERTICAL) ? IDB_VertButtons : IDB_HorizButtons);
}

BOOL CButtonsPage::OnSetActive() 
{
    SetOrientation(m_iOrientation);
	
	return CPropertyPage::OnSetActive();
}

void CButtonsPage::OnEnableChording() 
{
    CButton *pCBEnableChord = (CButton *)GetDlgItem(IDC_ENABLE_CHORDING);
    m_fEnableChording = pCBEnableChord->GetCheck();
    SetButtonNames();

    // turn on Apply button
    SetModified(TRUE);	
}

void CButtonsPage::OnChangeSelectedChord() 
{
	CComboBox *pCBChord = (CComboBox *)GetDlgItem(IDC_SelectedChord);
    pCBChord->GetLBText(pCBChord->GetCurSel(), m_csSelectedChord);
    SetButtonNames();
}


// change button names as appropriate
void CButtonsPage::SetButtonNames()
{
	CComboBox *pCBChord = (CComboBox *)GetDlgItem(IDC_SelectedChord);
    // clear out all the button names
    CComboBox *pCBA = (CComboBox *)GetDlgItem(IDC_BUTTONA);
    CComboBox *pCBB = (CComboBox *)GetDlgItem(IDC_BUTTONB);
    CComboBox *pCBC = (CComboBox *)GetDlgItem(IDC_BUTTONC);
    CComboBox *pCBD = (CComboBox *)GetDlgItem(IDC_BUTTOND);
    CComboBox *pCBE = (CComboBox *)GetDlgItem(IDC_BUTTONE);
    CComboBox *pCBF = (CComboBox *)GetDlgItem(IDC_BUTTONF);
    pCBA->ResetContent();
    pCBB->ResetContent();
    pCBC->ResetContent();
    pCBD->ResetContent();
    pCBE->ResetContent();
    pCBF->ResetContent();

    // enable disable pulldowns depending on enable chording checkbox
    if (m_fEnableChording) {
      pCBA->EnableWindow(FALSE);
      pCBB->EnableWindow(FALSE);
      pCBChord->EnableWindow();
    } else {
      pCBA->EnableWindow();
      pCBB->EnableWindow();
      pCBChord->EnableWindow(FALSE);
    }

    // change the display in all the button pulldowns
    if (m_fEnableChording) {
      pCBA->AddString("Button A");
      pCBB->AddString("Button B");
      if (!strcmp(m_csSelectedChord, ChordNames[0])) {
        pCBC->AddString("Joy1");
        pCBD->AddString("Joy2");
        pCBE->AddString("Joy3");
        pCBF->AddString("Joy4");
      } else if (!strcmp(m_csSelectedChord, ChordNames[1])) {
        pCBC->AddString("Joy5");
        pCBD->AddString("Joy6");
        pCBE->AddString("Joy7");
        pCBF->AddString("Joy8");
      } else if (!strcmp(m_csSelectedChord, ChordNames[2])) {
        pCBC->AddString("Joy9");
        pCBD->AddString("Joy10");
        pCBE->AddString("Joy11");
        pCBF->AddString("Joy12");
      } else if (!strcmp(m_csSelectedChord, ChordNames[3])) {
        pCBC->AddString("Joy13");
        pCBD->AddString("Joy14");
        pCBE->AddString("Joy15");
        pCBF->AddString("Joy16");
      }
    } else {
      pCBA->AddString("Joy1");
      pCBB->AddString("Joy2");
      pCBC->AddString("Joy3");
      pCBD->AddString("Joy4");
      pCBE->AddString("Joy5");
      pCBF->AddString("Joy6");
    }

    // set the first selection since we don't set custom button configs yet
      pCBA->SetCurSel(0);
      pCBB->SetCurSel(0);
      pCBC->SetCurSel(0);
      pCBD->SetCurSel(0);
      pCBE->SetCurSel(0);
      pCBF->SetCurSel(0);
}
