#pragma once
#include <thread>
#include <mutex>
#include <deque>

#include "Packet.h"
#include "Define.h"
#include "Friends.h"

FriendInfo fI;

std::mutex dLock;
std::deque<char*> RecvPacket;

	void EnqueuePacketData(char* recvPakcet_)
	{
		std::lock_guard<std::mutex> guard(dLock);
		RecvPacket.push_back(recvPakcet_);
	};

	void getFriendsFServer(SOCKET soc_, UINT16 fCnt_) {
		char rcvBuffer[PACKET_SIZE];
		memset(rcvBuffer, 0, PACKET_SIZE);

		FIND_FRIENDS_REQUEST lrp;
		lrp.PacketId = (UINT32)PACKET_ID::FIND_FRIENDS_REQUEST;
		lrp.PacketLength = sizeof(FIND_FRIENDS_REQUEST);
		lrp.userPKNum =fI.userPkNum;

		send(soc_, (char*)&lrp, sizeof(FIND_FRIENDS_REQUEST), 0);

		for (UINT16 i = 0; i < fCnt_; i++) {
			recv(soc_, rcvBuffer, PACKET_SIZE, 0);
			auto MyFriendDBResult = reinterpret_cast<FIND_FRIENDS_RESPONSE*>(rcvBuffer);
			auto tempF = (MyFriendDBResult->friendInfo);
			Friendss[tempF.userPkNum] = tempF;
			Friendsv.emplace_back(tempF);

		}
	};

	UINT32 login(SOCKET soc_) {
		for (int i = 0; i < 5; i++) {

			std::cout << "5�� ���н� ���� ȭ������ ���ư��ϴ�. Ʋ��Ƚ�� : " << i << "��" << std::endl;
			char rcvBuffer[PACKET_SIZE];
			memset(rcvBuffer, 0, PACKET_SIZE);

			LOGIN_REQUEST_PACKET lrp;
			lrp.PacketId = (UINT32)PACKET_ID::LOGIN_REQUEST;
			lrp.PacketLength = sizeof(LOGIN_REQUEST_PACKET);

			char id[MAX_USER_ID_LEN + 1];
			char pass[MAX_USER_ID_LEN + 1];

			std::cout << "���̵� �Է����ּ��� : ";
			std::cin >> id;
			std::cout << "��й�ȣ�� �Է����ּ��� : ";
			std::cin >> pass;
			std::cout << std::endl;

			strcpy_s(lrp.UserID, MAX_USER_ID_LEN + 1, id);
			strcpy_s(lrp.UserPW, MAX_USER_ID_LEN + 1, pass);

			send(soc_, (char*)&lrp, sizeof(LOGIN_REQUEST_PACKET), 0);
			recv(soc_, rcvBuffer, PACKET_SIZE, 0);

			LOGIN_RESPONSE_PACKET* LoginDBResult = reinterpret_cast<LOGIN_RESPONSE_PACKET*>(rcvBuffer);
			UINT16 result = LoginDBResult->LoginResult;
			
			if (result < (UINT16)30) {
				FriendInfo Myinfo = (LoginDBResult->MyInfo);
				fI = Myinfo;
				return result;
			}

			else if (result == (UINT16)31) {
				std::cout << "���� ���� ������" << std::endl;
				return 31;
			}

			else if (result == (UINT16)32) {
				std::cout << "���̵� �ٸ�" << std::endl;
				continue;
			}

			else if (result == (UINT16)33) {
				std::cout << "��� �ٸ�" << std::endl;
				continue;
			}
		}
		return 35;
	};
