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
			if (m_isUserListInitialized == false)
			{
				auto pktRes = (NCommon::PktRoomEnterRes*)pData;
				RequestRoomUserList(&pktRes->RoomInfo);
				SetCurSceneType(CLIENT_SCENE_TYPE::ROOM);
			}
		}
		break;

		case (short)PACKET_ID::ROOM_ENTER_USER_LIST_RES:
		{
			auto pktRes = (NCommon::PktEnterRoomUserInfoRes*)pData;
			if (pktRes->UserCount == 0)
			{
				return false;
			}
			
			for (int i = 0; i < pktRes->UserCount; ++i)
			{
				UpdateUserInfo(false, pktRes->UserInfo[i].UserID);
			}

			SetUserListGui();
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

	void UpdateUserInfo(bool isToRemove, std::string userID)
	{
		if (m_isUserListInitialized == false)
		{
			if (isToRemove == false)
			{
				auto findIter = std::find_if(std::begin(m_UserInfos), std::end(m_UserInfos), [&userID](auto& ID) { return ID == userID; });

				if (findIter == std::end(m_UserInfos))
				{
					m_UserInfos.push_back(userID);
				}
			}
			else
			{
				m_UserInfos.remove_if([&userID](auto& ID) { return ID == userID; });
			}
		}
		else
		{
			if (isToRemove == false)
			{
				for (auto& user : m_RoomUserList->at(0))
				{
					if (user.text(0) == userID) {
						return;
					}
				}

				m_RoomUserList->at(0).append(userID);
			}
			else
			{
				auto i = 0;
				for (auto& user : m_RoomUserList->at(0))
				{
					if (user.text(0) == userID)
					{
						m_RoomUserList->erase(user);
						return;
					}
				}
			}
		}
	}

	void SetUserListGui()
	{
		m_isUserListInitialized = true;

		m_RoomUserList->clear();

		for (auto & userId : m_UserInfos)
		{
			m_RoomUserList->at(0).append({ userId });
		}

		m_UserInfos.clear();
	}

private:
	form* m_pForm = nullptr;

	int m_MaxUserCount = 0;

	bool m_IsUserListWorking = false;
	bool m_isUserListInitialized = false;

	std::shared_ptr<listbox> m_RoomUserList;
	std::list<std::string> m_UserInfos;
};
