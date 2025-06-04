#pragma once

#pragma comment(lib, "ws2_32.lib") //���־󿡼� �������α׷��� �ϱ� ���� ��
#pragma comment(lib,"mswsock.lib") //AcceptEx�� ����ϱ� ���� ��

#include <thread>
#include <vector>
#include <mutex>
#include <queue>

#include "UserInfo.h"
#include "Define.h"
#include "Packet.h"
#include "MySQLManager.h"
#include "PacketManager.h"
#include "Packet.h"

class QuokkaServer {

public:

	QuokkaServer() {};
	~QuokkaServer() {
		WSACleanup();
	};

	void OnConnect(const UINT32 clientIndex_)
	{
		std::cout << "Ŭ���̾�Ʈ Connect: Index : " << clientIndex_ << std::endl;

		PacketInfo packet{ clientIndex_, (UINT16)PACKET_ID::SYS_USER_CONNECT, 0 };
		m_pPacketManager->PushSystemPacket(packet);
	}

	void OnClose(const UINT32 clientIndex_)
	{
		std::cout << "Ŭ���̾�Ʈ Close : Index : " << clientIndex_ << std::endl;

		PacketInfo packet{ clientIndex_, (UINT16)PACKET_ID::SYS_USER_DISCONNECT, 0 };
		m_pPacketManager->PushSystemPacket(packet);
	}

	void OnReceive(const UINT32 clientIndex_, const UINT32 size_, char* pData_)
	{
		std::cout << "Ŭ���̾�Ʈ Recv : Index : " << clientIndex_ << " dataSize : " << size_ << std::endl;

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


	// ���� ���ϻ��� �Լ�
	bool Init(const UINT32 maxIOWorkerThreadCnt)
	{
		WSADATA wsadata;

		int checki = WSAStartup(MAKEWORD(2, 2), &wsadata);
		if (checki != 0) {
			std::cout << "WSAStartup() ����" << std::endl;
			return false;
		}

		ListenSkt = WSASocket(PF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED);

		if (ListenSkt == INVALID_SOCKET) {
			std::cout << "socket���� ����" << std::endl;
			return false;
		}
		MaxIOWorkerThreadCnt = maxIOWorkerThreadCnt;

		std::cout << "���� ���� ����" << std::endl;
		return true;
	}

	// ���� ���ε� �Լ�
	bool BindandListen(int binport) {
		SOCKADDR_IN addr = { 0 };
		addr.sin_family = AF_INET;
		addr.sin_port = htons(binport);
		addr.sin_addr.s_addr = htonl(INADDR_ANY);

		int checki = bind(ListenSkt, (SOCKADDR*)&addr, sizeof(addr));
		if (checki != 0) {
			std::cout << "bind �Լ� ����" << std::endl;
		}

		checki = listen(ListenSkt, SOMAXCONN);
		if (checki != 0) {
			std::cout << "listen �Լ� ����" << std::endl;
			return false;
		}

		sIOCPHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, MaxIOWorkerThreadCnt);
		if (sIOCPHandle == NULL) {
			std::cout << "�ʱ� iocp �ڵ� ���� ����" << std::endl;
			return false;
		}

		auto bIOCPHandle = CreateIoCompletionPort((HANDLE)ListenSkt, sIOCPHandle, (UINT32)0, 0);
		if (bIOCPHandle == nullptr) {
			std::cout << "iocp �ڵ� ���ε� ����" << std::endl;
			return false;
		}

		std::cout << "���ε� ���� ����" << std::endl;
		return true;
	}

