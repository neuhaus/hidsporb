// AxesPage.cpp : implementation file
//

#include "stdafx.h"
#include "OrbProps.h"
#include "AxesPage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAxesPage property page

IMPLEMENT_DYNCREATE(CAxesPage, CPropertyPage)

CAxesPage::CAxesPage() : CPropertyPage(CAxesPage::IDD)
{
	//{{AFX_DATA_INIT(CAxesPage)
	m_fInvertXAxis = FALSE;
	m_fInvertXRotation = FALSE;
	m_fInvertYAxis = FALSE;
	m_fInvertYRotation = FALSE;
	m_fInvertZAxis = FALSE;
	m_fInvertZRotation = FALSE;
	m_csZRotation = _T("4 - Y Rotation");
	m_csXAxis = _T("0 - X Axis");
	m_csXRotation = _T("3 - X Rotation");
	m_csYAxis = _T("2 - Z Axis");
	m_csYRotation = _T("5 - Z Rotation");
	m_csZAxis = _T("1 - Y Axis");
	//}}AFX_DATA_INIT
    m_iOrientation = m_iOldOrientation = ORIENT_VERTICAL;
    m_sNamesList = NULL;
     // set up axis maps
    csMap[0] = &m_csXAxis;
    csMap[1] = &m_csZAxis;
    csMap[2] = &m_csYAxis;
    csMap[3] = &m_csXRotation;
    csMap[4] = &m_csZRotation;
    csMap[5] = &m_csYRotation;
    fMap[0] = &m_fInvertXAxis;
    fMap[1] = &m_fInvertZAxis;
    fMap[2] = &m_fInvertYAxis;
    fMap[3] = &m_fInvertXRotation;
    fMap[4] = &m_fInvertZRotation;
    fMap[5] = &m_fInvertYRotation;
}

CAxesPage::~CAxesPage()
{
}

void CAxesPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAxesPage)
	DDX_Check(pDX, IDC_INVERT_XAXIS, m_fInvertXAxis);
	DDX_Check(pDX, IDC_INVERT_XROT, m_fInvertXRotation);
	DDX_Check(pDX, IDC_INVERT_YAXIS, m_fInvertYAxis);
	DDX_Check(pDX, IDC_INVERT_YROT, m_fInvertYRotation);
	DDX_Check(pDX, IDC_INVERT_ZAXIS, m_fInvertZAxis);
	DDX_Check(pDX, IDC_INVERT_ZROT, m_fInvertZRotation);
	DDX_CBString(pDX, IDC_ZROT, m_csZRotation);
	DDX_CBString(pDX, IDC_XAXIS, m_csXAxis);
	DDX_CBString(pDX, IDC_XROT, m_csXRotation);
	DDX_CBString(pDX, IDC_YAXIS, m_csYAxis);
	DDX_CBString(pDX, IDC_YROT, m_csYRotation);
	DDX_CBString(pDX, IDC_ZAXIS, m_csZAxis);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAxesPage, CPropertyPage)
	//{{AFX_MSG_MAP(CAxesPage)
	ON_CBN_SELCHANGE(IDC_XAXIS, OnChange)
	ON_CBN_SELCHANGE(IDC_YAXIS, OnChange)
	ON_CBN_SELCHANGE(IDC_ZAXIS, OnChange)
	ON_CBN_SELCHANGE(IDC_XROT, OnChange)
	ON_CBN_SELCHANGE(IDC_YROT, OnChange)
	ON_CBN_SELCHANGE(IDC_ZROT, OnChange)
	ON_BN_CLICKED(IDC_INVERT_XAXIS, OnChange)
	ON_BN_CLICKED(IDC_INVERT_YAXIS, OnChange)
	ON_BN_CLICKED(IDC_INVERT_ZAXIS, OnChange)
	ON_BN_CLICKED(IDC_INVERT_XROT, OnChange)
	ON_BN_CLICKED(IDC_INVERT_YROT, OnChange)
	ON_BN_CLICKED(IDC_INVERT_ZROT, OnChange)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAxesPage message handlers

BOOL CAxesPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
     // set names for all the pulldowns
    int i;
    CComboBox *pCB0 = (CComboBox *)GetDlgItem(IDC_XAXIS);
    CComboBox *pCB1 = (CComboBox *)GetDlgItem(IDC_YAXIS);
    CComboBox *pCB2 = (CComboBox *)GetDlgItem(IDC_ZAXIS);
    CComboBox *pCB3 = (CComboBox *)GetDlgItem(IDC_XROT);
    CComboBox *pCB4 = (CComboBox *)GetDlgItem(IDC_YROT);
    CComboBox *pCB5 = (CComboBox *)GetDlgItem(IDC_ZROT);
    for (i = 0;  i < 6;  i++) {
      pCB0->AddString(m_sNamesList[i]);
      pCB1->AddString(m_sNamesList[i]);
      pCB2->AddString(m_sNamesList[i]);
      pCB3->AddString(m_sNamesList[i]);
      pCB4->AddString(m_sNamesList[i]);
      pCB5->AddString(m_sNamesList[i]);
    }

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


