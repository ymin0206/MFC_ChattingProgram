
// ChatClntView.h: CChatClntView 클래스의 인터페이스
//

#pragma once

class CChatClntView : public CFormView
{
protected: // serialization에서만 만들어집니다.
	CChatClntView() noexcept;
	DECLARE_DYNCREATE(CChatClntView)

public:
#ifdef AFX_DESIGN_TIME
	enum{ IDD = IDD_CHATCLNT_FORM };
#endif

// 특성입니다.
public:
	class CChatClntDoc* GetDocument() const;

// 작업입니다.
public:

// 재정의입니다.
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.
	virtual void OnInitialUpdate(); // 생성 후 처음 호출되었습니다.

// 구현입니다.
public:
	virtual ~CChatClntView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// 생성된 메시지 맵 함수
protected:
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnLvnItemchangedListFriend(NMHDR* pNMHDR, LRESULT* pResult);

public:
	CListCtrl m_CrtlFriendList;
	std::vector<CStringA>vecFriendList;
	std::vector<class CMsgDlg*> vecMsgDlg;
	// std::unordered_map<CStringA, class CMsgDlg*> mapMsgDlg;

	CStringA m_ID;
	CString m_strAddID;

public:
	class ClntSocket* m_pSock;

	std::vector<CStringA>* GetFriendList();
	void SetFriendList();
	void WakeUpDlg(const CStringA& str, const CStringA& Msg);

	int iSelectedIdx;



	virtual void OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/);
	afx_msg void OnBnClickedButtonAddfreind();
	afx_msg void OnBnClickedButtonDeletefriend();
	afx_msg void OnNMDblclkListFriend(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnEnChangeEditAddfreind();
	afx_msg void OnHdnItemclickListFriend(NMHDR *pNMHDR, LRESULT *pResult);
//	afx_msg void OnLvnColumnclickListFriend(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickListFriend(NMHDR* pNMHDR, LRESULT* pResult);
};

#ifndef _DEBUG  // ChatClntView.cpp의 디버그 버전
inline CChatClntDoc* CChatClntView::GetDocument() const
   { return reinterpret_cast<CChatClntDoc*>(m_pDocument); }
#endif

