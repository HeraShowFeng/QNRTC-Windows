// CFullScreenDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CVdieoRenderWnd.h"
#include "afxdialogex.h"
#include "resource.h"


// CFullScreenDlg dialog

IMPLEMENT_DYNAMIC(CVdieoRenderWnd, CDialogEx)

CVdieoRenderWnd::CVdieoRenderWnd(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_FULL, pParent)
{

}

CVdieoRenderWnd::~CVdieoRenderWnd()
{
}

void CVdieoRenderWnd::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CVdieoRenderWnd, CDialogEx)
END_MESSAGE_MAP()


// CFullScreenDlg message handlers
