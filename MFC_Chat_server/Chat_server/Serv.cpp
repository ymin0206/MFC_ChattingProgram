#include "Serv.h"
#include "Common.h"

CServ::CServ()	:
	m_servSock(0)
{
	memset(&m_servAdr, 0, sizeof(m_servAdr));
	memset(&m_wsaData, 0, sizeof(m_wsaData));
}

CServ::~CServ()
{
	closesocket(m_servSock);
	WSACleanup();
}

bool CServ::Init(char* strPort, char* strIp)
{
	m_timeout.tv_sec = 5;
	m_timeout.tv_usec = 5000;

	if (WSAStartup(MAKEWORD(2, 2), &m_wsaData) != 0)
	{
		ErrMsg("WSAStartup() Err"); 
		return false;
	}
	else
		printf("WSAStartup()\n");

	m_servSock = socket(PF_INET, SOCK_STREAM, 0);
	if (m_servSock == INVALID_SOCKET)
	{
		ErrMsg("socket() Err");
		return false;
	}
	else
		printf("socket()\n");

	memset(&m_servAdr, 0, sizeof(m_servAdr));
	m_servAdr.sin_family = AF_INET;

	// serv_Adr.sin_addr.S_un.S_addr = inet_addr(argv[1]);
	m_servAdr.sin_addr.S_un.S_addr = inet_addr(strIp);
	m_servAdr.sin_port = htons(atoi(strPort));

	if (bind(m_servSock, (SOCKADDR*)&m_servAdr, sizeof(m_servAdr)) == SOCKET_ERROR)
	{	
		ErrMsg("bind() Err");
		return false;
	}
	else
		printf("bind()\n");

	// https://www.joinc.co.kr/w/man/4100/listend
	// listen() 함수의 관한 설명
	if (listen(m_servSock, 10)) 
	{
		ErrMsg("listen() Err");
		return false;
	}
	else
		printf("listen()\n");

	printf(".....................\n");
	FD_ZERO(&m_readfd);
	FD_SET(m_servSock, &m_readfd);

	return true;
}

bool CServ::Update()
{
	m_timeout.tv_sec = 5;
	m_timeout.tv_usec = 5000;
	m_copyreadfd = m_readfd;	// fdset의 원본을 복사함

	int fdNum;

	// 수신할 데이터가 있는 fd가 있는지 확인함.
	// 이 과정에서 fd_set 변수 내용이 변경되기 때문에 사본 변수를 둠.
	if ((fdNum = select(0, &m_copyreadfd, 0, 0, &m_timeout)) == SOCKET_ERROR)
	{
		ErrMsg("Select() Error! \n");
		return false;
	}

	if (fdNum == 0) // select() 에서 수신할 데이터가 있으면 1
		return true;

	// 원본의 총 등록된 fd중에 어떤 fd가 활성인지 검사함
	for (int i = 0; i < m_readfd.fd_count; ++i)
	{
		// 이벤트가 발생된 fd의 데이터를 처리함.  데이터 수신/발신, fd 삭제/추가 등
		if (FD_ISSET(m_readfd.fd_array[i], &m_copyreadfd))
		{
			SOCKET clntSock;
			SOCKADDR_IN clntAdr;

			if (m_readfd.fd_array[i] == m_servSock)
			{
				int adrSz = sizeof(clntAdr);
				// listen()에서 등록된 주소중 가장 오래된 연결요청을 가져와 연결 소켓을 만듬 
				clntSock = 0;
				clntSock = accept(m_servSock, (SOCKADDR*)&clntAdr, &adrSz);
				FD_SET(clntSock, &m_readfd);

				printf("Connected Client : %d \n", (int)clntSock);
			}
			else
			{
				int MsgType = -1;

				memset(&Buffer[0], 0, BUF_SIZE);
				int recvBufLen = recv(m_readfd.fd_array[i], Buffer, BUF_SIZE - 1, 0);

				if (recvBufLen == 0) // EOF를 의미함. (통신 종료)
				{
					// clnt_sock 과 같지만 소켓정보를 안가지고있을 수 도 있기때문에 배열정보로 닫음
					FD_CLR(m_readfd.fd_array[i], &m_readfd);
					closesocket(m_readfd.fd_array[i]);
				}
				else // 메세지 처리
				{
					if(recvBufLen != -1)
						memcpy(&MsgType, (int*)&Buffer[0], sizeof(int));

					switch (MsgType)
					{
					case MT_TEXT:
						SendMsg(MsgType, m_readfd.fd_array[i]);
						break;

					case MT_LOGIN:
						Login(MsgType, m_readfd.fd_array[i]);
						break;

					case MT_JOIN:
						Join(MsgType, m_readfd.fd_array[i]);
						break;

					case MT_REFRESHLIST:
						SendFriendList(MsgType, m_readfd.fd_array[i]);
						break;

					case MT_ISONLINE:
						CheckOnLine(MsgType, m_readfd.fd_array[i]);
						break;

					case MT_LOGOUT:
						Logout(MsgType, m_readfd.fd_array[i]);
						break;

					case MT_ADDFRIEND:
						AddFreind(MsgType, m_readfd.fd_array[i]);
						break;

					case MT_DELETEFRIEND:
						DeleteFreind(MsgType, m_readfd.fd_array[i]);
						break;
					}

				}
			}
		}
	}

	return true;
}