// changes orientation of axes and bitmap
void CAxesPage::SetOrientation(int orientation)
{
    m_iOrientation = orientation;
    CStatic *pImage = (CStatic *)GetDlgItem(IDC_AXES_IMAGE);
    ChangeStaticBitmap(pImage, (m_iOrientation == ORIENT_VERTICAL) ? IDB_VertAxes : IDB_HorizAxes);

    if (m_iOrientation != m_iOldOrientation) {
      ReMap();
      m_iOldOrientation = m_iOrientation;
    }
}

// sets list of names to load up the axes with
void CAxesPage::SetNames(char* names[])
{
    m_sNamesList = names;
}


// Used to remap from horizontal mode to vertical and vice-versa
void CAxesPage::ReMap()
{
   // these correspond to the horizontal mapping displayed by OrbControl
   // since vertical mode is just 1 to 1
  static int AxisMap[] = {0, 2, 1, 3, 5, 4};
  static int InvertMap[] = {0, 0, 1, 0, 0, 1};
  static int IDCAxisMap[] = {IDC_XAXIS, IDC_ZAXIS, IDC_YAXIS, IDC_XROT, IDC_ZROT, IDC_YROT};
  static int IDCInvertMap[] = {IDC_INVERT_XAXIS, IDC_INVERT_ZAXIS, IDC_INVERT_YAXIS, IDC_INVERT_XROT, IDC_INVERT_ZROT, IDC_INVERT_YROT};

   // we actually have to modify member variables because 
   // we get called in OnSetActive before the DDX

  int i, j;
  int currentAxisMap[6];
   // first, figure out what our current mapping is
  for (i = 0;  i < 6;  i++) {
    for (j = 0;  j < 6;  j++) {
      if (csMap[i]->Compare(m_sNamesList[j]) == 0) {
        currentAxisMap[i] = j;
        break;
      }
    }
  }
   // next, swap as appropriate to create new map
  int newAxisMap[6];
  for (i = 0;  i < 6;  i++) {
    newAxisMap[i] = currentAxisMap[AxisMap[i]];
  }
   // then set the new ones into the UI
  for (i = 0;  i < 6;  i++) {
    *(csMap[i]) = m_sNamesList[newAxisMap[i]];
  }

   // inversion is easy...we just need to flip the appropriate axes
  for (i = 0;  i < 6;  i++) {
    if (InvertMap[i]) {
      *(fMap[i]) = !*(fMap[i]);
    }
  }
}


BOOL CAxesPage::OnSetActive() 
{
    SetOrientation(m_iOrientation);
	
	return CPropertyPage::OnSetActive();
}


// gets orb control strings for setting the axes
// we do it here so this is the only class that worries about
// how the axes are mapped
CString CAxesPage::GetBatchAxes()
{
  CString csBatchCmds;
  int i, j;

  for (i = 0;  i < 6;  i++) {
    for (j = 0;  j < 6;  j++) {
      if (csMap[i]->Compare(m_sNamesList[j]) == 0) {
        CString csCmd;
        csCmd.Format("%d %d,", i, j);
        csBatchCmds += csCmd;
        break;
      }
    }
  }
  return (csBatchCmds);
}

// gets comma separated  list of polarities
CString CAxesPage::GetPolarities()
{
  CString csPolarities;
  int i, j;

  for (i = 0;  i < 6;  i++) {
    for (j = 0;  j < 6;  j++) {
      if (csMap[i]->Compare(m_sNamesList[j]) == 0) {
        char cPolarity;
        if (sensopts->GetAxisEnabled(i))
          cPolarity = *(fMap[j]) ? '-' : '+';
        else
          cPolarity = '0';
        CString csPolarity;
        csPolarity.Format("%d %c,", j, cPolarity);
        csPolarities += csPolarity;
        break;
      }
    }
  }
  return (csPolarities);
}

// give us pointer to sensitivity page (needed to disable axes)
void CAxesPage::SetPageRefs(CGainPage *g, CSensitivityPage *s)
{
  gainopts = g;
  sensopts = s;
}

// gets comma separated list of sensitivities
CString CAxesPage::GetSensitivities()
{
  CString csSensitivities;
  int i, j;

  for (i = 0;  i < 6;  i++) {
    for (j = 0;  j < 6;  j++) {
      if (csMap[i]->Compare(m_sNamesList[j]) == 0) {
        CString csSensitivity;
        if (sensopts->GetAxisSensitivity(i) != -1)
          csSensitivity.Format("%d %d,", j, sensopts->GetAxisSensitivity(i));
        csSensitivities += csSensitivity;
        break;
      }
    }
  }
  return (csSensitivities);
}


// gets comma separated list of gains
CString CAxesPage::GetGains()
{
  CString csGains;
  int i, j;

  for (i = 0;  i < 6;  i++) {
    for (j = 0;  j < 6;  j++) {
      if (csMap[i]->Compare(m_sNamesList[j]) == 0) {
        CString csGain;
        csGain.Format("%d %d,", j, gainopts->GetAxisGain(i));
        csGains += csGain;
        break;
      }
    }
  }
  return (csGains);
}


// sets specified logical axis to 
void CAxesPage::SetAxis(int logical, int physical)
{
   *(csMap[logical]) = m_sNamesList[physical];
}


// sets specified axis inversion
void CAxesPage::SetAxisInvert(int axisnum, BOOL fInvert)
{
   *(fMap[axisnum]) = fInvert;
}

void CAxesPage::OnChange() 
{
    // turn on Apply button
    SetModified(TRUE);	
}
