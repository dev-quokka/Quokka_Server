#include "QuokkaServer.h"
#include <string>
#include <iostream>

const UINT16 SERVER_PORT = 9000;
const UINT16 MAX_CLIENT = 50;		    //총 접속할수 있는 클라이언트 수
const UINT32 MAX_IO_WORKER_THREAD = 4;  //쓰레드 풀에 넣을 쓰레드 수

int main()
{
	QuokkaServer server;

	//소켓을 초기화
	server.Init(MAX_IO_WORKER_THREAD);

	//소켓과 서버 주소를 연결하고 등록 시킨다.
	server.BindandListen(SERVER_PORT);

	server.Run(MAX_CLIENT);

	std::string inputCmd;
	std::cout << "서버를 끝내려면 quokka를 입력해주세요" << std::endl;

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