#pragma once
#define PACKET_SIZE 1024
#include <winsock2.h>
#include <ws2tcpip.h>
#include <mswsock.h>
#include <unordered_map>
#include <thread>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <deque>
#define PACKET_SIZE 1024

#pragma comment(lib, "ws2_32.lib") //비주얼에서 소켓프로그래밍 하기 위한 것

enum class USER_STATE
{
	NONE = 0,
	LOGIN = 1,
	PARYTY = 2,
	PLAY = 3
};

struct FriendInfo {
	UINT32 userPkNum;
	std::string id;
	UINT16 userLevel = 1;
	UINT16 partyIdx = 0;
	INT16 Check = 0; // 내가 검색할 땐 로그인 요청 확인, 친구 검색시에는 로그인 상태 확인
};