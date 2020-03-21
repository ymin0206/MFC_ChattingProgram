#pragma once
#include "Common.h"
#include <WinSock2.h>

class CServ
{
	SOCKET m_servSock;
	SOCKADDR_IN m_servAdr;

	WSADATA m_wsaData;
	TIMEVAL m_timeout;
	  
	fd_set m_readfd, m_copyreadfd;
	fd_set m_epctfd, m_epctCopyfd;

	enum MSGTYPE
	{
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

	char Buffer[BUF_SIZE];

	unordered_map<string, SOCKET> m_mapOnline;
	// vector<(*pf)(int)(SOCKET)> vecMsgFunc;

public:
	CServ();
	~CServ();

	bool Init(char* strPort, char* strIp);
	bool Update();

	string RefreshList(const char* str, unordered_map<string, SOCKET>& map);

public:
	void Join(int MsgType, SOCKET iSock);
	bool InsertUesrID(const char* str);
	void Login(int iMsgType, SOCKET iSock);
	int CheckUser(const char* str, string& strID);
	void SendFriendList(const char* ID, vector<string>* vecfriend);
	void Logout(int MsgType, SOCKET iSock);
	void AddFreind(int MsgType, SOCKET iSock);
	bool AddFreindList(const char* ADDID, const string& ID);
	void DeleteFreind(int MsgType, SOCKET iSock);
	bool DeleteFreindList(const char* DeleteID, const string& ID);
	void SendMsg(int iMsgType, SOCKET iSock);
	void SendFriendList(int iMsgType, SOCKET iSock);
	void CheckOnLine(int MsgType, SOCKET iSock);
};