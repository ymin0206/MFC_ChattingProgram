// LoginDlg.cpp: 구현 파일
//

#include "pch.h"
#include "ChatClnt.h"
#include "LoginDlg.h"
#include "afxdialogex.h"
#include "MainFrm.h"
#include "ClntSocket.h"
#include "ChatClntView.h"

// CLoginDlg 대화 상자

IMPLEMENT_DYNAMIC(CLoginDlg, CDialog)

CLoginDlg::CLoginDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_PROPPAGE_LOGIN, pParent)
	, m_strID(_T(""))
{

}

CLoginDlg::~CLoginDlg()
{
}

void CLoginDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_ID, m_strID);
	DDX_Text(pDX, IDC_EDIT_PW, m_strPW);
}


BEGIN_MESSAGE_MAP(CLoginDlg, CDialog)
	ON_EN_CHANGE(IDC_EDIT_ID, &CLoginDlg::OnEnChangeEditId)
	ON_EN_CHANGE(IDC_EDIT_PW, &CLoginDlg::OnEnChangeEditPw)
	ON_BN_CLICKED(IDC_BUTTON_LOGIN, &CLoginDlg::OnBnClickedButtonLogin)
	ON_BN_CLICKED(IDC_BUTTON_CANCEL, &CLoginDlg::OnBnClickedButtonCancel)
	ON_BN_CLICKED(IDC_BUTTON_JOIN, &CLoginDlg::OnBnClickedButtonJoin)
END_MESSAGE_MAP()


// CLoginDlg 메시지 처리기


void CLoginDlg::OnEnChangeEditId()
{
	UpdateData(TRUE);
}


void CLoginDlg::OnEnChangeEditPw()
{
	UpdateData(TRUE);
}


void CLoginDlg::OnBnClickedButtonLogin()
{
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	ClntSocket* pSocket = pFrame->GetSocket();

	CChatClntView* pView = (CChatClntView*)pFrame->GetActiveView();
	std::vector<CStringA>* vecStr = pView->GetFriendList();

	switch (pSocket->TryLogin(CStringA(m_strID), CStringA(m_strPW), vecStr))
	{
	case 1000:
		AfxMessageBox(TEXT("로그인 성공"));

		pView->SetFriendList();

		pSocket->SetUserID(m_strID);

		ShowWindow(SW_HIDE);
		break;
	case 1001:
		AfxMessageBox(TEXT("아이디 혹은 비밀번호 오류"));
		break;
	case 1002:
		AfxMessageBox(TEXT("이미 접속중인 아이디"));
		break;
	case 1010:
		AfxMessageBox(TEXT("서버 무응답"));
		break;

	}
}


void CLoginDlg::OnBnClickedButtonCancel()
{

}

// 아이디 생성
void CLoginDlg::OnBnClickedButtonJoin()
{
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	ClntSocket* pSocket = pFrame->GetSocket();
	if (pSocket->TryJoin(CStringA(m_strID), CStringA(m_strPW)))
	{
		AfxMessageBox(TEXT("생성 완료"));
		ShowWindow(SW_HIDE);
	}
	else
	{
		AfxMessageBox(TEXT("생성 실패"));
	}
}


BOOL CLoginDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}
