#pragma once
#pragma comment(lib, "ws2_32")
#pragma comment(lib, "mswsock.lib")

#include <thread>
#include <vector>
#include <mutex>
#include <queue>

#include "UserInfo.h"
#include "Define.h"
#include "Packet.h"
#include "MySQLManger.h"
#include "PacketManager.h"
#include "Packet.h"

class QuokkaServer {

public:

	QuokkaServer() {};
	~QuokkaServer() {
		// 소멸할때 윈속사용, 데이터베이스 사용 종료
		WSACleanup();
	};

	void OnConnect(const UINT32 clientIndex_)
	{
		std::cout << "[OnConnect] 클라이언트: Index : " << clientIndex_ << std::endl;

		PacketInfo packet{ clientIndex_, (UINT16)PACKET_ID::SYS_USER_CONNECT, 0 };
		m_pPacketManager->PushSystemPacket(packet);
	}

	void OnClose(const UINT32 clientIndex_)
	{
		std::cout << "[OnClose] 클라이언트: Index : " << clientIndex_ << std::endl;

		PacketInfo packet{ clientIndex_, (UINT16)PACKET_ID::SYS_USER_DISCONNECT, 0 };
		m_pPacketManager->PushSystemPacket(packet);
	}

	void OnReceive(const UINT32 clientIndex_, const UINT32 size_, char* pData_)
	{
		std::cout << "[OnReceive] 클라이언트: Index : "<< clientIndex_<< " dataSize : " << size_ << std::endl;

		m_pPacketManager->ReceivePacketData(clientIndex_, size_, pData_);
	}

	void Run(const UINT32 maxClient)
	{
		auto sendPacketFunc = [&](UINT32 clientIndex_, UINT16 packetSize, char* pSendPacket)
			{
				SendMsg(clientIndex_, packetSize, pSendPacket);
			};

		m_pPacketManager = std::make_unique<PacketManager>();
		m_pPacketManager->SendPacketFunc = sendPacketFunc;
		m_pPacketManager->Init(maxClient);
		m_pPacketManager->Run();

		StartServer(maxClient);
	}


	// 리슨 소켓생성 함수
	bool Init(const UINT32 maxIOWorkerThreadCnt)
	{
		WSADATA wsadata;

		int checki = WSAStartup(MAKEWORD(2, 2), &wsadata);
		if (checki != 0) {
			std::cout << "WSAStartup() 실패" << std::endl;
			return false;
		}

		ListenSkt = WSASocket(PF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED);

		if (ListenSkt == INVALID_SOCKET) {
			std::cout << "socket생성 실패" << std::endl;
			return false;
		}
		MaxIOWorkerThreadCnt = maxIOWorkerThreadCnt;

		std::cout << "소켓 생성 성공" << std::endl;
		return true;
	}

	// 소켓 바인드 함수
	bool BindandListen(int binport) {
		SOCKADDR_IN addr = { 0 };
		addr.sin_family = AF_INET;
		addr.sin_port = htons(binport);
		addr.sin_addr.s_addr = htonl(INADDR_ANY);

		int checki = bind(ListenSkt, (SOCKADDR*)&addr, sizeof(addr));
		if (checki != 0) {
			std::cout << "bind 함수 실패" << std::endl;
		}

		checki = listen(ListenSkt, 5);
		if (checki != 0) {
			std::cout << "listen 함수 실패" << std::endl;
			return false;
		}

		sIOCPHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, MaxIOWorkerThreadCnt);
		if (sIOCPHandle == NULL) {
			std::cout << "초기 iocp 핸들 생성 실패" << std::endl;
			return false;
		}

		auto bIOCPHandle = CreateIoCompletionPort((HANDLE)ListenSkt, sIOCPHandle, 0, 0);
		if (bIOCPHandle == nullptr) {
			std::cout << "iocp 핸들 바인드 실패" << std::endl;
			return false;
		}

		std::cout << "서버시작" << std::endl;
		std::cout << "======================" << std::endl << std::endl;
		return true;
	}

	bool SendMsg(const UINT32 clientIndex_, const UINT32 dataSize_, char* pData)
	{
		auto Client = GetClientInfo(clientIndex_);
		return Client->SendMsg(dataSize_, pData);
	}

	void DestroyThread()
	{
		WorkRun = false;
		CloseHandle(sIOCPHandle);

		for (auto& th : IOWorkerThread)
		{
			if (th.joinable())
			{
				th.join();
			}
		}

		//Accepter 쓰레드를 종요한다.
		AccepterRun = false;
		closesocket(ListenSkt);

		if (AcceptThread.joinable())
		{
			AcceptThread.join();
		}
	}

	void End()
	{
		m_pPacketManager->End();

		DestroyThread();
	}

