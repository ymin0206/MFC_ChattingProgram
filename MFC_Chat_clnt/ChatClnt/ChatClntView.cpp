
// ChatClntView.cpp: CChatClntView 클래스의 구현
//

#include "pch.h"
#include "framework.h"
// SHARED_HANDLERS는 미리 보기, 축소판 그림 및 검색 필터 처리기를 구현하는 ATL 프로젝트에서 정의할 수 있으며
// 해당 프로젝트와 문서 코드를 공유하도록 해 줍니다.
#ifndef SHARED_HANDLERS
#include "ChatClnt.h"
#endif

#include "ChatClntDoc.h"
#include "ChatClntView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#include "MainFrm.h"
#include "LoginDlg.h"
#include "ClntSocket.h"

#include "MsgDlg.h"

// CChatClntView

IMPLEMENT_DYNCREATE(CChatClntView, CFormView)

BEGIN_MESSAGE_MAP(CChatClntView, CFormView)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_FRIEND, &CChatClntView::OnLvnItemchangedListFriend)
	ON_BN_CLICKED(IDC_BUTTON_ADDFREIND, &CChatClntView::OnBnClickedButtonAddfreind)
	ON_BN_CLICKED(IDC_BUTTON_DELETEFRIEND, &CChatClntView::OnBnClickedButtonDeletefriend)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_FRIEND, &CChatClntView::OnNMDblclkListFriend)
	ON_WM_TIMER()
	ON_EN_CHANGE(IDC_EDIT_ADDFREIND, &CChatClntView::OnEnChangeEditAddfreind)
	ON_NOTIFY(HDN_ITEMCLICK, 0, &CChatClntView::OnHdnItemclickListFriend)
//	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST_FRIEND, &CChatClntView::OnLvnColumnclickListFriend)
	ON_NOTIFY(NM_CLICK, IDC_LIST_FRIEND, &CChatClntView::OnNMClickListFriend)
END_MESSAGE_MAP()

// CChatClntView 생성/소멸

CChatClntView::CChatClntView() noexcept : 
	m_strAddID(_T("")),
	CFormView(IDD_CHATCLNT_FORM),
	m_pSock(nullptr),
	iSelectedIdx(-1)
{
	// TODO: 여기에 생성 코드를 추가합니다.
	vecMsgDlg.clear();
}

CChatClntView::~CChatClntView()
{
	for (unsigned int i = 0; i < vecMsgDlg.size(); ++i)
	{
		vecMsgDlg[i]->DestroyWindow();
	}
}

void CChatClntView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_FRIEND, m_CrtlFriendList);
	DDX_Text(pDX, IDC_EDIT_ADDFREIND, m_strAddID);
}

BOOL CChatClntView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: CREATESTRUCT cs를 수정하여 여기에서
	//  Window 클래스 또는 스타일을 수정합니다.

	return CFormView::PreCreateWindow(cs);
}

void CChatClntView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();
	GetParentFrame()->RecalcLayout();
	ResizeParentToFit();

	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	m_pSock = pFrame->GetSocket();

	CRect rt;
	m_CrtlFriendList.GetWindowRect(&rt);
	m_CrtlFriendList.InsertColumn(0, TEXT("ID"), LVCFMT_CENTER, rt.Width() * 0.3f);
	m_CrtlFriendList.InsertColumn(1, TEXT("내용"), LVCFMT_CENTER, rt.Width() * 0.5f);
	m_CrtlFriendList.InsertColumn(2, TEXT("접속"), LVCFMT_CENTER, rt.Width() * 0.192f);

	SetTimer(1, 5000, NULL);
}

void CChatClntView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CChatClntView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CChatClntView 진단

#ifdef _DEBUG
void CChatClntView::AssertValid() const
{
	CFormView::AssertValid();
}

void CChatClntView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}

CChatClntDoc* CChatClntView::GetDocument() const // 디버그되지 않은 버전은 인라인으로 지정됩니다.
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CChatClntDoc)));
	return (CChatClntDoc*)m_pDocument;
}
#endif //_DEBUG


// CChatClntView 메시지 처리기


void CChatClntView::OnLvnItemchangedListFriend(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	*pResult = 0;
}


std::vector<CStringA>* CChatClntView::GetFriendList()
{
	return &vecFriendList;
}

void CChatClntView::SetFriendList()	// 접속중 새로고침 (타이머)
{
	m_CrtlFriendList.DeleteAllItems();

	if (vecFriendList.size() > 0)
	{
		CStringA RefreshList = m_pSock->RefreshList(vecFriendList);

		for (int i = 0; i < vecFriendList.size(); ++i)
		{
			m_CrtlFriendList.InsertItem(i, CString(vecFriendList[i]));

			if (RefreshList[i] == 'o')
				m_CrtlFriendList.SetItem(i, 2, LVIF_TEXT, _T("접속중"), 0, 0, 0, NULL);
			else
				m_CrtlFriendList.SetItem(i, 2, LVIF_TEXT, _T("비접속"), 0, 0, 0, NULL);
		}
	}
}

