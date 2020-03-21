#include "Serv.h"

int main(int argc, char* argv[])
{
	CServ* Serv = new CServ;

	Serv->Init((char*)"9190", (char*)"192.168.1.4");

	while (1)
	{
		if (!Serv->Update())
			break;
	}

	delete Serv;

	return 0;
}

//argv[1] = (char*)"192.168.1.4";
//argv[1] = (char*)"27.1.112.147";
//argv[2] = (char*)"9190";
