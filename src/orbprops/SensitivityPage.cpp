// SensitivityPage.cpp : implementation file
//

#include "stdafx.h"
#include "OrbProps.h"
#include "SensitivityPage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSensitivityPage property page

IMPLEMENT_DYNCREATE(CSensitivityPage, CPropertyPage)

CSensitivityPage::CSensitivityPage() : CPropertyPage(CSensitivityPage::IDD)
{
	//{{AFX_DATA_INIT(CSensitivityPage)
	m_fMasterSensitivity = FALSE;
	m_iXAxis = 4;
	m_iXRotation = 4;
	m_iYAxis = 4;
	m_iYRotation = 4;
	m_iZAxis = 4;
	m_iZRotation = 4;
	m_iSensitivity = 4;
	m_iNullRegion = 65;
	//}}AFX_DATA_INIT
    iMap[0] = &m_iXAxis;
    iMap[1] = &m_iZAxis;
    iMap[2] = &m_iYAxis;
    iMap[3] = &m_iXRotation;
    iMap[4] = &m_iZRotation;
    iMap[5] = &m_iYRotation;
}

CSensitivityPage::~CSensitivityPage()
{
}

void CSensitivityPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSensitivityPage)
	DDX_Check(pDX, IDC_USE_MASTER_SENS, m_fMasterSensitivity);
	DDX_Slider(pDX, IDC_SENS_XAXIS, m_iXAxis);
	DDX_Slider(pDX, IDC_SENS_XROT, m_iXRotation);
	DDX_Slider(pDX, IDC_SENS_YAXIS, m_iYAxis);
	DDX_Slider(pDX, IDC_SENS_YROT, m_iYRotation);
	DDX_Slider(pDX, IDC_SENS_ZAXIS, m_iZAxis);
	DDX_Slider(pDX, IDC_SENS_ZROT, m_iZRotation);
	DDX_Slider(pDX, IDC_SENSITIVITY, m_iSensitivity);
	DDX_Slider(pDX, IDC_NULLREGION, m_iNullRegion);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSensitivityPage, CPropertyPage)
	//{{AFX_MSG_MAP(CSensitivityPage)
	ON_BN_CLICKED(IDC_USE_MASTER_SENS, OnUseMasterSensitivity)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SENS_YAXIS, OnSliderChange)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_NULLREGION, OnSliderChange)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SENS_XAXIS, OnSliderChange)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SENS_XROT, OnSliderChange)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SENS_YROT, OnSliderChange)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SENS_ZAXIS, OnSliderChange)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SENS_ZROT, OnSliderChange)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SENSITIVITY, OnSliderChange)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSensitivityPage message handlers

BOOL CSensitivityPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
    SetOrientation(m_iOrientation);
	
    // set sensitivity values from -1 to 5; -1 means disable the axis!!!
    CSliderCtrl *pS0 = (CSliderCtrl *)GetDlgItem(IDC_SENS_XAXIS);
    CSliderCtrl *pS1 = (CSliderCtrl *)GetDlgItem(IDC_SENS_YAXIS);
    CSliderCtrl *pS2 = (CSliderCtrl *)GetDlgItem(IDC_SENS_ZAXIS);
    CSliderCtrl *pS3 = (CSliderCtrl *)GetDlgItem(IDC_SENS_XROT);
    CSliderCtrl *pS4 = (CSliderCtrl *)GetDlgItem(IDC_SENS_YROT);
    CSliderCtrl *pS5 = (CSliderCtrl *)GetDlgItem(IDC_SENS_ZROT);
    CSliderCtrl *pS6 = (CSliderCtrl *)GetDlgItem(IDC_SENSITIVITY);
    pS0->SetRange(-1, 5);
    pS0->SetTicFreq(1);
    pS1->SetRange(-1, 5);
    pS1->SetTicFreq(1);
    pS2->SetRange(-1, 5);
    pS2->SetTicFreq(1);
    pS3->SetRange(-1, 5);
    pS3->SetTicFreq(1);
    pS4->SetRange(-1, 5);
    pS4->SetTicFreq(1);
    pS5->SetRange(-1, 5);
    pS5->SetTicFreq(1);
    pS6->SetRange(-1, 5);
    pS6->SetTicFreq(1);

    // set null region
    CSliderCtrl *pSN = (CSliderCtrl *)GetDlgItem(IDC_NULLREGION);
    pSN->SetRange(0, 127);
    pSN->SetTicFreq(10);
    pSN->SetPageSize(10);

    HandlerMasterEnable();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


