#pragma once

#include "Packet.h"
#include <unordered_map>
#include <iostream>
enum class USER_STATE
{
	NONE = 0,
	LOGIN = 1,
	PARYTY = 2,
	PLAY = 3
};

enum class PartyProperty : UINT8 {
	SOLO = 1,
	DUO = 2,
	SQUARD = 4
};

class User {

	const UINT32 PACKET_DATA_BUFFER_SIZE = 8096;

public :

	void Init(const INT32 index_)
	{
		UserIdx = index_;
		PakcetDataBuffer = new char[PACKET_DATA_BUFFER_SIZE];
	}

	void setUser(FriendInfo* friendInfo_) {

	}
	
	void Clear()
	{
		PartyIdx = -1;
		UserID = "";
		/*mIsConfirm = false;*/
		CurDomainState = USER_STATE::NONE;
		uPartyProperty = PartyProperty::SOLO;
		UserLevel = 1;

		PakcetDataBufferWPos = 0;
		PakcetDataBufferRPos = 0;
	}

	void SetUserPartyProperty(PartyProperty partyProperty_) {
		uPartyProperty = partyProperty_;
	}

	int SetLogin(FriendInfo* friendInfo_)
	{
		CurDomainState = USER_STATE::LOGIN;
		UserID = friendInfo_->id;
		UserPkNum = friendInfo_->userPkNum;
		UserLevel = friendInfo_->userLevel;
		PartyIdx = friendInfo_->partyIdx;
		return 0;
	}

	// ������� ������ ������ ó���Ǵ� �Լ�
	bool PartyRequest(int clientIdx_) {
		
	}

	// ������� ������ ������ ó���Ǵ� �Լ�
	bool PartyFollow(int clientIdx_) {
		
	}

	PartyProperty GetUserPartyProperty() {
		return uPartyProperty;
	}

	UINT16 GetUserPKNum() {
		return UserPkNum;
	}

	UINT16 GetUserLevel() {
		return UserLevel;
	}

	std::string GetUserId() {
		return UserID;
	}

	void SetPacketData(const UINT32 dataSize_, char* pData_)
	{
		if ((PakcetDataBufferWPos + dataSize_) >= PACKET_DATA_BUFFER_SIZE)
		{
			auto remainDataSize = PakcetDataBufferWPos - PakcetDataBufferRPos;

			if (remainDataSize > 0)
			{
				CopyMemory(&PakcetDataBuffer[0], &PakcetDataBuffer[PakcetDataBufferRPos], remainDataSize);
				PakcetDataBufferWPos = remainDataSize;
			}
			else
			{
				PakcetDataBufferWPos = 0;
			}

			PakcetDataBufferRPos = 0;
		}

		CopyMemory(&PakcetDataBuffer[PakcetDataBufferWPos], pData_, dataSize_);
		PakcetDataBufferWPos += dataSize_;

	}

	PacketInfo GetPacket()
	{
		const int PACKET_SIZE_LENGTH = 2;
		const int PACKET_TYPE_LENGTH = 2;
		short packetSize = 0;

		UINT32 remainByte = PakcetDataBufferWPos - PakcetDataBufferRPos;

		if (remainByte < PACKET_HEADER_LENGTH)
		{
			return PacketInfo();
		}

		auto pHeader = (PACKET_HEADER*)&PakcetDataBuffer[PakcetDataBufferRPos];

		if (pHeader->PacketLength > remainByte)
		{
			return PacketInfo();
		}

		PacketInfo packetInfo;
		packetInfo.PacketId = pHeader->PacketId;
		packetInfo.DataSize = pHeader->PacketLength;
		packetInfo.pDataPtr = &PakcetDataBuffer[PakcetDataBufferRPos];

		PakcetDataBufferRPos += pHeader->PacketLength;

		return packetInfo;
	}

private:
	UINT8 UserIdx;
	UINT8 PartyIdx;
	PartyProperty uPartyProperty;

	UINT16 UserLevel;
	UINT16 UserPkNum;

	std::string UserID;

	USER_STATE CurDomainState = USER_STATE::NONE;

	UINT32 PakcetDataBufferWPos = 0;
	UINT32 PakcetDataBufferRPos = 0;

	char* PakcetDataBuffer = nullptr;

	//// ó�� �����ϸ� �������� ģ�������� ģ�� ��û ����
	//std::unordered_map<int, FriendInfo*> Friends;

	// Ŭ���̾�Ʈ���� ó�� ���� �� �ް� ���� �صΰ� ���������� pk��ȣ�� �����س��� ���߿� �� ���̿��� ��Ƽ ���󰡱�� �ٷ� �� �� �ְ� �����ϱ�

};
