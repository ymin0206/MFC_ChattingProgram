// CMsgDlg.cpp: 구현 파일
//

#include "pch.h"
#include "ChatClnt.h"
#include "MsgDlg.h"
#include "afxdialogex.h"
#include "ChatClntView.h"
#include "ClntSocket.h"

// CMsgDlg 대화 상자

IMPLEMENT_DYNAMIC(CMsgDlg, CDialogEx)

CMsgDlg::CMsgDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_PROPPAGE_MSGDLG, pParent),
	m_pView(nullptr),
	m_pSock(nullptr),
	m_bEditTextFocus(false)
	, m_Text(_T("")),
	m_iLine(0)
{
}

CMsgDlg::~CMsgDlg()
{
}

void CMsgDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_MSGLOG, m_Text);
	DDX_Control(pDX, IDC_EDIT_MSGLOG, m_EditText);
	DDX_Control(pDX, IDC_LIST_MSGSCR, m_ListBoX);
}


BEGIN_MESSAGE_MAP(CMsgDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_SENDMSG, &CMsgDlg::OnBnClickedButtonSendmsg)
	ON_LBN_SELCHANGE(IDC_LIST_MSGSCR, &CMsgDlg::OnLbnSelchangeListMsgscreen)
	ON_EN_CHANGE(IDC_EDIT_MSGLOG, &CMsgDlg::OnEnChangeEditMsglog)
	ON_EN_SETFOCUS(IDC_EDIT_MSGLOG, &CMsgDlg::OnEnSetfocusEditMsglog)
	ON_EN_KILLFOCUS(IDC_EDIT_MSGLOG, &CMsgDlg::OnEnKillfocusEditMsglog)
	ON_LBN_SELCHANGE(IDC_LIST_MSGSCR, &CMsgDlg::OnLbnSelchangeListMsgscr)
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()


// CMsgDlg 메시지 처리기


void CMsgDlg::OnBnClickedButtonSendmsg()
{
	m_pSock->SendMsg(m_Text, m_DestID);

	m_Text = TEXT("나: ") + m_Text;
	m_ListBoX.InsertString(m_iLine, m_Text.GetString());

	m_Text = "";
	m_EditText.SetWindowTextW(m_Text.GetString());

	++m_iLine;
}

void CMsgDlg::OnLbnSelchangeListMsgscreen()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}

void CMsgDlg::SetView(CChatClntView* pView)
{
	m_pView = pView;
}

void CMsgDlg::SetSock(ClntSocket* pSock)
{
	m_pSock = pSock;
}

void CMsgDlg::RecvMsg(const CString& Msg)
{
	m_ListBoX.InsertString(m_iLine, Msg);
	++m_iLine;
}


void CMsgDlg::OnEnChangeEditMsglog()
{
	UpdateData(TRUE);
}


BOOL CMsgDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN)
	{
		//CStringA(str);
		if (pMsg->wParam == VK_RETURN)
		{
			if (m_bEditTextFocus)
				OnBnClickedButtonSendmsg();

			return TRUE;
		}
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}


void CMsgDlg::OnEnSetfocusEditMsglog()
{
	m_bEditTextFocus = true;

	SetActiveWindow();
}


void CMsgDlg::OnEnKillfocusEditMsglog()
{
	m_bEditTextFocus = false;
}


void CMsgDlg::OnLbnSelchangeListMsgscr()
{

}


void CMsgDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.

	this->SetFocus();

	CDialogEx::OnLButtonUp(nFlags, point);
}
