#if !defined(AFX_PRECISIONPAGE_H__D6C63EFD_9153_4DA8_9C61_DEFE63F4AE1D__INCLUDED_)
#define AFX_PRECISIONPAGE_H__D6C63EFD_9153_4DA8_9C61_DEFE63F4AE1D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PrecisionPage.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPrecisionPage dialog

class CPrecisionPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CPrecisionPage)

// Construction
public:
	CPrecisionPage();
	~CPrecisionPage();
    void SetOrientation(int orientation);
    CString GetPrecisionButton();

// Dialog Data
	//{{AFX_DATA(CPrecisionPage)
	enum { IDD = IDD_Prop_Precision };
	CString	m_csLogicalButton;
	int		m_iGain;
	int		m_iSensitivity;
	int		m_iPrecisionButton;
	BOOL	m_fEnablePrecision;
	//}}AFX_DATA
   	int		m_iOrientation;


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPrecisionPage)
	public:
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CPrecisionPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnUseLogical();
	afx_msg void OnPrecisionClicked();
	afx_msg void OnEnablePrecision();
	afx_msg void OnChange();
	afx_msg void OnSliderChange(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
    void HandleEnable();
    char* m_csButtonNames[16];
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PRECISIONPAGE_H__D6C63EFD_9153_4DA8_9C61_DEFE63F4AE1D__INCLUDED_)
