// PrecisionPage.cpp : implementation file
//

#include "stdafx.h"
#include "OrbProps.h"
#include "PrecisionPage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPrecisionPage property page

IMPLEMENT_DYNCREATE(CPrecisionPage, CPropertyPage)

CPrecisionPage::CPrecisionPage() : CPropertyPage(CPrecisionPage::IDD)
{
	//{{AFX_DATA_INIT(CPrecisionPage)
	m_csLogicalButton = _T("Joy1");
	m_iGain = 50;
	m_iSensitivity = 4;
	m_iPrecisionButton = 0;
	m_fEnablePrecision = FALSE;
	//}}AFX_DATA_INIT
    m_csButtonNames[0] = "Joy1";
    m_csButtonNames[1] = "Joy2";
    m_csButtonNames[2] = "Joy3";
    m_csButtonNames[3] = "Joy4";
    m_csButtonNames[4] = "Joy5";
    m_csButtonNames[5] = "Joy6";
    m_csButtonNames[6] = "Joy7";
    m_csButtonNames[7] = "Joy8";
    m_csButtonNames[8] = "Joy9";
    m_csButtonNames[9] = "Joy10";
    m_csButtonNames[10] = "Joy11";
    m_csButtonNames[11] = "Joy12";
    m_csButtonNames[12] = "Joy13";
    m_csButtonNames[13] = "Joy14";
    m_csButtonNames[14] = "Joy15";
    m_csButtonNames[15] = "Joy16";
}

CPrecisionPage::~CPrecisionPage()
{
}

void CPrecisionPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrecisionPage)
	DDX_CBString(pDX, IDC_LOGICAL_BUTTON, m_csLogicalButton);
	DDX_Slider(pDX, IDC_PRECISION_GAIN, m_iGain);
	DDX_Slider(pDX, IDC_PRECISION_SENSITIVITY, m_iSensitivity);
	DDX_Radio(pDX, IDC_PRECISION_A, m_iPrecisionButton);
	DDX_Check(pDX, IDC_ENABLE_PRECISION, m_fEnablePrecision);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CPrecisionPage, CPropertyPage)
	//{{AFX_MSG_MAP(CPrecisionPage)
	ON_BN_CLICKED(IDC_PRECISION_LOGICAL, OnUseLogical)
	ON_BN_CLICKED(IDC_PRECISION_A, OnPrecisionClicked)
	ON_BN_CLICKED(IDC_ENABLE_PRECISION, OnEnablePrecision)
	ON_BN_CLICKED(IDC_PRECISION_B, OnPrecisionClicked)
	ON_BN_CLICKED(IDC_PRECISION_C, OnPrecisionClicked)
	ON_BN_CLICKED(IDC_PRECISION_D, OnPrecisionClicked)
	ON_BN_CLICKED(IDC_PRECISION_E, OnPrecisionClicked)
	ON_BN_CLICKED(IDC_PRECISION_F, OnPrecisionClicked)
	ON_CBN_SELCHANGE(IDC_LOGICAL_BUTTON, OnChange)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_PRECISION_GAIN, OnSliderChange)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_PRECISION_SENSITIVITY, OnSliderChange)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrecisionPage message handlers

BOOL CPrecisionPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
    SetOrientation(m_iOrientation);

    // set logical button names
    CComboBox *pCBLogicalButtons = (CComboBox *)GetDlgItem(IDC_LOGICAL_BUTTON);
    for (int i = 0;  i < 16;  i++) {
      pCBLogicalButtons->AddString(m_csButtonNames[i]);
    }

    // set gain values from 0 to 100
    CSliderCtrl *pSGain = (CSliderCtrl *)GetDlgItem(IDC_PRECISION_GAIN);
    pSGain->SetRange(0, 100);
    pSGain->SetTicFreq(10);
    pSGain->SetPageSize(10);
    CSliderCtrl *pSSensitivity = (CSliderCtrl *)GetDlgItem(IDC_PRECISION_SENSITIVITY);
    pSSensitivity->SetRange(0, 5);
    pSSensitivity->SetTicFreq(1);

    HandleEnable();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