void CChatClntView::WakeUpDlg(const CStringA& str, const CStringA& Msg)
{
	for (int i = 0; i < vecMsgDlg.size(); ++i)
	{
		if (vecMsgDlg[i]->m_DestID.Compare(CString(str)) == 0)
		{
			vecMsgDlg[i]->ShowWindow(SW_SHOW);

			vecMsgDlg[i]->RecvMsg(CString(str + ": " + Msg));

			return;
		}
	}

	CMsgDlg* Dlg = new CMsgDlg;
	Dlg->Create(IDD_PROPPAGE_MSGDLG);
	Dlg->SetView(this);
	Dlg->SetSock(m_pSock);
	Dlg->ShowWindow(SW_SHOW);
	Dlg->m_SrcID = m_pSock->GetUserID();
	Dlg->m_DestID = CString(str);
	Dlg->RecvMsg(CString(str + ": " + Msg));

	vecMsgDlg.push_back(Dlg);
}

void CChatClntView::OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/)
{
	int a;
}

// 친구 추가
void CChatClntView::OnBnClickedButtonAddfreind()
{
	if (m_strAddID.GetLength() != 0)
	{
		if (m_pSock->AddFriend(CStringA(m_strAddID)))
		{
			vecFriendList.push_back(CStringA(m_strAddID));

			SetFriendList();

			AfxMessageBox(TEXT("추가 완료"));
		}
	}
	else
	{
		AfxMessageBox(TEXT("추가 실패"));
	}

}

// 친구 삭제
void CChatClntView::OnBnClickedButtonDeletefriend()
{	 
	 if (iSelectedIdx != -1)
	 {
	 	if (m_pSock->DeleteFriend(vecFriendList[iSelectedIdx]))
	 	{
	 		vecFriendList.erase(vecFriendList.begin() + iSelectedIdx);
	 
	 		SetFriendList();
	 
	 		AfxMessageBox(TEXT("삭제 완료"));
	 	}
	 	else
	 	{
	 		AfxMessageBox(TEXT("삭제 실패"));
	 	}
	 }
	 else
	 {
	 	AfxMessageBox(TEXT("삭제 실패"));
	 }

	 m_CrtlFriendList.SetItemState(-1, 0, LVIS_SELECTED);
	 iSelectedIdx = -1;
}

// 친구 더블클릭
void CChatClntView::OnNMDblclkListFriend(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	*pResult = 0;

	if (pNMItemActivate->iItem != -1)
	{
		CStringA str(m_CrtlFriendList.GetItemText(pNMItemActivate->iItem, 0));

		if (m_pSock->GetIsOnline(str))
		{
			for (int i = 0; i < vecMsgDlg.size(); ++i)
			{
				if (vecMsgDlg[i]->m_DestID.Compare(CString(str)) == 0)
				{
					vecMsgDlg[i]->ShowWindow(SW_SHOW);
					return;
				}
			}

			CMsgDlg* Dlg = new CMsgDlg;
			Dlg->Create(IDD_PROPPAGE_MSGDLG);
			Dlg->SetView(this); 
			Dlg->SetSock(m_pSock);
			Dlg->ShowWindow(SW_SHOW);
			Dlg->m_SrcID = m_pSock->GetUserID();
			Dlg->m_DestID = CString(str);
			vecMsgDlg.push_back(Dlg);
		}
		else
		{
			AfxMessageBox(TEXT("로그아웃상태 입니다."));
		}
	}
}



void CChatClntView::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	if (nIDEvent == 1)
	{
		SetFriendList();
	}

	CFormView::OnTimer(nIDEvent);
}


void CChatClntView::OnEnChangeEditAddfreind()
{
	UpdateData(TRUE);
}


void CChatClntView::OnHdnItemclickListFriend(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	*pResult = 0;
}


//void CChatClntView::OnLvnColumnclickListFriend(NMHDR *pNMHDR, LRESULT *pResult)
//{
//	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
//	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
//
//	iSelectedIdx = pNMLV->iItem;
//
//	*pResult = 0;
//}


void CChatClntView::OnNMClickListFriend(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

	if (-1 == pNMItemActivate->iItem)
		return;

	iSelectedIdx = pNMItemActivate->iItem;

	// m_CrtlFriendList.GetItemText(pNMItemActivate->iItem, 0);

	*pResult = 0;

}
