#pragma once

#include <winsock2.h>
#include <Ws2tcpip.h>
#include <mswsock.h>

constexpr UINT32 MAX_SOCK_RECVBUF = 1024;	//���� ������ ũ��

enum class IOOperation {
	ACCEPT,
	RECV,
	SEND
};

struct OverlappedEx {
	WSAOVERLAPPED m_wsaOverlapped;		//Overlapped I/O����ü
	WSABUF		m_wsaBuf;				//Overlapped I/O�۾� ����
	IOOperation m_eOperation;			//�۾� ���� ����
	UINT32 SessionIndex = 0;			//���� �ε���
};

// ��ȣ 256*101 - 256/3 + ����