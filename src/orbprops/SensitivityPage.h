#if !defined(AFX_SENSITIVITYPAGE_H__12B2CB39_A38F_411D_9B9F_618989DBAB58__INCLUDED_)
#define AFX_SENSITIVITYPAGE_H__12B2CB39_A38F_411D_9B9F_618989DBAB58__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SensitivityPage.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSensitivityPage dialog

class CSensitivityPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CSensitivityPage)

// Construction
public:
	CSensitivityPage();
	~CSensitivityPage();
    void SetOrientation(int orientation);
    BOOL GetAxisEnabled(int axisnum);
    int GetAxisSensitivity(int axisnum);
    void SetAxisSensitivity(int axisnum, int value);

// Dialog Data
	//{{AFX_DATA(CSensitivityPage)
	enum { IDD = IDD_Prop_Sensitivity };
	BOOL	m_fMasterSensitivity;
	int		m_iXAxis;
	int		m_iXRotation;
	int		m_iYAxis;
	int		m_iYRotation;
	int		m_iZAxis;
	int		m_iZRotation;
	int		m_iSensitivity;
	int		m_iNullRegion;
	//}}AFX_DATA
   	int		m_iOrientation;


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CSensitivityPage)
	public:
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CSensitivityPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnUseMasterSensitivity();
	afx_msg void OnSliderChange(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
    void HandlerMasterEnable();
    int *iMap[6];

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SENSITIVITYPAGE_H__12B2CB39_A38F_411D_9B9F_618989DBAB58__INCLUDED_)