int CServ::CheckUser(const char* str, string& strID)
{
	ifstream ifile("DB//IDlist.txt");

	string InsertID;
	string InsertPW;

	SplitIDPW(true, str, &InsertID, &InsertPW);

	strID = InsertID;

	bool bID = false;

	if (ifile.is_open())
	{
		string temp;

		while (getline(ifile, temp, '/'))
		{
			string cmpID{};
			string cmpPW{};
			SplitIDPW(false, temp.c_str(), &cmpID, &cmpPW);

			if (InsertID.compare(cmpID) == 0)
			{
				bID = true;

				if (InsertPW.compare(cmpPW) == 0)
				{
					ifile.close();
					return MT_LOGIN;
				}
			}
		}
	}
	
	ifile.close();

	if (bID)
		return MT_ISONLINE;

	return MT_DENIENDACCESS;
}

void CServ::SendFriendList(const char* ID, vector<string>* vecfriend)
{
	vector<string> vecFriend;
	string strFilename = "DB//";
	strFilename += ID;
	strFilename += ".txt";

	ifstream ifile(strFilename);

	if (ifile.is_open())
	{
		string Temp;
		while (getline(ifile, Temp, ','))
		{
			vecfriend->push_back(Temp + ',');
		}
	}
	else
	{
		ofstream ofile(strFilename);
		ofile.close();
	}

	ifile.close();
}

void CServ::Logout(int MsgType, SOCKET iSock)
{
	string strID;

	unordered_map<string, SOCKET>::iterator iter;
	unordered_map<string, SOCKET>::iterator iterEnd = m_mapOnline.end();
	for (iter = m_mapOnline.begin(); iter != iterEnd; ++iter)
	{
		if ((*iter).second == iSock)
		{
			strID = (*iter).first;
			(*iter).second = 0;
			break;
		}
	}

	FD_CLR(iSock, &m_readfd);
	closesocket(iSock);

	printf("User %s Logout (%d) \n", strID.c_str(), (int)iSock);
}

void CServ::Join(int MsgType, SOCKET iSock)
{
	int iResultType;
	string ID;
	ID = &Buffer[4];

	if (InsertUesrID(ID.c_str()))
		iResultType = MT_UNUSED;
	else
		iResultType = MT_USED;
	memset(&Buffer[0], 0, BUF_SIZE);
	memcpy((int*)Buffer, &iResultType, sizeof(int));

	send(iSock, Buffer, sizeof(int), 0);

	ofstream ofile("DB//" + ID + ".txt");
	ofile.close();
}

bool CServ::InsertUesrID(const char* str)
{
	ifstream ifile("DB//IDlist.txt");

	string cmpID;
	string cmpPW;

	string InsertID;
	string InsertPW;

	SplitIDPW(true, str, &InsertID, &InsertPW);

	if (ifile.is_open())
	{
		while (getline(ifile, cmpPW, '/'))
		{
			int i = 0;

			while (cmpPW[i] != ',')
			{
				cmpID += cmpPW[i];
				++i;
			}

			if (InsertID.compare(cmpID) == 0)
			{
				ifile.close();
				return false;
			}

			cmpID = "";
		}
	}

	ifile.close();

	ofstream ofile("DB//IDlist.txt", fstream::out | fstream::app);
	ofile << InsertID + "," + InsertPW + "/";
	ofile.close();

	return true;
}

