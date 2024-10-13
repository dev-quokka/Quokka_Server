#include "QuokkaServer.h"
#include <string>
#include <iostream>

const UINT16 SERVER_PORT = 9000;
const UINT16 MAX_CLIENT = 50;		    //�� �����Ҽ� �ִ� Ŭ���̾�Ʈ ��
const UINT32 MAX_IO_WORKER_THREAD = 4;  //������ Ǯ�� ���� ������ ��

int main()
{
	QuokkaServer server;

	//������ �ʱ�ȭ
	server.Init(MAX_IO_WORKER_THREAD);

	//���ϰ� ���� �ּҸ� �����ϰ� ��� ��Ų��.
	server.BindandListen(SERVER_PORT);

	server.Run(MAX_CLIENT);

	std::string inputCmd;
	std::cout << "������ �������� quokka�� �Է����ּ���" << std::endl;

	while (true)
	{
		std::cin >> inputCmd;
		if (inputCmd == "quokka")
		{
			break;
		}
	}

	server.End();
	return 0;
}