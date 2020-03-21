#include "pch.h"
#include "ClntSocket.h"
#include "MainFrm.h"
#include "ChatClntView.h"

ClntSocket::ClntSocket()
{
	m_bLogin = false;
	argc = 3;
	//argv[1] = (char*)"27.1.112.147";
	argv[1] = (char*)"192.168.1.4";
	argv[2] = (char*)"9190";
}

ClntSocket::~ClntSocket()
{
}

bool ClntSocket::Init()
{
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		return false;

	clnt_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (clnt_sock == INVALID_SOCKET)
		return false;

	DWORD recvTimeout = 100;  // 3초.
	int Rs = setsockopt(clnt_sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&recvTimeout, sizeof(recvTimeout));

	// 서버의 소켓을 작성한다.
	memset(&serv_Adr, 0, sizeof(serv_Adr));
	serv_Adr.sin_family = AF_INET;
	serv_Adr.sin_addr.S_un.S_addr = inet_addr(argv[1]);
	serv_Adr.sin_port = htons(atoi(argv[2]));

	return true;
}

bool ClntSocket::GetIsOnline()
{
	return m_bLogin;
}

bool ClntSocket::GetIsOnline(const CStringA& strID)
{
	char Buf[BUF_SIZE];
	memset(Buf, 0, BUF_SIZE);
	unsigned int data = MT_ISONLINE;
	memcpy((int*)&Buf[0], &data, sizeof(int));
	memcpy(&Buf[4], strID.GetString(), strID.GetLength());
	send(clnt_sock, Buf, strID.GetLength() + 4, 0);

	int BufLen = 0;
	memset(Buf, 0, BUF_SIZE);
	while (BufLen == 0)
	{
		BufLen = recv(clnt_sock, Buf, BUF_SIZE - 1, 0);
	}

	memcpy(&data, (int*)&Buf[0], sizeof(int));

	if (data == MT_ISONLINE)
		return true;
	else
		return false;
}

void ClntSocket::SetUserID(const CString& strID)
{
	m_strID = strID;
}

const CString& ClntSocket::GetUserID() const
{
	return m_strID;
}

CStringA ClntSocket::RefreshList(vector<CStringA>& strID)
{
	memset(Buffer, 0, BUF_SIZE);

	unsigned int data = MT_REFRESHLIST;
	memcpy((int*)&Buffer[0], &data, sizeof(int));

	int Idx = 4;
	for (int i = 0; i < strID.size(); ++i)
	{
		CStringA Src = strID[i] + ',';

		if(i == strID.size() - 1)
			Src += '/';

		memcpy(&Buffer[Idx], Src.GetBuffer(), Src.GetLength());
		Idx += Src.GetLength();
	}

	send(clnt_sock, Buffer, Idx, 0);

	int BufLen = 0;

	memset(Buffer, 0, BUF_SIZE);

	do
	{
		BufLen = recv(clnt_sock, Buffer, BUF_SIZE - 1, 0);
	} while (BufLen == 0);

	return CStringA(Buffer);
}


void ClntSocket::Logout()
{
	int iBuf = MT_LOGOUT;
	send(clnt_sock, (char*)&iBuf, sizeof(int), 0);
	m_bLogin = false;
	closesocket(clnt_sock);
	WSACleanup();
}

int ClntSocket::TryLogin(const CStringA& ID, const CStringA& PW, std::vector<CStringA>* vecFriend)
{
	if (ID == "" || PW == "")
		return 1001;

	if (connect(clnt_sock, (SOCKADDR*)&serv_Adr, sizeof(serv_Adr)) == SOCKET_ERROR)
	{
		m_bLogin = false;

		// 임시
		Logout();
		Init();

		if (connect(clnt_sock, (SOCKADDR*)&serv_Adr, sizeof(serv_Adr)) == SOCKET_ERROR)
		{
			return 1010;
		}
	}

	CStringA IDinfo;
	IDinfo = ID + ',' + PW + '/';

	unsigned int data = MT_LOGIN;

	memcpy((int*)&Buffer[0], &data, sizeof(int));

	memcpy(&Buffer[4], IDinfo.GetBuffer(), IDinfo.GetLength());

	send(clnt_sock, Buffer, IDinfo.GetLength() + 4, 0);

	while (1)
	{
		int BufLen = recv(clnt_sock, Buffer, BUF_SIZE - 1, 0);

		if (BufLen > 0)
		{
			int Result = -1;

			memcpy(&Result, (int*)&Buffer[0], sizeof(int));

			if (Result == MT_LOGIN)
			{
				m_bLogin = true;

				int FriendCount = 0;
				memcpy(&FriendCount, (int*)&Buffer[4], sizeof(int));

				if (FriendCount > 1000000)
					break;

				while (FriendCount != vecFriend->size())
				{
					memset(Buffer, 0, BUF_SIZE);

					BufLen = recv(clnt_sock, Buffer, BUF_SIZE - 1, 0);

					// 받은 버퍼에 ,이전까지만 아이디로 정해서 넣도록햐아겠음
					if (BufLen != 0)
					{
							CStringA Temp;
							int iCount = 0;

							while (iCount < BufLen)
							{
								if(Buffer[iCount] != ',' && Buffer[iCount] != '\n')
									Temp += Buffer[iCount];
								else if(Buffer[iCount] == ',')
								{
									vecFriend->push_back(Temp);
									Temp = "";
								}

								++iCount;
							}
					}
				}

				return 1000;
			}
			else if (Result == MT_DENIENDACCESS)
			{
				m_bLogin = false;
				return 1001;
			}
			else if (Result == MT_ISONLINE)
			{
				m_bLogin = false;
				return 1002;
			}

		}
	}

}

