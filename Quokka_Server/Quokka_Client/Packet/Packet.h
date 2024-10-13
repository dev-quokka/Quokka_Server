#pragma once

#include "Define.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

struct PACKET_HEADER
{
	UINT16 PacketLength;
	UINT16 PacketId;
	UINT8 encryption = 0; // 압축여부 암호화여부 등 속성을 알아내는 값
};

const UINT32 PACKET_HEADER_LENGTH = sizeof(PACKET_HEADER);

const int MAX_USER_ID_LEN = 32;
const int MAX_USER_PW_LEN = 32;

struct LOGIN_REQUEST_PACKET : public PACKET_HEADER
{
	char UserID[MAX_USER_ID_LEN + 1];
	char UserPW[MAX_USER_PW_LEN + 1];

};

const size_t LOGIN_REQUEST_PACKET_SIZE = sizeof(LOGIN_REQUEST_PACKET);

struct LOGIN_RESPONSE_PACKET : public PACKET_HEADER
{
	UINT16 LoginResult;
	FriendInfo MyInfo; 
};


struct FIND_USER_REQUEST : public PACKET_HEADER
{
	char UserID[MAX_USER_ID_LEN + 1];
};

struct FIND_USER_RESPONSE : public PACKET_HEADER
{
	UINT32 userPKNum;
};

struct FIND_FRIENDS_REQUEST : public PACKET_HEADER
{
	UINT32 userPKNum;
};

struct FIND_FRIENDS_RESPONSE : public PACKET_HEADER
{
	FriendInfo friendInfo;
};

struct FRIEND_REQUEST_REQUEST : public PACKET_HEADER
{
	UINT32 reqUserPKNum;
	UINT32 resUserPKNum;
};

struct FRIEND_REQUEST_RESPONSE : public PACKET_HEADER
{
	UINT16 FriendsReq_Res;
};

struct FRIEND_REQUEST_CANCEL_REQUEST : public PACKET_HEADER
{
	UINT32 reqUserPKNum;
	UINT32 resUserPKNum;
};

struct FRIEND_REQUEST_CANCEL_RESPONSE : public PACKET_HEADER
{
	UINT16 FriendsReqCancel_Res;
};

struct DELETE_FRIEND_REQUEST :public PACKET_HEADER {
	UINT32 reqUserPKNum;
	UINT32 resUserPKNum;
};

struct DELETE_FRIEND_RESPONSE :public PACKET_HEADER {
	UINT16 DelFriendRes;
};

struct DELETE_FRIEND_RESPONSE_TO_RESPONSE_USER :public PACKET_HEADER {
	UINT32 reqUserPKNum;
};

struct CONNECT_RESPONSE_TO_FRIENDS : public PACKET_HEADER {
	UINT32 reqUserPKNum;
};

struct MAKE_PARTY_REQUEST : public PACKET_HEADER
{
	UINT32 reqUserPKNum;
	UINT32 resUserPKNum;
};

struct MAKE_PARTY_RESPONSE : public PACKET_HEADER
{
	UINT16 partyRes;
};

struct PARTY_ENTER_REQUEST : public PACKET_HEADER
{
	UINT16 partyIdx;
	UINT32 reqUserPKNum;
};

struct PARTY_ENTER_RESPONSE : public PACKET_HEADER
{
	UINT16 partyRes;
};

enum class  PACKET_ID : UINT16
{
	//SYSTEM
	SYS_USER_CONNECT = 11,
	SYS_USER_DISCONNECT = 12,
	SYS_END = 30,

	//DB
	DB_END = 199,

	//Client
	LOGIN_REQUEST = 201,
	LOGIN_RESPONSE = 202,

	MAKE_PARTY_REQUEST = 204,
	MAKE_PARTY_RESPONSE = 205,

	PARTY_ENTER_REQUEST = 206,
	PARTY_ENTER_RESPONSE = 207,

	PARTY_ORGANIZER_CHANGE_REQUEST = 209,
	PARTY_ORGANIZER_CHANGE_RESPONSE = 210,

	PARTY_EXILE_REQUEST = 211,
	PARTY_EXILE_RESPONSE = 212,

	PARTY_LEAVE_REQUEST = 215,
	PARTY_LEAVE_RESPONSE = 216,

	PARTY_CHAT_REQUEST = 221,
	PARTY_CHAT_RESPONSE = 222,
	PARTY_CHAT_NOTIFY = 223,

	WHISPER_CHAT_REQUEST = 225,

	FIND_USER_REQUEST = 241,
	FIND_USER_RESPONSE = 242,
	FIND_FRIENDS_REQUEST = 243,
	FIND_FRIENDS_RESPONSE = 244,

	CONNECT_RESPONSE_TO_FRIENDS = 250,
	FRIEND_REQUEST_REQUEST = 251,
	FRIEND_REQUEST_RESPONSE = 252,
	FRIEND_REQUEST_CANCEL_REQUEST = 253,
	FRIEND_REQUEST_CANCEL_RESPONSE = 254,

	FRIEND_DELETE_REQUEST = 255,
	FRIEND_DELETE_RESPONSE = 256,

	DELETE_FRIEND_RESPONSE_TO_RESPONSE_USER = 256,
};