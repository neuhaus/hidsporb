#include "AxesPage.h"
#include "ButtonsPage.h"
#include "GainPage.h"
#include "PrecisionPage.h"
#include "SensitivityPage.h"

#if !defined(AFX_ORIENTATIONPAGE_H__BF36D5E1_70D7_4D29_B802_D1D8A3D6AC19__INCLUDED_)
#define AFX_ORIENTATIONPAGE_H__BF36D5E1_70D7_4D29_B802_D1D8A3D6AC19__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OrientationPage.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COrientationPage dialog

class COrientationPage : public CPropertyPage
{
	DECLARE_DYNCREATE(COrientationPage)

// Construction
public:
	COrientationPage();
	~COrientationPage();
    void SetPageRefs(CAxesPage *a, CButtonsPage *b, CGainPage *g, CPrecisionPage *p, CSensitivityPage *s);

// Dialog Data
	//{{AFX_DATA(COrientationPage)
	enum { IDD = IDD_Prop_Orientation };
	int		m_iOrientation;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(COrientationPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(COrientationPage)
	afx_msg void OnHorizontal();
	afx_msg void OnVertical();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

    CAxesPage *axesopts;
    CButtonsPage *buttonsopts;
    CGainPage *gainopts;
    CPrecisionPage *precisionopts;
    CSensitivityPage *sensopts;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ORIENTATIONPAGE_H__BF36D5E1_70D7_4D29_B802_D1D8A3D6AC19__INCLUDED_)