bool ClntSocket::TryJoin(const CStringA& ID, const CStringA& PW)
{
	if (ID == "" || PW == "")
		return false;

	if (connect(clnt_sock, (SOCKADDR*)&serv_Adr, sizeof(serv_Adr)) == SOCKET_ERROR)
	{
		m_bLogin = false;

		// 임시
		Logout();
		Init();

		if (connect(clnt_sock, (SOCKADDR*)&serv_Adr, sizeof(serv_Adr)) == SOCKET_ERROR)
		{
			return false;
		}
	}
	CStringA IDinfo;
	IDinfo = ID + ',' + PW + '/';

	char Buf[BUF_SIZE]{};
	int MsgType = MT_JOIN;
	memcpy((int*)Buf, &MsgType, sizeof(int));
	memcpy(&Buf[4], IDinfo.GetBuffer(), IDinfo.GetLength());

	send(clnt_sock, Buf, IDinfo.GetLength() + 4, 0);

	while (1)
	{
		int Buflen = recv(clnt_sock, Buf, BUF_SIZE - 1, 0);

		if (Buflen != 0)
		{
			int Result = 0;

			memcpy(&Result, Buf, sizeof(int));

			switch (Result)
			{
			case MT_USED:
				return false;
			case MT_UNUSED:
				return true;
			}

		}
	}


	return false;
}

bool ClntSocket::SendMsg(const CString& Msg, const CString& DestID)
{
	char TempMsg[1024]{};
	int Data = MT_TEXT;
	CStringA MsgA(Msg);
	CStringA DestIDA(DestID);
	memcpy((int*)&TempMsg[0], &Data, sizeof(int));

	Data = DestIDA.GetLength();
	memcpy((int*)&TempMsg[4], &Data, sizeof(int));

	memcpy((int*)&TempMsg[8], DestIDA.GetBuffer(), DestIDA.GetLength());

	strcpy(&TempMsg[8 + Data], MsgA.GetBuffer());
	send(clnt_sock, TempMsg, 8 + Data + MsgA.GetLength(), 0);

	return true;
}

bool ClntSocket::AddFriend(CStringA& str)
{
	char TempMsg[100]{};
	int Data = MT_ADDFRIEND;

	memcpy((int*)&TempMsg[0], &Data, sizeof(int));
	Data = str.GetLength();
	memcpy((int*)&TempMsg[4], &Data, sizeof(int));
	strcpy(&TempMsg[8], str.GetBuffer()); // 야이 병신아
	
	send(clnt_sock, TempMsg, 4 + 4 + str.GetLength(), 0);

	memset(TempMsg, 0, 100);
	int result = 0;

	while (1)
	{
		result = recv(clnt_sock, TempMsg, 99, 0);
		if (result > 1)
		{
			memcpy(&Data, (int*)&TempMsg, sizeof(int));
			break;
		}
	}

	if (Data == MT_ADDFRIEND)
	{
		return true;
	}
	else	
		return false;
}

bool ClntSocket::DeleteFriend(CStringA& str)
{
	char TempMsg[100]{};
	int Data = MT_DELETEFRIEND;

	memcpy((int*)&TempMsg[0], &Data, sizeof(int));

	Data = str.GetLength();
	memcpy((int*)&TempMsg[4], &Data, sizeof(int));
	strcpy(&TempMsg[8], str.GetBuffer());

	send(clnt_sock, TempMsg, 4 + 4 + str.GetLength(), 0);

	memset(TempMsg, 0, 100);
	int result = 0;

	while (1)
	{
		result = recv(clnt_sock, TempMsg, 99, 0);
		if (result > 1)
		{
			memcpy(&Data, (int*)&TempMsg, sizeof(int));
			break;
		}
	}
	if (Data == MT_DELETEFRIEND)
	{
		return true;
	}
	else
		return false;
}

void ClntSocket::Update()
{
	char Buf[BUF_SIZE]{};
	int RecvLen = recv(clnt_sock, Buf, BUF_SIZE - 1, 0);
	
	if (RecvLen > 0)
	{
		int Data = 0;
		memcpy(&Data, (int*)&Buf, sizeof(int));
	
		switch (Data)
		{
		case MT_TEXT:
			memcpy(&Data, (int*)&Buf[0], sizeof(int));
			memcpy(&Data, (int*)&Buf[4], sizeof(int)); 
			
			CStringA ID;
			ID.Append(&Buf[8], Data);
	
			CStringA Msg = &Buf[8 + Data];
	
			CChatClntView* pView = ((CMainFrame*)AfxGetMainWnd())->GetView();
	
			pView->WakeUpDlg(ID, Msg);
	
			break;
		}
	
	
	}
}

void ClntSocket::SplitIDPW(bool bslash, const char* str, string* ID, string* PW)
{
	int i = 0;

	while (str[i] != ',')
	{
		*ID += str[i];
		++i;
	}
	++i;

	if (PW)
	{
		if (bslash)
		{
			while (str[i] != '/')
			{
				*PW += str[i];
				++i;
			}
		}
		else
		{
			while (str[i] != '\0')
			{
				*PW += str[i];
				++i;
			}
		}
	}
}