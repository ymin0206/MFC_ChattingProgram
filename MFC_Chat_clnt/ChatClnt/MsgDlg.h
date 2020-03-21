#pragma once


// CMsgDlg 대화 상자

class CMsgDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CMsgDlg)

public:
	CMsgDlg(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CMsgDlg();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PROPPAGE_MSGDLG };
#endif
	class CChatClntView* m_pView;
	class ClntSocket* m_pSock;
	bool m_bEditTextFocus;

	unsigned int m_iLine;

	CString m_SrcID;
	CString m_DestID;

	CString m_Text;
	CEdit m_EditText;

	CListBox m_ListBoX;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonSendmsg();
//	afx_msg void OnEnChangeEditMsglog();
	afx_msg void OnLbnSelchangeListMsgscreen();

public:
	void SetView(class CChatClntView* pView);
	void SetSock(class ClntSocket* pSock);
	void RecvMsg(const CString& Msg);


public:
	afx_msg void OnEnChangeEditMsglog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnEnSetfocusEditMsglog();
	afx_msg void OnEnKillfocusEditMsglog();
	afx_msg void OnLbnSelchangeListMsgscr();
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
};
