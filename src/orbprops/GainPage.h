#if !defined(AFX_GAINPAGE_H__7DDC53E8_A441_4082_A45C_DE4BF74BD584__INCLUDED_)
#define AFX_GAINPAGE_H__7DDC53E8_A441_4082_A45C_DE4BF74BD584__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GainPage.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CGainPage dialog

class CGainPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CGainPage)

// Construction
public:
	CGainPage();
	~CGainPage();
    void SetOrientation(int orientation);
    int GetAxisGain(int axisnum);
    void SetAxisGain(int axisnum, int value);

// Dialog Data
	//{{AFX_DATA(CGainPage)
	enum { IDD = IDD_Prop_Gain };
	int		m_iGain;
	int		m_iXAxis;
	int		m_iXRotation;
	int		m_iYAxis;
	int		m_iYRotation;
	int		m_iZAxis;
	int		m_iZRotation;
	BOOL	m_fMasterGain;
	//}}AFX_DATA
   	int		m_iOrientation;

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CGainPage)
	public:
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CGainPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnUseMasterGain();
	afx_msg void OnChange(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
    void HandleMasterEnable();
    int *iMap[6];

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GAINPAGE_H__7DDC53E8_A441_4082_A45C_DE4BF74BD584__INCLUDED_)
