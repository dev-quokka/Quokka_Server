#pragma once

#include <winsock2.h>
#include <Ws2tcpip.h>
#include <mswsock.h>

constexpr UINT32 MAX_SOCK_RECVBUF = 1024;	//소켓 버퍼의 크기

enum class IOOperation {
	ACCEPT,
	RECV,
	SEND
};

struct OverlappedEx {
	WSAOVERLAPPED m_wsaOverlapped;		//Overlapped I/O구조체
	WSABUF		m_wsaBuf;				//Overlapped I/O작업 버퍼
	IOOperation m_eOperation;			//작업 동작 종류
	UINT32 SessionIndex = 0;			//유저 인덱스
};

// 암호 256*101 - 256/3 + 소켓