// changes orientation of axes and bitmap
void CPrecisionPage::SetOrientation(int orientation)
{
    m_iOrientation = orientation;
    CStatic *pImage = (CStatic *)GetDlgItem(IDC_BUTTONS_IMAGE);
    ChangeStaticBitmap(pImage, (m_iOrientation == ORIENT_VERTICAL) ? IDB_VertButtons : IDB_HorizButtons);
}

BOOL CPrecisionPage::OnSetActive() 
{
    SetOrientation(m_iOrientation);
	
	return CPropertyPage::OnSetActive();
}

void CPrecisionPage::OnUseLogical() 
{
    UpdateData();
    HandleEnable();
}

void CPrecisionPage::OnPrecisionClicked() 
{
    UpdateData();
    HandleEnable();
}

void CPrecisionPage::OnEnablePrecision() 
{
    UpdateData();
    HandleEnable();
}


// handles disabling controls out as we enable/disable
void CPrecisionPage::HandleEnable()
{
  CComboBox *pCBLogicalButtons = (CComboBox *)GetDlgItem(IDC_LOGICAL_BUTTON);
  CButton *pCB0 = (CButton *)GetDlgItem(IDC_PRECISION_A);
  CButton *pCB1 = (CButton *)GetDlgItem(IDC_PRECISION_B);
  CButton *pCB2 = (CButton *)GetDlgItem(IDC_PRECISION_C);
  CButton *pCB3 = (CButton *)GetDlgItem(IDC_PRECISION_D);
  CButton *pCB4 = (CButton *)GetDlgItem(IDC_PRECISION_E);
  CButton *pCB5 = (CButton *)GetDlgItem(IDC_PRECISION_F);
  CButton *pCBLogical = (CButton *)GetDlgItem(IDC_PRECISION_LOGICAL);
  CSliderCtrl *pS0 = (CSliderCtrl *)GetDlgItem(IDC_PRECISION_GAIN);
  CSliderCtrl *pS1 = (CSliderCtrl *)GetDlgItem(IDC_PRECISION_SENSITIVITY);

  // enable as appropriate
  pCBLogicalButtons->EnableWindow(m_fEnablePrecision && (m_iPrecisionButton == 6)); // this means logical is selected
  pCB0->EnableWindow(m_fEnablePrecision);
  pCB1->EnableWindow(m_fEnablePrecision);
  pCB2->EnableWindow(m_fEnablePrecision);
  pCB3->EnableWindow(m_fEnablePrecision);
  pCB4->EnableWindow(m_fEnablePrecision);
  pCB5->EnableWindow(m_fEnablePrecision);
  pCBLogical->EnableWindow(m_fEnablePrecision);
  pS0->EnableWindow(m_fEnablePrecision);
  pS1->EnableWindow(m_fEnablePrecision);

  // turn on Apply button
  SetModified(TRUE);	
}


// change precision button
CString CPrecisionPage::GetPrecisionButton()
{
  if (!m_fEnablePrecision)
    return ("none");
  if (m_iPrecisionButton == 6) {
    // logical precision button
    for (int i = 0;  i < 16;  i++) {
      if (m_csLogicalButton.Compare(m_csButtonNames[i]) == 0) {
        CString csButton;
        csButton.Format("logical %d", i);
        return csButton;
      }
    }
  } else {
    // physical precision button
    CString csButton;
    csButton.Format("physical %d", m_iPrecisionButton);
    return csButton;
  }
  return "unknown";
}

void CPrecisionPage::OnChange() 
{
    // turn on Apply button
    SetModified(TRUE);	
}

void CPrecisionPage::OnSliderChange(NMHDR* pNMHDR, LRESULT* pResult) 
{
	*pResult = 0;

    // turn on Apply button
    SetModified(TRUE);	
}
