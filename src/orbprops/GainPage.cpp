// GainPage.cpp : implementation file
//

#include "stdafx.h"
#include "OrbProps.h"
#include "GainPage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGainPage property page

IMPLEMENT_DYNCREATE(CGainPage, CPropertyPage)

CGainPage::CGainPage() : CPropertyPage(CGainPage::IDD)
{
	//{{AFX_DATA_INIT(CGainPage)
	m_iGain = 50;
	m_iXAxis = 50;
	m_iXRotation = 50;
	m_iYAxis = 50;
	m_iYRotation = 50;
	m_iZAxis = 50;
	m_iZRotation = 50;
	m_fMasterGain = FALSE;
	//}}AFX_DATA_INIT
    iMap[0] = &m_iXAxis;
    iMap[1] = &m_iZAxis;
    iMap[2] = &m_iYAxis;
    iMap[3] = &m_iXRotation;
    iMap[4] = &m_iZRotation;
    iMap[5] = &m_iYRotation;
}

CGainPage::~CGainPage()
{
}

void CGainPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGainPage)
	DDX_Slider(pDX, IDC_GAIN, m_iGain);
	DDX_Slider(pDX, IDC_GAIN_XAXIS, m_iXAxis);
	DDX_Slider(pDX, IDC_GAIN_XROT, m_iXRotation);
	DDX_Slider(pDX, IDC_GAIN_YAXIS, m_iYAxis);
	DDX_Slider(pDX, IDC_GAIN_YROT, m_iYRotation);
	DDX_Slider(pDX, IDC_GAIN_ZAXIS, m_iZAxis);
	DDX_Slider(pDX, IDC_GAIN_ZROT, m_iZRotation);
	DDX_Check(pDX, IDC_USE_MASTER_GAIN, m_fMasterGain);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGainPage, CPropertyPage)
	//{{AFX_MSG_MAP(CGainPage)
	ON_BN_CLICKED(IDC_USE_MASTER_GAIN, OnUseMasterGain)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_GAIN, OnChange)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_GAIN_YAXIS, OnChange)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_GAIN_ZROT, OnChange)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_GAIN_XAXIS, OnChange)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_GAIN_XROT, OnChange)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_GAIN_YROT, OnChange)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_GAIN_ZAXIS, OnChange)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGainPage message handlers

BOOL CGainPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
    SetOrientation(m_iOrientation);

    // set gain values from 0 to 100
    CSliderCtrl *pS0 = (CSliderCtrl *)GetDlgItem(IDC_GAIN_XAXIS);
    CSliderCtrl *pS1 = (CSliderCtrl *)GetDlgItem(IDC_GAIN_YAXIS);
    CSliderCtrl *pS2 = (CSliderCtrl *)GetDlgItem(IDC_GAIN_ZAXIS);
    CSliderCtrl *pS3 = (CSliderCtrl *)GetDlgItem(IDC_GAIN_XROT);
    CSliderCtrl *pS4 = (CSliderCtrl *)GetDlgItem(IDC_GAIN_YROT);
    CSliderCtrl *pS5 = (CSliderCtrl *)GetDlgItem(IDC_GAIN_ZROT);
    CSliderCtrl *pS6 = (CSliderCtrl *)GetDlgItem(IDC_GAIN);
    pS0->SetRange(0, 100);
    pS0->SetTicFreq(10);
    pS0->SetPageSize(10);
    pS1->SetRange(0, 100);
    pS1->SetTicFreq(10);
    pS1->SetPageSize(10);
    pS2->SetRange(0, 100);
    pS2->SetTicFreq(10);
    pS2->SetPageSize(10);
    pS3->SetRange(0, 100);
    pS3->SetTicFreq(10);
    pS3->SetPageSize(10);
    pS4->SetRange(0, 100);
    pS4->SetTicFreq(10);
    pS4->SetPageSize(10);
    pS5->SetRange(0, 100);
    pS5->SetTicFreq(10);
    pS5->SetPageSize(10);
    pS6->SetRange(0, 100);
    pS6->SetTicFreq(10);
    pS6->SetPageSize(10);

    HandleMasterEnable();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}



// changes orientation of axes and bitmap
void CGainPage::SetOrientation(int orientation)
{
    m_iOrientation = orientation;
    CStatic *pImage = (CStatic *)GetDlgItem(IDC_AXES_IMAGE);
    ChangeStaticBitmap(pImage, (m_iOrientation == ORIENT_VERTICAL) ? IDB_VertAxes : IDB_HorizAxes);
}

BOOL CGainPage::OnSetActive() 
{
    SetOrientation(m_iOrientation);
	
	return CPropertyPage::OnSetActive();
}

void CGainPage::OnUseMasterGain() 
{
    CButton *pCBEnableChord = (CButton *)GetDlgItem(IDC_USE_MASTER_GAIN);
    m_fMasterGain = pCBEnableChord->GetCheck();
    HandleMasterEnable();

    // turn on Apply button
    SetModified(TRUE);	
}



// handles disabling controls out as we enable/disable master sensitivity
void CGainPage::HandleMasterEnable()
{
    CSliderCtrl *pS0 = (CSliderCtrl *)GetDlgItem(IDC_GAIN_XAXIS);
    CSliderCtrl *pS1 = (CSliderCtrl *)GetDlgItem(IDC_GAIN_YAXIS);
    CSliderCtrl *pS2 = (CSliderCtrl *)GetDlgItem(IDC_GAIN_ZAXIS);
    CSliderCtrl *pS3 = (CSliderCtrl *)GetDlgItem(IDC_GAIN_XROT);
    CSliderCtrl *pS4 = (CSliderCtrl *)GetDlgItem(IDC_GAIN_YROT);
    CSliderCtrl *pS5 = (CSliderCtrl *)GetDlgItem(IDC_GAIN_ZROT);
    CSliderCtrl *pS6 = (CSliderCtrl *)GetDlgItem(IDC_GAIN);
    pS0->EnableWindow(!m_fMasterGain);
    pS1->EnableWindow(!m_fMasterGain);
    pS2->EnableWindow(!m_fMasterGain);
    pS3->EnableWindow(!m_fMasterGain);
    pS4->EnableWindow(!m_fMasterGain);
    pS5->EnableWindow(!m_fMasterGain);
    pS6->EnableWindow(m_fMasterGain);
}


// return axis gain
// note that this is an unmapped axis number
int CGainPage::GetAxisGain(int axisnum)
{
  if (m_fMasterGain)
    return m_iGain;
  else
    return *(iMap[axisnum]);
}


// sets gain for specified axis
void CGainPage::SetAxisGain(int axisnum, int value)
{
  *(iMap[axisnum]) = value;
}


void CGainPage::OnChange(NMHDR* pNMHDR, LRESULT* pResult) 
{
	*pResult = 0;

    // turn on Apply button
    SetModified(TRUE);	
}