// changes orientation of axes and bitmap
void CSensitivityPage::SetOrientation(int orientation)
{
    m_iOrientation = orientation;
    CStatic *pImage = (CStatic *)GetDlgItem(IDC_AXES_IMAGE);
    ChangeStaticBitmap(pImage, (m_iOrientation == ORIENT_VERTICAL) ? IDB_VertAxes : IDB_HorizAxes);
}

BOOL CSensitivityPage::OnSetActive() 
{
    SetOrientation(m_iOrientation);
	
	return CPropertyPage::OnSetActive();
}

void CSensitivityPage::OnUseMasterSensitivity() 
{
    CButton *pCBEnableChord = (CButton *)GetDlgItem(IDC_USE_MASTER_SENS);
    m_fMasterSensitivity = pCBEnableChord->GetCheck();
    HandlerMasterEnable();

    // turn on Apply button
    SetModified(TRUE);	
}

// handles disabling controls out as we enable/disable master sensitivity
void CSensitivityPage::HandlerMasterEnable()
{
    CSliderCtrl *pS0 = (CSliderCtrl *)GetDlgItem(IDC_SENS_XAXIS);
    CSliderCtrl *pS1 = (CSliderCtrl *)GetDlgItem(IDC_SENS_YAXIS);
    CSliderCtrl *pS2 = (CSliderCtrl *)GetDlgItem(IDC_SENS_ZAXIS);
    CSliderCtrl *pS3 = (CSliderCtrl *)GetDlgItem(IDC_SENS_XROT);
    CSliderCtrl *pS4 = (CSliderCtrl *)GetDlgItem(IDC_SENS_YROT);
    CSliderCtrl *pS5 = (CSliderCtrl *)GetDlgItem(IDC_SENS_ZROT);
    CSliderCtrl *pS6 = (CSliderCtrl *)GetDlgItem(IDC_SENSITIVITY);
    pS0->EnableWindow(!m_fMasterSensitivity);
    pS1->EnableWindow(!m_fMasterSensitivity);
    pS2->EnableWindow(!m_fMasterSensitivity);
    pS3->EnableWindow(!m_fMasterSensitivity);
    pS4->EnableWindow(!m_fMasterSensitivity);
    pS5->EnableWindow(!m_fMasterSensitivity);
    pS6->EnableWindow(m_fMasterSensitivity);
}


// determines if an axis was enabled
// note that this is an unmapped axis number
BOOL CSensitivityPage::GetAxisEnabled(int axisnum)
{
  if (m_fMasterSensitivity)
    return (m_iSensitivity == -1) ? FALSE : TRUE;
  else
    return (*(iMap[axisnum]) == -1) ? FALSE : TRUE;
}

// return axis sensitivity
// note that this is an unmapped axis number
int CSensitivityPage::GetAxisSensitivity(int axisnum)
{
  if (m_fMasterSensitivity)
    return m_iSensitivity;
  else
    return *(iMap[axisnum]);
}

// sets sensitivity for specified axis
void CSensitivityPage::SetAxisSensitivity(int axisnum, int value)
{
  *(iMap[axisnum]) = value;
}


void CSensitivityPage::OnSliderChange(NMHDR* pNMHDR, LRESULT* pResult) 
{
	*pResult = 0;

    // turn on Apply button
    SetModified(TRUE);	
}
