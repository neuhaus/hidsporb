#include "AxesPage.h"
#include "ButtonsPage.h"
#include "GainPage.h"
#include "OrientationPage.h"
#include "PrecisionPage.h"
#include "SensitivityPage.h"


#if !defined(AFX_BATCHPAGE_H__34C065EE_F528_4968_81A3_9FCFCBCE9E9E__INCLUDED_)
#define AFX_BATCHPAGE_H__34C065EE_F528_4968_81A3_9FCFCBCE9E9E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BatchPage.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CBatchPage dialog

// forward refs since we can't include the header files w/o causing header recursion :-P
class COrbPropsApp;
class orb_control;

class CBatchPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CBatchPage)

// Construction
public:
	CBatchPage();
	~CBatchPage();
    void SetPageRefs(CAxesPage *a, CButtonsPage *b, CGainPage *g, COrientationPage *o, CPrecisionPage *p, CSensitivityPage *s);
    void SetOrbRef(COrbPropsApp *pApp, orb_control *control);
    void UpdateBatchCommands();

// Dialog Data
	//{{AFX_DATA(CBatchPage)
	enum { IDD = IDD_Prop_Batchfile };
	CListBox	m_lbBatchCmds;
	//}}AFX_DATA
    CString     m_csBatchCommands;

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CBatchPage)
	public:
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CBatchPage)
	afx_msg void OnSaveBatch();
	afx_msg void OnLoadBatch();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

    CAxesPage *axesopts;
    CButtonsPage *buttonsopts;
    CGainPage *gainopts;
    COrientationPage *orientopts;
    CPrecisionPage *precisionopts;
    CSensitivityPage *sensopts;

    COrbPropsApp *m_pApp;
    orb_control *m_pOrb;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BATCHPAGE_H__34C065EE_F528_4968_81A3_9FCFCBCE9E9E__INCLUDED_)