	bool StartServer(const UINT32 maxClientCount_)
	{
		CreateClient(maxClientCount_);

		bool bRet = CreateWorkThread();
		if (false == bRet) {
			return false;
		}

		bRet = CreateAccepterThread();
		if (false == bRet) {
			return false;
		}

		std::cout << "���� ����" << std::endl;
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

		//Accepter �����带 �����Ѵ�.
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

	void WorkThread() {
		LPOVERLAPPED lpOverlapped = NULL;
		UserInfo* userInfo = nullptr;
		DWORD dwIoSize = 0;
		bool gqSucces = TRUE;

		while (WorkRun) {
			gqSucces = GetQueuedCompletionStatus(
				sIOCPHandle,
				&dwIoSize,
				(PULONG_PTR)&userInfo,
				&lpOverlapped,
				INFINITE
			);

			// ���� ���� ��û
			if (TRUE == gqSucces && 0 == dwIoSize && NULL == lpOverlapped)
			{
				WorkRun = false;
				continue;
			}

			if (NULL == lpOverlapped)
			{
				continue;
			}

			auto pOverlappedEx = (OverlappedEx*)lpOverlapped;

			//client�� ������ ��������
			if (FALSE == gqSucces || (0 == dwIoSize && IOOperation::ACCEPT != pOverlappedEx->m_eOperation))
			{
				std::cout << "socket " << userInfo->getSocketNum() << " ���� ����" << std::endl;
				CloseSocket(userInfo); //Caller WokerThread()
				continue;
			}

			if (IOOperation::ACCEPT == pOverlappedEx->m_eOperation)
			{
				userInfo = GetClientInfo(pOverlappedEx->SessionIndex);
				std::cout << "���� Accept ��û" << std::endl;
				if (userInfo->AcceptCompletion())
				{
					usercnt_mutex.lock();
					//Ŭ���̾�Ʈ ���� ����
					++sUserCnt;
					OnConnect(userInfo->GetUserIdx());
					usercnt_mutex.unlock();
				}
				else
				{
					CloseSocket(userInfo, true);  //Caller WokerThread()
				}
			}
			//Overlapped I/O Recv�۾� ��� �� ó��
			else if (IOOperation::RECV == pOverlappedEx->m_eOperation)
			{
				OnReceive(userInfo->GetUserIdx(), dwIoSize, userInfo->RecvBuffer());
				std::cout << "���� Recv ��û" << std::endl;
				userInfo->BindRecv();
			}
			//Overlapped I/O Send�۾� ��� �� ó��
			else if (IOOperation::SEND == pOverlappedEx->m_eOperation)
			{
				std::cout << "���� Send ��û" << std::endl;
				userInfo->SendCompleted(dwIoSize);
			}
		}

	}

	//������ ������ ���� ��Ų��.
	void CloseSocket(UserInfo* user, bool isForce_ = false)
	{
		if (user->IsConnect() == false)
		{
			return;
		}

		auto clientIndex = user->GetUserIdx();

		user->Close(isForce_);

		// ��Ŷ�ܿ����� ���� ����
		OnClose(clientIndex);

		std::cout << "���� ���� ����" << std::endl;
	}

	//I/O�۾� �ϷḦ �޾� ó���� ������ ����
	bool CreateWorkThread() {
		auto threadCount = (MaxIOWorkerThreadCnt * 2) + 1;
		for (int i = 0; i < threadCount; i++) {
			IOWorkerThread.emplace_back([this]() {WorkThread(); });
		}
		std::cout << "Work ������ ����" << std::endl;
		return true;
	}

	//Accept��û ó���ϴ� ������ 
	bool CreateAccepterThread()
	{
		AcceptThread = std::thread([this]() { AccepterThread(); });
		std::cout << "AcceptThread ����" << std::endl;
		return true;
	}

	//AcceptEx ���� �ɾ�� ��ü �̸� ����
	void CreateClient(const UINT16 maxClientCount)
	{
		for (UINT16 i = 0; i < maxClientCount; i++)
		{
			auto user = new UserInfo;
			user->Init(i, sIOCPHandle);
			uInfos.emplace_back(user);
		}
	}

	void AccepterThread()
	{
		while (AccepterRun)
		{
			auto curTimeSec = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
			for (auto user : uInfos)
			{
				if (user->IsConnect())
				{
					continue;
				}
				if ((UINT64)curTimeSec < user->GetLatestClosedTimeSec())
				{
					continue;
				}

				auto diff = curTimeSec - user->GetLatestClosedTimeSec();
				if (diff <= 3)
				{
					continue;
				}

				user->PostAccept(ListenSkt);
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		}
	}

	UINT32 MaxIOWorkerThreadCnt = 0;

	SOCKET ListenSkt = INVALID_SOCKET;

	HANDLE sIOCPHandle = INVALID_HANDLE_VALUE;

	std::mutex usercnt_mutex;

	std::thread AcceptThread;

	std::queue<int> CheckuInfos; // ���� ���� �� �ִ� ���� �ڸ� �ִ��� Ȯ�ο� ť (set���� ���ĵǴ� �迭���� �ݹ� ������ ���ٴ� ������ ������ ť�� �� ������ ����)

	std::vector<UserInfo*> uInfos; // �������� ���� ����ü
	std::vector<std::thread> IOWorkerThread;

	std::unique_ptr<PacketManager> m_pPacketManager;

	bool AccepterRun = true;
	bool WorkRun = true;
	bool ComQueStatus = false;

	int sUserCnt = 0;
};