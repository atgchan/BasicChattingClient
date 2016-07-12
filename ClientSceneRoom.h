#pragma once

#include <list>
#include <string>
#include <vector>
#include <algorithm>

#include "IClientScene.h"



class ClientSceneRoom : public IClientScene
{
public:
	ClientSceneRoom() {}
	virtual ~ClientSceneRoom() {}

	virtual void Update() override
	{
	}

	bool ProcessPacket(const short packetId, char* pData) override
	{
		switch (packetId)
		{
		case (short)PACKET_ID::ROOM_ENTER_RES:
		{
			auto pktRes = (NCommon::PktRoomEnterRes*)pData;
		//	RequestRoomUserList(&pktRes->RoomInfo); //To-do
			SetCurSceneType(CLIENT_SCENE_TYPE::ROOM);
		}
		break;

		case (short)PACKET_ID::ROOM_ENTER_USER_LIST_RES:
		{
			auto pktRes = (NCommon::PktEnterRoomUserInfoRes*)pData;
			if (pktRes->UserCount == 0)
			{
				return false;
			}
		}
		break;


		default:
			return false;
		}

		return true;
	}

	void CreateUI(form* pform)
	{
		m_pForm = pform;

		m_RoomUserList = std::make_shared<listbox>((form&)*m_pForm, nana::rectangle(22, 522, 120, 166));
		m_RoomUserList->append_header("UserID", 90);
	}

	void Init(const int maxUserCount)
	{
		m_MaxUserCount = maxUserCount;
	}

	void RequestRoomUserList(NCommon::RoomSmallInfo* pRoomInfo)
	{
		NCommon::PktEnterRoomUserInfoReq reqPkt;
		reqPkt.RoomIndex = pRoomInfo->RoomIndex;
		m_pRefNetwork->SendPacket((short)PACKET_ID::ROOM_ENTER_USER_LIST_REQ, sizeof(reqPkt), (char*)&reqPkt);
	}

private:
	form* m_pForm = nullptr;
	std::shared_ptr<listbox> m_RoomUserList;

	int m_MaxUserCount = 0;

	bool m_IsUserListWorking = false;
	std::list<std::string> m_UserList;
};
