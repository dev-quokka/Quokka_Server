#pragma once

#include <iostream>

#include "Define.h"

class UserInfo {

public:
	UINT64 mLatestClosedTimeSec = 0;
	UINT64 GetLatestClosedTimeSec() { return mLatestClosedTimeSec; }
	bool IsConnect() { return uIsConnect == 1; }

	int getUserPartyIdx() { return UserPartyIdx; }

	int GetUserIdx() { return UserIdx; }

	char* RecvBuffer() { return uRecvBuf; }

	SOCKET getSocketNum() { return uSocket; }

	void Init(const UINT32 index, HANDLE iocpHandle_)
	{
		UserIdx = index;
		uIOCPHandle = iocpHandle_;
	}

	// �񵿱� AcceptEx ����
	bool PostAccept(SOCKET listenSocket_) {

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

		if (false == AcceptEx(listenSocket_, uSocket, uAcceptBuf, 0,
			sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &bytes, (LPWSAOVERLAPPED) & (uAcceptContext)))
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
			{
				std::cout << "AcceptEx Error : " << GetLastError() << std::endl;
				return false;
			}
		}
		else std::cout << "PostAccept ���� ����" << std::endl;

		return true;
	}

	bool BindIOCompletionPort(HANDLE iocpHandle_)
	{
		//socket�� pClientInfo�� CompletionPort��ü�� �����Ų��.
		auto hIOCP = CreateIoCompletionPort((HANDLE)getSocketNum()
			, iocpHandle_
			, (ULONG_PTR)(this), 0);

		if (hIOCP == INVALID_HANDLE_VALUE)
		{
			std::cout << "reateIoCompletionPort()�Լ� ���� :"<< GetLastError()<<std::endl;
			return false;
		}

		return true;
	}

	bool OnConnect(HANDLE iocpHandle_, SOCKET socket_)
	{
		uSocket = socket_;
		uIsConnect = 1;

		//I/O Completion Port��ü�� ������ �����Ų��.
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

		//Overlapped I/O�� ���� �� ������ ������ �ش�.
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

		//socket_error�̸� client socket�� �������ɷ� ó���Ѵ�.
		if (nRet == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING))
		{
			std::cout << "WSARecv()�Լ� ���� : " << WSAGetLastError() << std::endl;
			return false;
		}

		return true;
	}

	bool AcceptCompletion()
	{
		if (OnConnect(uIOCPHandle, uSocket) == false) return false;

		SOCKADDR_IN	stClientAddr;
		int nAddrLen = sizeof(SOCKADDR_IN);
		char clientIP[32] = { 0, };
		inet_ntop(AF_INET, &(stClientAddr.sin_addr), clientIP, 32 - 1);
		std::cout << "Ŭ���̾�Ʈ ���� : IP(" << clientIP<< ")" <<  ", SOCKET(" << (int)uSocket<<")" << std::endl;

		return true;
	}

	void Close(bool bIsForce_ = false)
	{
		struct linger stLinger = { 0, 0 };	// SO_DONTLINGER�� ����

		// bIsForce�� true�̸� SO_LINGER, timeout = 0���� �����Ͽ� ���� ���� ��Ų��. ���� : ������ �ս��� ������ ���� 
		if (true == bIsForce_)
		{
			stLinger.l_onoff = 1;
		}

		//socketClose������ ������ �ۼ����� ��� �ߴ� ��Ų��.
		shutdown(uSocket, SD_BOTH);

		//���� �ɼ��� �����Ѵ�.
		setsockopt(uSocket, SOL_SOCKET, SO_LINGER, (char*)&stLinger, sizeof(stLinger));

		//���� ������ ���� ��Ų��.
		closesocket(uSocket);

		uIsConnect = 0;
		uSocket = INVALID_SOCKET;
	}

	bool SendMsg(const UINT32 dataSize_, char* pMsg_)
	{
		auto sendOverlappedEx = new OverlappedEx;
		ZeroMemory(sendOverlappedEx, sizeof(OverlappedEx));
		sendOverlappedEx->m_wsaBuf.len = dataSize_;
		sendOverlappedEx->m_wsaBuf.buf = new char[dataSize_];
		CopyMemory(sendOverlappedEx->m_wsaBuf.buf, pMsg_, dataSize_);
		sendOverlappedEx->m_eOperation = IOOperation::SEND;

		{
			std::lock_guard<std::mutex> guard(SendLock);

			SendDataqueue.push(sendOverlappedEx);

			if (SendDataqueue.size() == 1)
			{
				SendIO();
			}

		}

		return true;
	}

	void SendCompleted(const UINT32 dataSize_)
	{
		std::cout << "�۽ſϷ� bytes : "<< dataSize_ << std::endl;

		std::lock_guard<std::mutex> guard(SendLock);

		delete[] SendDataqueue.front()->m_wsaBuf.buf;
		delete SendDataqueue.front();

		SendDataqueue.pop();

		if (SendDataqueue.empty() == false)
		{
			SendIO();
		}
	}

private:

	bool SendIO()
	{
		auto sendOverlappedEx = SendDataqueue.front();

		DWORD dwRecvNumBytes = 0;
		int sCheck = WSASend(uSocket,
			&(sendOverlappedEx->m_wsaBuf),
			1,
			&dwRecvNumBytes,
			0,
			(LPWSAOVERLAPPED)sendOverlappedEx,
			NULL);

		//socket_error�̸� client socket�� �������ɷ� ó���Ѵ�.
		if (sCheck == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING))
		{
			std::cout<< "WSASend()�Լ� ���� : " << WSAGetLastError() << std::endl;
			return false;
		}

		return true;
	}

	char uAcceptBuf[64];
	char uRecvBuf[MAX_SOCK_RECVBUF]; //������ ���� ����

	UINT16 userPkNum;
	UINT16 UserIdx;
	UINT16 UserPartyIdx = 0;
	INT16 uIsConnect = 0;

	std::mutex rLock;
	std::mutex SendLock;

	HANDLE uIOCPHandle = INVALID_HANDLE_VALUE;

	SOCKET uSocket;

	std::queue<OverlappedEx*> SendDataqueue;
	
	OverlappedEx uAcceptContext;
	OverlappedEx uAcceptOverlappedEx;
};