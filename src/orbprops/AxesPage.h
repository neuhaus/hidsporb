#include "SensitivityPage.h"
#include "GainPage.h"

#if !defined(AFX_AXESPAGE_H__1174C6CD_5F60_4C9F_B1D0_A91727C22752__INCLUDED_)
#define AFX_AXESPAGE_H__1174C6CD_5F60_4C9F_B1D0_A91727C22752__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AxesPage.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAxesPage dialog

class CAxesPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CAxesPage)

// Construction
public:
	CAxesPage();
	~CAxesPage();
    void SetOrientation(int orientation);
    void SetNames(char* names[]);
    CString GetBatchAxes();
    CString GetPolarities();
    CString GetSensitivities();
    CString GetGains();
    void SetPageRefs(CGainPage *g, CSensitivityPage *s);
    void SetAxis(int logical, int physical);
    void SetAxisInvert(int axisnum, BOOL fInvert);

// Dialog Data
	//{{AFX_DATA(CAxesPage)
	enum { IDD = IDD_Prop_Axes };
	BOOL	m_fInvertXAxis;
	BOOL	m_fInvertXRotation;
	BOOL	m_fInvertYAxis;
	BOOL	m_fInvertYRotation;
	BOOL	m_fInvertZAxis;
	BOOL	m_fInvertZRotation;
	CString	m_csZRotation;
	CString	m_csXAxis;
	CString	m_csXRotation;
	CString	m_csYAxis;
	CString	m_csYRotation;
	CString	m_csZAxis;
	//}}AFX_DATA
   	int		m_iOrientation;
    int     m_iOldOrientation;


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CAxesPage)
	public:
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CAxesPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnChange();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

    void ReMap();
    char **m_sNamesList;
    CString *csMap[6];
    BOOL *fMap[6];
    CSensitivityPage *sensopts;
    CGainPage *gainopts;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_AXESPAGE_H__1174C6CD_5F60_4C9F_B1D0_A91727C22752__INCLUDED_)
