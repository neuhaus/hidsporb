// BatchPage.cpp : implementation file
//

#include "stdafx.h"
#include "OrbProps.h"
#include "BatchPage.h"
#include "TokenEx.h"

void
parse_args_from_file( char* filename, 
		      orb_control& control );

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBatchPage property page

IMPLEMENT_DYNCREATE(CBatchPage, CPropertyPage)

CBatchPage::CBatchPage() : CPropertyPage(CBatchPage::IDD)
{
	//{{AFX_DATA_INIT(CBatchPage)
	//}}AFX_DATA_INIT
}

CBatchPage::~CBatchPage()
{
}

void CBatchPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBatchPage)
	DDX_Control(pDX, IDC_BATCH_COMMANDS, m_lbBatchCmds);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CBatchPage, CPropertyPage)
	//{{AFX_MSG_MAP(CBatchPage)
	ON_BN_CLICKED(IDC_SAVE_BATCH, OnSaveBatch)
	ON_BN_CLICKED(IDC_LOAD_BATCH, OnLoadBatch)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBatchPage message handlers

void CBatchPage::OnSaveBatch() 
{
    CFileDialog dlg(FALSE, "orb", NULL, OFN_OVERWRITEPROMPT, "SpaceOrb Batch Files (*.orb)|*.orb|All Files (*.*)|*.*||", this);
    if (dlg.DoModal() == IDOK) {
      CString csFilePath = dlg.GetPathName();
      CFile orbfile(csFilePath, CFile::modeCreate|CFile::modeWrite);
      int linecount = m_lbBatchCmds.GetCount();
      for (int i = 0;  i < linecount;  i++) {
        CString csLine;
        int n = m_lbBatchCmds.GetTextLen(i);
        m_lbBatchCmds.GetText(i, csLine.GetBuffer(n));
        csLine.ReleaseBuffer();
        csLine += "\r\n";
        orbfile.Write(csLine, csLine.GetLength());
      }
      orbfile.Flush();
    }
}

void CBatchPage::OnLoadBatch() 
{
    CFileDialog dlg(TRUE, "orb", NULL, OFN_PATHMUSTEXIST, "SpaceOrb Batch Files (*.orb)|*.orb|All Files (*.*)|*.*||", this);
    if (dlg.DoModal() == IDOK) {
      CString csFilePath = dlg.GetPathName();
      parse_args_from_file((char*)((const char*)csFilePath), *m_pOrb);
      m_pApp->InitializePropertiesFromOrb(*axesopts, *buttonsopts, *gainopts, *orientopts, *precisionopts, *sensopts, *m_pOrb);
      OnSetActive();  // this reloads the list box
    }
}

BOOL CBatchPage::OnSetActive() 
{
	// go through all the property pages and figure out what what batch commands we need
    UpdateBatchCommands();

    // update listbox with all the commands
    m_lbBatchCmds.ResetContent();
    
    CTokenEx tok;
    CString csCmds = m_csBatchCommands;
    do {
      CString csCmd;
      csCmd += tok.GetString(csCmds, "\n");
      m_lbBatchCmds.AddString("orbcontrol " + csCmd);
    } while (!csCmds.IsEmpty());

	return CPropertyPage::OnSetActive();
}


// give this page references to pages that need to be notified when user switches orientation in the dialog
void CBatchPage::SetPageRefs(CAxesPage *a, CButtonsPage *b, CGainPage *g, COrientationPage *o, CPrecisionPage *p, CSensitivityPage *s)
{
    axesopts = a;
    buttonsopts = b;
    gainopts = g;
    orientopts = o;
    precisionopts = p;
    sensopts = s;
}


// updates m_csBatchCommands with a list of batch commands
void CBatchPage::UpdateBatchCommands()
{
    // first, initialize the orb
    m_csBatchCommands = "--set-defaults\n";

    // go through the axes first
    CTokenEx tok;
    CString csAxes = axesopts->GetBatchAxes();
    do {
      CString csCmd;
      csCmd = "--set-axis ";
      csCmd += tok.GetString(csAxes, ",");
      m_csBatchCommands += csCmd + "\n";
    } while (!csAxes.IsEmpty());

    // check chording mode
    CString csChord;
    csChord.Format("--set-chording %s", buttonsopts->m_fEnableChording ? "on" : "off");
    m_csBatchCommands += csChord + "\n";

    // check individual axis sensitivities
    CTokenEx tok4;
    CString csSensitivities = axesopts->GetSensitivities();
    do {
      CString csCmd;
      csCmd = "--set-sensitivity ";
      csCmd += tok4.GetString(csSensitivities, ",");
      m_csBatchCommands += csCmd + "\n";
    } while (!csSensitivities.IsEmpty());

    // check individual axis polarities
    CTokenEx tok2;
    CString csPolarities = axesopts->GetPolarities();
    do {
      CString csCmd;
      csCmd = "--set-polarity ";
      csCmd += tok2.GetString(csPolarities, ",");
      m_csBatchCommands += csCmd + "\n";
    } while (!csPolarities.IsEmpty());

    // check individual axis gains
    CTokenEx tok3;
    CString csGains = axesopts->GetGains();
    do {
      CString csCmd;
      csCmd = "--set-gain ";
      csCmd += tok3.GetString(csGains, ",");
      m_csBatchCommands += csCmd + "\n";
    } while (!csGains.IsEmpty());

    // check null region
    CString csNull;
    csNull.Format("--set-null-region %d", sensopts->m_iNullRegion);
    m_csBatchCommands += csNull + "\n";
	
    // check precision button
    CString csPrecision;
    csPrecision.Format("--set-precision-button %s", precisionopts->GetPrecisionButton());
    m_csBatchCommands += csPrecision + "\n";
    csPrecision.Format("--set-precision-gain %d", precisionopts->m_iGain);
    m_csBatchCommands += csPrecision + "\n";
    csPrecision.Format("--set-precision-sensitivity %d", precisionopts->m_iSensitivity);
    m_csBatchCommands += csPrecision + "\n";
}


void CBatchPage::SetOrbRef(COrbPropsApp *pApp, orb_control *control)
{
    m_pApp = pApp;
    m_pOrb = control;
}
