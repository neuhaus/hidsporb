// OrbPropsDlg.h : header file
//

#if !defined(AFX_ORBPROPSDLG_H__6946EA77_E755_486C_BB44_3D4F9A501E7B__INCLUDED_)
#define AFX_ORBPROPSDLG_H__6946EA77_E755_486C_BB44_3D4F9A501E7B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// COrbPropsDlg dialog

class COrbPropsDlg : public CPropertySheet
{
// Construction
public:
	COrbPropsDlg(CWnd* pParent = NULL);	// standard constructor
    void SetRefs(COrbPropsApp *pApp, CBatchPage *batch, CAxesPage *a, CButtonsPage *b, CGainPage *g, COrientationPage *o, CPrecisionPage *p, CSensitivityPage *s, orb_control *c);

// Dialog Data
	//{{AFX_DATA(COrbPropsDlg)
	enum { IDD = IDD_ORBPROPS_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COrbPropsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(COrbPropsDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnApplyNow();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

    CAxesPage *axesopts;
    CButtonsPage *buttonsopts;
    CGainPage *gainopts;
    COrientationPage *orientopts;
    CPrecisionPage *precisionopts;
    CSensitivityPage *sensopts;
    COrbPropsApp *m_pApp;
    CBatchPage *batchopts;
    orb_control *control;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ORBPROPSDLG_H__6946EA77_E755_486C_BB44_3D4F9A501E7B__INCLUDED_)
