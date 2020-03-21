#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <WinSock2.h>

#define BUF_SIZE 1024

using namespace std;

class ClntSocket
{
private:
	WSAData wsaData;
	SOCKET clnt_sock;
	SOCKADDR_IN serv_Adr;
	char Buffer[BUF_SIZE]{};
	char Msg[BUF_SIZE]{};
	int AdrSz, recvLen, recvCnt;
	bool m_bLogin = false;
	int argc;
	char* argv[3];

private:
	CString m_strID;

	enum MSGTYPE {
		MT_TEXT = 0,
		MT_LOGIN,
		MT_JOIN,
		MT_DENIENDACCESS,
		MT_ISONLINE,
		MT_ISOFFLINE,
		MT_REFRESHLIST,
		MT_USED,
		MT_UNUSED,
		MT_LOGOUT,
		MT_ADDFRIEND,
		MT_ADDFALSE,
		MT_DELETEFRIEND
	};

public:
	ClntSocket();
	~ClntSocket();
	bool Init();
	bool GetIsOnline();
	bool GetIsOnline(const CStringA& strID);

	CStringA RefreshList(vector<CStringA>& strID);
	void Logout();
	int TryLogin(const CStringA& ID, const CStringA& PW, std::vector<CStringA>* vecFriend = nullptr);
	bool TryJoin(const CStringA& ID, const CStringA& PW);
	bool SendMsg(const CString& Msg, const CString& DestID);
	bool AddFriend(CStringA& str);
	bool DeleteFriend(CStringA& str);
	void Update();

public:
	void SplitIDPW(bool bslash, const char* str, string* ID, string* PW);
	void SetUserID(const CString& strID);
	const CString& GetUserID() const;
};