private:
	
	UserInfo* GetClientInfo(const UINT32 clientIndex_)
	{
		return uInfos[clientIndex_];
	}

	// 일단 50명으로 세팅
	void CreateClient(const UINT32 maxClientCount)
	{
		for (UINT32 i = 0; i < maxClientCount; ++i)
		{
			auto user = new UserInfo;
			user->Init(i,sIOCPHandle);
			CheckuInfos.push(i);

			uInfos.push_back(user);
		}
	}

	void WorkThread() {
		LPOVERLAPPED lpOverlapped = NULL;
		UserInfo* userInfo = nullptr;
		DWORD dwIoSize = 0;

		while (WorkRun) {

			ComQueStatus = GetQueuedCompletionStatus(
				(HANDLE)ListenSkt,
				&dwIoSize,
				(PULONG_PTR)&userInfo,
				//컴플릿트 아이오 키
				&lpOverlapped,
				INFINITE
			);

			auto pOverlappedEx = (OverlappedEx*)lpOverlapped;

			if (IOOperation::ACCEPT == pOverlappedEx->m_eOperation)
			{
				userInfo = GetClientInfo(pOverlappedEx->SessionIndex);

				if (userInfo->AcceptCompletion())
				{
					usercnt_mutex.lock();
					//클라이언트 갯수 증가
					++sUserCnt;
					usercnt_mutex.unlock();

					OnConnect(userInfo->getUserIdx());
				}
				else
				{
					CloseSocket(userInfo, true);  //Caller WokerThread()
				}
			}
			//Overlapped I/O Recv작업 결과 뒤 처리
			else if (IOOperation::RECV == pOverlappedEx->m_eOperation)
			{
				OnReceive(userInfo->getUserIdx(), dwIoSize, userInfo->RecvBuffer());

				userInfo->BindRecv();
			}
			//Overlapped I/O Send작업 결과 뒤 처리
			else if (IOOperation::SEND == pOverlappedEx->m_eOperation)
			{
				userInfo->SendCompleted(dwIoSize);
			}

		}

	}

	//소켓의 연결을 종료 시킨다.
	void CloseSocket(UserInfo* user, bool isForce_ = false)
	{
		if (user->IsConnect() == false)
		{
			return;
		}

		auto clientIndex = user->getUserIdx();

		user->Close(isForce_);

		// 유저에게 디스컨넥 패킷 보내는 close 만들자.
		OnClose(clientIndex);
	}

	bool CreateWorkerThread() {
		for (int i = 0; i < MaxIOWorkerThreadCnt; i++) {
			IOWorkerThread.emplace_back([this]() {WorkThread(); });
		}
	}

	//accept요청을 처리하는 쓰레드 생성
	bool CreateAccepterThread()
	{
		AcceptThread = std::thread([this]() { AccepterThread(); });
		std::cout << "AcceptThread 시작" << std::endl;
		return true;
	}

	void AccepterThread()
	{
		while (AccepterRun)
		{

			for (auto user : uInfos)
			{
				if (user->IsConnect())
				{
					continue;
				}

				user->PostAccept(ListenSkt);
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(30));
		}
	}

	UINT32 MaxIOWorkerThreadCnt = 0;

	SOCKET ListenSkt = INVALID_SOCKET;

	HANDLE sIOCPHandle = INVALID_HANDLE_VALUE;

	std::mutex usercnt_mutex;

	std::thread AcceptThread;

	std::queue<int> CheckuInfos; // 서버 받을 수 있는 남은 자리 있는지 확인용 큐 (set으로 정렬되는 배열보다 금방 유저가 찬다는 가정이 있으면 큐가 더 좋을것 같다)

	std::vector<UserInfo*> uInfos; // 접속중인 유저 구조체
	std::vector<std::thread> IOWorkerThread;

	std::unique_ptr<PacketManager> m_pPacketManager;

	bool AccepterRun = true;
	bool WorkRun = false;
	bool ComQueStatus = false;

	int sUserCnt = 0;
};