#if !defined(AFX_BUTTONSPAGE_H__D346E5BE_E662_43A2_A210_AEBC5238DF7C__INCLUDED_)
#define AFX_BUTTONSPAGE_H__D346E5BE_E662_43A2_A210_AEBC5238DF7C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ButtonsPage.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CButtonsPage dialog

class CButtonsPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CButtonsPage)

// Construction
public:
	CButtonsPage();
	~CButtonsPage();
    void SetOrientation(int orientation);


// Dialog Data
	//{{AFX_DATA(CButtonsPage)
	enum { IDD = IDD_Prop_Buttons };
	BOOL	m_fEnableChording;
	CString	m_csSelectedChord;
	CString	m_csButtonA;
	CString	m_csButtonB;
	CString	m_csButtonC;
	CString	m_csButtonD;
	CString	m_csButtonE;
	CString	m_csButtonF;
	//}}AFX_DATA
   	int		m_iOrientation;


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CButtonsPage)
	public:
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CButtonsPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnEnableChording();
	afx_msg void OnChangeSelectedChord();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
    void SetButtonNames();

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BUTTONSPAGE_H__D346E5BE_E662_43A2_A210_AEBC5238DF7C__INCLUDED_)