void CServ::Login(int iMsgType, SOCKET iSock)
{
	int iResultType;
	int iFriendCount;
	string strUserID;
	vector<string> vecfriend;

	// MT_LOGIN MT_ISONLINE MT_DENIENDACCESS
	iResultType = (MSGTYPE)CheckUser(&Buffer[4], strUserID);

	//로그인 성공이라면 로그인 풀에 등록함
	unordered_map<string, SOCKET>::iterator iter = m_mapOnline.find(strUserID);

	if (iter == m_mapOnline.end())
		m_mapOnline.insert(make_pair(strUserID, iSock));
	else
	{
		if ((*iter).second == 0)
			(*iter).second = iSock;
		else
		{
			iResultType = MT_ISONLINE;
		}
	}

	if (iResultType == MT_LOGIN)
	{// 서버로그
		printf("User %s Login (%d) \n", strUserID.c_str(), (int)iSock);

		// 로그인 결과값 전송//////////////////////////////////
		memset(&Buffer[0], 0, BUF_SIZE);
		memcpy((int*)&Buffer[0], &iResultType, sizeof(int));
		// 친구목록 확인
		SendFriendList(strUserID.c_str(), &vecfriend);
		iFriendCount = vecfriend.size();
		// 전송확인용 친구명 수
		memcpy((int*)&Buffer[4], &iFriendCount, sizeof(int));
		send(iSock, Buffer, sizeof(int) * 2, 0);
		/////////////////////////////////////////////////////

		memset(&Buffer[0], 0, BUF_SIZE);
		for (int k = 0; k < vecfriend.size(); ++k)
		{
			send(iSock, vecfriend[k].c_str(), vecfriend[k].length(), 0);
		}
	}
	else
	{
		send(iSock, (char*)(&iResultType), sizeof(int), 0);

		Logout(iMsgType, iSock);
	}
}

string CServ::RefreshList(const char* str, unordered_map<string, SOCKET>& map)
{
	int i = 0;
	string result;

	while (1)
	{
		string strId;
		while (str[i] != ',')
		{
			strId += str[i];
			++i;
		}

		auto iter = map.find(strId);

		if (iter == map.end())
			result += 'x';
		else if ((*iter).second == 0)
		{
			result += 'x';
		}
		else
		{
			result += 'o';
		}

		++i;

		if (str[i] == '/')
			break;
	}

	return result;
}

bool CServ::AddFreindList(const char* ADDID, const string& ID)
{
	string filename = "DB//" + ID + ".txt";
	ifstream ifile(filename);

	string cmpID;
	string cmpPW;

	if (ifile.is_open())
	{
		while (getline(ifile, cmpPW, '/'))
		{
			int i = 0;

			while (cmpPW[i] != ',')
			{
				cmpID += cmpPW[i];
				++i;
			}

			if (cmpID.compare(ADDID) == 0)
			{
				ifile.close();
				return false;
			}

			cmpID = "";
		}
	}

	ofstream ofile(filename, fstream::out | fstream::app);
	ofile << '\n' << ADDID << ',';
	ofile.close();
	ifile.close();

	return true;
}

void CServ::AddFreind(int MsgType, SOCKET iSock)
{
	int ResultType;
	string ID = "";
	string Buf = "";

	unordered_map<string, SOCKET>::iterator iter;
	unordered_map<string, SOCKET>::iterator iterEnd = m_mapOnline.end();
	for (iter = m_mapOnline.begin(); iter != iterEnd; ++iter)
	{
		if ((*iter).second == iSock)
		{
			ID = (*iter).first;
			break;
		}
	}
	int result = -1;
	if (!ID.empty())
	{
		int numStr = 0;
		memcpy(&numStr, (int*)&Buffer[4], sizeof(int));
		Buf = &Buffer[8];
		if (AddFreindList(Buf.c_str(), ID))
			ResultType = MT_ADDFRIEND;
		else
			ResultType = MT_ADDFALSE;
	}

	send(iSock, (char*)&ResultType, sizeof(int), 0);

}

