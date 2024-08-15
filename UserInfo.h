#pragma once
#include <iostream>

#include "Define.h"


class UserInfo {

public:

	int getUserPartyIdx() { return UserPartyIdx; }
	int getUserIdx() { return UserIdx; }

	bool IsConnect() { return uIsConnect == 1; }
	
	SOCKET getSocketNum() { return uSocket; }

	void Init(const UINT32 index, HANDLE iocpHandle_)
	{
		UserIdx = index;
		uIOCPHandle = iocpHandle_;
	}

	// accpet 비동기 처리
	bool PostAccept(SOCKET listenSocket) {
		uSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_IP,
			NULL, 0, WSA_FLAG_OVERLAPPED);

		if (INVALID_SOCKET == uSocket)
		{
			std::cout << "client Socket WSASocket Error : " << GetLastError() << std::endl;
			return false;
		}

		ZeroMemory(&uAcceptContext, sizeof(OverlappedEx));

		DWORD bytes = 0;
		DWORD flags = 0;
		uAcceptContext.m_wsaBuf.len = 0;
		uAcceptContext.m_wsaBuf.buf = nullptr;
		uAcceptContext.m_eOperation = IOOperation::ACCEPT;
		uAcceptContext.SessionIndex = UserIdx;

		if (FALSE == AcceptEx(listenSocket, uSocket, uAcceptBuf, 0,
			sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &bytes, (LPWSAOVERLAPPED) & (uAcceptContext)))
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
			{
				std::cout << "AcceptEx Error : " << GetLastError() << std::endl;
				return false;
			}
		}

		return true;
	}

	bool BindIOCompletionPort(HANDLE iocpHandle_)
	{
		//socket과 pClientInfo를 CompletionPort객체와 연결시킨다.
		auto hIOCP = CreateIoCompletionPort((HANDLE)getSocketNum()
			, iocpHandle_
			, (ULONG_PTR)(this), 0);

		if (hIOCP == INVALID_HANDLE_VALUE)
		{
			std::cout << "[에러] CreateIoCompletionPort()함수 실패 :"<< GetLastError()<<std::endl;
			return false;
		}

		return true;
	}

	bool OnConnect(HANDLE iocpHandle_, SOCKET socket_)
	{
		uSocket = socket_;
		uIsConnect = 1;

		//I/O Completion Port객체와 소켓을 연결시킨다.
		if (BindIOCompletionPort(iocpHandle_) == false)
		{
			return false;
		}

		return BindRecv();
	}

	bool BindRecv()
	{
		DWORD dwFlag = 0;
		DWORD dwRecvNumBytes = 0;

		//Overlapped I/O을 위해 각 정보를 셋팅해 준다.
		uAcceptOverlappedEx.m_wsaBuf.len = MAX_SOCK_RECVBUF;
		uAcceptOverlappedEx.m_wsaBuf.buf = uRecvBuf;
		uAcceptOverlappedEx.m_eOperation = IOOperation::RECV;

		int nRet = WSARecv(uSocket,
			&(uAcceptOverlappedEx.m_wsaBuf),
			1,
			&dwRecvNumBytes,
			&dwFlag,
			(LPWSAOVERLAPPED) & (uAcceptOverlappedEx),
			NULL);

		//socket_error이면 client socket이 끊어진걸로 처리한다.
		if (nRet == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING))
		{
			std::cout << "[에러] WSARecv()함수 실패 : " << WSAGetLastError() << std::endl;
			return false;
		}

		return true;
	}

	bool AcceptCompletion()
	{

		if (OnConnect(uIOCPHandle, uSocket) == false)
		{
			return false;
		}

		SOCKADDR_IN		stClientAddr;
		int nAddrLen = sizeof(SOCKADDR_IN);
		char clientIP[32] = { 0, };
		inet_ntop(AF_INET, &(stClientAddr.sin_addr), clientIP, 32 - 1);
		std::cout << "클라이언트 접속 : IP(" << clientIP<< ")" <<  ", SOCKET(" << (int)uSocket<<")" << std::endl;

		return true;
	}

	void Close(bool bIsForce_ = false)
	{
		struct linger stLinger = { 0, 0 };	// SO_DONTLINGER로 설정

		// bIsForce가 true이면 SO_LINGER, timeout = 0으로 설정하여 강제 종료 시킨다. 주의 : 데이터 손실이 있을수 있음 
		if (true == bIsForce_)
		{
			stLinger.l_onoff = 1;
		}

		//socketClose소켓의 데이터 송수신을 모두 중단 시킨다.
		shutdown(uSocket, SD_BOTH);

		//소켓 옵션을 설정한다.
		setsockopt(uSocket, SOL_SOCKET, SO_LINGER, (char*)&stLinger, sizeof(stLinger));

		uIsConnect = 0;
		//소켓 연결을 종료 시킨다.
		closesocket(uSocket);
		uSocket = INVALID_SOCKET;
	}


private:

	bool SendIO()
	{
		auto sendOverlappedEx = SendDataqueue.front();

		DWORD dwRecvNumBytes = 0;
		int nRet = WSASend(uSocket,
			&(sendOverlappedEx->m_wsaBuf),
			1,
			&dwRecvNumBytes,
			0,
			(LPWSAOVERLAPPED)sendOverlappedEx,
			NULL);

		//socket_error이면 client socket이 끊어진걸로 처리한다.
		if (nRet == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING))
		{
			std::cout<< "[에러] WSASend()함수 실패 : " << WSAGetLastError() << std::endl;
			return false;
		}

		return true;
	}


	// 유저 인덱스 번호
	int UserIdx;
	int UserPartyIdx = 0;
	int uIsConnect = 0;

	char uAcceptBuf[64];
	char uRecvBuf[MAX_SOCK_RECVBUF]; //데이터 수신 버퍼

	std::mutex rLock;
	std::mutex SendLock;

	HANDLE uIOCPHandle = INVALID_HANDLE_VALUE;

	SOCKET uSocket;

	std::queue<OverlappedEx*> SendDataqueue;

	OverlappedEx uAcceptContext;
	OverlappedEx uAcceptOverlappedEx;
};