#pragma once


// CLoginDlg 대화 상자

class CLoginDlg : public CDialog
{
	DECLARE_DYNAMIC(CLoginDlg)

public:
	CLoginDlg(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CLoginDlg();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PROPPAGE_LOGIN };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	CString m_strID;
	CString m_strPW;

	afx_msg void OnEnChangeEditId();
	afx_msg void OnEnChangeEditPw();
	afx_msg void OnBnClickedButtonLogin();
	afx_msg void OnBnClickedButtonCancel();
	afx_msg void OnBnClickedButtonJoin();
	virtual BOOL OnInitDialog();
};