void CServ::DeleteFreind(int MsgType, SOCKET iSock)
{
	int result = MT_DELETEFRIEND;
	int numStr = 0;
	string Buf;
	string ID = "";

	unordered_map<string, SOCKET>::iterator iter;
	unordered_map<string, SOCKET>::iterator iterEnd = m_mapOnline.end();
	for (iter = m_mapOnline.begin(); iter != iterEnd; ++iter)
	{
		if ((*iter).second == iSock)
		{
			ID = (*iter).first;
			break;
		}
	}

	memcpy(&numStr, (int*)&Buffer[4], sizeof(int));
	Buf = &Buffer[8];

	DeleteFreindList(Buf.c_str(), ID);
	send(iSock, (char*)&result, sizeof(int), 0);
}

bool CServ::DeleteFreindList(const char* DeleteID, const string& ID)
{
	string NewList;
	string OldList;
	NewList = "DB//NewList.txt";
	OldList = "DB//" + ID + ".txt";

	ifstream ifile(OldList);
	ofstream ofile(NewList);

	if (ifile.is_open())
	{
		string Temp;
		while (getline(ifile, Temp, ','))
		{
			if (Temp[0] == '\n')
			{
				Temp = &Temp[1];
				if (Temp.compare(DeleteID) != 0)
					ofile << '\n' + Temp + ',';
			}
			else
			{
				if (Temp.compare(DeleteID) != 0)
					ofile << Temp + ',';
			}
		}

	}

	ofile.close();
	ifile.close();

	// 파일이 열려있을 경우 동작하지 않음.
	rename(OldList.c_str(), "DB//OldFile.txt");
	rename(NewList.c_str(), OldList.c_str());
	remove("DB//OldFile.txt");

	return true;
}

void CServ::SendMsg(int iMsgType, SOCKET iSock)
{
	int IdCount;
	string ReceiverID, SenderID,Msg;

	printf("TEXT (%d)\n", (int)iSock);

	memcpy(&IdCount, (int*)&Buffer[4], sizeof(int));
	ReceiverID.append(&Buffer[8], IdCount);

	unordered_map<string, SOCKET>::iterator iter = m_mapOnline.find(ReceiverID);

	if (iter != m_mapOnline.end())
	{
		if ((*iter).second != 0)
		{
			SOCKET RecvSock = (*iter).second;

			Msg = &Buffer[8 + IdCount];

			unordered_map<string, SOCKET>::iterator iter;
			unordered_map<string, SOCKET>::iterator iterEnd = m_mapOnline.end();
			for (iter = m_mapOnline.begin(); iter != iterEnd; ++iter)
			{
				if ((*iter).second == iSock)
				{
					SenderID = (*iter).first;
					break;
				}
			}
			if (iter != iterEnd)
			{
				memset(Buffer, 0, BUF_SIZE);

				IdCount = SenderID.length();	// int(4), int(4), strID(x), Msg(x)

				memcpy((int*)&Buffer[0], &iMsgType, sizeof(int));
				memcpy((int*)&Buffer[4], &IdCount, sizeof(int));	// ID 길이
				strcpy(&Buffer[8], SenderID.c_str());
				strcpy(&Buffer[8 + IdCount], Msg.c_str());

				send(RecvSock, Buffer, 8 + IdCount + Msg.length(), 0);
			}
		}
	}
	else
	{

	}
}

void CServ::SendFriendList(int MsgType,SOCKET iSOck)
{
	string Buf;

	Buf = RefreshList(&Buffer[4], m_mapOnline);

	memset(&Buffer[0], 0, BUF_SIZE);
	memcpy((int*)&Buffer[0], &MsgType, sizeof(int));
	memcpy(&Buffer[0], Buf.c_str(), Buf.length());

	send(iSOck, Buffer, Buf.length() + 4, 0);
}

void CServ::CheckOnLine(int MsgType, SOCKET iSock)
{
	int ResultType;
	string strID;
	strID = &Buffer[4];

	unordered_map<string, SOCKET>::iterator iter = m_mapOnline.find(strID);

	if (iter == m_mapOnline.end())
		ResultType = MT_ISOFFLINE;
	else
	{
		if ((*iter).second == 0)
			ResultType = MT_ISOFFLINE;
		else
		{
			ResultType = MT_ISONLINE;
		}
	}

	memset(&Buffer[0], 0, BUF_SIZE);

	memcpy((int*)&Buffer[0], &ResultType, sizeof(int));

	send(iSock, Buffer, sizeof(int) * 2, 0);
}

